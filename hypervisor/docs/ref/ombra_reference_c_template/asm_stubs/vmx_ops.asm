;-----------------------------------------------------------------------------
; vmx_ops.asm - VMX Operation Assembly Stubs
;
; MASM (ml64) syntax for Windows x64
; These functions implement low-level VMX instructions.
;-----------------------------------------------------------------------------

.CODE

;=============================================================================
; AsmVmxon - Enter VMX Operation
;
; VMX_RESULT AsmVmxon(U64 vmxonPhysical);
;
; Input:  RCX = Physical address of VMXON region
; Output: RAX = VMX_RESULT (0=success, 1=fail_invalid, 2=fail_valid)
;=============================================================================
AsmVmxon PROC
    ; VMXON [m64] - operand is memory containing physical address
    push    rcx                     ; Push physical address to stack
    vmxon   QWORD PTR [rsp]         ; VMXON with address from stack
    pop     rcx                     ; Clean up stack
    
    ; Check result via RFLAGS
    jz      VmxonFailValid          ; ZF=1 means fail with error code
    jc      VmxonFailInvalid        ; CF=1 means fail, invalid operand
    
    xor     rax, rax                ; Success: return 0
    ret
    
VmxonFailValid:
    mov     rax, 2                  ; Fail with valid error code
    ret
    
VmxonFailInvalid:
    mov     rax, 1                  ; Fail, invalid operand
    ret
AsmVmxon ENDP

;=============================================================================
; AsmVmxoff - Exit VMX Operation
;
; VMX_RESULT AsmVmxoff(void);
;
; Output: RAX = VMX_RESULT
;=============================================================================
AsmVmxoff PROC
    vmxoff
    
    jz      VmxoffFailValid
    jc      VmxoffFailInvalid
    
    xor     rax, rax
    ret
    
VmxoffFailValid:
    mov     rax, 2
    ret
    
VmxoffFailInvalid:
    mov     rax, 1
    ret
AsmVmxoff ENDP

;=============================================================================
; AsmVmclear - Clear VMCS
;
; VMX_RESULT AsmVmclear(U64 vmcsPhysical);
;
; Input:  RCX = Physical address of VMCS region
; Output: RAX = VMX_RESULT
;=============================================================================
AsmVmclear PROC
    push    rcx
    vmclear QWORD PTR [rsp]
    pop     rcx
    
    jz      VmclearFailValid
    jc      VmclearFailInvalid
    
    xor     rax, rax
    ret
    
VmclearFailValid:
    mov     rax, 2
    ret
    
VmclearFailInvalid:
    mov     rax, 1
    ret
AsmVmclear ENDP

;=============================================================================
; AsmVmptrld - Load VMCS Pointer
;
; VMX_RESULT AsmVmptrld(U64 vmcsPhysical);
;
; Input:  RCX = Physical address of VMCS region
; Output: RAX = VMX_RESULT
;=============================================================================
AsmVmptrld PROC
    push    rcx
    vmptrld QWORD PTR [rsp]
    pop     rcx
    
    jz      VmptrldFailValid
    jc      VmptrldFailInvalid
    
    xor     rax, rax
    ret
    
VmptrldFailValid:
    mov     rax, 2
    ret
    
VmptrldFailInvalid:
    mov     rax, 1
    ret
AsmVmptrld ENDP

;=============================================================================
; AsmVmwrite - Write VMCS Field
;
; VMX_RESULT AsmVmwrite(U64 field, U64 value);
;
; Input:  RCX = VMCS field encoding
;         RDX = Value to write
; Output: RAX = VMX_RESULT
;=============================================================================
AsmVmwrite PROC
    vmwrite rcx, rdx
    
    jz      VmwriteFailValid
    jc      VmwriteFailInvalid
    
    xor     rax, rax
    ret
    
VmwriteFailValid:
    mov     rax, 2
    ret
    
VmwriteFailInvalid:
    mov     rax, 1
    ret
AsmVmwrite ENDP

;=============================================================================
; AsmVmread - Read VMCS Field
;
; VMX_RESULT AsmVmread(U64 field, U64* value);
;
; Input:  RCX = VMCS field encoding
;         RDX = Pointer to receive value
; Output: RAX = VMX_RESULT
;         [RDX] = Value read
;=============================================================================
AsmVmread PROC
    vmread  QWORD PTR [rdx], rcx
    
    jz      VmreadFailValid
    jc      VmreadFailInvalid
    
    xor     rax, rax
    ret
    
VmreadFailValid:
    mov     rax, 2
    ret
    
VmreadFailInvalid:
    mov     rax, 1
    ret
AsmVmread ENDP

;=============================================================================
; AsmVmlaunch - Launch VM
;
; VMX_RESULT AsmVmlaunch(void);
;
; Note: On success, this doesn't return - execution continues in guest.
;       On failure, returns with error code.
;=============================================================================
AsmVmlaunch PROC
    vmlaunch
    
    ; If we get here, VMLAUNCH failed
    jz      VmlaunchFailValid
    
    mov     rax, 1                  ; Fail invalid
    ret
    
VmlaunchFailValid:
    mov     rax, 2                  ; Fail with error code
    ret
AsmVmlaunch ENDP

;=============================================================================
; AsmVmresume - Resume VM
;
; VMX_RESULT AsmVmresume(void);
;
; Note: On success, doesn't return. On failure, returns with error.
;=============================================================================
AsmVmresume PROC
    vmresume
    
    jz      VmresumeFailValid
    
    mov     rax, 1
    ret
    
VmresumeFailValid:
    mov     rax, 2
    ret
AsmVmresume ENDP

;=============================================================================
; AsmInvept - Invalidate EPT Translations
;
; void AsmInvept(U64 type, INVEPT_DESC* descriptor);
;
; Input:  RCX = INVEPT type (1=single context, 2=all contexts)
;         RDX = Pointer to INVEPT descriptor
;=============================================================================
AsmInvept PROC
    invept  rcx, OWORD PTR [rdx]
    ret
AsmInvept ENDP

;=============================================================================
; AsmInvvpid - Invalidate VPID Translations
;
; void AsmInvvpid(U64 type, INVVPID_DESC* descriptor);
;
; Input:  RCX = INVVPID type
;         RDX = Pointer to INVVPID descriptor
;=============================================================================
AsmInvvpid PROC
    invvpid rcx, OWORD PTR [rdx]
    ret
AsmInvvpid ENDP

END
