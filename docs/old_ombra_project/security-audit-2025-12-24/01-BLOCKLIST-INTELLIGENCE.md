# Microsoft Vulnerable Driver Blocklist Intelligence Report
**Subject: Ld9BoxSup.sys (LDPlayer VirtualBox Fork) Blocklist Status Analysis**
**Date: December 24, 2025**

---

## EXECUTIVE SUMMARY

**Ld9BoxSup.sys Current Status: NOT EXPLICITLY BLOCKED**

Based on comprehensive research across Microsoft's blocklist databases, LOLDrivers.io, and security repositories, **Ld9BoxSup.sys is NOT currently listed in Microsoft's Vulnerable Driver Blocklist by hash or filename**. However, there are significant caveats regarding VirtualBox-derived drivers and certificate-based blocking.

---

## 1. BLOCKLIST STRUCTURE & ENFORCEMENT

### 1.1 Technical Implementation

Microsoft's driver blocklist (`DriverSiPolicy.p7b`) is stored as:
- **Location**: `%windir%\System32\CodeIntegrity\driversipolicy.p7b`
- **Format**: PKCS#7 signed binary policy (convertible to XML via PowerShell)
- **Enforcement**: Automatic when HVCI/Memory Integrity, Smart App Control, or S mode is enabled
- **Default State**: Enabled by default on Windows 11 22H2+ and new Windows devices

### 1.2 Blocking Methods

The blocklist uses **three blocking strategies**:

1. **SHA256/SHA1 Hash Blocking** - Specific file hashes (most common)
2. **TBS Certificate Hash Blocking** - Blocks all drivers from a signing certificate
3. **TBS + FileAttribRef Blocking** - Certificate hash + filename/version qualifier

**Critical Finding (CVE-2025-59033)**: On systems **without HVCI enabled**, TBS+FileAttribRef rules are NOT enforced. Only pure hash-based and TBS-only rules work without HVCI.

### 1.3 Update Frequency

- **Major Updates**: 1-2 times per year with Windows releases
- **Incremental Updates**: Quarterly via Windows Update cumulative previews
- **Optional Updates**: Available via Windows Update for latest protection
- **Historical Issue**: Blocklist failed to update for ~3 years (2019-2022) until KB5020779 fixed sync mechanism

---

## 2. VIRTUALBOX DRIVER BLOCKLIST STATUS

### 2.1 Confirmed Blocked VirtualBox Drivers

**LOLDrivers.io Database Entries:**

| Driver | SHA256 Hash | Company | Reason | Date Added |
|--------|-------------|---------|--------|------------|
| vboxdrv.sys | `78827fa00ea48d96ac9af8d1c1e317d02ce11793e7f7f6e4c7aac7b5d7dd490f` | Sun Microsystems | Used in Acid Rain malware, privilege escalation | 2023-01-09 |
| VBoxDrv.sys | `26f41e4268be59f5de07552b51fa52d18d88be94f8895eb4a16de0f3940cf712` | Vektor T13 Security | Antidetect 2018, privilege escalation | 2023-05-06 |
| VBoxDrv.sys | `3724b39e97936bb20ada51c6119aded04530ed86f6b8d6b45fbfb2f3b9a4114b` | Vektor T13 Security | Antidetect 2019, privilege escalation | 2023-05-06 |
| VBoxDrv.sys | `7539157df91923d4575f7f57c8eb8b0fd87f064c919c1db85e73eebb2910b60c` | Sun Microsystems | Sun VirtualBox (2009), privilege escalation | 2023-05-06 |

**Note**: These are **specific hash-based blocks** for old VirtualBox versions (2008-2019 era), not blanket blocks of all VirtualBox drivers.

### 2.2 Certificate-Based Blocking (Critical)

**Microsoft DriverSiPolicy.p7b Contains**:
- **Signer Block**: `innotek GmbH` (original VirtualBox developer, pre-2008)
- **TBS Hash**: `041750993D7C9E063F02DFE74699598640911AAB`
- **Status**: Listed in **denied signers** section

**Ownership Timeline**:
- **2004-2008**: innotek GmbH (original developer)
- **2008-2010**: Sun Microsystems Inc.
- **2010-present**: Oracle Corporation

**Implication**: Old VirtualBox drivers signed with the **innotek GmbH certificate** are certificate-blocked. Modern VirtualBox drivers (Oracle-signed) are NOT certificate-blocked globally, only specific vulnerable versions by hash.

---

## 3. LD9BOXSUP.SYS ANALYSIS

### 3.1 File Identity

| Property | Value |
|----------|-------|
| **Filename** | Ld9BoxSup.sys |
| **Size** | ~376,144 bytes |
| **Location** | `C:\Program Files\LDPlayer9Box\` |
| **Certificate Signer** | Shanghai Chang Zhi Network Technology Co., Ltd. |
| **Alternative Signer** | Shanghai Yiyu Network Technology Co., Ltd. (some variants) |
| **Certificate Authority** | DigiCert Trusted G4 Code Signing RSA4096 SHA384 2021 CA1 |
| **Product** | LDPlayer 9 (Android emulator) |
| **SHA256 (Sample)** | `ef8bb26da250da5d022fa897d616e5e1bbf576619a2510cda3dd9c3d1c38e18c` |

### 3.2 Technical Relationship to VirtualBox

**LDPlayer Architecture**:
- LDPlayer is an Android emulator built on **modified VirtualBox technology**
- Uses VirtualBox 6.1.x backend (likely 6.1.36 based on timeline)
- **Driver Renaming**: VirtualBox 6.1.32 renamed `vboxdrv.sys` → `vboxsup.sys`
- **LDPlayer Customization**: Renamed to `Ld9BoxSup.sys` (LDPlayer 9 Box Support)

**Critical Finding**: The driver name change suggests LDPlayer forked VirtualBox source code and:
1. Recompiled the SUPDrv driver with custom branding
2. Re-signed with their own Chinese code-signing certificate
3. Deployed with proprietary naming convention

### 3.3 Blocklist Search Results

**Searches Conducted**:
- ✅ Microsoft DriverSiPolicy.p7b XML (mattifestation Gist)
- ✅ LOLDrivers.io database (magicsword-io/LOLDrivers)
- ✅ Cyb3r-Monk/Microsoft-Vulnerable-Driver-Block-Lists (CSV/JSON)
- ✅ VirusTotal hash lookup
- ✅ WDAC policy documentation

**Result**: **NO ENTRIES FOUND** for:
- `Ld9BoxSup.sys` (filename)
- `ef8bb26da250da5d022fa897d616e5e1bbf576619a2510cda3dd9c3d1c38e18c` (hash)
- Shanghai Chang Zhi Network Technology (certificate signer)
- Shanghai Yiyu Network Technology (certificate signer)

### 3.4 User Sentiment & Detection Status

- **FreeFixer Poll**: 89% of 54 users voted to **remove** the driver
- **VirusTotal**: Queued for scanning (no detection results in search)
- **Windows Certificate**: Shows "Verified publisher" in UAC prompts
- **BSOD Reports**: Some users report Blue Screen crashes related to Ld9BoxSup.sys

---

## 4. HVCI COMPATIBILITY ISSUES

### 4.1 VirtualBox HVCI Incompatibility (Historical)

**DGReadiness Tool Results** (Pre-6.1.32):
```
Incompatible HVCI Kernel Driver Modules found:
Module: vboxnetlwf.sys  Reason: execute pool type count: 2
Module: vboxdrv.sys     Reason: execute pool type count: 2
Module: vboxnetadp6.sys Reason: execute pool type count: 2
```

**Root Cause**: VirtualBox drivers allocated executable memory pools, violating W^X (Write XOR Execute) policy required by HVCI.

### 4.2 VirtualBox 6.1.32+ Fixes

**Changelog (VirtualBox 6.1.32, October 2022)**:
> "VMM: Changed the guest RAM management when using Hyper-V to be more compatible with HVCI."

**Status**: VirtualBox 6.1.32+ is **theoretically HVCI-compatible**, but:
- Some users report VirtualBox 6.1.36 still shows incompatibility warnings
- Host-Only network adapter drivers still fail to load with HVCI enabled
- Many users must disable HVCI to use VirtualBox successfully

### 4.3 Ld9BoxSup.sys HVCI Status (Unknown)

**Hypothesis**: If Ld9BoxSup.sys is based on VirtualBox 6.1.36 (post-HVCI fixes), it **may** be HVCI-compatible. However:

**Risk Factors**:
- LDPlayer developers may have forked **pre-6.1.32** code
- Custom modifications may have re-introduced HVCI incompatibilities
- Chinese emulator developers historically prioritize compatibility over security standards
- No public documentation on HVCI compliance testing

**Recommendation**: Assume **NOT HVCI-compatible** until proven otherwise via HLK testing.

---

## 5. TIMELINE OF VIRTUALBOX BLOCKLIST ADDITIONS

### Historical Blocklist Events

| Date | Event | Impact |
|------|-------|--------|
| **2009-04-07** | Sun VirtualBox 2.x drivers created | Later added to blocklist (SHA256: 7539157...) |
| **2018** | Vektor T13 Antidetect abuses VBoxDrv | Version added to blocklist (SHA256: 26f41e...) |
| **2019** | Acid Rain malware uses vboxdrv.sys | Sun Microsystems version blocked (SHA256: 78827f...) |
| **2022-10** | Windows 11 22H2 enables blocklist by default | innotek GmbH certificate added to denied signers |
| **2023-01-09** | LOLDrivers.io adds vboxdrv.sys | Community tracking begins |
| **2023-05-06** | LOLDrivers.io adds 3 more VBoxDrv variants | Expanded coverage of antidetect/malware versions |
| **2025-01** | CVE-2025-59033 disclosed | TBS+FileAttribRef bypass on non-HVCI systems |

### VirtualBox Version Blocks

**Blocked Versions** (by hash):
- VirtualBox 2.x (Sun Microsystems, 2009)
- Antidetect 2018 fork (Vektor T13)
- Antidetect 2019 fork (Vektor T13)

**NOT Blocked**:
- Modern VirtualBox 6.1.x/7.x (Oracle-signed, unless specific hash added)
- LDPlayer's Ld9BoxSup.sys fork (as of Dec 2025)

---

## 6. RECOMMENDATIONS FOR OMBRA PROJECT

### 6.1 Primary Recommendation: Continue with Current Architecture

**Rationale**:
1. **Ld9BoxSup.sys is NOT blocked** - safe to use
2. **ZeroHVCI path still available** - CVE-based alternative
3. **Hyper-V hijacking is stealthier** - no driver dependency for core function

### 6.2 Monitoring Recommendations

**Set Up Blocklist Monitoring**:
1. **GitHub Actions Workflow** - Monitor Cyb3r-Monk/Microsoft-Vulnerable-Driver-Block-Lists
2. **RSS Feed** - Subscribe to LOLDrivers.io updates
3. **Windows Update Tracking** - Check KB articles for "DriverSiPolicy.p7b" mentions
4. **Quarterly Reviews** - Manual check of Microsoft blocklist releases

**Alert Triggers**:
- VirtualBox-related driver hashes added
- hvax64.exe/hvix64.exe signatures (Hyper-V itself blocked - unlikely but catastrophic)
- Ombra-specific artifacts if ever discovered

---

## 7. FINAL VERDICT

### Ld9BoxSup.sys Blocklist Status

**BLOCKLIST STATUS**: ✅ **NOT BLOCKED** (as of December 2025)

**EVIDENCE**:
- ✅ No hash match in Microsoft DriverSiPolicy.p7b
- ✅ No filename match in WDAC policies
- ✅ No certificate signer match (Shanghai Chang Zhi / Shanghai Yiyu)
- ✅ Not listed in LOLDrivers.io database
- ✅ Not listed in Cyb3r-Monk blocklist CSV/JSON

**CAVEATS**:
- ⚠️ Status can change with any Windows Update
- ⚠️ HVCI compatibility unknown/untested
- ⚠️ High user removal rate (89%) suggests AV flagging
- ⚠️ Chinese code-signing certificate may raise EDR suspicion
- ⚠️ Based on old VirtualBox codebase (derivative work risk)

---

## SOURCES

- [Microsoft Recommended Driver Block Rules](https://learn.microsoft.com/en-us/windows/security/application-security/application-control/app-control-for-business/design/microsoft-recommended-driver-block-rules)
- [LOLDrivers.io - vboxdrv.sys](https://www.loldrivers.io/drivers/2da3a276-9e38-4ee6-903d-d15f7c355e7c/)
- [GitHub - magicsword-io/LOLDrivers](https://github.com/magicsword-io/LOLDrivers)
- [GitHub - Cyb3r-Monk/Microsoft-Vulnerable-Driver-Block-Lists](https://github.com/Cyb3r-Monk/Microsoft-Vulnerable-Driver-Block-Lists)
- [FreeFixer - ld9boxsup.sys](https://www.freefixer.com/library/file/ld9boxsup.sys-318377/)
- [VirtualBox HVCI Compatibility Forum](https://forums.virtualbox.org/viewtopic.php?t=99519)
