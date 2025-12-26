#pragma once

// OmbraSELib Globals - UEFI global variable declarations
// These are provided by UefiBootServicesTableLib and UefiRuntimeServicesTableLib
// Must use C linkage to match EDK2 pre-built libraries

#ifdef __cplusplus
extern "C" {
#endif

#include <Uefi.h>

// Boot Services Table - provided by UefiBootServicesTableLib
extern EFI_BOOT_SERVICES* gBS;

// Runtime Services Table - provided by UefiRuntimeServicesTableLib
extern EFI_RUNTIME_SERVICES* gRT;

// System Table - provided by UefiLib
extern EFI_SYSTEM_TABLE* gST;

// Image Handle - provided by UefiBootServicesTableLib
extern EFI_HANDLE gImageHandle;

#ifdef __cplusplus
}
#endif
