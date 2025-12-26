; vmexit.asm — VM-Exit Handler Entry Point
; OmbraHypervisor
;
; GUEST_REGS layout (must match C struct in exit_dispatch.h):
;   Offset  Register
;   0x00    RAX
;   0x08    RCX
;   0x10    RDX
;   0x18    RBX
;   0x20    RSP (placeholder - actual RSP in VMCS)
;   0x28    RBP
;   0x30    RSI
;   0x38    RDI
;   0x40    R8
;   0x48    R9
;   0x50    R10
;   0x58    R11
;   0x60    R12
;   0x68    R13
;   0x70    R14
;   0x78    R15

.code

EXTERN VmexitDispatch:PROC

VmexitHandler PROC
    ; Save all GPRs in GUEST_REGS order (reverse for stack)
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rdi
    push rsi
    push rbp
    sub rsp, 8          ; RSP placeholder slot (can't meaningfully push RSP)
    push rbx
    push rdx
    push rcx
    push rax

    ; Save XMM registers (Windows x64 ABI: XMM0-5 volatile, XMM6-15 non-volatile)
    ; We save all to be safe since kernel code may use any of them
    sub rsp, 200h       ; 16 XMM regs * 16 bytes + alignment = 256 bytes, round to 512
    movaps [rsp + 000h], xmm0
    movaps [rsp + 010h], xmm1
    movaps [rsp + 020h], xmm2
    movaps [rsp + 030h], xmm3
    movaps [rsp + 040h], xmm4
    movaps [rsp + 050h], xmm5
    movaps [rsp + 060h], xmm6
    movaps [rsp + 070h], xmm7
    movaps [rsp + 080h], xmm8
    movaps [rsp + 090h], xmm9
    movaps [rsp + 0A0h], xmm10
    movaps [rsp + 0B0h], xmm11
    movaps [rsp + 0C0h], xmm12
    movaps [rsp + 0D0h], xmm13
    movaps [rsp + 0E0h], xmm14
    movaps [rsp + 0F0h], xmm15

    ; RCX = pointer to GUEST_REGS (after XMM save area)
    lea rcx, [rsp + 200h]

    ; Allocate shadow space (32 bytes) + align to 16 bytes
    sub rsp, 28h

    ; Call C handler
    call VmexitDispatch

    ; Remove shadow space
    add rsp, 28h

    ; Check return value
    cmp eax, 2              ; VMEXIT_SHUTDOWN
    je Shutdown

    ; Restore XMM registers
    movaps xmm0,  [rsp + 000h]
    movaps xmm1,  [rsp + 010h]
    movaps xmm2,  [rsp + 020h]
    movaps xmm3,  [rsp + 030h]
    movaps xmm4,  [rsp + 040h]
    movaps xmm5,  [rsp + 050h]
    movaps xmm6,  [rsp + 060h]
    movaps xmm7,  [rsp + 070h]
    movaps xmm8,  [rsp + 080h]
    movaps xmm9,  [rsp + 090h]
    movaps xmm10, [rsp + 0A0h]
    movaps xmm11, [rsp + 0B0h]
    movaps xmm12, [rsp + 0C0h]
    movaps xmm13, [rsp + 0D0h]
    movaps xmm14, [rsp + 0E0h]
    movaps xmm15, [rsp + 0F0h]
    add rsp, 200h

    ; Restore GPRs
    pop rax
    pop rcx
    pop rdx
    pop rbx
    add rsp, 8              ; Skip RSP placeholder
    pop rbp
    pop rsi
    pop rdi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    ; Resume guest execution
    vmresume

    ; If VMRESUME fails, we end up here
    ; RFLAGS.CF=1 means VMCS pointer invalid
    ; RFLAGS.ZF=1 means VMRESUME failed, error in VMCS
    jc VmresumeFailed
    jz VmresumeFailed

    ; Should never reach here
    int 3
    jmp $

VmresumeFailed:
    ; VMRESUME failed - read error from VMCS field 0x4400 (VM_INSTRUCTION_ERROR)
    ; For now, just halt. Debug builds can read the error.
    int 3
    cli
    hlt
    jmp VmresumeFailed

Shutdown:
    ; Graceful shutdown requested via VMCALL
    ; TODO: Implement proper devirtualization
    ;   1. Disable VMX on this CPU
    ;   2. Restore original state
    ;   3. Return to caller
    ; For now, just halt
    cli
    hlt
    jmp Shutdown
VmexitHandler ENDP

; =============================================================================
; AsmVmxLaunch - Execute VMLAUNCH and become guest
;
; This function:
;   1. Saves current RSP to GUEST_RSP (VMCS field 0x681C)
;   2. Sets GUEST_RIP to the success label (VMCS field 0x681E)
;   3. Executes VMLAUNCH
;
; If VMLAUNCH succeeds, execution resumes at LaunchSuccess as guest with RAX=0.
; If it fails, returns with RAX = error code (1, 2, or 3).
;
; VMCS field encodings:
;   GUEST_RSP = 0x681C (natural-width guest RSP)
;   GUEST_RIP = 0x681E (natural-width guest RIP)
; =============================================================================
AsmVmxLaunch PROC
    ; Save non-volatile registers we'll use
    push rbx
    push rbp
    push rdi
    push rsi
    push r12
    push r13
    push r14
    push r15

    ; Save current RFLAGS
    pushfq

    ; --- Write GUEST_RSP ---
    ; RSP after all our pushes will be the guest RSP
    mov rax, 0681Ch         ; VMCS_GUEST_RSP field encoding
    mov rbx, rsp            ; Current RSP value
    vmwrite rax, rbx
    jc LaunchSetupFailed
    jz LaunchSetupFailed

    ; --- Write GUEST_RIP ---
    ; Set to LaunchSuccess - where guest execution will resume
    mov rax, 0681Eh         ; VMCS_GUEST_RIP field encoding
    lea rbx, [LaunchSuccess]
    vmwrite rax, rbx
    jc LaunchSetupFailed
    jz LaunchSetupFailed

    ; Execute VMLAUNCH
    ; If successful, execution continues at LaunchSuccess as guest
    vmlaunch

    ; If we reach here, VMLAUNCH failed
    ; CF=1: invalid VMCS pointer
    ; ZF=1: VMLAUNCH failed, check VM_INSTRUCTION_ERROR
    jc LaunchFailInvalidVmcs
    jz LaunchFailError

    ; Unknown failure
    mov rax, 3
    jmp LaunchCleanup

LaunchSetupFailed:
    ; VMWRITE failed during setup
    mov rax, 4
    jmp LaunchCleanup

LaunchFailInvalidVmcs:
    mov rax, 1              ; Invalid VMCS pointer
    jmp LaunchCleanup

LaunchFailError:
    mov rax, 2              ; VMLAUNCH failed, error in VMCS
    jmp LaunchCleanup

LaunchSuccess:
    ; We are now running as guest!
    ; The VMLAUNCH succeeded, and we continued here as VMX non-root.
    ; Return 0 to indicate success.
    xor rax, rax

LaunchCleanup:
    ; Restore RFLAGS
    popfq

    ; Restore non-volatile registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop rsi
    pop rdi
    pop rbp
    pop rbx

    ret
AsmVmxLaunch ENDP

; =============================================================================
; AsmVmxResume - Resume guest execution (alternative entry)
; =============================================================================
AsmVmxResume PROC
    vmresume

    ; If we reach here, VMRESUME failed
    jc ResumeFailInvalidVmcs
    jz ResumeFailError

    mov rax, 3
    ret

ResumeFailInvalidVmcs:
    mov rax, 1
    ret

ResumeFailError:
    mov rax, 2
    ret
AsmVmxResume ENDP

END
