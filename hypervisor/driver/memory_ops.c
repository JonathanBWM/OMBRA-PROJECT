// memory_ops.c - Physical and Virtual Memory Operations
// OmbraDriver Phase 3
//
// Provides memory read/write primitives via hypervisor VMCALLs

#include "context.h"
#include "command_ring.h"
#include "vmcall.h"

// =============================================================================
// Physical Memory Operations
// =============================================================================

I32 HandleReadPhysical(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    U64 physAddr = cmd->PhysicalMem.PhysicalAddress;
    U64 size = cmd->PhysicalMem.Size;
    U64 scratchOffset = cmd->PhysicalMem.ScratchOffset;

    if (size == 0 || size > 0x10000) {  // Max 64KB per operation
        return OMBRA_STATUS_INVALID_PARAMETER;
    }

    // For small reads (<=8 bytes), return in response directly
    if (size <= 8 && scratchOffset == 0) {
        U64 value = 0;

        // Read physical memory via hypervisor
        // The hypervisor maps physical pages temporarily for access
        I64 status = VmCall(VMCALL_READ_PHYS, physAddr, (U64)&value, size);
        if (status != VMCALL_STATUS_SUCCESS) {
            return OMBRA_STATUS_VMCALL_FAILED;
        }

        // Copy to response
        volatile U8* dst = (volatile U8*)resp->Raw;
        volatile U8* src = (volatile U8*)&value;
        for (U64 i = 0; i < size; i++) {
            dst[i] = src[i];
        }

        resp->MemoryResult.BytesTransferred = size;
        return OMBRA_STATUS_SUCCESS;
    }

    // Large read - use scratch buffer
    U8* scratch = (U8*)g_DriverCtx.ScratchBuffer + scratchOffset;
    if (scratchOffset + size > g_DriverCtx.ScratchSize) {
        return OMBRA_STATUS_INVALID_PARAMETER;
    }

    // Read in chunks
    U64 bytesRead = 0;
    while (bytesRead < size) {
        U64 chunk = (size - bytesRead > 4096) ? 4096 : (size - bytesRead);

        I64 status = VmCall(VMCALL_READ_PHYS, physAddr + bytesRead,
                           (U64)(scratch + bytesRead), chunk);
        if (status != VMCALL_STATUS_SUCCESS) {
            resp->MemoryResult.BytesTransferred = bytesRead;
            return OMBRA_STATUS_VMCALL_FAILED;
        }

        bytesRead += chunk;
    }

    resp->MemoryResult.BytesTransferred = bytesRead;
    return OMBRA_STATUS_SUCCESS;
}

I32 HandleWritePhysical(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    U64 physAddr = cmd->PhysicalMem.PhysicalAddress;
    U64 size = cmd->PhysicalMem.Size;
    U64 scratchOffset = cmd->PhysicalMem.ScratchOffset;

    if (size == 0 || size > 0x10000) {
        return OMBRA_STATUS_INVALID_PARAMETER;
    }

    // For small writes (<=8 bytes), data is in command
    if (size <= 8 && scratchOffset == 0) {
        U64 value = 0;
        volatile U8* dst = (volatile U8*)&value;
        volatile U8* src = (volatile U8*)cmd->Raw;
        for (U64 i = 0; i < size; i++) {
            dst[i] = src[i];
        }

        I64 status = VmCall(VMCALL_WRITE_PHYS, physAddr, value, size);
        if (status != VMCALL_STATUS_SUCCESS) {
            return OMBRA_STATUS_VMCALL_FAILED;
        }

        resp->MemoryResult.BytesTransferred = size;
        return OMBRA_STATUS_SUCCESS;
    }

    // Large write - read from scratch buffer
    U8* scratch = (U8*)g_DriverCtx.ScratchBuffer + scratchOffset;
    if (scratchOffset + size > g_DriverCtx.ScratchSize) {
        return OMBRA_STATUS_INVALID_PARAMETER;
    }

    U64 bytesWritten = 0;
    while (bytesWritten < size) {
        U64 chunk = (size - bytesWritten > 4096) ? 4096 : (size - bytesWritten);

        I64 status = VmCall(VMCALL_WRITE_PHYS, physAddr + bytesWritten,
                           (U64)(scratch + bytesWritten), chunk);
        if (status != VMCALL_STATUS_SUCCESS) {
            resp->MemoryResult.BytesTransferred = bytesWritten;
            return OMBRA_STATUS_VMCALL_FAILED;
        }

        bytesWritten += chunk;
    }

    resp->MemoryResult.BytesTransferred = bytesWritten;
    return OMBRA_STATUS_SUCCESS;
}

// =============================================================================
// Virtual Memory Operations
// =============================================================================

I32 HandleReadVirtual(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    U64 cr3 = cmd->VirtualMem.Cr3;
    U64 va = cmd->VirtualMem.VirtualAddress;
    U64 size = cmd->VirtualMem.Size;
    U64 scratchOffset = cmd->VirtualMem.ScratchOffset;

    if (size == 0 || size > 0x10000) {
        return OMBRA_STATUS_INVALID_PARAMETER;
    }

    // Small read - return in response
    if (size <= sizeof(resp->Raw) && scratchOffset == 0) {
        I64 status = VmReadVirtual(cr3, va, (void*)resp->Raw, size);
        if (status != VMCALL_STATUS_SUCCESS) {
            return OMBRA_STATUS_VMCALL_FAILED;
        }
        resp->MemoryResult.BytesTransferred = size;
        return OMBRA_STATUS_SUCCESS;
    }

    // Large read - use scratch buffer
    U8* scratch = (U8*)g_DriverCtx.ScratchBuffer + scratchOffset;
    if (scratchOffset + size > g_DriverCtx.ScratchSize) {
        return OMBRA_STATUS_INVALID_PARAMETER;
    }

    I64 status = VmReadVirtual(cr3, va, scratch, size);
    if (status != VMCALL_STATUS_SUCCESS) {
        return OMBRA_STATUS_VMCALL_FAILED;
    }

    resp->MemoryResult.BytesTransferred = size;
    return OMBRA_STATUS_SUCCESS;
}

I32 HandleWriteVirtual(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    U64 cr3 = cmd->VirtualMem.Cr3;
    U64 va = cmd->VirtualMem.VirtualAddress;
    U64 size = cmd->VirtualMem.Size;
    U64 scratchOffset = cmd->VirtualMem.ScratchOffset;

    if (size == 0 || size > 0x10000) {
        return OMBRA_STATUS_INVALID_PARAMETER;
    }

    // Small write - data in command (after fixed fields)
    if (size <= 400 && scratchOffset == 0) {  // Leave room for VirtualMem struct
        // Data is embedded after the VirtualMem fields
        void* data = &cmd->Raw[32];

        I64 status = VmWriteVirtual(cr3, va, data, size);
        if (status != VMCALL_STATUS_SUCCESS) {
            return OMBRA_STATUS_VMCALL_FAILED;
        }
        resp->MemoryResult.BytesTransferred = size;
        return OMBRA_STATUS_SUCCESS;
    }

    // Large write - use scratch buffer
    U8* scratch = (U8*)g_DriverCtx.ScratchBuffer + scratchOffset;
    if (scratchOffset + size > g_DriverCtx.ScratchSize) {
        return OMBRA_STATUS_INVALID_PARAMETER;
    }

    I64 status = VmWriteVirtual(cr3, va, scratch, size);
    if (status != VMCALL_STATUS_SUCCESS) {
        return OMBRA_STATUS_VMCALL_FAILED;
    }

    resp->MemoryResult.BytesTransferred = size;
    return OMBRA_STATUS_SUCCESS;
}

// =============================================================================
// Identity Map Operations
// =============================================================================

I32 HandleGetIdentityMap(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    U64 size = cmd->Memory.Size;

    if (size == 0 || size > 0x100000) {  // Max 1MB identity map
        return OMBRA_STATUS_INVALID_PARAMETER;
    }

    // Identity map provides direct physical memory access
    // The hypervisor creates a VA = PA mapping in a reserved region
    // This is useful for DMA or physical memory scanning

    // For now, return scratch buffer as "identity mapped"
    // Real implementation would allocate from hypervisor-managed pool
    resp->IdentityMap.IdentityBase = (U64)g_DriverCtx.ScratchBuffer;

    return OMBRA_STATUS_SUCCESS;
}

I32 HandleReleaseIdentityMap(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;
    (void)resp;

    // Nothing to do for simple implementation
    return OMBRA_STATUS_SUCCESS;
}
