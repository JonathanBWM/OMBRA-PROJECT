#pragma once
// OmbraSELib Standalone Types for VMXRoot Context
// This file provides basic types without WDK dependencies
// NOTE: Windows types (ULONG, PVOID, etc) are NOT defined here -
// the PayLoad projects get them from Windows.h

#include <intrin.h>
#include <cstddef>

// Basic integer types - these are the primary types used by OmbraSELib
using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
using s8 = signed char;
using s16 = signed short;
using s32 = signed int;
using s64 = signed long long;

// Page constants - only define if not already defined by PayLoad
#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

#ifndef PAGE_4KB
#define PAGE_4KB 0x1000
#endif

#ifndef PAGE_2MB
#define PAGE_2MB (PAGE_4KB * 512)
#endif

#ifndef PAGE_1GB
#define PAGE_1GB (PAGE_2MB * 512)
#endif

// Segment descriptor type for interrupt gate
#ifndef SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE
#define SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE 0x0E
#endif
