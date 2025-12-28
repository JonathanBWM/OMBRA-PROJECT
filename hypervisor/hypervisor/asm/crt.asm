; crt.asm - Minimal CRT assembly routines for position-independent hypervisor
; OmbraHypervisor
;
; Provides __chkstk for large stack allocations when /NODEFAULTLIB is used.
;
; __chkstk is called by MSVC when a function allocates more than 4KB of
; local variables. It probes each page to ensure guard pages are touched
; and the stack can grow properly.

.code

; =============================================================================
; __chkstk - Stack probe for large allocations
; =============================================================================
;
; Input:  RAX = number of bytes to allocate
; Output: RAX preserved (caller uses it for sub rsp, rax)
;
; This function probes the stack pages from current RSP down to RSP-RAX
; to trigger guard page exceptions. Required for functions with >4KB locals.
;
; Note: In kernel mode / hypervisor context without guard pages, this is
; essentially a no-op but must exist to satisfy the linker.

__chkstk PROC
    ; Simple implementation: just return
    ; In hypervisor context we don't have user-mode guard pages,
    ; but the function must exist for the linker.
    ;
    ; For a fully correct implementation (if needed):
    ; - Probe each 4KB page from RSP down to RSP-RAX
    ; - Touch each page to ensure it's committed
    ;
    ; Since we're in kernel/hypervisor space with NonPagedPool stacks,
    ; pages are already committed and no probing is necessary.
    ret
__chkstk ENDP

; Also export as _chkstk (some linkers look for this name)
_chkstk PROC
    ret
_chkstk ENDP

END
