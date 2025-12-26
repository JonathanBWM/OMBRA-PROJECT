/**
 * @file types.h
 * @brief Base types for BYOVD loader
 *
 * C port from C++ reference implementation.
 */

#ifndef BYOVD_TYPES_H
#define BYOVD_TYPES_H

#include <Windows.h>
#include <stdint.h>
#include <stdbool.h>

//=============================================================================
// Basic Types
//=============================================================================

typedef int8_t      INT8;
typedef int16_t     INT16;
typedef int32_t     INT32;
typedef int64_t     INT64;

typedef uint8_t     UINT8;
typedef uint16_t    UINT16;
typedef uint32_t    UINT32;
typedef uint64_t    UINT64;

//=============================================================================
// Status Codes (VirtualBox/LDPlayer style)
//=============================================================================

#define VINF_SUCCESS                0
#define VERR_GENERAL_FAILURE        (-1)
#define VERR_INVALID_PARAMETER      (-2)
#define VERR_NO_MEMORY              (-3)
#define VERR_NOT_SUPPORTED          (-12)      // MSR_PROBER returns this when disabled
#define VERR_LDR_GENERAL_FAILURE    (-618)     // -618 flag validation failed

//=============================================================================
// Debug Logging
//=============================================================================

#ifndef BYOVD_DEBUG
#define BYOVD_DEBUG 1
#endif

#if BYOVD_DEBUG
    #include <stdio.h>
    #define DbgLog(fmt, ...) printf("[BYOVD] " fmt "\n", ##__VA_ARGS__)
#else
    #define DbgLog(fmt, ...) ((void)0)
#endif

#endif // BYOVD_TYPES_H
