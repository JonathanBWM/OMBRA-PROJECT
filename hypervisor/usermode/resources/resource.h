#ifndef OMBRA_RESOURCE_H
#define OMBRA_RESOURCE_H

// Vulnerable drivers for kernel access
#define IDR_DRIVER_LD9BOXSUP        101
#define IDR_DRIVER_THROTTLESTOP     102

// Our payloads
#define IDR_PAYLOAD_HYPERVISOR      103

// Resource type (using RCDATA for binary blobs)
#define RT_DRIVER_BINARY            RT_RCDATA

#endif // OMBRA_RESOURCE_H
