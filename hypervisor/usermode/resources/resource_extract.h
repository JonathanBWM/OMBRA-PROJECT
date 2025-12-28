#ifndef RESOURCE_EXTRACT_H
#define RESOURCE_EXTRACT_H

#include <windows.h>
#include <stdbool.h>
#include "resource.h"

typedef enum {
    EMBEDDED_DRIVER_LD9BOXSUP = IDR_DRIVER_LD9BOXSUP,
    EMBEDDED_DRIVER_THROTTLESTOP = IDR_DRIVER_THROTTLESTOP,
    EMBEDDED_PAYLOAD_HYPERVISOR = IDR_PAYLOAD_HYPERVISOR
} EMBEDDED_RESOURCE_ID;

// Extract resource to temp file, returns allocated path (caller frees)
wchar_t* Resource_ExtractToTemp(EMBEDDED_RESOURCE_ID resourceId);

// Extract resource to memory buffer (caller frees)
void* Resource_ExtractToMemory(EMBEDDED_RESOURCE_ID resourceId, size_t* outSize);

// Secure delete and cleanup extracted file
bool Resource_Cleanup(const wchar_t* extractedPath);

// Get display name for logging
const wchar_t* Resource_GetName(EMBEDDED_RESOURCE_ID resourceId);

// Check if resource exists in executable
bool Resource_Exists(EMBEDDED_RESOURCE_ID resourceId);

#endif // RESOURCE_EXTRACT_H
