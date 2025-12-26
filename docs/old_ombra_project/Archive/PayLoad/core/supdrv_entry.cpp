/**
 * @file supdrv_entry.cpp
 * @brief Kernel-mode entry point for SUPDrv-based loading
 *
 * Called by SUPDrv's pfnModuleInit callback after code is loaded into kernel.
 * This is a SINGLE CALL that must:
 * 1. Detect CPU architecture (Intel VMX vs AMD SVM)
 * 2. Find Hyper-V module (hvix64.exe or hvax64.exe)
 * 3. Scan for VMExit handler pattern
 * 4. Patch the CALL instruction to redirect to our handler
 * 5. Initialize hypervisor subsystems
 *
 * After this returns, ALL VMExits route through our handler.
 */

#include "../include/types.h"
#include "storage.h"
#include "dispatch.h"

// Forward declarations for architecture-specific handlers
namespace intel { extern void vmexit_handler(void*, void*); }
namespace amd { extern void* vmexit_handler(void*, void*, void*); }

namespace supdrv_entry {

//=============================================================================
// Kernel Types (minimal definitions for freestanding context)
//=============================================================================

#pragma pack(push, 1)

// Loaded module list entry (simplified KLDR_DATA_TABLE_ENTRY)
typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    // ... more fields follow but we don't need them
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

// Unicode string structure
typedef struct _UNICODE_STRING_KM {
    USHORT Length;
    USHORT MaximumLength;
    PWCHAR Buffer;
} UNICODE_STRING_KM;

#pragma pack(pop)

// Result codes
constexpr int OMBRA_SUCCESS = 0;
constexpr int OMBRA_ERROR_CPU_DETECT = 1;
constexpr int OMBRA_ERROR_HV_NOT_FOUND = 2;
constexpr int OMBRA_ERROR_PATTERN_NOT_FOUND = 3;
constexpr int OMBRA_ERROR_PATCH_FAILED = 4;
constexpr int OMBRA_ERROR_INIT_FAILED = 5;

//=============================================================================
// VMExit Pattern Database (embedded for kernel use)
//=============================================================================

struct VmExitPattern {
    u8 Pattern[32];
    u8 Mask[32];
    u32 PatternLen;
    bool AutoDiscoverCall;
    u32 ScanRangeStart;
    u32 ScanRangeEnd;
};

// Intel VMX signature - universal pattern for Windows 10/11
// gs:[0x6D] = 0, mov rcx from stack, mov rdx from stack, call, jmp
constexpr VmExitPattern g_IntelPattern = {
    { 0x65, 0xC6, 0x04, 0x25, 0x6D, 0x00, 0x00, 0x00, 0x00,  // gs:[0x6D] = 0
      0x48, 0x8B, 0x4C, 0x24, 0x00,                          // mov rcx, [rsp+?]
      0x48, 0x8B, 0x54, 0x24, 0x00,                          // mov rdx, [rsp+?]
      0xE8, 0x00, 0x00, 0x00, 0x00,                          // call rel32
      0xE9 },                                                 // jmp
    { 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
      'x', 'x', 'x', 'x', '?',
      'x', 'x', 'x', 'x', '?',
      'x', '?', '?', '?', '?',
      'x' },
    25,     // PatternLen
    true,   // AutoDiscoverCall
    0x50,   // ScanRangeStart
    0x180   // ScanRangeEnd
};

// AMD SVM signature - CALL is at start of pattern
// call vcpu_run, mov [rsp], rax, jmp
constexpr VmExitPattern g_AmdPattern = {
    { 0xE8, 0x00, 0x00, 0x00, 0x00,  // call rel32
      0x48, 0x89, 0x04, 0x24,        // mov [rsp], rax
      0xE9 },                         // jmp
    { 'x', '?', '?', '?', '?',
      'x', 'x', 'x', 'x',
      'x' },
    10,     // PatternLen
    false,  // AutoDiscoverCall (CALL is directly in pattern)
    0,      // ScanRangeStart (not used)
    0       // ScanRangeEnd (not used)
};

//=============================================================================
// CPU Detection
//=============================================================================

static bool IsIntelCpu()
{
    int cpuInfo[4];
    __cpuid(cpuInfo, 0);

    // Check for "GenuineIntel"
    char vendor[13] = { 0 };
    *reinterpret_cast<int*>(&vendor[0]) = cpuInfo[1];  // EBX
    *reinterpret_cast<int*>(&vendor[4]) = cpuInfo[3];  // EDX
    *reinterpret_cast<int*>(&vendor[8]) = cpuInfo[2];  // ECX

    return vendor[0] == 'G' && vendor[1] == 'e' && vendor[2] == 'n' &&
           vendor[3] == 'u' && vendor[4] == 'i' && vendor[5] == 'n' &&
           vendor[6] == 'e' && vendor[7] == 'I' && vendor[8] == 'n' &&
           vendor[9] == 't' && vendor[10] == 'e' && vendor[11] == 'l';
}

static bool IsAmdCpu()
{
    int cpuInfo[4];
    __cpuid(cpuInfo, 0);

    // Check for "AuthenticAMD"
    char vendor[13] = { 0 };
    *reinterpret_cast<int*>(&vendor[0]) = cpuInfo[1];  // EBX
    *reinterpret_cast<int*>(&vendor[4]) = cpuInfo[3];  // EDX
    *reinterpret_cast<int*>(&vendor[8]) = cpuInfo[2];  // ECX

    return vendor[0] == 'A' && vendor[1] == 'u' && vendor[2] == 't' &&
           vendor[3] == 'h' && vendor[4] == 'e' && vendor[5] == 'n' &&
           vendor[6] == 't' && vendor[7] == 'i' && vendor[8] == 'c' &&
           vendor[9] == 'A' && vendor[10] == 'M' && vendor[11] == 'D';
}

//=============================================================================
// Module Walking (finds hvix64.exe or hvax64.exe)
//=============================================================================

// PsLoadedModuleList is exported by ntoskrnl
extern "C" PLIST_ENTRY PsLoadedModuleList;

// Compare Unicode strings (case-insensitive)
static bool UnicodeStringContains(const UNICODE_STRING* str, const wchar_t* needle)
{
    if (!str || !str->Buffer || !needle)
        return false;

    // Convert needle length
    size_t needleLen = 0;
    while (needle[needleLen]) needleLen++;

    if (str->Length / sizeof(wchar_t) < needleLen)
        return false;

    // Case-insensitive substring search
    size_t strLen = str->Length / sizeof(wchar_t);
    for (size_t i = 0; i <= strLen - needleLen; i++) {
        bool match = true;
        for (size_t j = 0; j < needleLen && match; j++) {
            wchar_t c1 = str->Buffer[i + j];
            wchar_t c2 = needle[j];
            // Lowercase
            if (c1 >= L'A' && c1 <= L'Z') c1 += 32;
            if (c2 >= L'A' && c2 <= L'Z') c2 += 32;
            if (c1 != c2) match = false;
        }
        if (match) return true;
    }
    return false;
}

// Find Hyper-V module base and size
static bool FindHyperVModule(bool isIntel, PVOID* pBase, SIZE_T* pSize)
{
    const wchar_t* hvName = isIntel ? L"hvix64.exe" : L"hvax64.exe";

    // Walk PsLoadedModuleList
    if (!PsLoadedModuleList)
        return false;

    PLIST_ENTRY listHead = PsLoadedModuleList;
    PLIST_ENTRY entry = listHead->Flink;

    while (entry != listHead) {
        PLDR_DATA_TABLE_ENTRY module = CONTAINING_RECORD(
            entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        if (UnicodeStringContains(&module->BaseDllName, hvName)) {
            *pBase = module->DllBase;
            *pSize = module->SizeOfImage;
            return true;
        }

        entry = entry->Flink;
    }

    return false;
}

//=============================================================================
// Pattern Scanning
//=============================================================================

static bool MatchPattern(const u8* data, const VmExitPattern* pattern)
{
    for (u32 i = 0; i < pattern->PatternLen; i++) {
        if (pattern->Mask[i] == 'x' && data[i] != pattern->Pattern[i])
            return false;
    }
    return true;
}

static u8* ScanForPattern(PVOID base, SIZE_T size, const VmExitPattern* pattern)
{
    u8* start = reinterpret_cast<u8*>(base);
    u8* end = start + size - pattern->PatternLen;

    for (u8* p = start; p < end; p++) {
        if (MatchPattern(p, pattern))
            return p;
    }

    return nullptr;
}

// Find CALL instruction after pattern match (Intel auto-discovery)
static u8* FindCallInstruction(u8* patternPos, const VmExitPattern* pattern)
{
    if (!pattern->AutoDiscoverCall) {
        // CALL is at pattern start (AMD)
        return patternPos;
    }

    // Scan for E8 CALL in range after pattern
    u8* scanStart = patternPos + pattern->ScanRangeStart;
    u8* scanEnd = patternPos + pattern->ScanRangeEnd;

    for (u8* p = scanStart; p < scanEnd; p++) {
        if (*p == 0xE8) {
            // Verify it looks like a real CALL (target should be reasonable)
            i32 relOffset = *reinterpret_cast<i32*>(p + 1);
            u64 target = reinterpret_cast<u64>(p + 5 + relOffset);

            // Target should be in kernel space
            if (target >= 0xFFFF800000000000ULL) {
                return p;
            }
        }
    }

    return nullptr;
}

//=============================================================================
// VMExit Hook Patching
//=============================================================================

// Calculate relative offset for CALL instruction
static i32 CalculateCallOffset(u8* callAddr, u64 targetAddr)
{
    u64 nextInst = reinterpret_cast<u64>(callAddr) + 5;  // CALL is 5 bytes
    i64 offset = static_cast<i64>(targetAddr) - static_cast<i64>(nextInst);

    // Check if offset fits in INT32
    if (offset > INT32_MAX || offset < INT32_MIN) {
        return 0;  // Need trampoline
    }

    return static_cast<i32>(offset);
}

// Patch the CALL instruction to redirect to our handler
static bool PatchVmExitCall(u8* callAddr, u64 handlerAddr)
{
    // Verify it's actually a CALL instruction
    if (*callAddr != 0xE8) {
        return false;
    }

    // Calculate new relative offset
    i32 newOffset = CalculateCallOffset(callAddr, handlerAddr);

    if (newOffset == 0) {
        // Target too far - would need trampoline
        // TODO: Implement trampoline allocation in kernel slack space
        return false;
    }

    // Disable write protection temporarily
    CR0 cr0;
    cr0.Flags = __readcr0();
    CR0 cr0New = cr0;
    cr0New.WriteProtect = 0;
    __writecr0(cr0New.Flags);

    // Patch the relative offset (bytes 1-4 of CALL instruction)
    *reinterpret_cast<i32*>(callAddr + 1) = newOffset;

    // Restore write protection
    __writecr0(cr0.Flags);

    // Flush instruction cache
    _mm_mfence();

    return true;
}

//=============================================================================
// Main Entry Point
//=============================================================================

} // namespace supdrv_entry

/**
 * OmbraModuleInit - Called by SUPDrv after loading
 *
 * @param pImage Base address of loaded image
 * @return 0 on success (VINF_SUCCESS), non-zero on failure
 *
 * This function runs in Ring 0 with full kernel privileges.
 * After it returns 0, the hypervisor is active and intercepting VMExits.
 */
extern "C" int __stdcall OmbraModuleInit(void* pImage)
{
    using namespace supdrv_entry;

    // 1. Detect CPU architecture
    bool isIntel = IsIntelCpu();
    bool isAmd = IsAmdCpu();

    if (!isIntel && !isAmd) {
        DBG_PRINT("[OMBRA] CPU detection failed - not Intel or AMD\n");
        return OMBRA_ERROR_CPU_DETECT;
    }

    // 2. Find Hyper-V module
    PVOID hvBase = nullptr;
    SIZE_T hvSize = 0;

    if (!FindHyperVModule(isIntel, &hvBase, &hvSize)) {
        DBG_PRINT("[OMBRA] Hyper-V module not found\n");
        return OMBRA_ERROR_HV_NOT_FOUND;
    }

    // 3. Scan for VMExit handler pattern
    const VmExitPattern* pattern = isIntel ? &g_IntelPattern : &g_AmdPattern;
    u8* patternPos = ScanForPattern(hvBase, hvSize, pattern);

    if (!patternPos) {
        DBG_PRINT("[OMBRA] VMExit pattern not found\n");
        return OMBRA_ERROR_PATTERN_NOT_FOUND;
    }

    // 4. Find the CALL instruction
    u8* callAddr = FindCallInstruction(patternPos, pattern);

    if (!callAddr) {
        DBG_PRINT("[OMBRA] CALL instruction not found\n");
        return OMBRA_ERROR_PATTERN_NOT_FOUND;
    }

    // 5. Get our handler address
    u64 handlerAddr;
    if (isIntel) {
        handlerAddr = reinterpret_cast<u64>(&intel::vmexit_handler);
    } else {
        handlerAddr = reinterpret_cast<u64>(&amd::vmexit_handler);
    }

    // 6. Patch the CALL instruction
    if (!PatchVmExitCall(callAddr, handlerAddr)) {
        DBG_PRINT("[OMBRA] VMExit CALL patch failed\n");
        return OMBRA_ERROR_PATCH_FAILED;
    }

    // 7. Initialize subsystems
    storage::Initialize();

    // Store our base address in storage for later reference
    storage::Set(PAYLOAD_BASE, reinterpret_cast<u64>(pImage));

    // Register architecture-specific callbacks for unified dispatch
    if (isIntel) {
        // Intel callbacks registered in intel::vmx_handler.cpp initialization
    } else {
        // AMD callbacks registered in amd::svm_handler.cpp initialization
    }

    DBG_PRINT("[OMBRA] Hypervisor initialized successfully\n");

    return OMBRA_SUCCESS;
}
