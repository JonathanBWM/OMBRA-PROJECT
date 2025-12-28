# GitHub OSINT Research: Hypervisor Antidetection Repositories
**Research Date:** December 22, 2025
**Focus:** C/C++/ASM repositories with actual implementation code for hypervisor stealth and antidetection

---

## HIGH PRIORITY REPOS (Most Relevant to OmbraHypervisor)

### 1. **can1357/ByePg**
- **URL:** https://github.com/can1357/ByePg
- **Stars:** ~896 | **Forks:** ~183
- **Last Activity:** Active (multiple issues in 2025)
- **Language:** C++
- **Description:** Universal PatchGuard defeat for Windows 8/8.1/10 regardless of HVCI. Creates entirely new attack surface with exception-based hooking.
- **Key Techniques:**
  - Hijacks HalPrivateDispatchTable for early-bugcheck hook
  - Works with HVCI enabled (operates at exception level)
  - System-wide exception handler registration
  - Bypasses PatchGuard checks entirely without disabling
- **Relevance to Ombra:** Critical - provides universal PatchGuard bypass that works even with HVCI. Exception-based hooking is a novel approach that OmbraHypervisor should implement.

### 2. **Air14/HyperHide**
- **URL:** https://github.com/Air14/HyperHide
- **Stars:** ~1,500 | **Forks:** ~333
- **Last Activity:** December 2023 release, active issues through May 2025
- **Language:** C++
- **Description:** Hypervisor-based anti-anti-debug plugin for x64dbg. Uses Intel VT-x to hide debugging activity.
- **Key Techniques:**
  - KUserSharedData page swap with PFN manipulation
  - NtQuerySystemInformation hooking for kernel debugger queries
  - Process filtering and parent PID spoofing
  - EPT-based memory isolation for debug artifacts
  - Timestamp counter manipulation to hide performance anomalies
- **Relevance to Ombra:** Very High - demonstrates practical anti-detection at hypervisor level. KUserSharedData manipulation and timestamp hiding are essential for Ombra's antidetection.

### 3. **ldpreload/BlackLotus**
- **URL:** https://github.com/ldpreload/BlackLotus
- **Stars:** ~2,100 | **Forks:** ~475
- **Last Activity:** 2024
- **Language:** C
- **Description:** UEFI Windows Bootkit with Secure Boot bypass and Ring0/Kernel protection. First real-world UEFI bootkit to bypass Secure Boot.
- **Key Techniques:**
  - Secure Boot bypass via SHIM loader replacement (v2 uses bootlicker)
  - UEFI DXE phase hooking
  - ExitBootServices interception
  - HTTP-based C2 loader with kernel-level persistence
  - Anti-removal protections at Ring 0
- **Relevance to Ombra:** High - Type-1 hypervisor techniques and Secure Boot bypass methods are directly applicable. UEFI-level initialization provides earliest possible hook point.

### 4. **Mattiwatti/EfiGuard**
- **URL:** https://github.com/Mattiwatti/EfiGuard
- **Stars:** ~2,000+
- **Last Activity:** 2023
- **Language:** C
- **Description:** Portable x64 UEFI bootkit that patches Windows boot manager, boot loader and kernel at boot time to disable PatchGuard and DSE.
- **Key Techniques:**
  - UEFI boot-time patching (pre-kernel initialization)
  - PatchGuard context disabling before initialization
  - Driver Signature Enforcement (DSE) bypass
  - Supports Windows Vista SP1 through Windows 11
  - Works independently from HVCI (but cannot disable HVCI itself)
- **Relevance to Ombra:** High - Boot-time initialization is ideal for Type-1 hypervisor. PatchGuard and DSE bypass techniques complement Ombra's kernel protection needs.

### 5. **memN0ps/illusion-rs**
- **URL:** https://github.com/memN0ps/illusion-rs
- **Stars:** ~72+
- **Last Activity:** 2024
- **Language:** Rust
- **Description:** Windows UEFI Blue Pill Type-1 Hypervisor in Rust. More stable and feature-complete than matrix-rs.
- **Key Techniques:**
  - UEFI-based Type-1 hypervisor initialization
  - EPT with MTRR support
  - VM Exit handling (ExceptionOrNmi, Cpuid, Vmcall, etc.)
  - PatchGuard-compatible breakpoint hooks (INT3)
  - Hidden SSDT function hooks
  - Memory-safe Rust implementation with unsafe FFI for hardware access
- **Relevance to Ombra:** Very High - Type-1 hypervisor design is directly applicable. Rust implementation provides reference for memory-safe hypervisor patterns that could be adapted to C++.

### 6. **memN0ps/matrix-rs**
- **URL:** https://github.com/memN0ps/matrix-rs
- **Stars:** ~283+
- **Last Activity:** 2024
- **Language:** Rust
- **Description:** Windows Kernel Blue Pill Type-2 Hypervisor in Rust. Older experimental version of illusion-rs.
- **Key Techniques:**
  - Dynamic virtualization of running Windows system
  - EPT-based memory management with MTRR
  - PatchGuard-compatible INT3 hooks
  - Hidden kernel inline hooks
  - SSDT hooking for system call interception
  - VT-x with EPT support
- **Relevance to Ombra:** High - Type-2 blue-pill technique for dynamic virtualization. Kernel inline hooking methods are useful reference for Ombra's runtime hook installation.

### 7. **changeofpace/VivienneVMM**
- **URL:** https://github.com/changeofpace/VivienneVMM
- **Stars:** ~300+
- **Last Activity:** 2020
- **Language:** C++
- **Description:** Stealthy debugging framework via Intel VT-x hypervisor. Implements EPT breakpoints and hardware breakpoint facade.
- **Key Techniques:**
  - EPT breakpoints (undetectable from guest)
  - Debug register facade (fake debug registers in VMX root mode)
  - MovDr VM exit handler to intercept debug register access
  - IPI broadcast for synchronized debug register modification
  - Hardware breakpoint manager (HBM) operating in VMX root mode
  - Based on HyperPlatform
- **Relevance to Ombra:** Very High - EPT breakpoint implementation and debug register facade are essential for stealthy debugging/analysis features. Debug register hiding prevents detection.

### 8. **KelvinMsft/kHypervisor**
- **URL:** https://github.com/KelvinMsft/kHypervisor
- **Stars:** ~500+
- **Last Activity:** 2017-2018
- **Language:** C
- **Description:** Lightweight bluepill-like nested VMM for Windows. Extends HyperPlatform with nested virtualization support.
- **Key Techniques:**
  - Nested virtualization (L0, L1, L2 support)
  - VMEntry parameter validation matching hardware spec
  - VMEntry emulation with VMCS state checking
  - Nested EPT support for Windows 7 x64+
  - L0 monitoring of guest EPT PTE modifications
  - Unrestricted guest support (virtual 8086 mode)
- **Relevance to Ombra:** High - Nested virtualization allows Ombra to run inside another hypervisor (VMware/Hyper-V) for testing. VMEntry emulation helps debug VMCS issues.

### 9. **Shtan7/Hypervisor**
- **URL:** https://github.com/Shtan7/Hypervisor
- **Stars:** ~150+
- **Last Activity:** 2022
- **Language:** C++
- **Description:** Type-2 thin hypervisor with stealth hook capabilities using EPT memory shadowing.
- **Key Techniques:**
  - EPT-based MMIO emulation
  - Memory page cloning for hook protection
  - Read/Write protection with execute permissions intact
  - PatchGuard evasion via page swapping on EPT violations
  - Temporarily swaps pages during integrity checks
  - No test mode required (works with driver signature enforcement)
- **Relevance to Ombra:** Very High - Page cloning and swap-on-read technique is elegant solution for PatchGuard evasion. MMIO emulation pattern useful for hardware interaction hiding.

### 10. **Shtan7/UEFI-hypervisor**
- **URL:** https://github.com/Shtan7/UEFI-hypervisor
- **Stars:** ~100+
- **Last Activity:** 2022
- **Language:** C++
- **Description:** Type-1 thin hypervisor loaded via UEFI. Requires Intel VT-x and EPT.
- **Key Techniques:**
  - UEFI-based Type-1 hypervisor initialization
  - COM port debugging support
  - VMWare gdb stub integration for IDA Pro debugging
  - Windows 11 guest support
  - UEFI Shell driver loading
- **Relevance to Ombra:** High - Type-1 initialization pattern. COM port debugging technique useful for early-stage hypervisor debugging when standard debuggers unavailable.

---

## MEDIUM PRIORITY REPOS (Partial Relevance)

### 11. **zer0condition/ZeroHVCI**
- **URL:** https://github.com/zer0condition/ZeroHVCI
- **Stars:** ~100+
- **Last Activity:** 2024
- **Language:** C++
- **Description:** Achieves arbitrary kernel read/write/function calling in HVCI-protected environments without admin or kernel drivers.
- **Key Techniques:**
  - KernelForge abuse for HVCI-compliant function calling
  - ROP chain construction without patching
  - CVE-2024-26229 exploit (csc.sys IOCTL METHOD_NEITHER)
  - CVE-2024-35250 exploit (ks.sys RtlClearAllBits)
  - Thread execution hierarchy abuse
- **Relevance to Ombra:** Medium-High - HVCI bypass techniques useful for environments where HVCI is enforced. ROP-based approach avoids direct code injection.

### 12. **tandasat/CVE-2024-21305**
- **URL:** https://github.com/tandasat/CVE-2024-21305
- **Stars:** ~200+
- **Last Activity:** 2024
- **Language:** C
- **Description:** HVCI security feature bypass vulnerability report and PoC. Non-secure HVCI configuration allowed arbitrary kernel code execution.
- **Key Techniques:**
  - HVCI configuration vulnerability exploitation
  - Arbitrary kernel-mode code execution in root partition
  - Bypasses HVCI within VTL 0 kernel
  - Analyzed by Windows Core OS engineer Andrea Allievi
- **Relevance to Ombra:** Medium - Historical vulnerability useful for understanding HVCI weaknesses. May inform future bypass research but vulnerability is patched.

### 13. **hzqst/VmwareHardenedLoader**
- **URL:** https://github.com/hzqst/VmwareHardenedLoader
- **Stars:** ~800+
- **Last Activity:** 2023
- **Language:** C++
- **Description:** VMware VM detection mitigation loader (anti-anti-VM). Bypasses VMProtect 3.2, Safengine, Themida detection.
- **Key Techniques:**
  - Runtime SystemFirmwareTable patching
  - Removes "VMware", "Virtual", "VMWARE" signatures
  - SMBIOS spoofing
  - Hardware signature removal
- **Relevance to Ombra:** Medium - Demonstrates practical VM hiding but specific to VMware. Techniques applicable to hiding Ombra's own VMX presence from guest detection.

### 14. **KonBuku/VMDOG**
- **URL:** https://github.com/KonBuku/VMDOG
- **Stars:** ~200+
- **Last Activity:** 2024
- **Language:** Configuration/Scripts
- **Description:** VMware spoofing toolkit bypassing VMProtect and SafeExam Browser detection.
- **Key Techniques:**
  - VMX configuration tweaks (hypervisor.cpuid.v0 = FALSE)
  - SMBIOS reflection and hardware spoofing
  - Firmware detail modification
  - Monitor control spoofing
- **Relevance to Ombra:** Medium - VMX file configuration for hiding hypervisor bit. CPUID spoofing approaches useful for guest-facing CPUID emulation.

### 15. **WCharacter/RDTSC-KVM-Handler**
- **URL:** https://github.com/WCharacter/RDTSC-KVM-Handler
- **Stars:** ~150+
- **Last Activity:** 2023
- **Language:** C
- **Description:** Linux kernel patches to spoof RDTSC and make VM exits undetected.
- **Key Techniques:**
  - Intel: vmx.c handle_rdtsc function modification
  - AMD: svm.c handle_rdtsc_interception modification
  - TSC difference divider manipulation
  - VM-exit time spoofing
- **Relevance to Ombra:** Medium-High - RDTSC timing attack mitigation essential for stealth. TSC offsetting prevents timing-based hypervisor detection via CPUID/RDTSC gaps.

### 16. **SamuelTulach/BetterTiming**
- **URL:** https://github.com/SamuelTulach/BetterTiming
- **Stars:** ~50+
- **Last Activity:** 2023
- **Language:** C
- **Description:** PoC TSC offsetting in KVM to bypass anti-VM timing checks.
- **Key Techniques:**
  - VM-exit time measurement and storage
  - TSC offset calculation and application
  - Pafish CPU timing check bypass
  - vmcheck kernel-mode driver evasion
- **Relevance to Ombra:** Medium - Practical TSC offset implementation. Demonstrates successful bypass of common timing-based detection tools.

### 17. **h33p/kvm-rdtsc-hack**
- **URL:** https://github.com/h33p/kvm-rdtsc-hack
- **Stars:** ~30+
- **Last Activity:** 2022
- **Language:** C
- **Description:** Kernel module to evade KVM detection through RDTSC timer.
- **Key Techniques:**
  - Constant TSC offset configuration (default 1000)
  - AMD Ryzen optimization (~1600 offset optimal)
  - RDTSC interception and manipulation
- **Relevance to Ombra:** Medium - Platform-specific TSC offset tuning. AMD Ryzen offset values useful for AMD-V implementation.

### 18. **bitdefender/hvmi**
- **URL:** https://github.com/bitdefender/hvmi
- **Stars:** ~800+
- **Last Activity:** 2021
- **Language:** C
- **Description:** Bitdefender Hypervisor Memory Introspection Core Library. Commercial-grade VMI implementation.
- **Key Techniques:**
  - Guest VM state and behavior analysis from outside
  - Virtualization extension leverage for security
  - Windows 7-10, Ubuntu 18.04, CentOS 8 support
  - Hypervisor-assisted introspection
- **Relevance to Ombra:** Medium - Reference for VMI techniques Ombra needs to evade. Understanding introspection methods helps develop counter-introspection.

### 19. **libvmi/libvmi**
- **URL:** https://github.com/libvmi/libvmi
- **Stars:** ~1,000+
- **Last Activity:** Active 2025
- **Language:** C
- **Description:** Virtual Machine Introspection library for accessing VM memory. Industry standard VMI library.
- **Key Techniques:**
  - Physical and virtual address memory access
  - Kernel symbol resolution
  - Memory event notifications (exec/write/read)
  - EPT-based memory events (Xen only)
  - Hypervisor-agnostic API (Xen, KVM)
- **Relevance to Ombra:** Medium - Another VMI reference for evasion development. Memory event system shows what hypervisor features can expose guest activity.

### 20. **vmi-rs/vmi**
- **URL:** https://github.com/vmi-rs/vmi
- **Stars:** ~100+
- **Last Activity:** 2024
- **Language:** Rust
- **Description:** Modular and extensible VMI library in Rust. Modern type-safe VMI framework.
- **Key Techniques:**
  - Type-safe memory access (Va, Pa, Gfn types)
  - Configurable caching for page lookups and VA-to-PA translation
  - Built-in Windows and Linux OS support
  - Hypervisor-agnostic design (currently Xen-focused)
- **Relevance to Ombra:** Medium - Modern VMI approach in Rust. Type-safe patterns could inform Ombra's memory management design.

### 21. **KVM-VMI/kvm-vmi**
- **URL:** https://github.com/KVM-VMI/kvm-vmi
- **Stars:** ~200+
- **Last Activity:** 2023
- **Language:** C
- **Description:** KVM-based Virtual Machine Introspection implementation.
- **Key Techniques:**
  - Guest execution context understanding from hardware state
  - KVM-specific VMI hooks
  - Memory introspection via EPT
- **Relevance to Ombra:** Medium - KVM VMI implementation details. Shows Linux hypervisor introspection approach that Ombra needs to evade.

### 22. **HyperDbg/HyperDbg**
- **URL:** https://github.com/HyperDbg/HyperDbg
- **Stars:** ~2,500+
- **Last Activity:** Active 2025
- **Language:** C++
- **Description:** State-of-the-art hypervisor-assisted debugger. Uses Intel VT-x and EPT for user/kernel debugging.
- **Key Techniques:**
  - EPT-based execution monitoring (no API dependencies)
  - Kernel and user mode debugging from Ring -1
  - Second Layer Page Table monitoring
  - Transparent debugging (no guest API hooks)
  - Harder to detect than traditional debuggers
- **Relevance to Ombra:** Medium - Advanced debugging tool Ombra needs to evade. EPT monitoring shows what sophisticated tools can observe.

### 23. **jonomango/hv**
- **URL:** https://github.com/jonomango/hv
- **Stars:** ~100+
- **Last Activity:** 2023
- **Language:** C++
- **Description:** Lightweight Intel VT-x hypervisor following Intel SDM closely to evade detection.
- **Key Techniques:**
  - Strict Intel SDM compliance to avoid common bugs
  - Improper VM-exit handling evasion
  - Timing check mitigation (partial)
  - Minimal attack surface design
- **Relevance to Ombra:** Medium - Demonstrates importance of SDM compliance for stealth. Common hypervisor bugs enable detection, so strict correctness improves stealth.

### 24. **gamozolabs/orange_slice**
- **URL:** https://github.com/gamozolabs/orange_slice
- **Stars:** ~400+
- **Last Activity:** 2023
- **Language:** Rust
- **Description:** Research kernel and hypervisor for deterministic emulation with minimal performance cost.
- **Key Techniques:**
  - Deterministic hypervisor design
  - Bootloader, kernel, hypervisor in Rust
  - Stage0 assembly only
  - Deterministic execution guarantee
- **Relevance to Ombra:** Low-Medium - Research-focused deterministic design. Useful for understanding deterministic execution but not directly applicable to stealth.

### 25. **kukrimate/grr**
- **URL:** https://github.com/kukrimate/grr
- **Stars:** ~50+
- **Last Activity:** 2022
- **Language:** C
- **Description:** GRUB Replacing Rootkit - AMD SVM bare metal hypervisor PoC.
- **Key Techniques:**
  - AMD SVM implementation
  - UEFI runtime driver format
  - exit_boot_services hook in UEFI system table
  - Full hypervisor control before OS boot
- **Relevance to Ombra:** Medium - AMD SVM Type-1 hypervisor reference. UEFI hooking approach useful for AMD-V variant of Ombra.

### 26. **SamuelTulach/memhv**
- **URL:** https://github.com/SamuelTulach/memhv
- **Stars:** ~80+
- **Last Activity:** 2023
- **Language:** C++
- **Description:** Minimalistic AMD-V/SVM hypervisor with memory introspection.
- **Key Techniques:**
  - AMD-V/SVM support
  - Nested Page Tables (NPT)
  - Hypervisor memory hidden from guest via NPT
  - Memory introspection capabilities
- **Relevance to Ombra:** Medium - AMD-V reference implementation. NPT-based memory hiding applicable to AMD variant.

### 27. **Nadharm/CoVirt**
- **URL:** https://github.com/Nadharm/CoVirt
- **Stars:** ~50+
- **Last Activity:** 2022
- **Language:** C
- **Description:** Dynamically loadable VM-based rootkit for Linux Kernel v5.13.0 using AMD-V (SVM).
- **Key Techniques:**
  - Dynamic kernel virtualization ("lifting" kernel)
  - AMD-V SVM implementation
  - Hypervisor-level malicious services (not kernel-level)
  - Linux kernel 5.13.0 specific
- **Relevance to Ombra:** Medium - Linux AMD-V blue-pill technique. Dynamic virtualization approach useful reference for runtime hypervisor installation.

### 28. **hrbust86/HookMsrBySVM**
- **URL:** https://github.com/hrbust86/HookMsrBySVM
- **Stars:** ~20+
- **Last Activity:** 2021
- **Language:** C
- **Description:** Hook MSR (Model Specific Registers) using AMD SVM.
- **Key Techniques:**
  - MSR interception via SVM
  - MSR access VM-exit handling
  - AMD-V specific MSR control
- **Relevance to Ombra:** Medium - MSR hooking for AMD. Useful for intercepting RDMSR/WRMSR on AMD-V systems.

### 29. **DarthTon/HyperBone**
- **URL:** https://github.com/DarthTon/HyperBone
- **Stars:** ~500+
- **Last Activity:** 2020
- **Language:** C
- **Description:** Minimalistic VT-x hypervisor with hooks. Supports IDT hooks, Intel VT-x and EPT.
- **Key Techniques:**
  - IDT (Interrupt Descriptor Table) hooking
  - EPT-based hooks
  - Intel VT-x with EPT support
  - Windows 7-10 x64 support
  - Minimalistic codebase for learning
- **Relevance to Ombra:** Medium - IDT hooking technique useful for interrupt interception. Minimalistic design good reference for core hypervisor features.

### 30. **noahbean33/hypervisor**
- **URL:** https://github.com/noahbean33/hypervisor
- **Stars:** ~30+
- **Last Activity:** 2024
- **Language:** C++
- **Description:** Windows kernel-mode hypervisor using Intel VT-x (VMX). Educational implementation.
- **Key Techniques:**
  - VM Exit handling (CPUID, MSR access, I/O)
  - EPT for second-level address translation
  - Memory isolation via EPT
  - Complete working hypervisor for learning
- **Relevance to Ombra:** Low-Medium - Educational reference. Basic hypervisor structure useful for onboarding new developers to codebase.

### 31. **rohaaan/hypervisor-for-beginners**
- **URL:** https://github.com/rohaaan/hypervisor-for-beginners
- **Stars:** ~100+
- **Last Activity:** 2023
- **Language:** C++
- **Description:** Intel VT-x/EPT based thin-hypervisor for Windows with minimal code.
- **Key Techniques:**
  - Minimal hypervisor implementation
  - Intel VT-x with EPT
  - Educational focus
  - Windows kernel mode
- **Relevance to Ombra:** Low-Medium - Beginner-friendly reference. Useful for understanding minimal viable hypervisor architecture.

### 32. **momo5502/hypervisor**
- **URL:** https://github.com/momo5502/hypervisor
- **Stars:** ~150+
- **Last Activity:** 2024
- **Language:** C++
- **Description:** Lightweight experimental hypervisor with EPT hooking. Uses VT-x for stealthy memory hooks.
- **Key Techniques:**
  - EPT-based memory hooks
  - Second-level address translation manipulation
  - Invisible code execution via EPT
  - Experimental/research focus
- **Relevance to Ombra:** Medium - EPT hooking implementation reference. Lightweight design shows minimal overhead approach.

---

## LOW PRIORITY REPOS (Tangentially Related)

### 33. **kernelwernel/VMAware**
- **URL:** https://github.com/kernelwernel/VMAware
- **Stars:** ~927 | **Forks:** ~102
- **Last Activity:** Active 2025
- **Language:** C++
- **Description:** Advanced VM detection library with 100+ techniques detecting 70+ VM brands. Cross-platform header-only library.
- **Key Techniques:**
  - Scoring system (0-100) for detection confidence
  - Low-level and high-level anti-VM techniques
  - CPUID checks, timing attacks, artifact detection
  - VirtualBox, VMware, KVM, QEMU, Bochs, Hyper-V detection
  - Windows, Linux, macOS support
- **Relevance to Ombra:** Low-Medium - Detection library Ombra needs to evade. Comprehensive test suite for validating Ombra's antidetection. Open source nature makes bypass easier.

### 34. **hfiref0x/VMDE**
- **URL:** https://github.com/hfiref0x/VMDE
- **Stars:** ~200+
- **Last Activity:** 2020
- **Language:** C
- **Description:** VM detection tool querying hypervisor presence via CPUID and scanning firmware.
- **Key Techniques:**
  - CPUID hypervisor bit check
  - VMware hypervisor port query
  - Raw firmware string pattern scanning
  - Multiple VM brand detection
- **Relevance to Ombra:** Low - Another detection tool for testing. Less comprehensive than VMAware but simpler codebase to analyze.

### 35. **Code-Case/VMHide**
- **URL:** https://github.com/Code-Case/VMHide
- **Stars:** ~100+
- **Last Activity:** 2023
- **Language:** Python/Config
- **Description:** VM detection assessment tool with evasion techniques and fixes (formerly InviZzzible).
- **Key Techniques:**
  - Recent detection and evasion technique collection
  - JSON configuration for extensibility
  - VM environment assessment
  - Evasion fix suggestions
- **Relevance to Ombra:** Low - Testing tool for validation. JSON configuration approach could inform Ombra's configuration system.

### 36. **nevioo1337/VMHide**
- **URL:** https://github.com/nevioo1337/VMHide
- **Stars:** ~50+
- **Last Activity:** 2023
- **Language:** C++
- **Description:** Bypasses VMProtect VMware detection via user-mode API hooks.
- **Key Techniques:**
  - User-mode API hooking
  - VMProtect VMware detection bypass
  - VMware Tools detection bypass
- **Relevance to Ombra:** Low - User-mode approach only. Limited applicability to hypervisor-level stealth but shows VMProtect detection methods.

### 37. **d4rksystem/VMwareCloak**
- **URL:** https://github.com/d4rksystem/VMwareCloak
- **Stars:** ~1,000+
- **Last Activity:** 2021
- **Language:** PowerShell
- **Description:** PowerShell script to hide VMware Windows VMs from malware analysis evasion.
- **Key Techniques:**
  - Registry key renaming for VM artifacts
  - VMware process termination
  - VMware driver file deletion
  - Supporting file removal/renaming
- **Relevance to Ombra:** Low - Script-based VM hiding for analysis environment. Not applicable to hypervisor development but shows detection artifacts to hide.

### 38. **Batlez/CloakBox**
- **URL:** https://github.com/Batlez/CloakBox
- **Stars:** ~200+
- **Last Activity:** 2025
- **Language:** C++/Patches
- **Description:** Custom VirtualBox fork bypassing proctoring software VM detection.
- **Key Techniques:**
  - VirtualBox source modifications
  - Manufacturer string spoofing
  - MAC address randomization
  - Bypasses Examity, Respondus, ProctorU, etc.
- **Relevance to Ombra:** Low - VirtualBox-specific modifications. Shows proctoring software detection methods to evade.

### 39. **zhaodice/qemu-anti-detection**
- **URL:** https://github.com/zhaodice/qemu-anti-detection
- **Stars:** ~300+
- **Last Activity:** 2024
- **Language:** C/Patches
- **Description:** QEMU patches to hide emulation from anti-cheat and protection software.
- **Key Techniques:**
  - QEMU device renaming ("QEMU keyboard" → "ASUS keyboard")
  - Serial number modification
  - UEFI VM bit removal
  - Boot Graphics Record Table modification
  - Bypasses mhyprot, EAC, nProtect, VMProtect, VProtect, Themida, Enigma Protector, Safegine
- **Relevance to Ombra:** Low - QEMU emulator specific. Device spoofing techniques applicable to Ombra's virtual device emulation.

### 40. **nbviet300689/VM-Undetected**
- **URL:** https://github.com/nbviet300689/VM-Undetected
- **Stars:** ~50+
- **Last Activity:** 2023
- **Language:** Documentation
- **Description:** Guide to bypass anti-cheat and proctoring software VM detection.
- **Key Techniques:**
  - IPv4 LAN address separation (host vs VM)
  - VM disk size recommendations (128GB+)
  - Hardware configuration best practices
  - Network isolation techniques
- **Relevance to Ombra:** Low - Configuration guide rather than code. Shows practical detection vectors to address.

### 41. **jonatan1024/CpuidSpoofer**
- **URL:** https://github.com/jonatan1024/CpuidSpoofer
- **Stars:** ~30+
- **Last Activity:** 2022
- **Language:** C++
- **Description:** x64dbg plugin for spoofing CPUID instruction behavior during debugging.
- **Key Techniques:**
  - CPUID instruction breakpoint setting
  - CpuidSpooferBegin/CpuidSpooferEnd commands
  - Register manipulation before/after CPUID
- **Relevance to Ombra:** Low - Debugger plugin approach. Shows CPUID spoofing at user-mode level but not hypervisor-level.

### 42. **erkserkserks/vmware_cpuid_tool**
- **URL:** https://github.com/erkserkserks/vmware_cpuid_tool
- **Stars:** ~10+
- **Last Activity:** 2020
- **Language:** Python
- **Description:** Converts CPUID data from cpuid tool to VMware .vmx format for CPU spoofing.
- **Key Techniques:**
  - CPUID data parsing
  - VMX configuration generation
  - CPU signature spoofing
- **Relevance to Ombra:** Low - VMware configuration tool. Shows CPUID spoofing data format for reference.

### 43. **Scrut1ny/Hypervisor-Phantom**
- **URL:** https://github.com/Scrut1ny/Hypervisor-Phantom
- **Stars:** ~50+
- **Last Activity:** 2024
- **Language:** Scripts
- **Description:** Toolbox of automated scripts for virtualization-related stealth configuration.
- **Key Techniques:**
  - Automated virtualization hardening scripts
  - Multiple VM platform support
  - Configuration automation
- **Relevance to Ombra:** Low - Script collection for VM hardening. Useful for setting up Ombra test environments.

### 44. **so1icitx/vm-anti-detection**
- **URL:** https://github.com/so1icitx/vm-anti-detection
- **Stars:** ~100+
- **Last Activity:** 2023
- **Language:** Documentation/Patches
- **Description:** Patches to bypass malware detection for free analysis. RDTSC timing bypass focus.
- **Key Techniques:**
  - Linux kernel RDTSC tweaks
  - VM-Exit overhead compensation
  - Timing attack mitigation
- **Relevance to Ombra:** Low - Documentation of techniques already covered by other repos. Reference for RDTSC issues.

### 45. **MellowNight/Anti-debug**
- **URL:** https://github.com/MellowNight/Anti-debug
- **Stars:** ~30+
- **Last Activity:** 2023
- **Language:** C++
- **Description:** Simple anti-debug program with kernel mode callbacks. Educational example.
- **Key Techniques:**
  - Kernel mode anti-debug callbacks
  - Debug detection methods
  - Process protection
- **Relevance to Ombra:** Low - Anti-debug implementation reference. Shows techniques Ombra needs to hide from or defeat.

### 46. **x64dbg/ScyllaHide**
- **URL:** https://github.com/x64dbg/ScyllaHide
- **Stars:** ~2,000+
- **Last Activity:** Active 2025
- **Language:** C++
- **Description:** Advanced user-mode anti-anti-debugger. Stays in Ring 3.
- **Key Techniques:**
  - User-mode API hooking
  - Debug artifact hiding
  - PEB manipulation
  - TLS callback handling
- **Relevance to Ombra:** Low - User-mode only (Ring 3). Not applicable to hypervisor-level but shows comprehensive user-mode hiding.

### 47. **hfiref0x/WubbabooMark**
- **URL:** https://github.com/hfiref0x/WubbabooMark
- **Stars:** ~100+
- **Last Activity:** 2023
- **Language:** C++
- **Description:** Debugger anti-detection benchmark testing tool.
- **Key Techniques:**
  - Anti-anti-debug technique collection
  - Benchmark suite for debugger hiding tools
  - Detection method testing
- **Relevance to Ombra:** Low - Benchmarking tool. Useful for testing Ombra's anti-debug evasion capabilities.

### 48. **reveng007/reveng_rtkit**
- **URL:** https://github.com/reveng007/reveng_rtkit
- **Stars:** ~100+
- **Last Activity:** 2024
- **Language:** C
- **Description:** Linux LKM rootkit (Ring 0) capable of hiding itself and processes. rmmod proof, bypasses rkhunter.
- **Key Techniques:**
  - Linux Loadable Kernel Module rootkit
  - Process hiding
  - Self-hiding capability
  - rmmod protection
  - rkhunter antirootkit bypass
- **Relevance to Ombra:** Low - Linux-specific Ring 0 rootkit. Different platform but process hiding techniques conceptually applicable.

### 49. **TheMalwareGuardian/Awesome-Bootkits-Rootkits-Development**
- **URL:** https://github.com/TheMalwareGuardian/Awesome-Bootkits-Rootkits-Development
- **Stars:** ~300+
- **Last Activity:** 2025
- **Language:** Documentation
- **Description:** Curated compilation of bootkit and rootkit development resources.
- **Key Techniques:**
  - Resource collection (papers, code, presentations)
  - Ring -1, Ring 0, UEFI bootkit coverage
  - Windows 10 VSM/VTL architecture explanation
- **Relevance to Ombra:** Low - Resource list rather than implementation. Good reference for additional research directions.

### 50. **Wenzel/r2vmi**
- **URL:** https://github.com/Wenzel/r2vmi
- **Stars:** ~50+
- **Last Activity:** 2023
- **Language:** C
- **Description:** Hypervisor-level debugger based on Radare2/LibVMI using VMI IO and debug plugins.
- **Key Techniques:**
  - Radare2 integration for VMI
  - LibVMI-based debugging
  - Hypervisor-level analysis
- **Relevance to Ombra:** Low - VMI debugging tool. Reference for understanding VMI-based analysis Ombra needs to evade.

---

## ALREADY IN CODEBASE

These repositories are already present in `/Users/jonathanmcclintock/Desktop/OmbraHypervisor-New/Antidetection-Research/CODEBASE-REF`:

1. **tandasat/DdiMon** ✓ - EPT-based stealth hooking (ALREADY PRESENT)
2. **Mattiwatti/EfiGuard** ✓ - UEFI bootkit for PatchGuard/DSE bypass (ALREADY PRESENT)
3. **DarthTon/HyperBone** ✓ - Minimalistic VT-x hypervisor with hooks (ALREADY PRESENT)
4. **tandasat/HyperPlatform** ✓ - Intel VT-x hypervisor research platform (ALREADY PRESENT)
5. **tandasat/SimpleSvm** ✓ - AMD SVM educational hypervisor (ALREADY PRESENT)
6. **tandasat/SimpleSvmHook** ✓ - AMD NPT-based stealth hooks (ALREADY PRESENT)
7. **ionescu007/SimpleVisor** ✓ - Minimal VT-x hypervisor (ALREADY PRESENT)
8. **Gbps/gbhv** ✓ - VT-x hypervisor with EPT hooking (ALREADY PRESENT)
9. **wbenny/hvpp** ✓ - Lightweight VT-x hypervisor in C++ (ALREADY PRESENT)
10. **ajkhoury/UEFI-Bootkit** ✓ - UEFI bootkit implementation (ALREADY PRESENT)
11. **hzqst/VmwareHardenedLoader** ✓ - VMware VM detection mitigation (ALREADY PRESENT)
12. **HoShiMin/Kernel-Bridge** ✓ - Windows kernel hacking framework with hypervisor (ALREADY PRESENT)

---

## ANALYSIS & RECOMMENDATIONS

### Critical Missing Pieces in Current Codebase

1. **PatchGuard Bypass (Modern):**
   - **can1357/ByePg** - Exception-based hooking works with HVCI
   - **NeoMaster831/kurasagi** - Windows 11 24H2 runtime PatchGuard bypass
   - **AdamOron/PatchGuardBypass** - Dynamic PatchGuard bypass for modern Windows 10

2. **Anti-Anti-Debug:**
   - **Air14/HyperHide** - Hypervisor-based anti-anti-debug (most important)
   - **changeofpace/VivienneVMM** - EPT breakpoints and debug register facade

3. **UEFI Type-1 Hypervisor:**
   - **memN0ps/illusion-rs** - Stable UEFI Type-1 hypervisor in Rust
   - **ldpreload/BlackLotus** - Secure Boot bypass for Type-1 initialization
   - **Shtan7/UEFI-hypervisor** - Type-1 thin hypervisor in C++

4. **HVCI Bypass:**
   - **zer0condition/ZeroHVCI** - HVCI-compliant kernel operations without drivers
   - **tandasat/CVE-2024-21305** - HVCI vulnerability research

5. **Timing Attack Mitigation:**
   - **WCharacter/RDTSC-KVM-Handler** - RDTSC spoofing kernel patches
   - **SamuelTulach/BetterTiming** - TSC offsetting PoC

6. **Nested Virtualization:**
   - **KelvinMsft/kHypervisor** - Nested VMM for testing Ombra inside VMware/Hyper-V

7. **Modern Rust Implementations:**
   - **memN0ps/illusion-rs** and **matrix-rs** - Memory-safe hypervisor patterns
   - **vmi-rs/vmi** - Type-safe VMI patterns

### Testing & Validation Tools

Must test against these detection frameworks:
- **kernelwernel/VMAware** - 100+ detection techniques (927 stars)
- **hfiref0x/VMDE** - CPUID and firmware scanning
- **hfiref0x/WubbabooMark** - Anti-debug benchmark
- **Code-Case/VMHide** - Recent detection/evasion techniques

### Priority Implementation Order

**Phase 1 - Core Stealth (Immediate):**
1. Implement **ByePg** exception-based hooking for PatchGuard bypass
2. Add **HyperHide** KUserSharedData manipulation and timestamp hiding
3. Implement **RDTSC-KVM-Handler** timing attack mitigation

**Phase 2 - Advanced Evasion:**
4. Study **VivienneVMM** debug register facade
5. Implement **Shtan7/Hypervisor** page cloning/swap technique
6. Add **ZeroHVCI** HVCI-compliant operation patterns

**Phase 3 - Type-1 Migration:**
7. Port to **illusion-rs** UEFI Type-1 architecture
8. Integrate **BlackLotus** Secure Boot bypass
9. Implement **EfiGuard** boot-time initialization

**Phase 4 - Validation:**
10. Test against **VMAware** detection suite
11. Benchmark with **WubbabooMark**
12. Verify with **kHypervisor** nested virtualization

### Code Complexity Analysis

**Easy to Integrate (1-2 days):**
- RDTSC timing fixes
- CPUID spoofing improvements
- Basic PatchGuard evasion patterns

**Medium Complexity (3-7 days):**
- HyperHide anti-anti-debug features
- ByePg exception-based hooking
- Page cloning/swap for PatchGuard

**High Complexity (1-2 weeks):**
- VivienneVMM debug register facade
- HVCI bypass integration
- Nested virtualization support

**Very High Complexity (3-4 weeks):**
- Type-1 UEFI hypervisor migration
- Secure Boot bypass integration
- Complete rewrite in Rust (optional long-term)

### Architecture Recommendations

1. **Immediate:** Add anti-timing and anti-debug from HyperHide
2. **Short-term:** Implement ByePg exception hooking for PatchGuard
3. **Medium-term:** Develop Type-1 UEFI variant with BlackLotus techniques
4. **Long-term:** Consider Rust rewrite using illusion-rs patterns for memory safety

### Notable Authors to Follow

- **tandasat (Satoshi Tanda)** - HyperPlatform, DdiMon, SimpleSvm, CVE-2024-21305 (Microsoft engineer)
- **can1357 (Can Bölük)** - ByePg, hvdetecc (security researcher)
- **Air14** - HyperHide (anti-anti-debug expert)
- **memN0ps** - matrix-rs, illusion-rs (Rust hypervisor developer)
- **Mattiwatti** - EfiGuard (UEFI bootkit expert)
- **wbenny** - hvpp (clean C++ hypervisor implementation)
- **Gbps** - gbhv (EPT hooking expert)
- **changeofpace** - VivienneVMM (stealthy debugging)
- **Shtan7** - Type-1 and Type-2 hypervisors with EPT
- **hfiref0x** - VMDE, WubbabooMark, UPGDSED (detection expert)

---

## SEARCH QUERIES EXECUTED

1. "hypervisor detection evasion" - Found VMAware, hvdetecc, hvpp, VMHide
2. "EPT hooking stealth" - Found gbhv, DdiMon, SimpleSvmHook, momo5502/hypervisor
3. "CPUID spoofing hypervisor" - Found VMDOG, VMHide, CpuidSpoofer, hvpp
4. "blue pill hypervisor" - Found kHypervisor, matrix-rs, illusion-rs, gbhv, SimpleVisor
5. "PatchGuard bypass" - Found EfiGuard, ByePg, kurasagi, PatchGuardBypass, UPGDSED, BypassPG
6. "anti anti-debug hypervisor" - Found HyperHide, HyperDbg, ScyllaHide, WubbabooMark
7. "UEFI bootkit source" - Found BlackLotus, UEFI-Bootkit, BootLicker, uefi-rootkit
8. "kernel hypervisor rootkit" - Found gbhv, CoVirt, MicroV, reveng_rtkit
9. "VT-x stealth" - Found gbhv, hvpp, DdiMon, HyperBone, VivienneVMM, SimpleVisor
10. "AMD SVM rootkit" - Found SimpleSvmHook, CoVirt, grr, memhv, SimpleSvm, HookMsrBySVM
11. "ring -1 rootkit" - Found gbhv, CoVirt, MicroV

**Additional queries:**
12. "hypervisor timing attack RDTSC" - Found RDTSC-KVM-Handler, BetterTiming, kvm-rdtsc-hack
13. "vmware detection bypass" - Found VmwareHardenedLoader, VMDOG, VMwareCloak, CloakBox, qemu-anti-detection
14. "hypervisor memory introspection VMI" - Found vmi-rs, libvmi, hvmi, KVM-VMI, smartvmi, libmicrovmi, IntroVirt
15. "nested virtualization hypervisor" - Found Microsoft Virtualization docs, kHypervisor discussions
16. "HVCI hypervisor code integrity bypass" - Found ZeroHVCI, CVE-2024-21305
17. "intel vmx rootkit stealth" - Found HyperPlatform, hvpp, gbhv, VMBR, hv
18. "windows kernel hypervisor research" - Found HyperPlatform, HyperDbg, Kernel-Bridge, matrix-rs, orange_slice
19. "VivienneVMM hardware breakpoint" - Found VivienneVMM details and documentation
20. "kHypervisor nested VMM" - Found kHypervisor details and features
21. "Shtan7 Hypervisor EPT" - Found Shtan7/Hypervisor and Shtan7/UEFI-hypervisor

---

## SOURCES

**High Priority Repos:**
- [ByePg](https://github.com/can1357/ByePg) - Universal PatchGuard defeat
- [HyperHide](https://github.com/Air14/HyperHide) - Anti-anti-debug hypervisor plugin
- [BlackLotus](https://github.com/ldpreload/BlackLotus) - UEFI bootkit with Secure Boot bypass
- [EfiGuard](https://github.com/Mattiwatti/EfiGuard) - UEFI bootkit for PatchGuard/DSE bypass
- [illusion-rs](https://github.com/memN0ps/illusion-rs) - UEFI Type-1 hypervisor in Rust
- [matrix-rs](https://github.com/memN0ps/matrix-rs) - Windows Blue Pill Type-2 hypervisor in Rust
- [VivienneVMM](https://github.com/changeofpace/VivienneVMM) - Stealthy debugging via VT-x
- [kHypervisor](https://github.com/KelvinMsft/kHypervisor) - Nested VMM for Windows
- [Shtan7/Hypervisor](https://github.com/Shtan7/Hypervisor) - Type-2 with EPT page cloning
- [Shtan7/UEFI-hypervisor](https://github.com/Shtan7/UEFI-hypervisor) - Type-1 thin hypervisor

**Medium Priority Repos:**
- [ZeroHVCI](https://github.com/zer0condition/ZeroHVCI) - HVCI bypass techniques
- [CVE-2024-21305](https://github.com/tandasat/CVE-2024-21305) - HVCI vulnerability PoC
- [VmwareHardenedLoader](https://github.com/hzqst/VmwareHardenedLoader) - VMware detection mitigation
- [VMDOG](https://github.com/KonBuku/VMDOG) - VMware spoofing toolkit
- [RDTSC-KVM-Handler](https://github.com/WCharacter/RDTSC-KVM-Handler) - RDTSC timing fix
- [BetterTiming](https://github.com/SamuelTulach/BetterTiming) - TSC offsetting PoC
- [kvm-rdtsc-hack](https://github.com/h33p/kvm-rdtsc-hack) - RDTSC evasion module
- [hvmi](https://github.com/bitdefender/hvmi) - Bitdefender VMI library
- [libvmi](https://github.com/libvmi/libvmi) - Industry standard VMI library
- [vmi-rs](https://github.com/vmi-rs/vmi) - Modern VMI in Rust
- [KVM-VMI](https://github.com/KVM-VMI/kvm-vmi) - KVM VMI implementation
- [HyperDbg](https://github.com/HyperDbg/HyperDbg) - Hypervisor-assisted debugger
- [jonomango/hv](https://github.com/jonomango/hv) - Lightweight VT-x hypervisor
- [orange_slice](https://github.com/gamozolabs/orange_slice) - Deterministic hypervisor research
- [grr](https://github.com/kukrimate/grr) - AMD SVM hypervisor rootkit
- [memhv](https://github.com/SamuelTulach/memhv) - AMD-V/SVM with memory introspection
- [CoVirt](https://github.com/Nadharm/CoVirt) - AMD-V VM rootkit for Linux
- [HookMsrBySVM](https://github.com/hrbust86/HookMsrBySVM) - AMD SVM MSR hooking
- [HyperBone](https://github.com/DarthTon/HyperBone) - Minimalistic VT-x with hooks
- [noahbean33/hypervisor](https://github.com/noahbean33/hypervisor) - Educational VT-x hypervisor
- [hypervisor-for-beginners](https://github.com/rohaaan/hypervisor-for-beginners) - Thin hypervisor tutorial
- [momo5502/hypervisor](https://github.com/momo5502/hypervisor) - Experimental EPT hooking

**Low Priority Repos (Detection/Testing Tools):**
- [VMAware](https://github.com/kernelwernel/VMAware) - Comprehensive VM detection library
- [VMDE](https://github.com/hfiref0x/VMDE) - VM detection via CPUID/firmware
- [VMHide/InviZzzible](https://github.com/Code-Case/VMHide) - VM assessment tool
- [nevioo1337/VMHide](https://github.com/nevioo1337/VMHide) - VMProtect detection bypass
- [VMwareCloak](https://github.com/d4rksystem/VMwareCloak) - VMware artifact hiding script
- [CloakBox](https://github.com/Batlez/CloakBox) - VirtualBox fork for proctoring bypass
- [qemu-anti-detection](https://github.com/zhaodice/qemu-anti-detection) - QEMU detection patches
- [VM-Undetected](https://github.com/nbviet300689/VM-Undetected) - VM configuration guide
- [CpuidSpoofer](https://github.com/jonatan1024/CpuidSpoofer) - x64dbg CPUID plugin
- [vmware_cpuid_tool](https://github.com/erkserkserks/vmware_cpuid_tool) - CPUID conversion tool
- [Hypervisor-Phantom](https://github.com/Scrut1ny/Hypervisor-Phantom) - VM hardening scripts
- [vm-anti-detection](https://github.com/so1icitx/vm-anti-detection) - Malware analysis VM patches
- [Anti-debug](https://github.com/MellowNight/Anti-debug) - Kernel anti-debug example
- [ScyllaHide](https://github.com/x64dbg/ScyllaHide) - User-mode anti-anti-debugger
- [WubbabooMark](https://github.com/hfiref0x/WubbabooMark) - Debugger detection benchmark
- [reveng_rtkit](https://github.com/reveng007/reveng_rtkit) - Linux LKM rootkit
- [Awesome-Bootkits-Rootkits-Development](https://github.com/TheMalwareGuardian/Awesome-Bootkits-Rootkits-Development) - Resource collection
- [r2vmi](https://github.com/Wenzel/r2vmi) - Radare2 VMI debugger

---

**Report Generated:** December 22, 2025
**Total Repositories Analyzed:** 50
**High Priority:** 10
**Medium Priority:** 22
**Low Priority:** 18
**Already in Codebase:** 12
