// vmcall.c - VMCALL wrapper implementation for OmbraDriver
// Provides kernel-mode VMCALL interface to communicate with OmbraHypervisor

#include "vmcall.h"
#include <intrin.h>

// Global VMCALL authentication state
static U64 g_VmcallMagic = 0;
static U64 g_VmcallKey = 0;

// Initialize VMCALL interface with magic and key from loader
void VmCallInit(U64 magic, U64 key) {
    g_VmcallMagic = magic;
    g_VmcallKey = key;
}

// Execute VMCALL instruction with parameters
// Calling convention:
//   RAX = magic signature (authentication)
//   RCX = command code
//   RDX = key (authentication)
//   R8  = param1
//   R9  = param2
//   R10 = param3
//
// Return:
//   RAX = status code (I64)
//   RDX = return value 1 (optional)
//   R8  = return value 2 (optional)

#if defined(_M_AMD64) || defined(__x86_64__)

I64 VmCall(U64 command, U64 param1, U64 param2, U64 param3) {
    I64 status;
    U64 outRdx;
    U64 outR8;

    // Use inline assembly to execute VMCALL with proper register passing
    // Microsoft x64 calling convention: RCX, RDX, R8, R9 for first 4 params
    // We need to carefully orchestrate registers for the VMCALL ABI

    __asm {
        push rbx
        push rdi
        push rsi

        // Setup VMCALL parameters
        mov rax, g_VmcallMagic      // RAX = magic signature
        mov rcx, command             // RCX = command code
        mov rdx, g_VmcallKey         // RDX = key
        mov r8,  param1              // R8  = param1
        mov r9,  param2              // R9  = param2
        mov r10, param3              // R10 = param3

        // Execute VMCALL
        // This causes a VM-exit to the hypervisor's HandleVmcall()
        // The hypervisor processes the command and returns via VMRESUME
        vmcall

        // Capture return values
        mov status, rax              // Status code
        mov outRdx, rdx              // Optional return value 1
        mov outR8, r8                // Optional return value 2

        pop rsi
        pop rdi
        pop rbx
    }

    // Some commands use RDX/R8 for output - store in thread-local if needed
    // For now, we ignore output parameters in this generic wrapper
    (void)outRdx;
    (void)outR8;

    return status;
}

#else
#error "Unsupported architecture - x64 required"
#endif

// CR3 watching for process tracking
I64 VmWatchCr3(U64 cr3) {
    return VmCall(VMCALL_WATCH_CR3, cr3, 0, 0);
}

I64 VmUnwatchCr3(U64 cr3) {
    return VmCall(VMCALL_UNWATCH_CR3, cr3, 0, 0);
}

// Physical page management
I64 VmPinPage(U64 physicalAddr) {
    return VmCall(VMCALL_PIN_PAGE, physicalAddr, 0, 0);
}

I64 VmUnpinPage(U64 physicalAddr) {
    return VmCall(VMCALL_UNPIN_PAGE, physicalAddr, 0, 0);
}

I64 VmAllocPhysicalPage(U64* outPhysical) {
    I64 status;
    U64 physAddr = 0;

    if (!outPhysical) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    __asm {
        push rbx
        push rdi
        push rsi

        mov rax, g_VmcallMagic
        mov rcx, VMCALL_ALLOC_PHYSICAL_PAGE
        mov rdx, g_VmcallKey
        xor r8, r8
        xor r9, r9
        xor r10, r10

        vmcall

        mov status, rax
        mov physAddr, rdx            // Physical address returned in RDX

        pop rsi
        pop rdi
        pop rbx
    }

    *outPhysical = physAddr;
    return status;
}

I64 VmFreePhysicalPage(U64 physicalAddr) {
    return VmCall(VMCALL_FREE_PHYSICAL_PAGE, physicalAddr, 0, 0);
}

I64 VmCopyPhysicalPage(U64 srcPhysical, U64 dstPhysical) {
    return VmCall(VMCALL_COPY_PHYSICAL_PAGE, srcPhysical, dstPhysical, 0);
}

// EPT split for shadowing
I64 VmSplitEptPage(U64 targetCr3, U64 guestPhysical, U64 cleanPhysical) {
    return VmCall(VMCALL_SPLIT_EPT_PAGE, targetCr3, guestPhysical, cleanPhysical);
}

I64 VmUnsplitEptPage(U64 targetCr3, U64 guestPhysical) {
    return VmCall(VMCALL_UNSPLIT_EPT_PAGE, targetCr3, guestPhysical, 0);
}

// Virtual memory access (cross-process)
// These need special handling because they deal with buffers
I64 VmReadVirtual(U64 targetCr3, U64 va, void* buffer, U64 size) {
    if (!buffer || size == 0) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // For small reads (8 bytes or less), use single VMCALL
    if (size <= 8) {
        I64 status;
        U64 value = 0;

        __asm {
            push rbx
            push rdi
            push rsi

            mov rax, g_VmcallMagic
            mov rcx, VMCALL_READ_VIRT
            mov rdx, g_VmcallKey
            mov r8,  targetCr3
            mov r9,  va
            mov r10, size

            vmcall

            mov status, rax
            mov value, rdx               // Value returned in RDX

            pop rsi
            pop rdi
            pop rbx
        }

        if (status == VMCALL_STATUS_SUCCESS) {
            // Copy value to buffer based on size
            switch (size) {
            case 1:
                *(U8*)buffer = (U8)value;
                break;
            case 2:
                *(U16*)buffer = (U16)value;
                break;
            case 4:
                *(U32*)buffer = (U32)value;
                break;
            case 8:
                *(U64*)buffer = value;
                break;
            }
        }

        return status;
    }

    // For larger reads, loop and read in 8-byte chunks
    U8* dest = (U8*)buffer;
    U64 bytesRead = 0;

    while (bytesRead < size) {
        U64 chunkSize = (size - bytesRead >= 8) ? 8 : (size - bytesRead);
        U64 value = 0;
        I64 status;

        __asm {
            push rbx
            push rdi
            push rsi

            mov rax, g_VmcallMagic
            mov rcx, VMCALL_READ_VIRT
            mov rdx, g_VmcallKey
            mov r8,  targetCr3
            mov r9,  va
            add r9,  bytesRead
            mov r10, chunkSize

            vmcall

            mov status, rax
            mov value, rdx

            pop rsi
            pop rdi
            pop rbx
        }

        if (status != VMCALL_STATUS_SUCCESS) {
            return status;
        }

        // Copy chunk to buffer
        for (U64 i = 0; i < chunkSize; i++) {
            dest[bytesRead + i] = (U8)(value >> (i * 8));
        }

        bytesRead += chunkSize;
    }

    return VMCALL_STATUS_SUCCESS;
}

I64 VmWriteVirtual(U64 targetCr3, U64 va, const void* buffer, U64 size) {
    if (!buffer || size == 0) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    const U8* src = (const U8*)buffer;

    // For small writes (8 bytes or less), use single VMCALL
    if (size <= 8) {
        U64 value = 0;

        // Assemble value from buffer
        for (U64 i = 0; i < size; i++) {
            value |= ((U64)src[i]) << (i * 8);
        }

        I64 status;

        __asm {
            push rbx
            push rdi
            push rsi

            mov rax, g_VmcallMagic
            mov rcx, VMCALL_WRITE_VIRT
            mov rdx, g_VmcallKey
            mov r8,  targetCr3
            mov r9,  va
            mov r10, value

            vmcall

            mov status, rax

            pop rsi
            pop rdi
            pop rbx
        }

        return status;
    }

    // For larger writes, loop and write in 8-byte chunks
    U64 bytesWritten = 0;

    while (bytesWritten < size) {
        U64 chunkSize = (size - bytesWritten >= 8) ? 8 : (size - bytesWritten);
        U64 value = 0;

        // Assemble chunk value
        for (U64 i = 0; i < chunkSize; i++) {
            value |= ((U64)src[bytesWritten + i]) << (i * 8);
        }

        I64 status;

        __asm {
            push rbx
            push rdi
            push rsi

            mov rax, g_VmcallMagic
            mov rcx, VMCALL_WRITE_VIRT
            mov rdx, g_VmcallKey
            mov r8,  targetCr3
            mov r9,  va
            add r9,  bytesWritten
            mov r10, value

            vmcall

            mov status, rax

            pop rsi
            pop rdi
            pop rbx
        }

        if (status != VMCALL_STATUS_SUCCESS) {
            return status;
        }

        bytesWritten += chunkSize;
    }

    return VMCALL_STATUS_SUCCESS;
}

I64 VmVirtToPhys(U64 targetCr3, U64 va, U64* outPhysical) {
    I64 status;
    U64 physAddr = 0;

    if (!outPhysical) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    __asm {
        push rbx
        push rdi
        push rsi

        mov rax, g_VmcallMagic
        mov rcx, VMCALL_VIRT_TO_PHYS
        mov rdx, g_VmcallKey
        mov r8,  targetCr3
        mov r9,  va
        xor r10, r10

        vmcall

        mov status, rax
        mov physAddr, rdx            // Physical address returned in RDX

        pop rsi
        pop rdi
        pop rbx
    }

    *outPhysical = physAddr;
    return status;
}

// Driver lifecycle
I64 VmDriverReady(U64 ownerCr3) {
    return VmCall(VMCALL_DRIVER_READY, ownerCr3, 0, 0);
}

I64 VmDriverShutdown(void) {
    return VmCall(VMCALL_DRIVER_SHUTDOWN, 0, 0, 0);
}

// Memory hiding
I64 VmHideMemory(U64 addr, U64 size) {
    return VmCall(VMCALL_HIDE_MEMORY, addr, size, 0);
}

I64 VmUnhideMemory(U64 addr, U64 size) {
    return VmCall(VMCALL_UNHIDE_MEMORY, addr, size, 0);
}
