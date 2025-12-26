// PayLoad/core/dispatch.h
// Main hypercall dispatch interface
// Called by architecture backends (Intel VMX / AMD SVM)
#pragma once

#include "../include/context.h"

namespace core {

//===----------------------------------------------------------------------===//
// Main Dispatch Interface
//===----------------------------------------------------------------------===//

// Primary entry point - called by architecture backends after populating VmExitContext
// Returns the VMX_ROOT_ERROR result code to be placed in guest RAX
VMX_ROOT_ERROR HandleVmcall(ombra::VmExitContext* ctx);

// Register architecture-specific callbacks
// Must be called by each backend during initialization
void RegisterArchCallbacks(const ombra::ArchCallbacks& callbacks);

// Check if callbacks have been registered
bool HasArchCallbacks();

//===----------------------------------------------------------------------===//
// Memory Operations Interface
// These are implemented by each backend in their mm.cpp
//===----------------------------------------------------------------------===//

namespace mm {

// Copy memory between guest virtual addresses
// Uses page table walking to translate addresses
VMX_ROOT_ERROR copy_guest_virt(
    u64 dirbase_src,
    guest_virt_t virt_src,
    u64 dirbase_dest,
    guest_virt_t virt_dest,
    u64 size
);

// Read from guest physical address to host virtual
VMX_ROOT_ERROR read_guest_phys(
    u64 guest_dirbase,
    guest_phys_t phys_addr,
    guest_virt_t out_buffer,
    u64 size
);

// Write from host virtual to guest physical address
VMX_ROOT_ERROR write_guest_phys(
    u64 guest_dirbase,
    guest_phys_t phys_addr,
    guest_virt_t in_buffer,
    u64 size
);

// Translate guest virtual to guest physical
// Note: default for map_type is in mm.h - only specify once per ODR
u64 translate_guest_virtual(
    u64 dirbase,
    guest_virt_t virt_addr,
    map_type_t map_type
);

// Translate host virtual to host physical
host_phys_t translate(host_virt_t virt_addr);

// Note: init() is NOT declared here because Intel returns VMX_ROOT_ERROR
// and AMD returns u64. Each backend calls their mm::init() directly.

} // namespace mm

//===----------------------------------------------------------------------===//
// Individual Command Handlers
//===----------------------------------------------------------------------===//

namespace handlers {

VMX_ROOT_ERROR HandleGetCr3(ombra::VmExitContext* ctx);
VMX_ROOT_ERROR HandleGetCr3Root(ombra::VmExitContext* ctx);
VMX_ROOT_ERROR HandleReadPhys(ombra::VmExitContext* ctx);
VMX_ROOT_ERROR HandleWritePhys(ombra::VmExitContext* ctx);
VMX_ROOT_ERROR HandleReadVirt(ombra::VmExitContext* ctx);
VMX_ROOT_ERROR HandleWriteVirt(ombra::VmExitContext* ctx);
VMX_ROOT_ERROR HandleVirtToPhys(ombra::VmExitContext* ctx);
VMX_ROOT_ERROR HandleStorageQuery(ombra::VmExitContext* ctx);
VMX_ROOT_ERROR HandleGetEptBase(ombra::VmExitContext* ctx);
VMX_ROOT_ERROR HandleSetEptBase(ombra::VmExitContext* ctx);
VMX_ROOT_ERROR HandleEnableEpt(ombra::VmExitContext* ctx);
VMX_ROOT_ERROR HandleDisableEpt(ombra::VmExitContext* ctx);
VMX_ROOT_ERROR HandleSetCommKey(ombra::VmExitContext* ctx);

// AMD-only command
VMX_ROOT_ERROR HandleGetVmcb(ombra::VmExitContext* ctx);

// ETW Threat Intelligence Provider Control (Phase 3 Artifact Elimination)
VMX_ROOT_ERROR HandleDisableEtwTi(ombra::VmExitContext* ctx);
VMX_ROOT_ERROR HandleEnableEtwTi(ombra::VmExitContext* ctx);
VMX_ROOT_ERROR HandleWipeEtwBuffers(ombra::VmExitContext* ctx);

// Event Log Cleanup (Phase 3 Artifact Elimination)
VMX_ROOT_ERROR HandleClearEventLogs(ombra::VmExitContext* ctx);

} // namespace handlers

} // namespace core
