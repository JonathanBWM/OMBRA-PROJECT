# A10: Detection Knowledge Base - Anti-Cheat and Security Evasion

## Overview

This document catalogs all known hypervisor detection techniques used by anti-cheat software, security products, and malware sandboxes. Each technique includes the detection method, risk level, bypass strategy, and implementation location within OmbraHypervisor.

**Primary Reference Sources:**
- `/Refs/al-khaser/` - Comprehensive VM/sandbox detection test suite
- `/Refs/HyperHide/` - Kernel-mode anti-detection driver
- `/Refs/Sputnik/` - EasyAntiCheat bypass implementation
- `/Refs/VBoxHardenedLoader/` and `/Refs/VmwareHardenedLoader/` - VM hardening tools

---

## 1. CPUID-Based Detection

### 1.1 Hypervisor Present Bit (CPUID.1:ECX[31])

**Detection Method:**
```cpp
// al-khaser/AntiVM/Generic.cpp:987-997
BOOL cpuid_is_hypervisor()
{
    INT CPUInfo[4] = { -1 };
    __cpuid(CPUInfo, 1);
    if ((CPUInfo[2] >> 31) & 1)  // Bit 31 of ECX
        return TRUE;
    return FALSE;
}
```

**Risk Level:** CRITICAL

**What It Detects:** The hypervisor present bit is set by all compliant hypervisors to indicate virtualization is active. This is the most fundamental hypervisor detection check.

**Bypass Method:**
- Intercept CPUID VM exits for leaf 0x1
- Clear bit 31 of ECX before returning to guest
- HyperHide implements via `VMCALL_HIDE_HV_PRESENCE`

**Implementation Location:** `PayLoad/Core/VmExitHandler.cpp` - CPUID exit handler

**Code Reference (HyperHide):**
```cpp
// HyperHide/HyperHideDrv/HypervisorGateway.cpp:104-111
void hypervisor_visible(bool value)
{
    if (value == true)
        __vm_call(VMCALL_UNHIDE_HV_PRESENCE, 0, 0, 0);
    else
        __vm_call(VMCALL_HIDE_HV_PRESENCE, 0, 0, 0);
}
```

### 1.2 Hypervisor Vendor String (CPUID.40000000h)

**Detection Method:**
```cpp
// al-khaser/AntiVM/Generic.cpp:1004-1047
BOOL cpuid_hypervisor_vendor()
{
    INT CPUInfo[4] = { -1 };
    CHAR szHypervisorVendor[0x40];

    const TCHAR *szBlacklistedHypervisors[] = {
        _T("KVMKVMKVM\0\0\0"),    // KVM
        _T("Microsoft Hv"),       // Microsoft Hyper-V
        _T("VMwareVMware"),       // VMware
        _T("XenVMMXenVMM"),       // Xen
        _T("prl hyperv  "),       // Parallels
        _T("VBoxVBoxVBox"),       // VirtualBox
    };

    __cpuid(CPUInfo, 0x40000000);
    memcpy(szHypervisorVendor, CPUInfo + 1, 12);
    // Compare against blacklist...
}
```

**Risk Level:** CRITICAL

**What It Detects:** Hypervisor identification string. Anti-cheat specifically looks for known hypervisor signatures.

**Bypass Method:**
- For Hyper-V hijacking: Return original "Microsoft Hv" (we ARE Hyper-V)
- Option: Return empty/fake vendor string
- Do NOT return known VM vendor strings if trying to hide

**Implementation Location:** `PayLoad/Core/VmExitHandler.cpp` - CPUID leaf 0x40000000 handler

### 1.3 CPU Feature Flags Anomalies

**Detection Method:**
Anti-cheat may check for CPU feature inconsistencies that indicate virtualization:
- VMX/SVM capability bits present but hypervisor not expected
- Feature bits that differ from physical CPU

**Risk Level:** MEDIUM

**Bypass Method:**
- Passthrough most feature bits from host CPU
- Be careful with VMX/SVM related bits
- Match physical CPU capabilities exactly

---

## 2. Timing-Based Detection

### 2.1 RDTSC Timing Attack (VM Exit Overhead)

**Detection Method:**
```cpp
// al-khaser/TimingAttacks/timing.cpp:218-239
BOOL rdtsc_diff_vmexit()
{
    ULONGLONG tsc1 = 0;
    ULONGLONG tsc2 = 0;
    ULONGLONG avg = 0;
    INT cpuInfo[4] = {};

    for (INT i = 0; i < 10; i++)
    {
        tsc1 = __rdtsc();
        __cpuid(cpuInfo, 0);  // Forces VM exit
        tsc2 = __rdtsc();
        avg += (tsc2 - tsc1);
    }

    avg = avg / 10;
    return (avg < 1000 && avg > 0) ? FALSE : TRUE;
    // If avg >= 1000 cycles, VM detected
}
```

**Risk Level:** HIGH

**What It Detects:** VM exits add significant cycle overhead. On bare metal, CPUID takes ~200-500 cycles. Under virtualization, it can take 1000-5000+ cycles.

**Bypass Method (HyperHide - KUSER_SHARED_DATA manipulation):**
```cpp
// HyperHide/HyperHideDrv/KuserSharedData.cpp:79-124
VOID CounterUpdater(PVOID Context)
{
    while (Hider::StopCounterThread == FALSE)
    {
        KeDelayExecutionThread(KernelMode, FALSE, &TimeToWait);

        // Update fake KUSER_SHARED_DATA values
        *(ULONG64*)&HiddenProcess->Kusd.KuserSharedData->InterruptTime =
            *(ULONG64*)&KernelKuserSharedData->InterruptTime.LowPart -
            HiddenProcess->Kusd.DeltaInterruptTime;

        // Also update: SystemTime, TickCount, etc.
    }
}
```

**Implementation Strategy:**
1. TSC Offsetting: Use VMCS TSC offset field to adjust returned values
2. KUSER_SHARED_DATA Shadowing: Provide fake timing data to usermode
3. TSC Scaling: Use TSC scaling features on newer CPUs

**Implementation Location:**
- `PayLoad/Intel/IntelVmcs.cpp` - TSC offset configuration
- `SKLib/Src/Detection/TimingSpoofing.cpp` - KUSER_SHARED_DATA manipulation

### 2.2 Locky-Style RDTSC Ratio Check

**Detection Method:**
```cpp
// al-khaser/TimingAttacks/timing.cpp:180-210
BOOL rdtsc_diff_locky()
{
    ULONGLONG tsc1, tsc2, tsc3;

    for (DWORD i = 0; i < 10; i++)
    {
        tsc1 = __rdtsc();
        GetProcessHeap();  // Fast syscall
        tsc2 = __rdtsc();
        CloseHandle(0);    // Slower syscall
        tsc3 = __rdtsc();

        // Ratio check - should be ~10x difference on real hardware
        if ((tsc3 - tsc2) / (tsc2 - tsc1) >= 10)
            return FALSE;  // Looks real
    }
    return TRUE;  // VM detected - ratio too consistent
}
```

**Risk Level:** MEDIUM

**Bypass Method:** TSC offsetting must maintain realistic timing ratios, not just absolute values.

---

## 3. MSR-Based Detection

### 3.1 VMX MSR Detection

**Detection Method:**
Reading VMX-related MSRs can reveal hypervisor presence:
- `IA32_VMX_BASIC` (0x480)
- `IA32_FEATURE_CONTROL` (0x3A)
- `IA32_VMX_PINBASED_CTLS` (0x481)
- etc.

**Risk Level:** HIGH

**Bypass Method:**
- Intercept RDMSR VM exits
- Return host values or zeros for VMX MSRs
- Be consistent - if CPUID says no VMX, MSRs should agree

**Implementation Location:** `PayLoad/Core/VmExitHandler.cpp` - RDMSR handler

### 3.2 Hyper-V Synthetic MSRs

**Detection Method:**
Hyper-V exposes synthetic MSRs that anti-cheat can check:
- `HV_X64_MSR_GUEST_OS_ID` (0x40000000)
- `HV_X64_MSR_HYPERCALL` (0x40000001)
- `HV_X64_MSR_VP_INDEX` (0x40000002)

**Risk Level:** MEDIUM (for Hyper-V hijacking, these are expected)

**Note:** Since OmbraHypervisor hijacks Hyper-V, these MSRs should be passed through normally. The goal is to appear as standard Hyper-V, not to hide Hyper-V entirely.

---

## 4. Memory-Based Detection

### 4.1 IDT/GDT/LDT Base Address Anomalies

**Detection Method:**
```cpp
// al-khaser/AntiVM/Generic.cpp:396-431
BOOL idt_trick()
{
    UINT idt_base = get_idt_base();
    if ((idt_base >> 24) == 0xff)  // High address = VM
        return TRUE;
    return FALSE;
}

BOOL gdt_trick()
{
    UINT gdt_base = get_gdt_base();
    if ((gdt_base >> 24) == 0xff)
        return TRUE;  // VMware detected
    return FALSE;
}
```

**Risk Level:** LOW (unreliable on modern systems)

**What It Detects:** Early VMs placed IDT/GDT at high addresses. Modern VMs and Windows 10+ make this unreliable.

**Bypass Method:** Generally not needed for modern implementations.

### 4.2 STR (Store Task Register) Trick

**Detection Method:**
```cpp
// al-khaser/AntiVM/Generic.cpp:447-459
BOOL str_trick()
{
    UCHAR mem[4] = { 0, 0, 0, 0 };
    __asm str mem;
    if ((mem[0] == 0x00) && (mem[1] == 0x40))
        return TRUE;  // VMware detected
    return FALSE;
}
```

**Risk Level:** LOW (32-bit only, legacy)

---

## 5. EasyAntiCheat (EAC) Specific Detection and Bypass

### 5.1 EAC CR3 Tracking

**Detection Method:**
EAC monitors CR3 (page table base register) changes to detect memory manipulation and unauthorized page table modifications.

**Bypass Implementation (Sputnik):**
```cpp
// Sputnik/SKLib/SKLib/SKLib-v/src/eac.cpp:32-47
void eac::UpdateCr3(CR3 cr3)
{
    if (!bEacInitialized)
        return;

    identity::PhysicalAccess pa(vmm::pIdentityMap, cr3.Flags);
    for (auto& data : *pCr3s) {
        if (data.pCr3 && pa.getPhysicalAddress((DWORD64)data.pImageBase + PAGE_SIZE)) {
            identity::PhysicalAccess paSrc(vmm::pIdentityMap, data.srcCr3);
            paSrc.Write<CR3>(data.pCr3, cr3);
        }
    }
}
```

**Risk Level:** CRITICAL for EAC-protected games

**Implementation Location:** `SKLib/Src/Detection/EacBypass.cpp`

### 5.2 EAC NMI (Non-Maskable Interrupt) Detection

**Detection Method:**
EAC uses NMIs to catch cheats that disable interrupts or are in critical sections.

**Bypass Implementation (Sputnik):**
```cpp
// Sputnik/SKLib/SKLib/SKLib-v/src/eac.cpp:75-127
void eac::BlockNmi(CR3 cr3)
{
    // Block NMI delivery to specific processes
    CR3_TRACKING tracking;
    tracking.srcCr3 = cr3.Flags;
    pNmiBlockedCr3s->Append(tracking);
}

bool eac::IsNmiBlocked(CR3 cr3)
{
    for (auto& tracked : *pNmiBlockedCr3s) {
        if (tracked.srcCr3 == cr3.Flags)
            return true;
    }
    return false;
}
```

**Risk Level:** HIGH

**Implementation Location:** `SKLib/Src/Detection/NmiHandler.cpp`

### 5.3 EAC Kernel Callback Detection

EAC registers kernel callbacks for:
- Process creation/termination
- Thread creation
- Image load
- Registry access

**Bypass Strategy:**
- Use EPT/NPT hooks instead of kernel callbacks
- Avoid triggering callback-monitored events
- Hook at hypervisor level (below EAC visibility)

---

## 6. Process Blocking and Scoring System

### 6.1 Blocked Process List (Sputnik Implementation)

**Reference:**
```cpp
// Sputnik/CheatDriver/src/comms.cpp:863-889
vBlockedProcesses->emplace_back("x64dbg", 50);
vBlockedProcesses->emplace_back("windbg", 50);
vBlockedProcesses->emplace_back("ghidra", 50);
vBlockedProcesses->emplace_back("ida64", 50);
vBlockedProcesses->emplace_back("wireshark", 50);
vBlockedProcesses->emplace_back("procmon", 50);
vBlockedProcesses->emplace_back("apimonitor", 50);
vBlockedProcesses->emplace_back("ollydbg", 50);
vBlockedProcesses->emplace_back("fiddler", 50);
vBlockedProcesses->emplace_back("scylla", 50);
vBlockedProcesses->emplace_back("processhacker", 30);
vBlockedProcesses->emplace_back("systeminformer", 30);
vBlockedProcesses->emplace_back("pe-bear", 30);
vBlockedProcesses->emplace_back("dbgview", 30);
```

### 6.2 Threat Scoring System

**Implementation (Sputnik):**
```cpp
// Sputnik/CheatDriver/src/comms.cpp:84-96
DWORD64 currScore = 0;
DWORD64 maxScore = 0;
DWORD64 warnScore = 0;

// Score accumulation and threshold checks
// comms.cpp:257-266
currScore += blocked.score;

if (!blocked.bRan && maxScore) {
    if (currScore >= warnScore)
        TriggerWarning();
    if (currScore >= maxScore)
        TriggerDetection();
    blocked.bRan = true;
}
```

### 6.3 Process Monitoring Hooks

**Implementation (Sputnik):**
```cpp
// Sputnik/CheatDriver/src/comms.cpp:917-943
// Hook PspInsertThread for thread creation monitoring
EPT::Hook((PVOID)winternl::PspInsertThread, PspInsertThread, ...);

// Hook PspInsertProcess for process creation monitoring
EPT::Hook((PVOID)winternl::PspInsertProcess, PspInsertProcess, ...);

// Hook PspRundownSingleProcess for process termination
EPT::Hook((PVOID)winternl::PspRundownSingleProcess, PspRundownSingleProcess, ...);
```

---

## 7. Hardware-Based Detection

### 7.1 MAC Address Detection

**Detection Method:**
```cpp
// al-khaser/AntiVM/VirtualBox.cpp:144-148
BOOL vbox_check_mac()
{
    return check_mac_addr(_T("\x08\x00\x27"));  // VirtualBox OUI
}

// al-khaser/AntiVM/VMWare.cpp:137-142
const TCHAR *szMac[][2] = {
    { _T("\x00\x05\x69"), _T("00:05:69") },  // VMware
    { _T("\x00\x0C\x29"), _T("00:0c:29") },
    { _T("\x00\x1C\x14"), _T("00:1C:14") },
    { _T("\x00\x50\x56"), _T("00:50:56") },
};
```

**Risk Level:** LOW (not applicable for Type-1 hijacking)

**Note:** OmbraHypervisor hijacks existing Hyper-V, so physical hardware MAC addresses are unchanged.

### 7.2 SMBIOS/DMI String Detection

**Detection Method:**
```cpp
// al-khaser/AntiVM/VirtualBox.cpp:410-436
BOOL vbox_firmware_SMBIOS()
{
    PBYTE smbios = get_system_firmware(static_cast<DWORD>('RSMB'), 0x0000, &smbiosSize);
    // Search for "VirtualBox", "vbox", "VBOX" in SMBIOS tables
}
```

**Risk Level:** LOW (physical hardware maintains real SMBIOS)

### 7.3 ACPI Table Detection

**Detection Method:**
```cpp
// al-khaser/AntiVM/VirtualBox.cpp:439-491
BOOL vbox_firmware_ACPI()
{
    // Enumerate ACPI tables looking for VM signatures
    // Check for VBOX, VMWARE strings in ACPI data
}
```

**Risk Level:** LOW (physical ACPI tables unchanged)

---

## 8. Hyper-V Specific Detection

### 8.1 Hyper-V Driver Objects

**Detection Method:**
```cpp
// al-khaser/AntiVM/HyperV.cpp:5-40
BOOL check_hyperv_driver_objects()
{
    const wchar_t* drivers[] = {
        L"VMBusHID",
        L"vmbus",
        L"vmgid",
        L"IndirectKmd",
        L"HyperVideo",
        L"hyperkbd"
    };
    // Enumerate \Driver\ object directory
}
```

**Risk Level:** EXPECTED for Hyper-V environments

**Note:** Since OmbraHypervisor operates within Hyper-V, these drivers being present is normal and expected behavior.

### 8.2 Hyper-V Global Objects

**Detection Method:**
```cpp
// al-khaser/AntiVM/HyperV.cpp:43-70
BOOL check_hyperv_global_objects()
{
    // Check \GLOBAL?? for:
    // - VMBUS#*
    // - VDRVROOT
    // - VmGenerationCounter
    // - VmGid
}
```

**Risk Level:** EXPECTED

---

## 9. Anti-Debug Detection (Relevant for Hypervisor)

### 9.1 Debug Register Detection

**Detection Method:**
Anti-cheat checks hardware debug registers (DR0-DR7) for breakpoints.

**Bypass Method:**
- Intercept MOV DR VM exits
- Return clean/expected values
- Shadow debug registers per-process if needed

**Implementation Location:** `PayLoad/Core/VmExitHandler.cpp` - MOV DR handler

### 9.2 NtQueryInformationProcess Detection

**Detection Method:**
```cpp
// Checks ProcessDebugPort, ProcessDebugFlags, ProcessDebugObjectHandle
NtQueryInformationProcess(handle, ProcessDebugPort, ...);
```

**Bypass Method (HyperHide approach):**
- Hook NtQueryInformationProcess via EPT
- Return clean values for debug-related information classes

**Reference:** `HyperHide/HyperHideDrv/HookedFunctions.cpp`

---

## 10. WMI-Based Detection

### 10.1 Win32_ComputerSystem Checks

**Detection Method:**
```cpp
// al-khaser/AntiVM/Generic.cpp:1118-1177
BOOL model_computer_system_wmi()
{
    // WMI Query: SELECT * FROM Win32_ComputerSystem
    // Check Model for: "VirtualBox", "HVM domU", "VMWare"
}

BOOL manufacturer_computer_system_wmi()
{
    // Check Manufacturer for: "VMWare", "Xen", "innotek GmbH", "QEMU"
}
```

**Risk Level:** LOW (physical hardware data unchanged)

### 10.2 Win32_BIOS Serial Number

**Detection Method:**
```cpp
// al-khaser/AntiVM/Generic.cpp:1052-1113
BOOL serial_number_bios_wmi()
{
    // Check SerialNumber for: "VMWare", "0" (VBox), "Xen", "Virtual", "A M I"
}
```

**Risk Level:** LOW

---

## 11. Behavioral Detection

### 11.1 Processor Count Check

**Detection Method:**
```cpp
// al-khaser/AntiVM/Generic.cpp:372-386
BOOL NumberOfProcessors()
{
    PULONG ulNumberProcessors = (PULONG)(__readgsqword(0x60) + 0xB8);
    if (*ulNumberProcessors < 2)
        return TRUE;  // Likely VM
    return FALSE;
}
```

**Risk Level:** LOW (VMs commonly have multiple vCPUs now)

### 11.2 Memory Size Check

**Detection Method:**
```cpp
// al-khaser/AntiVM/Generic.cpp:919-928
BOOL memory_space()
{
    DWORDLONG ullMinRam = (1024LL * 1024LL * 1024LL * 1LL);  // 1GB
    MEMORYSTATUSEX statex = { 0 };
    GlobalMemoryStatusEx(&statex);
    return (statex.ullTotalPhys < ullMinRam) ? TRUE : FALSE;
}
```

**Risk Level:** LOW

### 11.3 Disk Size Check

**Detection Method:**
```cpp
// al-khaser/AntiVM/Generic.cpp:554-626
BOOL disk_size_wmi()
{
    // Check if disk < 80GB (common VM default)
    UINT64 minHardDiskSize = (80ULL * 1024ULL * 1024ULL * 1024ULL);
}
```

**Risk Level:** LOW

---

## 12. Recommended Bypass Priority Matrix

| Detection Type | Risk Level | Bypass Priority | Implementation |
|---------------|------------|-----------------|----------------|
| CPUID Hypervisor Bit | CRITICAL | P0 | VmExitHandler |
| CPUID Vendor String | CRITICAL | P0 | VmExitHandler |
| RDTSC Timing | HIGH | P1 | TSC Offset/Scaling |
| EAC CR3 Tracking | CRITICAL | P0 | EacBypass module |
| EAC NMI Handling | HIGH | P1 | NmiHandler module |
| VMX MSR Detection | HIGH | P1 | RDMSR handler |
| Debug Register Check | MEDIUM | P2 | MOV DR handler |
| Process Blocking | HIGH | P1 | Process Monitor |
| WMI Queries | LOW | P3 | Not needed |
| Hardware Artifacts | LOW | P3 | Not needed |

---

## 13. Implementation Checklist for OmbraHypervisor

### Phase 1: Core Detection Bypass
- [ ] CPUID leaf 0x1 - Clear hypervisor present bit (configurable)
- [ ] CPUID leaf 0x40000000 - Handle vendor string
- [ ] TSC offsetting via VMCS
- [ ] Basic RDMSR spoofing for VMX MSRs

### Phase 2: Anti-Cheat Specific
- [ ] EAC CR3 tracking bypass
- [ ] EAC NMI blocking mechanism
- [ ] Process creation monitoring hooks
- [ ] Blocked process list with scoring

### Phase 3: Advanced Evasion
- [ ] KUSER_SHARED_DATA shadowing for timing
- [ ] Debug register virtualization
- [ ] EPT-based function hooking for NtQueryInformationProcess

### Phase 4: Testing
- [ ] Run al-khaser test suite
- [ ] Test against EAC-protected games
- [ ] Timing attack validation
- [ ] False positive rate assessment

---

## 14. Code Reference Summary

| Component | Source File | Lines | Purpose |
|-----------|------------|-------|---------|
| CPUID Detection | al-khaser/AntiVM/Generic.cpp | 987-1047 | Hypervisor bit and vendor string checks |
| Timing Attacks | al-khaser/TimingAttacks/timing.cpp | 180-239 | RDTSC-based VM detection |
| Hyper-V Detection | al-khaser/AntiVM/HyperV.cpp | 5-70 | Driver and object enumeration |
| VBox Detection | al-khaser/AntiVM/VirtualBox.cpp | 1-922 | Comprehensive VirtualBox detection |
| VMware Detection | al-khaser/AntiVM/VMWare.cpp | 1-298 | VMware artifact detection |
| HyperHide Core | HyperHide/HyperHideDrv/Hider.cpp | Full | Process hiding implementation |
| HyperHide Timing | HyperHide/HyperHideDrv/KuserSharedData.cpp | 79-177 | KUSER_SHARED_DATA manipulation |
| HyperHide Gateway | HyperHide/HyperHideDrv/HypervisorGateway.cpp | 1-222 | Hypervisor control interface |
| EAC Bypass | Sputnik/SKLib/SKLib/SKLib-v/src/eac.cpp | 1-173 | EasyAntiCheat specific bypasses |
| Process Blocking | Sputnik/CheatDriver/src/comms.cpp | 210-326 | Blocked process and scoring system |
| Process Monitoring | Sputnik/CheatDriver/src/comms.cpp | 446-784 | PspInsertProcess/Thread hooks |

---

## 15. Risk Assessment: R09 - Anti-Cheat Signature Miss

**Risk Description:** OmbraHypervisor may be detected by anti-cheat signature updates or new detection techniques.

**Mitigation Strategies:**
1. **Modular Detection Bypass:** Keep bypass code modular for easy updates
2. **Signature Database:** Maintain database of known AC signatures
3. **Behavioral Analysis:** Monitor for new detection patterns
4. **Update Mechanism:** Support hot-patching of detection bypass rules

**Monitoring Points:**
- EAC driver hash changes
- BattlEye module updates
- Vanguard signature updates
- New CPUID leaf queries

---

*Document Version: 1.0*
*Last Updated: 2025-12-18*
*Author: Detection Agent (Phase 14)*
