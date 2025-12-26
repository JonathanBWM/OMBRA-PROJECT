.code

AsmReadCs PROC
    mov ax, cs
    ret
AsmReadCs ENDP

AsmReadSs PROC
    mov ax, ss
    ret
AsmReadSs ENDP

AsmReadDs PROC
    mov ax, ds
    ret
AsmReadDs ENDP

AsmReadEs PROC
    mov ax, es
    ret
AsmReadEs ENDP

AsmReadFs PROC
    mov ax, fs
    ret
AsmReadFs ENDP

AsmReadGs PROC
    mov ax, gs
    ret
AsmReadGs ENDP

AsmReadTr PROC
    str ax
    ret
AsmReadTr ENDP

AsmReadLdtr PROC
    sldt ax
    ret
AsmReadLdtr ENDP

AsmReadGdtr PROC
    sgdt [rcx]
    ret
AsmReadGdtr ENDP

AsmReadIdtr PROC
    sidt [rcx]
    ret
AsmReadIdtr ENDP

AsmReadRflags PROC
    pushfq
    pop rax
    ret
AsmReadRflags ENDP

AsmReadDr7 PROC
    mov rax, dr7
    ret
AsmReadDr7 ENDP

END
