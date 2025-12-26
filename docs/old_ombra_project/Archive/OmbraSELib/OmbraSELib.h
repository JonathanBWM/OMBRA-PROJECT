#pragma once

// OmbraSELib - Ombra System Extension Library
// Aggregates UEFI and low-level system headers
// Provides SELib-compatible interfaces originally from Sputnik

// EDK2 headers are C code - wrap in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/FileHandleLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>
#include <IndustryStandard/PeImage.h>

#ifdef __cplusplus
}
#endif

//
// Windows kernel compatibility macros for UEFI context
// These map Windows kernel memory functions to UEFI equivalents
//
#ifndef RtlZeroMemory
#define RtlZeroMemory(Destination, Length) ZeroMem((Destination), (Length))
#endif

#ifndef RtlCopyMemory
#define RtlCopyMemory(Destination, Source, Length) CopyMem((Destination), (Source), (Length))
#endif

// Include globals (has its own extern "C" wrapper)
#include "Globals.h"

//
// Debug output support
//

// Serial port for debug output
#define OMBRA_DEBUG_PORT 0x2F8

// Intrinsics for port I/O
extern "C" {
    void __outbyte(unsigned short Port, unsigned char Data);
    void __outbytestring(unsigned short Port, unsigned char* Buffer, unsigned long Count);
}
#pragma intrinsic(__outbyte)
#pragma intrinsic(__outbytestring)

// Debug print buffer
static CHAR8 _OmbraDbgBuffer[0x100];

// DBG_PRINT - ASCII debug output to serial port
#define DBG_PRINT(...) \
    do { \
        AsciiSPrint(_OmbraDbgBuffer, sizeof(_OmbraDbgBuffer), __VA_ARGS__); \
        __outbytestring(OMBRA_DEBUG_PORT, (unsigned char*)_OmbraDbgBuffer, (unsigned long)AsciiStrLen(_OmbraDbgBuffer)); \
    } while(0)

// DbgMsg - Wide string debug output (SELib compatible)
// Converts wide string format to ASCII for serial output
// Note: Cast fmt to CHAR16* because L"..." literals are wchar_t* in C++ but UnicodeSPrint expects CHAR16*
static CHAR16 _OmbraDbgWideBuffer[0x100];
#define DbgMsg(fmt, ...) \
    do { \
        UnicodeSPrint(_OmbraDbgWideBuffer, sizeof(_OmbraDbgWideBuffer), (const CHAR16*)(fmt), ##__VA_ARGS__); \
        UnicodeStrToAsciiStrS(_OmbraDbgWideBuffer, _OmbraDbgBuffer, sizeof(_OmbraDbgBuffer)); \
        __outbytestring(OMBRA_DEBUG_PORT, (unsigned char*)_OmbraDbgBuffer, (unsigned long)AsciiStrLen(_OmbraDbgBuffer)); \
    } while(0)

//
// Memory management namespace (SELib compatible)
//
namespace memory {
    // Allocate pool memory
    inline VOID* eMalloc(UINTN Size) {
        VOID* Buffer = nullptr;
        if (gBS && !EFI_ERROR(gBS->AllocatePool(EfiBootServicesData, Size, &Buffer))) {
            return Buffer;
        }
        return nullptr;
    }

    // Free pool memory
    inline VOID eFree(VOID* Buffer) {
        if (gBS && Buffer) {
            gBS->FreePool(Buffer);
        }
    }

    // Allocate zeroed memory
    inline VOID* eCalloc(UINTN Count, UINTN Size) {
        VOID* Buffer = eMalloc(Count * Size);
        if (Buffer) {
            ZeroMem(Buffer, Count * Size);
        }
        return Buffer;
    }
}

//
// VGA/Display output namespace (SELib compatible)
//
namespace io {
    namespace vga {
        // Clear console output
        inline VOID Clear() {
            if (gST && gST->ConOut) {
                gST->ConOut->ClearScreen(gST->ConOut);
            }
        }

        // Output wide string to console (primary interface used by Sputnik/Ombra)
        // On MSVC, wchar_t is the same size as CHAR16 so we can cast directly
        inline VOID Output(const wchar_t* String) {
            if (gST && gST->ConOut && String) {
                gST->ConOut->OutputString(gST->ConOut, (CHAR16*)String);
            }
        }

        // Output CHAR16 string to console (UEFI native)
        inline VOID OutputW(const CHAR16* String) {
            if (gST && gST->ConOut && String) {
                gST->ConOut->OutputString(gST->ConOut, (CHAR16*)String);
            }
        }
    }
}

//
// Threading namespace (SELib compatible)
//
namespace threading {
    // Sleep for microseconds (uses UEFI Stall)
    inline VOID Sleep(UINTN Microseconds) {
        if (gBS) {
            gBS->Stall(Microseconds);
        }
    }
}
