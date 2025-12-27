# ETW & Event Log Evasion Research Report
**Project**: Ombra Hypervisor V3
**Research Focus**: ETW blinding and event log evasion during driver loading
**Date**: December 24, 2025

---

## Executive Summary

This report documents ETW (Event Tracing for Windows) evasion techniques for eliminating telemetry during the Ombra Hypervisor driver loading phase. Key finding: user-mode ETW patching is obsolete in 2025 - modern EDRs use kernel-mode ETW consumers that require Ring 0/-1 access to blind.

**Critical Discovery**: Windows PatchGuard does NOT protect ETW structures. Only VBS/KDP does, and Ombra operates at Ring -1 (below VBS security boundary).

---

## 1. Detection Surfaces

### Event Log Events

| Event ID | Log | Description | Triggered By |
|----------|-----|-------------|--------------|
| 7045 | System | Service installed | CreateService() |
| 7036 | System | Service started/stopped | StartService() |
| 6 | Sysmon | Driver loaded | NtLoadDriver() |
| 1102 | Security | Audit log cleared | WevtClearLog() |

### ETW Providers

| Provider | GUID | Coverage |
|----------|------|----------|
| Microsoft-Windows-Kernel-File | {EDD08927-9CC4-4E65-B970-C2560FB5C289} | File operations |
| Microsoft-Windows-Kernel-Registry | {70EB4F03-C1DE-4F73-A051-33D13D5413BD} | Registry operations |
| Microsoft-Windows-Kernel-Process | {22FB2CD6-0E7B-422B-A0C7-2FAD1FD0E716} | Process/thread events |
| Microsoft-Windows-Threat-Intelligence | {F4E1897C-BB5D-5668-F1D8-040F4D8DD344} | ETW-TI for EDRs |

---

## 2. ETW Blinding Technique

### 2.1 Target Structure

**Attack Path**: `nt!EtwThreatIntProvRegHandle` → `ETW_REG_ENTRY` → `ETW_GUID_ENTRY` → `ProviderEnableInfo`

**Structure Navigation**:
```
ntoskrnl.exe + offset → ETW_REG_ENTRY*
ETW_REG_ENTRY + 0x20 → ETW_GUID_ENTRY*
ETW_GUID_ENTRY + 0x60 → ProviderEnableInfo (BYTE)
Write 0x0 = disabled, 0x1 = enabled
```

### 2.2 Windows Build Offsets

| Windows Version | Build | EtwThreatIntProvRegHandle Offset |
|-----------------|-------|----------------------------------|
| Windows 10 22H2 | 19045 | 0xC1D6F0 |
| Windows 11 22H2 | 22621 | 0xC2E8A0 |
| Windows 11 23H2 | 22631 | 0xC2F1C0 |
| Windows 11 24H2 | 26100 | Requires PDB lookup |

### 2.3 Implementation via VMCALL

```cpp
// New VMCALLs needed
enum VMCALL_TYPE {
    // ... existing ...
    VMCALL_DISABLE_ETW_TI = 0x1020,
    VMCALL_ENABLE_ETW_TI  = 0x1021,
};

// In dispatch.cpp
case VMCALL_DISABLE_ETW_TI: {
    // Read ETW_REG_ENTRY pointer
    u64 etwRegHandle = mm::read_guest_phys(ntoskrnl_base + g_etwOffset);
    // Navigate to ETW_GUID_ENTRY
    u64 guidEntry = mm::read_guest_phys(etwRegHandle + 0x20);
    // Write 0x0 to ProviderEnableInfo
    u8 disabled = 0;
    mm::write_guest_phys(guidEntry + 0x60, &disabled, 1);
    break;
}

case VMCALL_ENABLE_ETW_TI: {
    // Same path, write 0x1 to restore
    u8 enabled = 1;
    mm::write_guest_phys(guidEntry + 0x60, &enabled, 1);
    break;
}
```

---

## 3. Attack Flow

```
OmbraLoader.exe
    ↓
ZeroHVCI: Kernel R/W via CVE-2024-26229
    ↓
RuntimeHijacker: Patch Hyper-V VMExit handler
    ↓
Hypervisor Active (Ring -1)
    ↓
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃   STEALTH WINDOW (~500ms)     ┃
┃                                ┃
┃  1. Resolve ETW provider addr ┃
┃  2. VMCALL: DISABLE_ETW_TI    ┃
┃  3. map_driver (OmbraDriver)  ┃
┃  4. VMCALL: ENABLE_ETW_TI     ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
    ↓
Driver hidden via EPT shadow paging
    ↓
No events logged, driver invisible
```

---

## 4. Why Manual Mapping Bypasses Events

Ombra already uses manual driver mapping, which inherently bypasses:

| Event | Bypassed? | Reason |
|-------|-----------|--------|
| Event 7045 | ✅ Yes | No CreateService() call |
| Event 7036 | ✅ Yes | No StartService() call |
| Sysmon Event 6 | ✅ Yes | No NtLoadDriver() syscall |
| ETW-TI | ⚠️ Partial | Still logs kernel allocations |

**The ETW blinding is for eliminating the remaining ETW-TI telemetry** during the brief window when driver mapping occurs.

---

## 5. Event Log Clearing (NOT RECOMMENDED)

**Why Clearing is Counterproductive**:
- Event 1102 (audit log cleared) is a high-severity forensic indicator
- Record number gaps detectable by forensic tools
- Triggers immediate SOC/SIEM alerts
- More suspicious than the events you're trying to hide

**Recommendation**: Don't clear logs. Rely on:
1. Manual mapping (no service events)
2. ETW-TI blinding (no kernel telemetry)
3. EPT shadow paging (driver invisible to enumeration)

---

## 6. Sysmon Event 6 Bypass

Sysmon Event 6 (Driver Loaded) is triggered by:
- `NtLoadDriver()` syscall
- Minifilter attachment

**Bypass Methods**:

1. **Manual Mapping** (already implemented)
   - Doesn't call NtLoadDriver
   - Sysmon never sees the driver load

2. **IoCreateDriver** (alternative)
   - Creates driver object without NtLoadDriver
   - Also bypasses Sysmon Event 6

3. **Disable Sysmon Driver** (aggressive)
   - Locate SysmonDrv.sys in memory
   - Patch callback registration
   - **Not recommended**: Too invasive, easily detected

---

## 7. Tools Analyzed

### EDRSandblast
- Reference implementation for ETW-TI disabling
- Uses similar structure navigation
- Kernel-mode execution required

### TamperETW
- User-mode CLR ETW patching
- **Not applicable** - only affects current process

### InfinityHook
- ETW circular logger hijacking
- Complex, requires persistent hook

### MemoryRanger
- Defensive hypervisor implementation
- Shows what detection capabilities exist at Ring -1

---

## 8. Implementation Scope

### Files to Modify

| File | Changes |
|------|---------|
| `OmbraShared/communication.hpp` | Add VMCALL_DISABLE_ETW_TI, VMCALL_ENABLE_ETW_TI |
| `PayLoad/core/dispatch.cpp` | Add ETW handler cases |
| `libombra/libombra.hpp` | Add disable_etw_ti(), enable_etw_ti() |
| `libombra/libombra.cpp` | Implement wrapper functions |
| `OmbraLoader/main.cpp` | Integrate around map_driver() call |

### New Files

| File | Purpose |
|------|---------|
| `OmbraLoader/etw_resolver.h` | Offset resolution by Windows build |
| `OmbraLoader/etw_resolver.cpp` | Address calculation implementation |

### Estimated Effort
- Development: 8-12 hours
- Testing: 4-6 hours
- Total: 12-18 hours

---

## 9. Security Assessment

| Factor | Assessment |
|--------|------------|
| **Detection Risk** | LOW - PatchGuard doesn't protect ETW structures |
| **VBS Bypass** | ✅ Ring -1 access bypasses VBS/KDP |
| **Timing Window** | <1 second - unlikely to trigger alerts |
| **Forensic Evidence** | None if restoration occurs promptly |
| **Complexity** | MEDIUM - clear integration points |

**Recommendation**: **PROCEED WITH IMPLEMENTATION**

---

## SOURCES

- [IBM X-Force: DKOM Attacks on ETW Providers](https://securityintelligence.com/x-force/direct-kernel-object-manipulation-dkom-attacks-etw-providers/)
- [White Knight Labs: Bypassing ETW](https://whiteknightlabs.com/2021/12/11/bypassing-etw-for-fun-and-profit/)
- [Binarly: Design Issues of Modern EDRs](https://binarly.io/posts/Design_issues_of_modern_EDRs_bypassing_ETW_based_solutions/)
- [EDRSandblast GitHub](https://github.com/wavestone-cdt/EDRSandblast)
- [FluxSec: Full Spectrum ETW Detection](https://fluxsec.red/full-spectrum-etw-detection)
