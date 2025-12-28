#ifndef OMBRA_RESOURCE_H
#define OMBRA_RESOURCE_H

// =============================================================================
// Embedded Driver Resources
// =============================================================================
//
// All 4 drivers are embedded as encrypted RCDATA resources:
// - BYOVD exploits (Ld9BoxSup, ThrottleStop) provide kernel code execution
// - Hypervisor payload runs in VMX root mode
// - OmbraDriver provides usermode-to-hypervisor communication

// BYOVD vulnerable drivers for kernel access
#define IDR_DRIVER_LD9BOXSUP        101     // LDPlayer VBox fork - kernel code loading
#define IDR_DRIVER_THROTTLESTOP     102     // CPU tuner - physical memory R/W

// Our payloads
#define IDR_PAYLOAD_HYPERVISOR      103     // VMX hypervisor (entry: HvEntry)
#define IDR_PAYLOAD_OMBRADRIVER     104     // Windows driver (VMCALL interface)

// Resource type (using RCDATA for binary blobs)
#define RT_DRIVER_BINARY            RT_RCDATA

#endif // OMBRA_RESOURCE_H
