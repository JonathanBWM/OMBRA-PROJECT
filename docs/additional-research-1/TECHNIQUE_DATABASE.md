# Antidetection Technique Database

**Generated**: 2025-12-22
**Source**: 28 reference codebases in CODEBASE-REF/
**Purpose**: Unified catalog of all antidetection techniques for OmbraHypervisor

---

## 1. CPUID TECHNIQUES

### 1.1 Hypervisor Present Bit Clearing (CPUID.01H:ECX[31])
| Repo | Implementation | Reference |
|------|----------------|-----------|
| ksm | `cpuid[2] &= ~(1 << 31)` in CPUID handler | ksm.md:158-160 |
| VBoxHardenedLoader | `setextradata "VBoxInternal/CPUM/EnableHVP" 0` | VBoxHardenedLoader.md:13-14 |
| VmwareHardenedLoader | `hypervisor.cpuid.v0 = "FALSE"` in VMX config | VmwareHardenedLoader.md:33 |
| NoirVisor | Conditional based on stealth mode flag | NoirVisor.md:varies |

### 1.2 VMX Feature Bit Clearing (CPUID.01H:ECX[5])
| Repo | Implementation | Reference |
|------|----------------|-----------|
| ksm | `cpuid[2] &= ~(1 << (X86_FEATURE_VMX & 31))` | ksm.md:24-26 |
| HyperPlatform | Pass-through (no hiding) | HyperPlatform.md:reference impl |

### 1.3 Hypervisor Vendor Leaf Suppression (0x40000000+)
| Repo | Implementation | Reference |
|------|----------------|-----------|
| ksm | Return zeros for 0x40000000-0x400000FF | ksm.md:321-324 |
| VBoxHardenedLoader | Paravirt provider = "legacy" | VBoxHardenedLoader.md:57 |
| Voyager | Uses CPUID for VMCALL dispatch (key in RCX) | Voyager.md:234-237 |
| Sputnik | CPUID-based hypercall with authentication key | Sputnik.md:234 |

### 1.4 Vendor String Manipulation
| Repo | Implementation | Reference |
|------|----------------|-----------|
| VBoxHardenedLoader | Patches "VBOXCPU" to generic string | VBoxHardenedLoader.md:27-40 |
| VmwareHardenedLoader | Masks "VMwareVMware" in EBX/ECX/EDX | VmwareHardenedLoader.md:54-58 |

### 1.5 Magic CPUID Commands
| Repo | Implementation | Reference |
|------|----------------|-----------|
| Voyager | CPUID exit + RCX=key triggers VMCALL dispatch | Voyager.md:234-237 |
| Sputnik | CPUID-based hypercall interface | Sputnik.md:varies |

---

## 2. TIMING TECHNIQUES

### 2.1 TSC Offset via VMCS/VMCB
| Repo | Implementation | Reference |
|------|----------------|-----------|
| VBoxHardenedLoader | `setextradata "VBoxInternal/TM/TSCMode" RealTSCOffset` | VBoxHardenedLoader.md:62-65 |
| ksm | VMCS field TSC_OFFSET (0x2010) | ksm.md:266-277 |
| HyperPlatform | Pass-through (no offsetting) | HyperPlatform.md:reference impl |

### 2.2 RDTSC/RDTSCP Interception
| Repo | Implementation | Reference |
|------|----------------|-----------|
| ksm | `vcpu_handle_rdtsc()` returns actual TSC | ksm.md:196-203 |
| ksm | `vcpu_handle_rdtscp()` returns TSC + TSC_AUX | ksm.md:235-245 |
| HyperPlatform | `VmmpHandleRdtsc()` / `VmmpHandleRdtscp()` | HyperPlatform.md:94,107 |

### 2.3 VM-Exit Timing Compensation
| Repo | Implementation | Reference |
|------|----------------|-----------|
| ksm | Proposed but not implemented | ksm.md:450-470 |
| hvpp | `tsc_entry_` / `tsc_delta_sum_` tracking per-VCPU | hvpp.md:220-222 |

### 2.4 Hardware vs Software Approaches
- **Hardware (preferred)**: Use VMCS TSC_OFFSET field - zero overhead
- **Software**: Intercept RDTSC, subtract estimated VM-exit cycles
- **Risk**: Software approach adds measurable latency

---

## 3. MSR TECHNIQUES

### 3.1 MSR Bitmap Configuration
| Repo | Implementation | Reference |
|------|----------------|-----------|
| ksm | 4KB bitmap, intercept 0x480-0x491 | ksm.md:54-87 |
| HyperPlatform | Default intercept all, whitelist exceptions | HyperPlatform.md:184-197 |
| hvpp | `vmx::msr_bitmap_t` page-aligned | hvpp.md:189 |

### 3.2 VMX Capability Hiding (MSR 0x480-0x491)
| Repo | Implementation | Reference |
|------|----------------|-----------|
| ksm | Inject #GP(0) on RDMSR of VMX MSRs | ksm.md:107-121 |
| ksm | Filter VMX capabilities when nested | ksm.md:111-119 |

### 3.3 IA32_FEATURE_CONTROL (0x3A) Manipulation
| Repo | Implementation | Reference |
|------|----------------|-----------|
| ksm | Clear VMXON enable bits (1-2) | ksm.md:99-101 |
| EfiGuard | N/A (operates before VMX enabled) | EfiGuard.md:varies |

### 3.4 VMCS-Shadowed MSRs
| Repo | MSRs Shadowed | Reference |
|------|---------------|-----------|
| HyperPlatform | SYSENTER_CS/ESP/EIP, DEBUGCTL, GS/FS_BASE | HyperPlatform.md:205-212 |

---

## 4. EPT/NPT TECHNIQUES

### 4.1 Execute-Only Page Shadowing
| Repo | Implementation | Reference |
|------|----------------|-----------|
| CheatDriver | `pgPermission.Exec = true` with substitute page | CheatDriver.md:53-61 |
| DdiMon | EPT hook with shadow pages for DdiMon | DdiMon.md:varies |
| hvpp | `epte_t::access_type::execute` permission | hvpp.md:385-391 |

### 4.2 2MB→4KB Page Splitting
| Repo | Implementation | Reference |
|------|----------------|-----------|
| hvpp | `split_2mb_to_4kb()` method | hvpp.md:496 |
| HyperPlatform | On-demand splitting during EPT violation | HyperPlatform.md:164-165 |

### 4.3 EPT Violation Handling
| Repo | Implementation | Reference |
|------|----------------|-----------|
| HyperPlatform | `EptHandleEptViolation()` - lazy table construction | HyperPlatform.md:160-165 |
| CheatDriver | Swap pages based on access type (R vs X) | CheatDriver.md:337-358 |

### 4.4 Memory Type from MTRR
| Repo | Implementation | Reference |
|------|----------------|-----------|
| HyperPlatform | `EptpGetMemoryType()` MTRR lookup | HyperPlatform.md:169-172 |
| hvpp | Memory type in EPT entry union | hvpp.md:411 |

### 4.5 NPT (AMD) Specifics
| Repo | Implementation | Reference |
|------|----------------|-----------|
| SimpleSvm | NPT enable via VMCB | SimpleSvm.md:varies |
| NoirVisor | `noir_npt_manager` structure | NoirVisor.md:243-244 |

---

## 5. DRIVER HIDING TECHNIQUES

### 5.1 PiDDBCacheTable Manipulation
| Repo | Implementation | Reference |
|------|----------------|-----------|
| Sputnik/libsputnik | `clear_piddb_cache()` in kernel_ctx | Sputnik.md:205 |
| SKLib | PiDDBCacheTable clearing | SKLib.md:varies |

### 5.2 KernelHashBucketList Cleanup
| Repo | Implementation | Reference |
|------|----------------|-----------|
| SKLib | Referenced in driver hiding routines | SKLib.md:varies |

### 5.3 MmUnloadedDrivers Clearing
| Repo | Implementation | Reference |
|------|----------------|-----------|
| SKLib | Clears unloaded driver history | SKLib.md:varies |

### 5.4 EPT-Based Driver Hiding
| Repo | Implementation | Reference |
|------|----------------|-----------|
| CheatDriver | `vTrackedHiddenRanges` vector for auto-cleanup | CheatDriver.md:62,69 |

---

## 6. BOOT CHAIN TECHNIQUES

### 6.1 bootmgfw.efi Hooking
| Repo | Implementation | Reference |
|------|----------------|-----------|
| Voyager | Hook `ArchStartBootApplication` | Voyager.md:43-48 |
| Sputnik | Hook via `MakeInlineHook()` | Sputnik.md:282-284 |
| EfiGuard | Hook `gBS->LoadImage` to intercept | EfiGuard.md:259-279 |

### 6.2 winload.efi Hooking
| Repo | Implementation | Reference |
|------|----------------|-----------|
| Voyager | Hook `BlLdrLoadImage` (1709+) or `BlImgLoadPEImageEx` (<1703) | Voyager.md:69-96 |
| Sputnik | Dual-path hook strategy | Sputnik.md:749-750 |
| EfiGuard | Hook `OslFwpKernelSetupPhase1` | EfiGuard.md:341-387 |

### 6.3 hvloader.efi Hooking
| Repo | Implementation | Reference |
|------|----------------|-----------|
| Voyager | Hook `HvBlImgAllocateImageBuffer` | Voyager.md:101-118 |
| Sputnik | Hook hvloader's PE load functions | Sputnik.md:366-386 |

### 6.4 Hyper-V VMExit Hook
| Repo | Implementation | Reference |
|------|----------------|-----------|
| Voyager | Patch RIP-relative call in hv.exe | Voyager.md:188-216 |
| Sputnik | `HookVmExit()` with Intel/AMD patterns | Sputnik.md:585-636 |

### 6.5 PE Section Addition
| Repo | Implementation | Reference |
|------|----------------|-----------|
| Voyager | `AddSection()` for payload injection | Voyager.md:141-156 |
| Sputnik | Same pattern with "payload" section | Sputnik.md:479-512 |

### 6.6 Payload Self-Deletion
| Repo | Implementation | Reference |
|------|----------------|-----------|
| Voyager | `PayLoadFile->Delete()` after load | Voyager.md:130 |
| Sputnik | Payload deleted from ESP after load | Sputnik.md:467 |

---

## 7. SYSCALL/HOOK TECHNIQUES

### 7.1 InfinityHook Pattern
| Repo | Implementation | Reference |
|------|----------------|-----------|
| InfinityHook | ETW-based syscall hook via WMI trace | InfinityHook.md:varies |

### 7.2 EPT-Based Syscall Hooks
| Repo | Implementation | Reference |
|------|----------------|-----------|
| CheatDriver | Hook `PspInsertThread/Process` via EPT | CheatDriver.md:287-307 |
| DdiMon | EPT hooks on kernel functions | DdiMon.md:varies |

### 7.3 SMBIOS/Firmware Table Hooks
| Repo | Implementation | Reference |
|------|----------------|-----------|
| VmwareHardenedLoader | Hook `ExpFirmwareTableProviderListHead` | VmwareHardenedLoader.md:236-327 |

---

## 8. CR/DEBUG REGISTER TECHNIQUES

### 8.1 CR3 Load Exiting
| Repo | Implementation | Reference |
|------|----------------|-----------|
| HyperPlatform | `cr3_load_exiting = true` | HyperPlatform.md:23 |
| ksm | CR3 tracking for TLB management | ksm.md:varies |

### 8.2 Debug Register Interception
| Repo | Implementation | Reference |
|------|----------------|-----------|
| HyperPlatform | `mov_dr_exiting = true` + `VmmpHandleDrAccess()` | HyperPlatform.md:24,96 |

### 8.3 CR0/CR4 Masking
| Repo | Implementation | Reference |
|------|----------------|-----------|
| HyperPlatform | VMCS CR0/CR4 guest/host mask fields | HyperPlatform.md:varies |

---

## 9. VMCS/VMCB CONFIGURATION

### 9.1 Secondary Processor Controls (Intel)
| Control | Purpose | Repos |
|---------|---------|-------|
| `enable_ept` | Extended page tables | HyperPlatform, hvpp, ksm |
| `enable_vpid` | TLB tagging | HyperPlatform, hvpp |
| `enable_rdtscp` | Win10+ requirement | HyperPlatform |
| `enable_invpcid` | Win10+ requirement | HyperPlatform |
| `descriptor_table_exiting` | GDTR/IDTR access | HyperPlatform |

### 9.2 Exception Bitmap Configuration
| Repo | Exceptions Intercepted | Reference |
|------|------------------------|-----------|
| HyperPlatform | #DE (divide error) for PG init | HyperPlatform.md:varies |
| ksm | Minimal (reference impl) | ksm.md:varies |

### 9.3 I/O Bitmap Configuration
| Repo | Implementation | Reference |
|------|----------------|-----------|
| HyperPlatform | `use_io_bitmaps = true` | HyperPlatform.md:22 |
| VmwareHardenedLoader | Block port 0x5658 (VMware backdoor) | VmwareHardenedLoader.md:17-35 |

---

## 10. ANTICHEAT EVASION

### 10.1 Detection Vector Counters
| Vector | Counter Technique | Repos |
|--------|-------------------|-------|
| CPUID.01H:ECX[31] | Clear hypervisor bit | ksm, VBox/VMware loaders |
| CPUID.01H:ECX[5] | Clear VMX bit | ksm |
| CPUID.40000000 | Return zeros | ksm, VBox/VMware loaders |
| I/O port 0x5658 | Block or disable | VmwareHardenedLoader |
| SMBIOS strings | Scrub "VMware"/"Virtual" | VmwareHardenedLoader |
| ACPI tables | Replace "VBOX" OEM IDs | VBoxHardenedLoader |
| MAC address | Avoid VM vendor prefixes | VBoxHardenedLoader |
| PCI device IDs | Spoof to NVIDIA/etc | VBoxHardenedLoader |

### 10.2 Specific Anticheat Notes
| Anticheat | Detection Method | Evasion |
|-----------|-----------------|---------|
| EasyAntiCheat | CPUID, timing, driver checks | Full CPUID masking + TSC offset |
| BattlEye | Kernel callbacks, driver signatures | EPT hooks avoid callback registration |
| Vanguard | Ring-0 integrity, timing | Boot-time injection before Vanguard loads |

---

## CROSS-REFERENCE MATRIX

| Technique Category | ksm | NoirVisor | hvpp | HyperPlatform | Voyager | Sputnik | EfiGuard | CheatDriver | VBox/VMware |
|-------------------|-----|-----------|------|---------------|---------|---------|----------|-------------|-------------|
| CPUID Hiding | YES | YES | - | - | - | - | - | - | YES |
| TSC Offsetting | YES | - | YES | - | - | - | - | - | YES |
| MSR Filtering | YES | YES | YES | YES | - | - | - | - | - |
| EPT Hooks | - | YES | YES | YES | - | - | - | YES | - |
| NPT Hooks | - | YES | - | - | - | - | - | - | - |
| Boot Chain Hook | - | - | - | - | YES | YES | YES | - | - |
| Driver Hiding | - | - | - | - | - | YES | - | YES | - |
| Syscall Hooks | - | - | - | - | - | - | - | YES | - |
| SMBIOS Hiding | - | - | - | - | - | - | - | - | YES |
| PatchGuard Defeat | - | - | - | - | - | - | YES | - | - |
| DSE Bypass | - | - | - | - | - | - | YES | - | - |
| Dual CPU (Intel+AMD) | - | YES | - | - | YES | YES | - | YES | - |

---

## PRIORITY TECHNIQUES FOR OMBRA

### Critical (Must Implement)
1. **CPUID.01H:ECX[31] = 0** - Hypervisor present bit
2. **CPUID.01H:ECX[5] = 0** - VMX feature bit (optional)
3. **CPUID.40000000+ = zeros** - Hypervisor vendor leaves
4. **TSC offset via VMCS** - Hardware offsetting preferred
5. **MSR bitmap for 0x480-0x491** - Inject #GP on VMX MSRs

### Important (Should Implement)
6. **EPT execute-only shadowing** - For memory hiding
7. **Boot chain hooks** - For pre-boot injection
8. **Dual CPU support** - Intel VMX + AMD SVM

### Nice to Have
9. **SMBIOS/ACPI hiding** - For full VM detection evasion
10. **Driver hiding** - PiDDBCacheTable etc.
11. **PatchGuard considerations** - Ombra operates above PG

---

**End of Technique Database**
