#ifndef OMBRA_TYPES_H
#define OMBRA_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t  U8;  typedef int8_t  I8;
typedef uint16_t U16; typedef int16_t I16;
typedef uint32_t U32; typedef int32_t I32;
typedef uint64_t U64; typedef int64_t I64;

typedef U64 GPA, GVA, HPA, HVA;

#define PAGE_SIZE   0x1000ULL
#define PAGE_SHIFT  12
#define BIT(n)      (1ULL << (n))

#define ALIGN_UP(x, a)   (((x) + ((a)-1)) & ~((a)-1))
#define ALIGN_DOWN(x, a) ((x) & ~((a)-1))

typedef enum {
    OMBRA_SUCCESS = 0,
    OMBRA_ERROR_NOT_SUPPORTED,
    OMBRA_ERROR_VMX_DISABLED,
    OMBRA_ERROR_ALREADY_RUNNING,
    OMBRA_ERROR_VMXON_FAILED,
    OMBRA_ERROR_VMCS_FAILED,
    OMBRA_ERROR_VMLAUNCH_FAILED,
} OMBRA_STATUS;

#define OMBRA_SUCCESS(s) ((s) == OMBRA_SUCCESS)
#define OMBRA_FAILED(s)  ((s) != OMBRA_SUCCESS)

#endif
