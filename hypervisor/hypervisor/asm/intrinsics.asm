.code

AsmReadMsr PROC
    rdmsr
    shl rdx, 32
    or rax, rdx
    ret
AsmReadMsr ENDP

AsmWriteMsr PROC
    mov eax, edx
    shr rdx, 32
    wrmsr
    ret
AsmWriteMsr ENDP

AsmReadCr0 PROC
    mov rax, cr0
    ret
AsmReadCr0 ENDP

AsmWriteCr0 PROC
    mov cr0, rcx
    ret
AsmWriteCr0 ENDP

AsmReadCr3 PROC
    mov rax, cr3
    ret
AsmReadCr3 ENDP

AsmReadCr4 PROC
    mov rax, cr4
    ret
AsmReadCr4 ENDP

AsmWriteCr4 PROC
    mov cr4, rcx
    ret
AsmWriteCr4 ENDP

AsmInvept PROC
    invept rcx, OWORD PTR [rdx]
    ret
AsmInvept ENDP

END
