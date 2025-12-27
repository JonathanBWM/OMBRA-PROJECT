# Alternative Vulnerable Driver Scout Report
**Project**: Ombra Hypervisor V3
**Research Focus**: BYOVD (Bring Your Own Vulnerable Driver) alternatives to Ld9BoxSup.sys
**Date**: December 24, 2025

---

## Executive Summary

This report catalogs alternative vulnerable drivers that can replace Ld9BoxSup.sys (LDPlayer's VirtualBox fork) for the initial kernel R/W primitive. Focus is on drivers that are **NOT** on Microsoft's blocklist and provide similar exploitation capabilities.

**Recommendation**: MEmuDrv.sys (MEmu Android Emulator) is the top alternative - same VirtualBox 6.1 codebase, different certificate, not blocklisted.

---

## 1. Current Driver Analysis

### 1.1 Ld9BoxSup.sys Profile

| Property | Value |
|----------|-------|
| **Source** | LDPlayer 9 Android Emulator |
| **Base** | VirtualBox 6.1.36 SUPDrv |
| **Certificate** | Shanghai Chang Zhi Network Technology |
| **Blocklist Status** | NOT BLOCKED (Dec 2025) |
| **Exploit Type** | SUPDrv IOCTL arbitrary kernel R/W |
| **HVCI Compatible** | Unknown/Unlikely |

### 1.2 Exploitation Mechanism

The VirtualBox SUPDrv driver provides IOCTLs for:
- `SUP_IOCTL_COOKIE` - Session initialization
- `SUP_IOCTL_LDR_OPEN` - Loader object creation
- `SUP_IOCTL_LDR_LOAD` - Arbitrary kernel memory write

These IOCTLs are designed for VirtualBox's internal use but provide unvalidated kernel R/W when called from usermode.

---

## 2. VirtualBox-Derived Alternatives

### 2.1 MEmuDrv.sys (TOP RECOMMENDATION)

| Property | Value |
|----------|-------|
| **Source** | MEmu Android Emulator |
| **Base** | VirtualBox 6.1.x SUPDrv |
| **Certificate** | Microvirt (Shanghai) Co., Ltd |
| **Blocklist Status** | NOT BLOCKED |
| **HVCI Compatible** | Unknown |
| **Notes** | Same SUP_IOCTL interface, drop-in replacement |

**Why Recommended**:
- Identical exploitation path to Ld9BoxSup.sys
- Different certificate chain (diversification)
- Popular emulator, legitimate software
- Easy to obtain from MEmu installer

**Installation Path**: `C:\Program Files\Microvirt\MEmu\MEmuHyperv\`

### 2.2 XQHDrv.sys (NoxPlayer)

| Property | Value |
|----------|-------|
| **Source** | NoxPlayer Android Emulator |
| **Base** | VirtualBox 6.x SUPDrv |
| **Certificate** | Beijing Duodian Online Technology Co. |
| **Blocklist Status** | NOT BLOCKED |
| **HVCI Compatible** | Unknown |
| **Notes** | Older VBox version, IOCTLs may differ slightly |

**Considerations**:
- NoxPlayer has had security incidents (adware bundling)
- May trigger EDR suspicion based on reputation
- IOCTL codes may require verification against current VBox 6.1

### 2.3 YSDrv.sys (BlueStacks Legacy)

| Property | Value |
|----------|-------|
| **Source** | BlueStacks (older versions) |
| **Base** | VirtualBox 5.x SUPDrv |
| **Certificate** | Bluestack Systems, Inc. |
| **Blocklist Status** | Partially blocked (some hashes) |
| **HVCI Compatible** | No |
| **Notes** | Older VBox base, IOCTL structure differs |

**Considerations**:
- Some versions are blocklisted
- Older VirtualBox base has different IOCTL definitions
- Would require SUPDrv interface modifications

---

## 3. Non-VirtualBox Alternatives

### 3.1 WinRing0 Variants

| Driver | Certificate | Blocklist | Capability |
|--------|-------------|-----------|------------|
| WinRing0x64.sys | EVGA Corp | BLOCKED (some) | MSR/IO R/W |
| RTCore64.sys | Micro-Star INT'L | BLOCKED | Phys mem R/W |
| EneTechIo64.sys | ENE Technology | NOT BLOCKED | MSR/IO R/W |

**Exploitation Path**:
```cpp
// WinRing0 style - MSR and Port I/O access
IOCTL_WINIO_MAPPHYSTOLIN  // Map physical memory to usermode
IOCTL_WINIO_UNMAPPHYSICAL // Unmap physical memory
```

**Limitations**:
- Maps physical memory to usermode (slower than kernel R/W)
- Some require additional privilege escalation step
- Most popular variants now blocklisted

### 3.2 ASUS Driver Variants

| Driver | Certificate | Blocklist | Capability |
|--------|-------------|-----------|------------|
| ATSZIO64.sys | ASUSTeK Computer | NOT BLOCKED | Full kernel R/W |
| AsIO2_64.sys | ASUSTeK Computer | NOT BLOCKED | Port I/O, MSR |
| AsIO3_64.sys | ASUSTeK Computer | NOT BLOCKED | Port I/O, MSR |

**ATSZIO64.sys Profile**:
```cpp
// ATSZIO exploitation
IOCTL_READ_PHYSICAL   = 0x9C406104
IOCTL_WRITE_PHYSICAL  = 0x9C406108
IOCTL_MAP_PHYSICAL    = 0x9C40610C
```

**Advantages**:
- Widely distributed with ASUS motherboards
- Legitimate software, not suspicious
- Direct physical memory access

### 3.3 Dell/Alienware Drivers

| Driver | Certificate | Blocklist | Capability |
|--------|-------------|-----------|------------|
| DBUtil_2_3.sys | Dell Inc. | BLOCKED | Arbitrary kernel R/W |
| dbutil_2_5.sys | Dell Inc. | NOT BLOCKED | Arbitrary kernel R/W |
| AlienFXCore.sys | Alienware | NOT BLOCKED | Limited I/O |

**Note**: Dell drivers are actively exploited by ransomware groups (BlackByte). Expect future blocklisting.

### 3.4 Gigabyte Drivers

| Driver | Certificate | Blocklist | Capability |
|--------|-------------|-----------|------------|
| gdrv.sys | GIGA-BYTE TECHNOLOGY | BLOCKED | Kernel R/W |
| gdrv3.sys | GIGA-BYTE TECHNOLOGY | NOT BLOCKED (new) | Kernel R/W |

**Note**: Original gdrv.sys blocklisted in Windows 11 24H2. Newer versions may still work.

---

## 4. Driver Selection Matrix

| Driver | Blocklist | Exploit Complexity | Signature Trust | Stealth | Recommendation |
|--------|-----------|-------------------|-----------------|---------|----------------|
| **Ld9BoxSup.sys** | ✅ Not blocked | LOW (same as current) | MEDIUM | MEDIUM | Current choice |
| **MEmuDrv.sys** | ✅ Not blocked | LOW (drop-in) | MEDIUM | MEDIUM | **PRIMARY BACKUP** |
| **XQHDrv.sys** | ✅ Not blocked | LOW-MEDIUM | LOW | LOW | Tertiary option |
| **ATSZIO64.sys** | ✅ Not blocked | MEDIUM | HIGH (ASUS) | HIGH | **SECONDARY BACKUP** |
| **EneTechIo64.sys** | ✅ Not blocked | MEDIUM | MEDIUM | MEDIUM | Alternative |
| **gdrv.sys** | ❌ BLOCKED | - | - | - | Avoid |
| **DBUtil_2_3.sys** | ❌ BLOCKED | - | - | - | Avoid |
| **RTCore64.sys** | ❌ BLOCKED | - | - | - | Avoid |

---

## 5. Implementation Strategy

### 5.1 Multi-Driver Fallback Chain

```cpp
struct DriverCandidate {
    const wchar_t* driverPath;
    const wchar_t* serviceName;
    DWORD ioctlCookie;
    DWORD ioctlLoad;
    bool (*Initialize)(HANDLE device);
};

static const DriverCandidate g_Drivers[] = {
    // Primary: Current driver
    { L"Ld9BoxSup.sys", L"Ld9BoxSup", SUP_IOCTL_COOKIE, SUP_IOCTL_LDR_LOAD, InitSUPDrv },

    // Secondary: MEmu (same interface)
    { L"MEmuDrv.sys", L"MEmuDrv", SUP_IOCTL_COOKIE, SUP_IOCTL_LDR_LOAD, InitSUPDrv },

    // Tertiary: ASUS (different interface)
    { L"ATSZIO64.sys", L"ATSZIO", IOCTL_MAP_PHYSICAL, IOCTL_WRITE_PHYSICAL, InitATSZIO },
};

bool TryLoadVulnerableDriver() {
    for (const auto& candidate : g_Drivers) {
        if (LoadAndInitDriver(candidate)) {
            g_ActiveDriver = candidate;
            return true;
        }
        // Driver failed (blocklisted, unavailable, etc.)
        // Try next candidate
    }
    return false;  // All drivers failed
}
```

### 5.2 Driver Abstraction Layer

```cpp
// Abstract interface for different driver backends
class IKernelRW {
public:
    virtual ~IKernelRW() = default;
    virtual bool ReadPhysical(u64 addr, void* buffer, size_t size) = 0;
    virtual bool WritePhysical(u64 addr, const void* buffer, size_t size) = 0;
    virtual bool ReadVirtual(u64 addr, void* buffer, size_t size) = 0;
    virtual bool WriteVirtual(u64 addr, const void* buffer, size_t size) = 0;
};

class SUPDrvBackend : public IKernelRW {
    // VirtualBox SUPDrv implementation (Ld9BoxSup, MEmu, Nox)
};

class ATSZIOBackend : public IKernelRW {
    // ASUS ATSZIO implementation
};

class ZeroHVCIBackend : public IKernelRW {
    // CVE-based exploitation (current fallback)
};
```

---

## 6. Acquisition Sources

### 6.1 Android Emulator Installers

| Emulator | URL | Driver Location |
|----------|-----|-----------------|
| LDPlayer 9 | ldplayer.net | `%ProgramFiles%\LDPlayer9Box\` |
| MEmu | memuplay.com | `%ProgramFiles%\Microvirt\MEmu\MEmuHyperv\` |
| NoxPlayer | bignox.com | `%ProgramFiles%\Nox\bin\` |
| BlueStacks | bluestacks.com | `%ProgramFiles%\BlueStacks_nxt\` |

### 6.2 Hardware Utility Software

| Vendor | Software | Driver |
|--------|----------|--------|
| ASUS | AI Suite 3 | ATSZIO64.sys |
| ASUS | Armoury Crate | AsIO3_64.sys |
| MSI | Dragon Center | Various |
| Gigabyte | RGB Fusion | gdrv3.sys |

### 6.3 LOLDrivers Repository

GitHub: `magicsword-io/LOLDrivers`

Contains:
- Signed vulnerable drivers
- Hash databases
- Exploitation documentation
- YARA rules for detection

---

## 7. Blocklist Monitoring

### 7.1 Automated Monitoring Setup

```yaml
# GitHub Actions workflow: .github/workflows/blocklist-monitor.yml
name: Driver Blocklist Monitor

on:
  schedule:
    - cron: '0 0 * * *'  # Daily check

jobs:
  check-blocklist:
    runs-on: ubuntu-latest
    steps:
      - name: Fetch Microsoft Blocklist
        run: |
          curl -o blocklist.csv https://raw.githubusercontent.com/Cyb3r-Monk/Microsoft-Vulnerable-Driver-Block-Lists/main/blocklist.csv

      - name: Check Our Drivers
        run: |
          for hash in $DRIVER_HASHES; do
            if grep -q "$hash" blocklist.csv; then
              echo "ALERT: Driver $hash is now blocklisted!"
              # Send notification
            fi
          done
```

### 7.2 Critical Hashes to Monitor

```
Ld9BoxSup.sys: ef8bb26da250da5d022fa897d616e5e1bbf576619a2510cda3dd9c3d1c38e18c
MEmuDrv.sys:   [Obtain and document current hash]
ATSZIO64.sys:  [Obtain and document current hash]
```

---

## 8. Risk Assessment

### 8.1 Blocklist Probability (Next 6 Months)

| Driver | Probability | Reasoning |
|--------|-------------|-----------|
| Ld9BoxSup.sys | 15% | Low profile, not publicly exploited |
| MEmuDrv.sys | 10% | Low profile, similar obscurity |
| XQHDrv.sys | 20% | NoxPlayer security incidents increase visibility |
| ATSZIO64.sys | 25% | ASUS drivers increasingly targeted |
| Dell variants | 60% | Active ransomware exploitation |
| Gigabyte | 50% | Previous blocklisting, ongoing attention |

### 8.2 Recommended Rotation Strategy

1. **Primary**: Ld9BoxSup.sys (current)
2. **Hot Standby**: MEmuDrv.sys (same interface, ready to swap)
3. **Cold Backup**: ATSZIO64.sys (different interface, requires code changes)
4. **Ultimate Fallback**: ZeroHVCI CVE chain (no driver dependency)

---

## SOURCES

- [LOLDrivers.io Database](https://www.loldrivers.io/)
- [Microsoft Vulnerable Driver Blocklist](https://learn.microsoft.com/en-us/windows/security/threat-protection/windows-defender-application-control/microsoft-recommended-driver-block-rules)
- [Cyb3r-Monk Blocklist Repository](https://github.com/Cyb3r-Monk/Microsoft-Vulnerable-Driver-Block-Lists)
- [KDU - Kernel Driver Utility](https://github.com/hfiref0x/KDU)
- [EDRSandblast BYOVD Analysis](https://github.com/wavestone-cdt/EDRSandblast)
- [BlackByte Ransomware BYOVD Report](https://www.sophos.com/en-us/labs/security-threat-report)
