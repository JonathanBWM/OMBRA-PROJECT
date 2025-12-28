# B6: What Does Anti-Cheat Detect?

## Executive Summary

Anti-cheat software employs multiple detection vectors to identify cheating tools, hypervisors, and system tampering. This document provides a comprehensive technical analysis of detection methods, their implementation details, and countermeasures available in the OmbraHypervisor codebase.

---

## Table of Contents

1. [Hypervisor Detection](#1-hypervisor-detection)
2. [Memory Integrity Checks](#2-memory-integrity-checks)
3. [Hook Detection](#3-hook-detection)
4. [Driver Enumeration](#4-driver-enumeration)
5. [Hardware Fingerprinting](#5-hardware-fingerprinting)
6. [Timing Attacks](#6-timing-attacks)
7. [Process Monitoring and Scoring](#7-process-monitoring-and-scoring)
8. [Anti-Cheat Specific Bypasses](#8-anti-cheat-specific-bypasses)
9. [Implementation Priority Matrix](#9-implementation-priority-matrix)

---

## 1. Hypervisor Detection

### 1.1 CPUID Hypervisor Present Bit (CPUID.1:ECX[31])

**Detection Method:**
Anti-cheat checks the hypervisor present bit in CPUID leaf 1, ECX register bit 31. This is the most fundamental virtualization detection.

```cpp
// Detection code pattern (from al-khaser reference)
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

**Countermeasure (from SKLib-v VMExitHandlers.cpp):**
```cpp
// Intercept CPUID VM exits for leaf 0x1
if (pContext->rax == PROC_FEATURES_CPUID)
{
    // Set hypervisor present bit (for Hyper-V compatibility)
    CpuInfo[2] |= HYPERV_HYPERVISOR_PRESENT_BIT;
    // Mask out virtualization-related features
    CpuInfo[2] &= VMM_CPUIDECX_MASK;
}
```

**Implementation Location:** `PayLoad/Core/VmExitHandler.cpp`

### 1.2 Hypervisor Vendor String (CPUID.40000000h)

**Detection Method:**
Anti-cheat enumerates known hypervisor vendor strings:

| Vendor String | Hypervisor |
|---------------|------------|
| "KVMKVMKVM" | KVM |
| "Microsoft Hv" | Hyper-V |
| "VMwareVMware" | VMware |
| "XenVMMXenVMM" | Xen |
| "prl hyperv  " | Parallels |
| "VBoxVBoxVBox" | VirtualBox |

**Risk Level:** CRITICAL

**Countermeasure:**
For Hyper-V hijacking, return "Microsoft Hv" (expected behavior). The goal is to appear as legitimate Hyper-V, not to hide virtualization entirely.

### 1.3 VMX/SVM Feature Flag Detection

**Detection Method:**
Anti-cheat checks for VMX (Intel) or SVM (AMD) capability bits:
- CPUID.1:ECX[5] - VMX available
- CPUID.80000001h:ECX[2] - SVM available

**Risk Level:** HIGH

**Countermeasure:**
Mask VMX/SVM bits in CPUID responses when hypervisor hiding is enabled.

```cpp
// From VMExitHandlers.cpp
#define VMM_CPUIDECX_MASK ~(CPUIDECX_EST | CPUIDECX_TM2 | CPUIDECX_MWAIT | \
        CPUIDECX_PDCM | CPUIDECX_VMX | CPUIDECX_DTES64 | \
        CPUIDECX_DSCPL | CPUIDECX_SMX | CPUIDECX_CNXTID | \
        CPUIDECX_SDBG | CPUIDECX_XTPR | CPUIDECX_PCID | \
        CPUIDECX_DCA | CPUIDECX_X2APIC | CPUIDECX_DEADLINE)
```

---

## 2. Memory Integrity Checks

### 2.1 Page Hash Verification

**Detection Method:**
Anti-cheat computes hashes of critical memory regions:
- Kernel code sections (.text of ntoskrnl.exe)
- Driver code sections
- Game module code sections
- System call table entries

**Risk Level:** HIGH

**Countermeasure:**
EPT/NPT hook hiding using dual page tables:

```cpp
// From AMD_NPT_HOOK_HIDING_RESEARCH.md
// Primary NPT: Original pages marked NX (no execute)
// Secondary NPT: Shadow pages mapped executable

// NPF handler toggles between page tables
if(fault.execute) {
    // Execution caused NPF - switch to secondary NPT (hooked pages)
    noir_svm_vmwrite64(vcpu->vmcb.virt, npt_cr3,
        pri_nptm->ncr3.phys == cur_ncr3 ? sec_nptm->ncr3.phys : pri_nptm->ncr3.phys);
}
```

### 2.2 .text Section Scanning

**Detection Method:**
Anti-cheat scans executable sections for:
- Modified bytes compared to on-disk image
- Unknown code patterns
- Inline hook signatures (JMP/CALL redirections)

**Risk Level:** HIGH

**Countermeasure:**
EPT execute-only pages (Intel) or dual NPT (AMD) ensure reads see original bytes while execution uses hooked bytes.

**AMD Limitation:**
AMD NPT does not support execute-only pages. Must use dual page table approach which has higher performance overhead (minimum 4 VM exits per hooked function call).

### 2.3 PatchGuard/KPP Compatibility

**Detection Method:**
Windows Kernel Patch Protection monitors:
- System Service Dispatch Table (SSDT)
- Interrupt Descriptor Table (IDT)
- Global Descriptor Table (GDT)
- Critical kernel structures

**Risk Level:** MEDIUM (for hypervisor-based hooks)

**Note:** Hypervisor-level hooks operate below PatchGuard visibility. EPT/NPT hooks do not modify the actual memory content visible to PatchGuard.

---

## 3. Hook Detection

### 3.1 Inline Hook Patterns

**Detection Method:**
Anti-cheat scans for common inline hook signatures:

| Pattern | Bytes | Description |
|---------|-------|-------------|
| JMP rel32 | E9 XX XX XX XX | Near jump |
| JMP r/m64 | FF 25 XX XX XX XX | Far jump via memory |
| CALL rel32 | E8 XX XX XX XX | Near call |
| MOV RAX + JMP RAX | 48 B8 + FF E0 | Absolute jump |
| INT3 | CC | Breakpoint |

**Risk Level:** HIGH

**Countermeasure:**
EPT/NPT-based hooks modify a shadow page, not the original. Read operations see clean bytes.

### 3.2 IAT/EAT Modifications

**Detection Method:**
Anti-cheat validates Import/Export Address Tables:
- Compare IAT entries against expected addresses
- Check EAT entries for unexpected redirections
- Verify function pointers in driver dispatch tables

**Risk Level:** MEDIUM

**Countermeasure:**
Hypervisor-level hooks do not modify IAT/EAT. They intercept at the execution level.

### 3.3 SSDT/Shadow SSDT Integrity

**Detection Method:**
System Service Dispatch Table validation:
- Compare SSDT entries against known-good values
- Check for unexpected kernel-mode hooks

**Risk Level:** MEDIUM

**Countermeasure:**
Use EPT/NPT hooks instead of SSDT modifications.

---

## 4. Driver Enumeration

### 4.1 PsLoadedModuleList Walking

**Detection Method:**
Anti-cheat enumerates loaded drivers via:
- PsLoadedModuleList kernel structure
- ZwQuerySystemInformation(SystemModuleInformation)
- NtQuerySystemInformation

```cpp
// Detection pattern
SYSTEM_MODULE_INFORMATION* pModInfo;
NtQuerySystemInformation(SystemModuleInformation, pModInfo, ...);
// Enumerate all loaded modules
for(int i = 0; i < pModInfo->Count; i++) {
    CheckModuleSignature(&pModInfo->Module[i]);
}
```

**Risk Level:** HIGH

**Countermeasure:**
OmbraHypervisor operates from within Hyper-V's address space. The payload is injected into hv.exe (hvix64/hvax64), not loaded as a separate driver.

### 4.2 MmUnloadedDrivers Checking

**Detection Method:**
Anti-cheat examines the unloaded drivers list for suspicious entries:
- Recent driver unloads
- Known vulnerable driver names
- Pattern matching for exploit tools

**Risk Level:** MEDIUM

**Countermeasure:**
OmbraHypervisor does not use traditional driver loading. The UEFI bootloader injects directly into Hyper-V during boot.

### 4.3 Driver Object Enumeration

**Detection Method:**
```cpp
// Enumerate \Driver\ object directory
UNICODE_STRING dirName = RTL_CONSTANT_STRING(L"\\Driver");
ZwOpenDirectoryObject(&hDir, DIRECTORY_QUERY, &objAttr);
// List all driver objects
```

**Risk Level:** MEDIUM

**Note:** Hyper-V drivers (VMBusHID, vmbus, etc.) are expected and should not trigger detection.

---

## 5. Hardware Fingerprinting

### 5.1 SMBIOS/DMI String Detection

**Detection Method:**
Anti-cheat reads SMBIOS tables for:
- System manufacturer/model
- Serial numbers
- UUID
- Virtual machine indicators

**Risk Level:** LOW (for Type-1 hijacking)

**Countermeasure (from smbiosspoof.cpp):**
```cpp
bool smbios::Spoof(DWORD64 seed)
{
    // Shadow SMBIOS table via EPT
    for (DWORD64 pageOffset = 0; pageOffset < totLen; pageOffset += PAGE_SIZE) {
        PVOID pTarget = (PVOID)(pOrigSMBiosTable + pageOffset);
        EPT::Hook(pTarget, hkSecondaryInfo.pSubstitutePage, ...);
    }

    // Randomize serial numbers
    switch (entry->type) {
    case TYPE_SYSTEM_INFO:
        rnd.random_shuffle_ignore_chars(entry->data.sysinfo.SerialNumber, ...);
        break;
    case TYPE_BASEBOARD_INFO:
        rnd.random_shuffle_ignore_chars(entry->data.baseboard.SerialNumber, ...);
        break;
    // ... other types
    }
}
```

### 5.2 Disk Serial Numbers

**Detection Method:**
Anti-cheat queries disk serial numbers via:
- IOCTL_STORAGE_QUERY_PROPERTY
- IOCTL_ATA_PASS_THROUGH
- IOCTL_SCSI_MINIPORT
- SMART_RCV_DRIVE_DATA

**Risk Level:** LOW (for hypervisor purposes, HIGH for hardware bans)

**Countermeasure (from diskspoof.cpp):**
```cpp
bool disks::Spoof(DWORD64 seed)
{
    // Hook partition manager
    EPT::HookExec(pDrivObj->MajorFunction[IRP_MJ_DEVICE_CONTROL], &PartmgrIoCtrlHook, ...);

    // Hook SCSI/NVME/StorAHCI drivers
    EPT::HookExec(pDrivObj->MajorFunction[IRP_MJ_DEVICE_CONTROL], &NvmeControl, ...);

    // Spoof RAID unit serials
    SpoofRaid();
}

// IRP completion callback spoofs returned serial
NTSTATUS SmartDataIoc(PDEVICE_OBJECT device, PIRP irp, PVOID context) {
    PCHAR serial = ((PIDSECTOR)((PSENDCMDOUTPARAMS)request.Buffer)->bBuffer)->sSerialNumber;
    FindFakeDiskSerial(serial);  // Replace with spoofed value
}
```

### 5.3 NIC MAC Addresses

**Detection Method:**
Anti-cheat queries network adapter MAC addresses via:
- IOCTL_NDIS_QUERY_GLOBAL_STATS (OID_802_3_PERMANENT_ADDRESS)
- GetAdaptersInfo()
- WMI queries

**Risk Level:** LOW (for hypervisor purposes)

**Countermeasure (from nicspoof.cpp):**
```cpp
bool nics::Spoof(DWORD64 seed)
{
    // Hook nsiproxy driver
    EPT::HookExec(pDrivObj->MajorFunction[IRP_MJ_DEVICE_CONTROL], NsiControl, ...);

    // Hook TCP/IP driver
    EPT::HookExec(pDrivObj->MajorFunction[IRP_MJ_DEVICE_CONTROL], TcpControl, ...);

    // Modify NDIS filter blocks
    FindFakeNicMac((char*)pIfPhysAddress->Address);
}
```

### 5.4 GPU Information

**Detection Method:**
Anti-cheat queries GPU via:
- Direct3D device enumeration
- WMI Win32_VideoController
- Registry hardware IDs

**Risk Level:** LOW

**Countermeasure:**
GPU spoofing available via gpuspoof.cpp in SKLib-v.

---

## 6. Timing Attacks

### 6.1 RDTSC VM Exit Latency

**Detection Method:**
Anti-cheat measures time delta across instructions that cause VM exits:

```cpp
// From al-khaser timing.cpp
BOOL rdtsc_diff_vmexit()
{
    ULONGLONG tsc1 = 0, tsc2 = 0, avg = 0;
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

**Typical Values:**
- Bare metal CPUID: 200-500 cycles
- Virtualized CPUID: 1000-5000+ cycles

**Risk Level:** HIGH

**Countermeasure:**
1. **TSC Offsetting:** Use VMCS/VMCB TSC offset field
2. **TSC Scaling:** Available on newer CPUs
3. **KUSER_SHARED_DATA Shadowing:** Manipulate usermode timing data

```cpp
// From VMExitHandlers.cpp - Update TSC on sensitive operations
vmm::UpdateLastValidTsc();
```

### 6.2 KUSER_SHARED_DATA Manipulation

**Detection Method:**
Usermode programs read timing from KUSER_SHARED_DATA:
- InterruptTime
- SystemTime
- TickCount

**Risk Level:** MEDIUM

**Countermeasure:**
Shadow KUSER_SHARED_DATA page and periodically update fake timing values:

```cpp
// Concept from HyperHide KuserSharedData.cpp
VOID CounterUpdater(PVOID Context)
{
    while (!StopCounterThread)
    {
        KeDelayExecutionThread(KernelMode, FALSE, &TimeToWait);

        // Update fake values
        HiddenProcess->Kusd.KuserSharedData->InterruptTime =
            KernelKuserSharedData->InterruptTime - DeltaInterruptTime;
    }
}
```

### 6.3 Ratio-Based Timing Detection

**Detection Method:**
```cpp
// Locky-style detection
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

**Note:** TSC offsetting must maintain realistic timing ratios, not just absolute values.

---

## 7. Process Monitoring and Scoring

### 7.1 Threat Scoring System

**Implementation (from comms.cpp):**
```cpp
DWORD64 currScore = 0;
DWORD64 maxScore = 0;      // Detection threshold
DWORD64 warnScore = 0;     // Warning threshold

// Score accumulation
currScore += blocked.score;

if (!blocked.bRan && maxScore) {
    if (currScore >= warnScore)
        TriggerWarning();
    if (currScore >= maxScore)
        TriggerDetection();
    blocked.bRan = true;
}
```

### 7.2 Blocked Process List

**Default Blocked Processes (from comms.cpp):**

| Process | Score | Category |
|---------|-------|----------|
| x64dbg.exe | 50 | Debugger |
| windbg.exe | 50 | Debugger |
| ida64.exe | 50 | Disassembler |
| ghidra.exe | 50 | RE Tool |
| wireshark.exe | 50 | Network Capture |
| procmon.exe | 50 | Process Monitor |
| apimonitor.exe | 50 | API Monitor |
| ollydbg.exe | 50 | Debugger |
| fiddler.exe | 50 | HTTP Debugger |
| scylla.exe | 50 | PE Tool |
| processhacker.exe | 30 | Admin Tool |
| systeminformer.exe | 30 | Admin Tool |
| pe-bear.exe | 30 | PE Tool |
| dbgview.exe | 30 | Debug Output |

### 7.3 Process Creation Hooks

**Implementation (from comms.cpp):**
```cpp
// Hook PspInsertThread for thread creation monitoring
EPT::Hook((PVOID)winternl::PspInsertThread, PspInsertThread, ...);

// Hook PspInsertProcess for process creation monitoring
EPT::Hook((PVOID)winternl::PspInsertProcess, PspInsertProcess, ...);

// Hook PspRundownSingleProcess for process termination
EPT::Hook((PVOID)winternl::PspRundownSingleProcess, PspRundownSingleProcess, ...);

// On process creation
NTSTATUS RestrickAndBlockProcess(PEPROCESS pEprocess, PVOID pCtx)
{
    // Check against blocked list
    for (auto& blocked : *vBlockedProcesses) {
        if (strstr(pdbPath.c_str(), blocked.name.c_str())) {
            currScore += blocked.score;

            // Block or terminate
            if (!bStopBlock) {
                return STATUS_ACCESS_DENIED;  // Block creation
            }
            ZwTerminateProcess(hProc, STATUS_ACCESS_DENIED);  // Kill existing
        }
    }
}
```

---

## 8. Anti-Cheat Specific Bypasses

### 8.1 EasyAntiCheat (EAC)

**CR3 Tracking Bypass (from eac.cpp):**
```cpp
void eac::UpdateCr3(CR3 cr3)
{
    identity::PhysicalAccess pa(vmm::pIdentityMap, cr3.Flags);
    for (auto& data : *pCr3s) {
        if (data.pCr3 && pa.getPhysicalAddress((DWORD64)data.pImageBase + PAGE_SIZE)) {
            identity::PhysicalAccess paSrc(vmm::pIdentityMap, data.srcCr3);
            paSrc.Write<CR3>(data.pCr3, cr3);
        }
    }
}
```

**NMI Blocking (from eac.cpp):**
```cpp
void eac::BlockNmi(CR3 cr3)
{
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

### 8.2 MSR Detection Bypass

**RDMSR Handler (from VMExitHandlers.cpp):**
```cpp
bool VTx::VMExitHandlers::HandleRDMSR(PREGS pContext)
{
    MSR msr = { 0 };
    msr.Content = __readmsr(pContext->rcx);

    if (pContext->rcx == IA32_FEATURE_CONTROL)
    {
        // Hide VMX capability
        msr.Content |= IA32_FEATURE_CONTROL_LOCK_BIT_FLAG;
        msr.Content &= ~(IA32_FEATURE_CONTROL_ENABLE_VMX_INSIDE_SMX_FLAG);
        msr.Content &= ~(IA32_FEATURE_CONTROL_ENABLE_VMX_OUTSIDE_SMX_FLAG);
    }

    pContext->rax = msr.Low;
    pContext->rdx = msr.High;
    return false;
}
```

### 8.3 CR Access Handling

**CR3 Monitoring (from VMExitHandlers.cpp):**
```cpp
bool VTx::VMExitHandlers::HandleCR(PREGS pContext)
{
    switch (data->Fields.ControlRegister) {
    case 3:  // CR3
        vmm::UpdateLastValidTsc();

        CR3 cr3 = { 0 };
        cr3.Flags = *RegPtr;

        pState->lastExitedCr3 = *RegPtr;
        __vmx_vmwrite(GUEST_CR3, *RegPtr);

        // Invalidate VPID for TLB flush
        INVVPID_DESCRIPTOR desc = { 0 };
        desc.Vpid = dwCore + 1;
        CPU::InvalidateVPID(3, &desc);
        break;
    }
}
```

---

## 9. Implementation Priority Matrix

| Detection Type | Risk Level | Priority | Implementation Status |
|---------------|------------|----------|----------------------|
| CPUID Hypervisor Bit | CRITICAL | P0 | VMExitHandler CPUID interception |
| CPUID Vendor String | CRITICAL | P0 | VMExitHandler CPUID.40000000h |
| RDTSC Timing | HIGH | P1 | TSC offset in VMCS/VMCB |
| EAC CR3 Tracking | CRITICAL | P0 | eac.cpp implementation |
| EAC NMI Handling | HIGH | P1 | eac.cpp NMI blocking |
| VMX MSR Detection | HIGH | P1 | RDMSR handler |
| Memory Integrity | HIGH | P1 | EPT/NPT dual page tables |
| Hook Detection | HIGH | P1 | EPT execute-only / dual NPT |
| Process Blocking | HIGH | P1 | PspInsertProcess hooks |
| Debug Register | MEDIUM | P2 | MOV DR handler |
| Hardware Fingerprinting | LOW | P3 | SKLib-v spoof modules |
| WMI Queries | LOW | P3 | Not critical for hypervisor |

---

## 10. Source Code References

### SKLib-v Anti-Detection Modules

| File | Purpose | Key Functions |
|------|---------|---------------|
| `/Refs/Sputnik/SKLib/SKLib/SKLib-v/src/eac.cpp` | EAC bypass | UpdateCr3(), BlockNmi() |
| `/Refs/Sputnik/SKLib/SKLib/SKLib-v/src/VMExitHandlers.cpp` | VM exit handling | HandleCPUID(), HandleRDMSR() |
| `/Refs/Sputnik/SKLib/SKLib/SKLib-v/src/diskspoof.cpp` | Disk spoofing | FindFakeDiskSerial() |
| `/Refs/Sputnik/SKLib/SKLib/SKLib-v/src/nicspoof.cpp` | NIC spoofing | FindFakeNicMac() |
| `/Refs/Sputnik/SKLib/SKLib/SKLib-v/src/smbiosspoof.cpp` | SMBIOS spoofing | Spoof() |
| `/Refs/Sputnik/SKLib/SKLib/SKLib-v/src/spoof.cpp` | Master spoof | SpoofAll() |
| `/Refs/Sputnik/CheatDriver/src/comms.cpp` | Process monitoring | RestrickAndBlockProcess() |

### Related Documentation

| Document | Path |
|----------|------|
| Detection Knowledge Base | `/OmbraHypervisor/Docs/Training/A10_DETECTION_KNOWLEDGE.md` |
| AMD NPT Hook Hiding | `/OmbraHypervisor/Docs/AMD_NPT_HOOK_HIDING_RESEARCH.md` |
| EPT/NPT Stealth Reference | `/OmbraHypervisor/Docs/EPT_NPT_HOOK_STEALTH_REFERENCE.md` |

---

## 11. Risk Assessment: R09 - Anti-Cheat Signature Miss

**Risk Description:**
OmbraHypervisor may be detected by anti-cheat signature updates or new detection techniques.

**Mitigation Strategies:**

1. **Modular Detection Bypass:** Keep bypass code modular for rapid updates
2. **Signature Database:** Maintain database of known AC signatures with Windows build versioning
3. **Behavioral Analysis:** Monitor for new detection patterns through community reports
4. **Update Mechanism:** Support hot-patching of detection bypass rules without full reboot

**Monitoring Points:**
- EAC driver hash changes (EasyAntiCheat.sys)
- BattlEye module updates (BEService.exe, BEClient.dll)
- Vanguard signature updates (vgk.sys, vgkbootstatus.dat)
- Ricochet kernel driver updates
- New CPUID leaf queries in AC binaries

---

*Document Version: 1.0*
*Last Updated: 2025-12-18*
*Author: Detection Agent (Phase 14)*
