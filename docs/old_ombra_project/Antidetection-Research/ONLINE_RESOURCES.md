# OmbraHypervisor - Essential Online Resources

**Last Updated**: 2025-12-22  
**Purpose**: Curated technical resources for hypervisor development and antidetection

---

## 1. CRITICAL CONFERENCE PRESENTATIONS

### Black Hat Presentations

**Blue Pill** - Joanna Rutkowska (2006)
- URL: https://media.kasperskycontenthub.com/wp-content/uploads/sites/43/2008/08/20084218/BH-US-06-Rutkowska.pdf
- First hardware virtualization rootkit demonstration
- Claims 100% undetectability (disputed via timing attacks)
- Foundational work for hypervisor stealth research

**My Ticks Don't Lie: New Timing Attacks** (Black Hat EU 2020)
- URL: https://i.blackhat.com/eu-20/Thursday/eu-20-DElia-My-Ticks-Dont-Lie-New-Timing-Attacks-For-Hypervisor-Detection.pdf
- Advanced RDTSC timing detection methods
- Critical for understanding modern timing-based detection

**Attacking Hypervisors Using Firmware** (2015)
- Firmware perspective on hypervisor vulnerabilities
- BIOS/UEFI attack vectors

### OffensiveCon

**Growing Hypervisor 0day with Hyperseed** (2019)
- URL: https://github.com/microsoft/MSRC-Security-Research/blob/master/presentations/2019_02_OffensiveCon/2019_02%20-%20OffensiveCon%20-%20Growing%20Hypervisor%200day%20with%20Hyperseed.pdf
- Microsoft MSRC fuzzing research
- Vulnerability discovery automation

**Hypervisor Development for Security Analysis** (Training)
- URL: https://www.offensivecon.org/trainings/2023/hypervisor-development-for-security-analysis.html
- UEFI hypervisor development course
- VT-x/EPT configuration techniques

---

## 2. FOUNDATIONAL ACADEMIC PAPERS

**SubVirt: Implementing Malware with Virtual Machines** (2006)
- Authors: King, Chen (UMich); Wang, Verbowski, Lorch (Microsoft)
- URL: https://www.microsoft.com/en-us/research/wp-content/uploads/2016/02/subvirt.pdf
- First major VM-based rootkit (VMBR) research
- Defense strategies still relevant today

**CloudSkulk: Nested VM-Based Rootkit** (DSN 2021)
- URL: https://xgao-work.github.io/paper/dsn2021.pdf
- Modern nested virtualization rootkit
- L1 hypervisor detection techniques

**VIC: Evasive Video Game Cheating via VMI** (2025)
- URL: https://arxiv.org/html/2502.12322v1
- Game anti-cheat bypass using hypervisors
- Directly relevant to Ombra's use case

**Detecting Hardware-Assisted Virtualization** (DIMVA 2016)
- URL: https://www.christian-rossow.de/publications/detectvt-dimva2016.pdf
- Comprehensive VT-x/AMD-V detection methods

---

## 3. ESSENTIAL TECHNICAL BLOGS

### Secret Club (secret.club) - Anti-Cheat Research

**BattlEye Hypervisor Detection**
- URL: https://secret.club/2020/01/12/battleye-hypervisor-detection.html
- RDTSC/CPUID timing attacks (10x slower in VM)
- TSC offsetting countermeasures
- BattlEye uses VMProtect for self-protection

**How Anti-Cheats Detect System Emulation**
- URL: https://secret.club/2020/04/13/how-anti-cheats-detect-system-emulation.html
- Modern anti-cheat detection methods
- Practical evasion challenges

### Rayanfam Blog - Hypervisor From Scratch

**Complete Tutorial Series** (8 Parts)
- Part 1: https://rayanfam.com/topics/hypervisor-from-scratch-part-1/
- GitHub: https://github.com/SinaKarvandi/Hypervisor-From-Scratch
- Last Updated: August 2022 (complete revision)
- Covers: VMX detection, VMCS, EPT, VM-exit handling
- **Recommended starting point for learning**

### Trustwave SpiderLabs

**Hypervisor Development in Rust** (2024)
- URL: https://www.trustwave.com/en-us/resources/blogs/spiderlabs-blog/hypervisor-development-in-rust-for-security-researchers-part-1/
- Modern Rust implementations (Illusion, Matrix)
- Lowered barrier to entry for game hacking
- Anti-cheat adaptation strategies

### Hexacorn Blog

**Protecting VMware from CPUID Detection** (2014)
- URL: https://www.hexacorn.com/blog/2014/08/25/protecting-vmware-from-cpuid-hypervisor-detection/
- VMX config: `cpuid.1.ecx="0---:----:----:----:----:----:----:----"`
- Masks hypervisor present bit (ECX[31])

### Medium - CPUID Detection

**Detect VM Environment Using CPUID**
- URL: https://medium.com/@andreabocchetti88/detect-vm-environment-using-cpuid-f461e89f81c0
- CPUID leaf 0x1, ECX bit 31 detection
- Hypervisor vendor signatures (VMware, Hyper-V, KVM)

### GuidedHacking Forums

**Hypervisor, Anti-Cheats, and What's in Between**
- URL: https://guidedhacking.com/threads/hypervisor-anti-cheats-and-whats-in-between.16190/
- "Use hypervisor to reverse anticheat, not as bypass"
- #DB exception boundary detection
- Community insights on open-source hypervisor signatures

---

## 4. OFFICIAL DOCUMENTATION

### Intel Software Developer Manual (SDM)

**Main Portal**: https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html

**Critical Volumes**:
- **Volume 3C**: Primary VMX documentation
  - Chapter 23.6: Discovering VMX support
  - Chapter 24.3: VMCS control area fields
  - VMX transitions and address translation
- **Volume 3B**: VMCS structure (Chapter 20)
- **Appendix A**: VMX Capability Reporting (MSRs)
- **Appendix H**: VMREAD/VMWRITE field encodings

**Key Concepts**:
- CPUID.1:ECX.VMX[bit 5] = 1 indicates VMX support
- CR4.VMXE[bit 13] = 1 to enable VMX
- EPT (Extended Page Tables) architecture
- VPID (Virtual Processor Identifiers)

### AMD Architecture Programmer's Manual (APM)

**Main Portal**: https://docs.amd.com

**Critical Volumes**:
- **Volume 2 (24593)**: https://docs.amd.com/v/u/en-US/24593_3.43
  - PRIMARY SVM DOCUMENTATION
  - SVM Hardware Overview
  - VMRUN, VMSAVE, VMLOAD instructions
  - #VMEXIT and Intercept Operation
  - NPT (Nested Page Tables)
- **SVM Architecture Reference (33047)**: http://www.0x04.net/doc/amd/33047.pdf
  - External DMA protection
  - Guest/host tagged TLB
  - Virtual interrupt injection

**Educational Resource**:
- SimpleSvm: https://github.com/tandasat/SimpleSvm (Windows AMD hypervisor)

### Microsoft Hypervisor TLFS

**Official Docs**: https://learn.microsoft.com/en-us/virtualization/hyper-v-on-windows/tlfs/tlfs

**GitHub PDFs**:
- v5.0C: https://github.com/MicrosoftDocs/Virtualization-Documentation/blob/main/tlfs/Hypervisor%20Top%20Level%20Functional%20Specification%20v5.0C.pdf
- Describes HV#1 interface, partition architecture, hypercalls

**HDK (Hyper-V Development Kit)**:
- URL: https://ionescu007.github.io/hdk/
- Updated HvGdk.h header with TLFS 5.0c definitions

---

## 5. FORUMS & COMMUNITIES

### OSR Developer Community
- URL: https://community.osr.com/
- Windows driver development, WDK, kernel debugging
- 50+ articles on drivers and minifilters
- Hypervisor debugging discussions

### Reddit

**r/ReverseEngineering**
- URL: https://www.reddit.com/r/ReverseEngineering/
- 110,000 members
- Binary reverse engineering, moderated discussions

**r/netsec**
- URL: https://www.reddit.com/r/netsec/
- Technical security news
- Community tools and scripts

**Related Info**: Hypervisor rootkits (Ring -1), SubVirt/Blue Pill discussions, MITRE ATT&CK T1014

### UnknownCheats (Game Hacking)
- Hypervisor use in cheating community
- EPT/EPTP switching techniques
- Open-source hypervisor modifications
- Contributors: @namazso, @Daax, @iPower, @vmcall

### Proxmox Forums

**Anti-Cheat KVM Settings**
- URL: https://forum.proxmox.com/threads/anti-cheat-kvm-settings.121249/
- CPU args: `-cpu host,-hypervisor,kvm=off`
- LSI SCSI controller, realistic MAC addresses

**RDTSC VM EXIT Checks**
- URL: https://forum.proxmox.com/threads/preventing-vm-detection-from-rdtsc-vm-exit-checks.73669/
- Timing attack mitigation

---

## 6. ESSENTIAL TESTING TOOLS

### Pafish (Paranoid Fish)

**GitHub**: https://github.com/a0rtega/pafish  
**Relevance**: HIGH - Primary testing tool

**Detection Categories**:
- VirtualBox, VMware, Bochs, QEMU, KVM, Xen, Hyper-V
- Sandboxie, Wine, Cuckoo Sandbox
- 10 generic sandbox checks

**Techniques**:
- RDTSC timing (VM exit overhead)
- CPUID execution time
- PEB debugger detection
- SCSI identifiers, BIOS version
- MAC address patterns (VBox: 08:00:27, VMware: 00:0C:29/00:1C:14/00:50:56)
- Power states (S1-S4 absent in VMs)
- Registry artifacts

**Use for Ombra**: Verify all detection vectors bypassed

### al-khaser

**GitHub**: https://github.com/ayoubfaouzi/al-khaser  
**Relevance**: HIGH - Comprehensive test suite

**Categories** (50+ checks):
- DEBUG, TLS, ANTI_DISASSM, DUMPING_CHECK
- VBOX, VMWARE, VPC, QEMU, KVM, XEN, HYPERV, PARALLELS, WINE
- GEN_SANDBOX, ANALYSIS_TOOLS, TIMING_ATTACKS
- INJECTION, CODE_INJECTIONS

**Techniques**:
- IsDebuggerPresent(), CheckRemoteDebuggerPresent()
- CloseHandle() exception (EXCEPTION_INVALID_HANDLE)
- Power state enumeration
- Disk space tricks

**Use for Ombra**: Comprehensive benchmark (50+ methods)

### VMAware

**GitHub**: https://github.com/kernelwernel/VMAware  
**Relevance**: HIGH - Most comprehensive library

**Capabilities**:
- ~100 distinct detection techniques
- Identifies 70+ VM brands
- Scoring system (0-100)
- Multi-level caching (memoization)

**Sample Techniques**:
- VM::GPU_CHIPTYPE, VM::DRIVER_NAMES, VM::VBOX_IDT
- VM::HDD_SERIAL, VM::ACPI_HYPERV, VM::VMWARE_DEVICES
- VM::IDT_GDT_MISMATCH, VM::PROCESSOR_NUMBER
- VM::WMI_MODEL, VM::POWER_CAPABILITIES

**Use for Ombra**: If bypasses VMAware, bypasses most real-world detection

### CloakBox (2025)

**GitHub**: https://github.com/Batlez/CloakBox  
**Relevance**: HIGH - Modern VM bypass

**Purpose**: Custom VirtualBox fork bypassing proctoring/anti-cheat  
**Claimed Bypasses**: Examity, Respondus, Safe Exam Browser, ProctorU, Pearson

**Use for Ombra**: Study 2025 bypass techniques

---

## 7. KEY GITHUB PROJECTS

### Educational Hypervisors

**SimpleVisor** - Alex Ionescu
- GitHub: https://github.com/ionescu007/SimpleVisor
- Intel VT-x, Windows/UEFI
- 500 LOC C + 10 LOC assembly
- Exhaustive comments, best starting point

**SimpleSvm** - Satoshi Tanda
- GitHub: https://github.com/tandasat/SimpleSvm
- AMD SVM, Windows
- Companion to SimpleVisor for AMD

**HyperPlatform** - Satoshi Tanda
- GitHub: https://github.com/tandasat/HyperPlatform
- 1.7k stars, production-quality
- Windows 7/8.1/10, VT-x + EPT
- Good for VM-exit handling patterns

**HyperBone** - DarthTon
- GitHub: https://github.com/DarthTon/HyperBone
- VT-x with hooks, EPT TLB splitting
- MSR/IDT hooks
- Known issues on Win10 15063+ (KeCheckStackAndTargetAddress BSOD)

**hvpp** - wbenny
- GitHub: https://github.com/wbenny/hvpp
- Modern C++17, OOP design
- EPT identity mapping (2MB pages)
- More polished than HyperPlatform

**gbhv** - Gbps
- GitHub: https://github.com/Gbps/gbhv
- Full EPT hooking support
- Blue-Pill technique
- Security research focused

**Hypervisor From Scratch** - Sina Karvandi
- GitHub: https://github.com/SinaKarvandi/Hypervisor-From-Scratch
- Blog: https://rayanfam.com/tutorials/
- Complete 8-part tutorial
- Revised August 2022

### Specialized Hypervisors

**illusion-rs** - memN0ps
- GitHub: https://github.com/memN0ps/illusion-rs
- Windows UEFI Blue Pill Type-1 (Rust)
- Template for hypervisor development

**matrix-rs** - memN0ps
- GitHub: https://github.com/memN0ps/matrix-rs
- Windows Kernel Blue Pill Type-2 (Rust)
- Educational template

**CoVirt** - Nadharm
- GitHub: https://github.com/Nadharm/CoVirt
- Linux kernel rootkit (AMD-V)
- Dynamically loadable, "lifts" kernel

### Resource Collections

**Comprehensive Hypervisor Resources** - husseinmuhaisen
- GitHub: https://github.com/husseinmuhaisen/Hypervisor
- Intel EPT details, Hyper-V exploitation
- Detection methods, VirtualBox exploits
- Hyper-V internals history (2006-2024)

**Awesome Hyper-V Exploitation** - shogunlab
- GitHub: https://github.com/shogunlab/awesome-hyper-v-exploitation
- Fuzzing para-virtualized devices
- VM Worker Process attacks
- Hypercall fuzzing techniques

---

## 8. CRITICAL DETECTION TECHNIQUES

### CPUID-Based Detection

**Hypervisor Present Bit** (CPUID.1:ECX[31])
- Set (1) = Hypervisor present, Clear (0) = Physical hardware
- **VMware Bypass**: `cpuid.1.ecx="0---:----:----:----:----:----:----:----"` or `hypervisor.cpuid.v0="FALSE"`
- **VirtualBox Bypass**: Paravirtualization Interface = None
- **Ombra**: HIGH priority - must handle

**Vendor Identification** (CPUID.0x40000000)
- VMware: "VMwareVMware", Hyper-V: "Microsoft Hv", KVM: "KVMKVMKVM"
- **Ombra**: Return benign values or suppress

**CPUID Exit Overhead** (Timing Attack)
- RDTSC; CPUID; RDTSC measurement
- Physical: ~200 cycles, VM: ~2000+ cycles (10x slower)
- Used by BattlEye, EasyAntiCheat, GuLoader
- **Countermeasures**: TSC offsetting, decrease TSC on CPUID exec
- **Ombra**: HIGH priority - critical for anti-cheat

### RDTSC-Based Detection

**Basic Timing Loops**
- Measure operations, detect higher values in VM
- Variants: RDTSC+CPUID, RDTSC+GetProcessHeap+CloseHandle
- **Countermeasures**: TSC control, subtract VM exit overhead
- **Challenge**: Cannot fully hide hypervisor, deep kernel tweaks required
- **Ombra**: HIGH priority

**Advanced Timing**
- Cache timing (L1/L2/L3), System call timing, TLB/RSB patterns
- Reference: "My Ticks Don't Lie" paper
- **Ombra**: MEDIUM-HIGH priority

### Hardware Detection

**MAC Addresses**: VBox (08:00:27), VMware (00:0C:29/00:1C:14/00:50:56)  
**BIOS/SMBIOS**: Check for "VMware", "VirtualBox", "QEMU", "Microsoft Corporation"  
**Disk/Storage**: Virtual controller IDs, serial patterns  
**Power Management**: S1-S4 states, thermal control, CPU fans (absent in VMs)  
**GPU**: Virtual GPU chiptype/names

### System Behavior

**Exception Handling** (#DB, #UD)
- #DB must deliver on exact instruction boundary with EFLAGS.TF set
- Anti-cheat systems check open-source hypervisor signatures
- **Ombra**: HIGH priority - must handle correctly

**MSR Detection**
- IA32_FEATURE_CONTROL, IA32_VMX_*, IA32_EFER
- EasyAntiCheat queries IA32_EFER after ~30 min
- **Ombra**: MEDIUM-HIGH priority

**VMX/SVM Instructions**
- VMXON, VMREAD, VMWRITE (Intel); VMRUN, VMSAVE, VMLOAD (AMD)
- EAC uses VMREAD on init (exception handler wrapped)
- **Bypass**: Inject #UD when VMM traps
- **Ombra**: HIGH priority

### Anti-Cheat Specific

**BattlEye**:
- RDTSC/CPUID timing (kernel mode)
- VMProtect protection
- Susceptible to time-forging
- **Ombra**: HIGH priority

**EasyAntiCheat**:
- VMREAD check, RDTSC/CPUID timing, IA32_EFER query
- More sophisticated than BattlEye
- **Bypass**: Inject #UD on vmread, proper TSC emulation
- **Ombra**: HIGH priority

**Advanced** (ESEA, FACEIT, B5, eSportal):
- More aggressive hypervisor checks
- Better success catching hypervisor-based cheats
- **Ombra**: HIGH priority for future-proofing

---

## 9. QUICK REFERENCE

### CPUID Leaves
- 0x0: Vendor ID
- 0x1: Feature flags (ECX[31] = hypervisor bit)
- 0x40000000: Hypervisor vendor
- 0x40000001+: Hypervisor info

### Key MSRs
- IA32_FEATURE_CONTROL (0x3A): VMX enable
- IA32_VMX_BASIC (0x480): VMX info
- IA32_VMX_* (0x480-0x491): VMX caps
- IA32_LSTAR (0xC0000082): Syscall entry

### VMCS Fields
- GUEST_*: Guest state
- HOST_*: Host state
- VM_EXEC_*: Execution control
- VM_EXIT_*: Exit control
- VM_ENTRY_*: Entry control

---

## 10. OMBRA TESTING CHECKLIST

**Must Pass**:
- [ ] Pafish (all checks)
- [ ] al-khaser (all 50+ categories)
- [ ] VMAware (100 techniques)
- [ ] BattlEye timing attacks
- [ ] EasyAntiCheat VMREAD/timing
- [ ] No CPUID hypervisor bit
- [ ] No suspicious timing deltas
- [ ] Proper #DB exception boundary
- [ ] No registry artifacts
- [ ] No driver/process artifacts
- [ ] BIOS/SMBIOS spoofed
- [ ] Realistic MAC address
- [ ] Realistic power management

**Must Implement**:
- TSC management (critical)
- CPUID spoofing (leaves 0x1, 0x40000000)
- Exception handling (#DB, #UD, #GP, #PF)
- EPT/NPT for memory virtualization
- MSR handling (VMX/SVM-related)

**Must Avoid**:
- Open-source code signatures
- VMware/VirtualBox/Hyper-V artifacts
- Guest tools/integration services
- Predictable timing patterns
- Exception boundary failures

---

## 11. RECOMMENDED LEARNING PATH

**Phase 1: Fundamentals**
1. Intel SDM Volume 3C (VMX chapters)
2. AMD APM Volume 2 (SVM chapters)
3. SimpleVisor + SimpleSvm source code

**Phase 2: Implementation**
1. "Hypervisor From Scratch" series (all 8 parts)
2. HyperPlatform (VM-exit handling)
3. hvpp (modern C++ patterns)
4. gbhv (EPT hooking)

**Phase 3: Detection & Evasion**
1. Pafish source code
2. al-khaser checks
3. VMAware testing
4. Secret Club blog posts

**Phase 4: Anti-Cheat**
1. BattlEye detection analysis
2. EasyAntiCheat techniques
3. GuidedHacking/UnknownCheats discussions
4. Real anti-cheat testing (controlled)

**Phase 5: Advanced**
1. Microsoft TLFS
2. Academic papers (SubVirt, Blue Pill, CloudSkulk)
3. Black Hat/DEF CON presentations
4. Nested virtualization countermeasures

**Phase 6: Production**
1. Comprehensive timing mitigation
2. Hardware identifier spoofing
3. Exception handling correctness
4. Full tool suite testing
5. Performance benchmarking
6. Secure unload/cleanup

---

**End of Essential Resources**

For the full comprehensive version with 100+ additional resources, expanded explanations, and complete source listings, see ONLINE_RESOURCES_EXTENDED.md (if generated).
