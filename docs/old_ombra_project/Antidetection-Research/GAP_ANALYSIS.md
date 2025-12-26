# OmbraHypervisor Gap Analysis

**Generated**: 2025-12-22
**Purpose**: Identify missing techniques and capabilities for comprehensive antidetection

---

## 1. CPUID GAPS

### Currently Available (from reference codebases)
- Hypervisor present bit clearing (ECX[31])
- VMX feature bit clearing (ECX[5])
- Hypervisor leaf suppression (0x40000000+)
- Vendor string manipulation

### Gaps for Ombra
| Gap | Priority | Difficulty | Notes |
|-----|----------|------------|-------|
| **AMD CPUID equivalents** | HIGH | Medium | SVM feature bits in different locations |
| **Brand string anomaly detection** | MEDIUM | Low | Some VMs expose brand string artifacts |
| **CPUID timing normalization** | LOW | High | Pad CPUID execution time to match bare metal |
| **Nested hypervisor leaf handling** | LOW | High | If supporting nested virt |

### Recommended Actions
1. Implement unified CPUID handler supporting both Intel and AMD
2. Test against pafish/al-khaser CPUID detection routines
3. Consider Hyper-V enlightenment spoofing for Windows compatibility

---

## 2. TIMING GAPS

### Currently Available
- TSC offset via VMCS (Intel) / VMCB (AMD)
- Basic RDTSC/RDTSCP interception
- TSC tracking per-VCPU (hvpp)

### Gaps for Ombra
| Gap | Priority | Difficulty | Notes |
|-----|----------|------------|-------|
| **VM-exit latency compensation** | HIGH | High | Anti-cheat measures instruction timing |
| **CPUID timing normalization** | MEDIUM | Medium | CPUID takes longer under hypervisor |
| **Performance counter hiding** | MEDIUM | Medium | PMC MSRs can leak virtualization |
| **APIC timer virtualization** | LOW | Medium | Timer-based detection possible |

### Recommended Actions
1. Profile baseline VM-exit overhead on target CPUs
2. Implement adaptive TSC offset that compensates for exit latency
3. Test with timing attack PoCs (RDTSC around CPUID)

---

## 3. MSR GAPS

### Currently Available
- MSR bitmap configuration
- VMX capability MSR hiding (0x480-0x491)
- IA32_FEATURE_CONTROL manipulation
- VMCS-shadowed MSRs (SYSENTER, DEBUGCTL, GS/FS_BASE)

### Gaps for Ombra
| Gap | Priority | Difficulty | Notes |
|-----|----------|------------|-------|
| **AMD MSR equivalents** | HIGH | Medium | SVM uses different MSR ranges |
| **IA32_TSC_ADJUST interception** | MEDIUM | Low | Can detect TSC manipulation |
| **IA32_DEBUGCTLMSR filtering** | MEDIUM | Low | LBR can leak hypervisor addresses |
| **Performance counter MSRs** | LOW | Medium | MSRs 0xC1-0xCF |
| **Hyper-V synthetic MSRs** | LOW | Medium | 0x40000000+ range |

### Recommended Actions
1. Create unified MSR handler with Intel/AMD dispatch
2. Add MSR_IA32_TSC_ADJUST to bitmap
3. Consider LBR (Last Branch Record) implications

---

## 4. EPT/NPT GAPS

### Currently Available
- Basic EPT/NPT setup
- Execute-only page shadowing
- 2MB→4KB page splitting
- MTRR-aware memory typing
- EPT violation handling

### Gaps for Ombra
| Gap | Priority | Difficulty | Notes |
|-----|----------|------------|-------|
| **Unified EPT/NPT API** | HIGH | Medium | Single interface for both architectures |
| **Suppress #VE (virtualization exception)** | MEDIUM | Low | EPT entry bit 63 |
| **Sub-page permissions** | LOW | High | Intel SPP feature for finer control |
| **IOMMU/VT-d integration** | LOW | High | DMA protection bypass considerations |

### Recommended Actions
1. Abstract EPT/NPT behind common interface
2. Implement page shadowing with automatic cleanup
3. Test EPT hooks against kernel integrity checks

---

## 5. DRIVER HIDING GAPS

### Currently Available
- PiDDBCacheTable manipulation
- KernelHashBucketList cleanup
- MmUnloadedDrivers clearing
- EPT-based driver hiding

### Gaps for Ombra
| Gap | Priority | Difficulty | Notes |
|-----|----------|------------|-------|
| **Pool tag hiding** | MEDIUM | Medium | ExAllocatePool tags |
| **Object directory hiding** | MEDIUM | Medium | \\Device\\Ombra etc |
| **Registry artifact hiding** | LOW | Medium | Service registry keys |
| **ETW provider hiding** | LOW | High | Event tracing leaks |

### Recommended Actions
1. Ombra operates from UEFI/hypervisor - minimal driver footprint
2. Avoid creating ANY kernel objects where possible
3. If driver needed, use EPT to shadow all driver memory

---

## 6. BOOT CHAIN GAPS

### Currently Available
- bootmgfw.efi hooking
- winload.efi hooking
- hvloader.efi hooking
- Hyper-V VMExit patching
- PE section addition
- Payload self-deletion

### Gaps for Ombra
| Gap | Priority | Difficulty | Notes |
|-----|----------|------------|-------|
| **Secure Boot bypass** | HIGH | High | Need signed shim or MOK enrollment |
| **UEFI Secure Variables** | MEDIUM | Medium | db/dbx manipulation |
| **Windows version detection** | MEDIUM | Low | Pattern signatures per build |
| **HVCI/VBS bypass** | MEDIUM | High | Virtualization-based security |

### Recommended Actions
1. Research Secure Boot enrollment or signed shim approach
2. Maintain signature database per Windows version
3. Test against HVCI-enabled systems

---

## 7. SYSCALL/HOOK GAPS

### Currently Available
- InfinityHook patterns (ETW-based)
- EPT-based syscall hooks
- Firmware table hooks

### Gaps for Ombra
| Gap | Priority | Difficulty | Notes |
|-----|----------|------------|-------|
| **KVAS/Meltdown mitigation awareness** | HIGH | Medium | Shadow page tables |
| **Syscall table shadowing** | MEDIUM | Medium | Alternative to InfinityHook |
| **SSDT hook detection evasion** | LOW | Medium | Anti-cheat scans for hooks |

### Recommended Actions
1. Use EPT hooks instead of inline hooks
2. Operate from hypervisor context to avoid usermode detection
3. Minimize guest-visible modifications

---

## 8. DEBUG/CR REGISTER GAPS

### Currently Available
- CR3 load exiting
- Debug register interception
- CR0/CR4 masking

### Gaps for Ombra
| Gap | Priority | Difficulty | Notes |
|-----|----------|------------|-------|
| **Hardware breakpoint hiding** | MEDIUM | Medium | DR0-DR3 virtualization |
| **Debug exception handling** | MEDIUM | Medium | #DB injection timing |
| **CR4.SMEP/SMAP handling** | LOW | Low | Security feature bits |

### Recommended Actions
1. Implement full DR0-DR7 virtualization
2. Test against anti-debug techniques

---

## 9. ANTICHEAT-SPECIFIC GAPS

### Detection Vectors NOT Covered
| Vector | Priority | Notes |
|--------|----------|-------|
| **Kernel callback detection** | HIGH | EAC/BE enumerate callbacks |
| **Integrity check evasion** | HIGH | Memory scanning of hooked pages |
| **Thread context inspection** | MEDIUM | Stack traces reveal hypervisor |
| **Exception chain analysis** | MEDIUM | Exception handlers leak info |
| **ETW trace analysis** | LOW | Event logging reveals behavior |

### Recommended Actions
1. Avoid registering any kernel callbacks
2. Use EPT shadow pages to pass integrity checks
3. Ensure stack traces don't show hypervisor addresses

---

## 10. ARCHITECTURE GAPS

### Currently Available
- Dual CPU detection (NoirVisor pattern)
- Runtime Intel/AMD dispatch
- Conditional compilation strategy

### Gaps for Ombra
| Gap | Priority | Difficulty | Notes |
|-----|----------|------------|-------|
| **Unified abstraction layer** | HIGH | Medium | Common vCPU/EPT interface |
| **SVM-specific stealth** | HIGH | Medium | AMD-specific detection vectors |
| **Nested virtualization** | LOW | High | Running under Hyper-V/VMware |
| **ARM64 support** | LOW | High | Future Windows ARM |

### Recommended Actions
1. Adopt NoirVisor's `selected_core` dispatch pattern
2. Create CVM (Common Virtual Machine) abstraction
3. Test on both Intel and AMD hardware

---

## PRIORITY ROADMAP

### Phase 1: Core Stealth (Critical)
1. CPUID hypervisor bit + VMX bit clearing
2. CPUID leaf 0x40000000 suppression
3. MSR bitmap for VMX capability MSRs
4. TSC hardware offsetting
5. Dual CPU (Intel/AMD) support

### Phase 2: Memory Stealth (Important)
6. EPT execute-only shadowing
7. Page splitting for fine-grained hooks
8. Driver memory hiding via EPT
9. Unified EPT/NPT abstraction

### Phase 3: Boot/Runtime (Important)
10. Full boot chain hooks (bootmgfw→winload→hvloader)
11. Payload injection into Hyper-V
12. Self-deletion after load

### Phase 4: Advanced Evasion (Nice to Have)
13. Timing attack mitigation
14. Performance counter hiding
15. SMBIOS/ACPI table sanitization
16. Kernel callback avoidance

### Phase 5: Hardening (Future)
17. Secure Boot integration
18. HVCI/VBS coexistence
19. Nested virtualization support

---

## TESTING REQUIREMENTS

### Detection Tools to Test Against
1. **pafish** - Paranoid Fish VM detection
2. **al-khaser** - Anti-debug/VM detection suite
3. **VMAware** - VM awareness library
4. **Custom timing probes** - RDTSC around instructions
5. **CPUID probes** - All relevant leaves

### Hardware Test Matrix
| Platform | CPU | Notes |
|----------|-----|-------|
| Intel Core 12th/13th Gen | Alder/Raptor Lake | P-core/E-core hybrid |
| Intel Core 10th/11th Gen | Comet/Tiger Lake | Common gaming CPUs |
| AMD Ryzen 5000/7000 | Zen 3/4 | SVM testing |
| AMD Ryzen 3000 | Zen 2 | Older SVM |

### Windows Version Matrix
| Version | Build | Notes |
|---------|-------|-------|
| Windows 11 23H2 | 22631 | Latest |
| Windows 11 22H2 | 22621 | Common |
| Windows 10 22H2 | 19045 | Legacy support |
| Windows 10 21H2 | 19044 | Anti-cheat common target |

---

**End of Gap Analysis**
