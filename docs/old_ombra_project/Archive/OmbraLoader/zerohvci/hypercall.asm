; hypercall.asm
; CPUID-based hypercall for Ombra hypervisor communication
; Matches libombra/com.asm implementation

_text segment

; VMX_ROOT_ERROR hypercall_asm(u64 code, PCOMMAND_DATA param1, u64 param2, u64 key)
; Parameters (Microsoft x64 calling convention):
;   RCX = code (VMCALL_TYPE command)
;   RDX = param1 (pointer to COMMAND_DATA)
;   R8  = param2 (optional parameter, e.g., target_cr3)
;   R9  = key (session authentication key)
;
; Return value:
;   RAX = VMX_ROOT_ERROR status code
;
; The payload's VMExit handler expects:
;   RCX = VMCALL_TYPE
;   RDX = &COMMAND_DATA
;   R8  = optional param
;   R9  = key XOR 0xBABAB00E (obfuscated)
;
; After CPUID triggers VMExit, payload processes command and sets RAX

hypercall_asm proc
    ; Save callee-saved register (RBX used by CPUID)
    push rbx

    ; Obfuscate key with magic constant
    ; The payload deobfuscates by XORing again with same constant
    mov rax, r9                 ; RAX = key from R9
    mov rbx, 0babab00eh         ; RBX = magic XOR mask
    xor r9, rbx                 ; R9 = key XOR 0xBABAB00E

    ; RCX, RDX, R8 already contain correct values (code, param1, param2)
    ; Just need to trigger VMExit

    ; Execute CPUID to cause VMExit
    ; EAX holds the CPUID leaf - we use a magic value for debugging
    ; The actual leaf doesn't matter since payload intercepts ALL CPUID
    ; and identifies our hypercalls via the obfuscated key in R9
    mov eax, 13371337h          ; Magic CPUID leaf (0x13371337)
    cpuid                       ; VMExit occurs here

    ; After VMRESUME, we return here with RAX = VMX_ROOT_ERROR
    ; RAX is already in the correct register for return value

    ; Restore callee-saved register
    pop rbx

    ret
hypercall_asm endp

_text ends
end
