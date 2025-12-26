# OmbraHypervisor Reference Repository Inventory

**Generated**: 2025-12-22
**Total Repositories**: 28
**Purpose**: Comprehensive catalog of all reference codebases for OmbraHypervisor antidetection research

---

## Classification Legend

| Tag | Description |
|-----|-------------|
| **[HYPERVISOR]** | Full hypervisor implementation |
| **[BOOTKIT]** | UEFI/boot-level persistence |
| **[ROOTKIT]** | Kernel-mode stealth/hooking |
| **[ANTICHEAT]** | Anticheat implementation (study the enemy) |
| **[ANTICHEATCHEAT]** | Anticheat bypass/evasion |
| **[DETECTION]** | Hypervisor/VM detection tools |
| **[LIBRARY]** | Support library/framework |
| **[RESEARCH]** | Academic/PoC/documentation |

## Architecture Legend

| Field | Values |
|-------|--------|
| **CPU** | INTEL-ONLY, AMD-ONLY, DUAL-VENDOR |
| **Context** | RING0, RING-1, UEFI, USERMODE-LOADER |
| **OS** | WINDOWS, LINUX, BARE-METAL, MULTI-OS |
| **Nested** | YES, NO, PARTIAL |

---

## Repository Catalog

### 1. CheatDriver
| Field | Value |
|-------|-------|
| **URL** | https://github.com/cutecatsandvirtualmachines/CheatDriver.git |
| **Author** | cutecatsandvirtualmachines |
| **Last Commit** | 2024-11-20 |
| **Languages** | C++, C, Assembly (x64) |
| **License** | Not specified |
| **Classification** | [ANTICHEATCHEAT] |
| **CPU** | DUAL-VENDOR |
| **Context** | RING-1 |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: VT-x hypervisor-based game cheat driver leveraging SKLib for hardware spoofing (SMBIOS, GPU, NIC, Volume, Disk), SLAT-based memory hiding via EPT, IOMMU protection, and PG-compatible hooks for process injection and anticheat evasion.

---

### 2. DdiMon
| Field | Value |
|-------|-------|
| **URL** | https://github.com/tandasat/DdiMon.git |
| **Author** | Satoshi Tanda (tandasat) |
| **Last Commit** | 2022-01-22 |
| **Languages** | C++, C |
| **License** | MIT |
| **Classification** | [DETECTION] |
| **CPU** | INTEL-ONLY |
| **Context** | RING-1 |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: Intel VT-x/EPT hypervisor implementing invisible inline hooks on kernel DDIs (Device Driver Interfaces) using memory shadowing to monitor PatchGuard activity and hide processes while remaining undetectable to guests.

---

### 3. EFI_Driver_Access
| Field | Value |
|-------|-------|
| **URL** | https://github.com/TheCruZ/EFI_Driver_Access.git |
| **Author** | TheCruZ |
| **Last Commit** | 2023-01-08 |
| **Languages** | C (EFI), C++ (Client) |
| **License** | Not specified |
| **Classification** | [BOOTKIT] |
| **CPU** | INTEL-ONLY |
| **Context** | UEFI |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: UEFI bootkit that loads during system boot to provide unrestricted kernel-mode memory read/write access by directly calling Windows kernel functions from UEFI context before OS fully initializes.

---

### 4. EfiGuard
| Field | Value |
|-------|-------|
| **URL** | https://github.com/Mattiwatti/EfiGuard.git |
| **Author** | Mattiwatti |
| **Last Commit** | 2025-08-03 |
| **Languages** | C |
| **License** | GPL v3 |
| **Classification** | [BOOTKIT] |
| **CPU** | INTEL-ONLY |
| **Context** | UEFI |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: Portable x64 UEFI bootkit that patches Windows boot manager, boot loader, and kernel at boot time using Zydis disassembler to disable PatchGuard and Driver Signature Enforcement (DSE) across all Windows versions from Vista SP1 to Windows 11.

---

### 5. HyperBone
| Field | Value |
|-------|-------|
| **URL** | https://github.com/DarthTon/HyperBone.git |
| **Author** | DarthTon |
| **Last Commit** | 2019-03-30 |
| **Languages** | C, Assembly (x64) |
| **License** | MIT |
| **Classification** | [HYPERVISOR] |
| **CPU** | INTEL-ONLY |
| **Context** | RING-1 |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: Minimalistic Intel VT-x hypervisor providing PatchGuard-compatible hooks including syscall hooks via MSR_LSTAR, kernel inline hooks, EPT-based page substitution with TLB splitting, MSR hooks, and IDT hooks for stealthy kernel monitoring.

---

### 6. HyperPlatform
| Field | Value |
|-------|-------|
| **URL** | https://github.com/tandasat/HyperPlatform.git |
| **Author** | Satoshi Tanda (tandasat) |
| **Last Commit** | 2023-11-24 |
| **Languages** | C++, Assembly (x86/x64) |
| **License** | MIT |
| **Classification** | [LIBRARY] |
| **CPU** | INTEL-ONLY |
| **Context** | RING-1 |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: Educational Intel VT-x hypervisor framework designed as a research platform for Windows, providing event monitoring capabilities for memory access, system registers, interrupts, and instruction execution with full source documentation and debugger support.

---

### 7. InfinityHook
| Field | Value |
|-------|-------|
| **URL** | https://github.com/everdox/InfinityHook.git |
| **Author** | Nemanja Mulasmajic (everdox) |
| **Last Commit** | 2019-07-25 |
| **Languages** | C++, C |
| **License** | Not specified |
| **Classification** | [ROOTKIT] |
| **CPU** | DUAL-VENDOR |
| **Context** | RING0 |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: Kernel hooking framework that hijacks ETW (Event Tracing for Windows) circular kernel context logger to hook system calls, context switches, page faults, and DPCs by overwriting WMI_LOGGER_CONTEXT GetCpuClock function pointer, working alongside PatchGuard and VBS/Hyperguard across Windows 7 to Windows 10.

---

### 8. Kernel-Bridge
| Field | Value |
|-------|-------|
| **URL** | https://github.com/HoShiMin/Kernel-Bridge.git |
| **Author** | HoShiMin |
| **Last Commit** | 2023-08-22 |
| **Languages** | C++, C, Assembly, Python |
| **License** | GPL v3 |
| **Classification** | [LIBRARY], [HYPERVISOR] |
| **CPU** | DUAL-VENDOR |
| **Context** | RING0 |
| **OS** | WINDOWS |
| **Nested** | YES (Hyper-V support) |

**Description**: Comprehensive C++20-ready Windows kernel driver framework with hypervisor capabilities for both Intel and AMD processors. Provides extensive kernel-mode API including memory management, process manipulation, driver mapping, hypervisor operations, and Python bindings.

---

### 9. KernelBhop
| Field | Value |
|-------|-------|
| **URL** | https://github.com/Zer0Mem0ry/KernelBhop.git |
| **Author** | Zer0Mem0ry |
| **Last Commit** | 2018-04-02 |
| **Languages** | C, C++ |
| **License** | Not specified |
| **Classification** | [ANTICHEATCHEAT], [RESEARCH] |
| **CPU** | INTEL-ONLY |
| **Context** | RING0 |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: VAC bypass proof-of-concept using kernel-mode driver for memory read/write operations without usermode handles. Demonstrates ring0-level memory access to avoid anticheat handle scanning on ring3.

---

### 10. NoirVisor
| Field | Value |
|-------|-------|
| **URL** | https://github.com/Zero-Tang/NoirVisor.git |
| **Author** | Zero Tang |
| **Last Commit** | 2024-10-31 |
| **Languages** | C, Assembly (MASM) |
| **License** | MIT |
| **Classification** | [HYPERVISOR], [RESEARCH] |
| **CPU** | DUAL-VENDOR |
| **Context** | RING-1, UEFI |
| **OS** | WINDOWS, UEFI, MULTI-OS |
| **Nested** | PARTIAL (AMD-V debugging, Intel VT-x not supported) |

**Description**: Hardware-accelerated Type-I and Type-II hypervisor for AMD64 processors with support for both Intel VT-x and AMD-V. Features Customizable VM engine, stealth hooks, Microsoft Hv#1 interface, and NPIEP security features.

---

### 11. PoolParty
| Field | Value |
|-------|-------|
| **URL** | https://github.com/SafeBreach-Labs/PoolParty.git |
| **Author** | Alon Leviev (SafeBreach Labs) |
| **Last Commit** | 2023-12-06 |
| **Languages** | C++ |
| **License** | BSD 3-Clause |
| **Classification** | [DETECTION], [RESEARCH] |
| **CPU** | N/A |
| **Context** | USERMODE-LOADER |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: Collection of eight fully-undetectable process injection techniques abusing Windows Thread Pools (Black Hat EU 2023). Demonstrates novel code injection vectors via worker factories, TP_WORK, TP_WAIT, TP_IO, TP_ALPC, TP_JOB, TP_DIRECT, and TP_TIMER work items.

---

### 12. SKLib
| Field | Value |
|-------|-------|
| **URL** | https://github.com/cutecatsandvirtualmachines/SKLib.git |
| **Author** | cutecatsandvirtualmachines |
| **Last Commit** | 2025-06-18 |
| **Languages** | C++, Assembly |
| **License** | AGPL v3 |
| **Classification** | [LIBRARY], [HYPERVISOR], [ANTICHEATCHEAT] |
| **CPU** | DUAL-VENDOR |
| **Context** | RING0, RING-1 |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: Kernel-mode C++ library with SKLib core utilities and SKLib-v hypervisor module supporting Intel/AMD system subversion. Features fast SLAT-based hooks, IOMMU/VT-d protection, and hardware spoofers (SMBIOS, GPU, NIC, disk, USB, EFI).

---

### 13. SimpleSvm
| Field | Value |
|-------|-------|
| **URL** | https://github.com/tandasat/SimpleSvm.git |
| **Author** | Satoshi Tanda |
| **Last Commit** | 2025-03-03 |
| **Languages** | C++, Assembly |
| **License** | MIT |
| **Classification** | [HYPERVISOR], [RESEARCH] |
| **CPU** | AMD-ONLY |
| **Context** | RING0 |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: Minimalistic educational hypervisor for Windows on AMD processors using Secure Virtual Machine (SVM) with Nested Page Tables (NPT). Inspired by SimpleVisor, designed as AMD counterpart to Intel VT-x examples.

---

### 14. SimpleSvmHook
| Field | Value |
|-------|-------|
| **URL** | https://github.com/tandasat/SimpleSvmHook.git |
| **Author** | Satoshi Tanda |
| **Last Commit** | 2021-02-17 |
| **Languages** | C++, Assembly |
| **License** | MIT |
| **Classification** | [HYPERVISOR], [RESEARCH], [DETECTION] |
| **CPU** | AMD-ONLY |
| **Context** | RING-1 |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: Research hypervisor demonstrating stealth function hooks on AMD processors using NPT-based memory view splitting for VMI. Shows AMD-specific limitations compared to Intel EPT implementations (no execute-only pages, hook visibility within same page).

---

### 15. SimpleVisor
| Field | Value |
|-------|-------|
| **URL** | https://github.com/ionescu007/SimpleVisor.git |
| **Author** | Alex Ionescu |
| **Last Commit** | 2018-12-08 |
| **Languages** | C, Assembly (x64) |
| **License** | BSD 2-Clause |
| **Classification** | [HYPERVISOR] |
| **CPU** | INTEL-ONLY |
| **Context** | RING-1 |
| **OS** | MULTI-OS (Windows, UEFI) |
| **Nested** | NO |

**Description**: Minimal Intel VT-x hypervisor with dynamic hyperjacking/unhyperjacking support, EPT, and VPID. Uses only 10 lines of assembly and ~500 lines of C code to demonstrate bare-metal hypervisor requirements.

---

### 16. Sputnik
| Field | Value |
|-------|-------|
| **URL** | https://github.com/cutecatsandvirtualmachines/Sputnik.git |
| **Author** | cutecatsandvirtualmachines |
| **Last Commit** | 2024-08-21 |
| **Languages** | C++, Assembly |
| **License** | AGPL v3 (from SKLib) |
| **Classification** | [ANTICHEATCHEAT] |
| **CPU** | AMD-ONLY (Intel incomplete) |
| **Context** | RING-1 |
| **OS** | WINDOWS |
| **Nested** | YES (hijacks Hyper-V) |

**Description**: Unofficial sequel to Voyager that hijacks Microsoft's Hyper-V hypervisor instead of implementing a standalone hypervisor. Tested only on AMD, includes PT/EPT/identity mapping and exception handling for anti-cheat evasion.

---

### 17. UEFI-Bootkit
| Field | Value |
|-------|-------|
| **URL** | https://github.com/ajkhoury/UEFI-Bootkit.git |
| **Author** | Aidan Khoury |
| **Last Commit** | 2019-08-29 |
| **Languages** | C |
| **License** | GPL v3 |
| **Classification** | [BOOTKIT] |
| **CPU** | DUAL-VENDOR |
| **Context** | UEFI |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: Small UEFI bootkit designed to use zero assembly code, compiled as EFI runtime driver to persist after ExitBootServices. Hooks Windows boot process before kernel initialization.

---

### 18. VBoxHardenedLoader
| Field | Value |
|-------|-------|
| **URL** | https://github.com/hfiref0x/VBoxHardenedLoader.git |
| **Author** | hfiref0x |
| **Last Commit** | 2023-04-02 |
| **Languages** | C, Assembly |
| **License** | BSD 2-Clause |
| **Classification** | [DETECTION] |
| **CPU** | INTEL-ONLY |
| **Context** | RING0 |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: VirtualBox VM detection mitigation loader that patches VirtualBox DLLs at runtime via Tsugumi driver. Project archived in 2023 due to Intel Nal driver incompatibility with Windows 11.

---

### 19. VmwareHardenedLoader
| Field | Value |
|-------|-------|
| **URL** | https://github.com/hzqst/VmwareHardenedLoader.git |
| **Author** | hzqst |
| **Last Commit** | 2022-12-02 |
| **Languages** | C++, C |
| **License** | MIT |
| **Classification** | [DETECTION] |
| **CPU** | DUAL-VENDOR |
| **Context** | RING0 |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: VMware VM detection mitigation that patches SystemFirmwareTable at runtime to remove VMware signatures. Successfully evades VMProtect 3.2, Safengine, and Themida anti-VM detection.

---

### 20. Voyager
| Field | Value |
|-------|-------|
| **URL** | https://github.com/backengineering/Voyager.git |
| **Author** | _xeroxz |
| **Last Commit** | 2023-09-03 |
| **Languages** | C++, C, Assembly |
| **License** | AGPL v3 |
| **Classification** | [ANTICHEATCHEAT] |
| **CPU** | DUAL-VENDOR |
| **Context** | RING-1 |
| **OS** | WINDOWS |
| **Nested** | YES (hijacks Hyper-V) |

**Description**: Hyper-V hacking framework that hijacks Microsoft's hypervisor to provide module injection and vmexit hooking. Works on all Windows 10 x64 versions (2004-1507), supports both AMD and Intel processors.

---

### 21. gbhv
| Field | Value |
|-------|-------|
| **URL** | https://github.com/Gbps/gbhv.git |
| **Author** | Gbps |
| **Last Commit** | 2021-11-17 |
| **Languages** | C, Assembly |
| **License** | CC BY 4.0 |
| **Classification** | [HYPERVISOR] |
| **CPU** | INTEL-ONLY |
| **Context** | RING-1 |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: Simple x64 Windows VT-x hypervisor proof-of-concept with full EPT support and shadow hooking capabilities. Designed as educational framework for security research with exhaustive code documentation.

---

### 22. hvpp
| Field | Value |
|-------|-------|
| **URL** | https://github.com/wbenny/hvpp.git |
| **Author** | Petr Benes |
| **Last Commit** | 2021-03-15 |
| **Languages** | C++, C, Assembly |
| **License** | MIT |
| **Classification** | [HYPERVISOR] |
| **CPU** | INTEL-ONLY |
| **Context** | RING-1 |
| **OS** | WINDOWS |
| **Nested** | PARTIAL |

**Description**: Lightweight Intel x64/VT-x hypervisor written in C++ focused on virtualizing already running operating systems with EPT support and 2MB identity mapping.

---

### 23. hyper-reV
| Field | Value |
|-------|-------|
| **URL** | https://github.com/noahware/hyper-reV.git |
| **Author** | noahware |
| **Last Commit** | 2025-11-15 |
| **Languages** | C, C++, Assembly |
| **License** | GPL v3 |
| **Classification** | [ANTICHEATCHEAT] |
| **CPU** | DUAL-VENDOR |
| **Context** | UEFI |
| **OS** | WINDOWS |
| **Nested** | YES |

**Description**: Memory introspection and reverse engineering hypervisor leveraging Hyper-V for pre-boot DMA attacks with SLAT code hooks, supporting both Intel EPT and AMD NPT on UEFI Secure Boot enabled platforms.

---

### 24. hypervisor (Bareflank)
| Field | Value |
|-------|-------|
| **URL** | https://github.com/Bareflank/hypervisor.git |
| **Author** | Assured Information Security, Inc. |
| **Last Commit** | 2021-10-21 |
| **Languages** | C++, C, Rust |
| **License** | MIT |
| **Classification** | [RESEARCH] |
| **CPU** | DUAL-VENDOR |
| **Context** | RING-1 |
| **OS** | MULTI-OS |
| **Nested** | PARTIAL |

**Description**: Open source hypervisor SDK for Rust and C++ that provides tools to rapidly prototype custom hypervisors on 64bit Intel and AMD platforms, designed for instructional/research purposes with AUTOSAR compliance.

---

### 25. ksm
| Field | Value |
|-------|-------|
| **URL** | https://github.com/asamy/ksm.git |
| **Author** | asamy |
| **Last Commit** | 2021-10-20 |
| **Languages** | C, Assembly |
| **License** | GPL v2 |
| **Classification** | [HYPERVISOR] |
| **CPU** | INTEL-ONLY |
| **Context** | RING-1 |
| **OS** | MULTI-OS |
| **Nested** | YES |

**Description**: Lightweight x64 hypervisor with self-contained physical memory introspection engine and userspace memory virtualization, supporting IDT shadowing, EPT violation #VE, and EPTP switching on Windows and Linux kernels.

---

### 26. rainbow
| Field | Value |
|-------|-------|
| **URL** | https://github.com/SamuelTulach/rainbow.git |
| **Author** | Samuel Tulach |
| **Last Commit** | 2021-05-14 |
| **Languages** | C, Assembly |
| **License** | Not specified |
| **Classification** | [BOOTKIT] |
| **CPU** | INTEL-ONLY |
| **Context** | UEFI |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: Bootkit-style HWID spoofer that hooks EFI runtime services to hide hardware serials through DKOM before any boot-time drivers are started, targeting SMBIOS, disk, and NIC identifiers.

---

### 27. s6_pcie_microblaze
| Field | Value |
|-------|-------|
| **URL** | https://github.com/Cr4sh/s6_pcie_microblaze.git |
| **Author** | Dmytro Oleksiuk (Cr4sh) |
| **Last Commit** | 2024-05-20 |
| **Languages** | Python, Verilog, C |
| **License** | Not specified |
| **Classification** | [RESEARCH] |
| **CPU** | INTEL-ONLY |
| **Context** | UEFI |
| **OS** | WINDOWS |
| **Nested** | NO |

**Description**: PCI-E DIY hacking toolkit implementing software-controllable gen 1.1 endpoint device for Xilinx SP605 with raw TLP operations, designed for pre-boot DMA attacks including Hyper-V VM exit handler backdoor injection and Boot Backdoor deployment.

---

### 28. umap
| Field | Value |
|-------|-------|
| **URL** | https://github.com/lmcinnes/umap.git |
| **Author** | Leland McInnes |
| **Last Commit** | 2025-12-12 |
| **Languages** | Python |
| **License** | BSD 3-Clause |
| **Classification** | [LIBRARY] |
| **CPU** | N/A |
| **Context** | USERMODE-LOADER |
| **OS** | MULTI-OS |
| **Nested** | N/A |

**Description**: Uniform Manifold Approximation and Projection dimension reduction technique. **NOTE**: This appears to be included by accident - not relevant to hypervisor research.

---

## Summary Statistics

### By Classification
| Classification | Count |
|---------------|-------|
| HYPERVISOR | 12 |
| BOOTKIT | 5 |
| ANTICHEATCHEAT | 6 |
| DETECTION | 4 |
| LIBRARY | 4 |
| RESEARCH | 5 |
| ROOTKIT | 1 |

### By CPU Support
| CPU Support | Count |
|-------------|-------|
| INTEL-ONLY | 13 |
| AMD-ONLY | 3 |
| DUAL-VENDOR | 11 |
| N/A | 1 |

### By Execution Context
| Context | Count |
|---------|-------|
| RING-1 | 14 |
| RING0 | 6 |
| UEFI | 7 |
| USERMODE-LOADER | 2 |

### Most Active (2024-2025)
1. **hyper-reV** - 2025-11-15
2. **SKLib** - 2025-06-18
3. **EfiGuard** - 2025-08-03
4. **SimpleSvm** - 2025-03-03
5. **CheatDriver** - 2024-11-20
6. **NoirVisor** - 2024-10-31
7. **Sputnik** - 2024-08-21

### Key Authors
| Author | Repositories |
|--------|--------------|
| Satoshi Tanda | DdiMon, HyperPlatform, SimpleSvm, SimpleSvmHook |
| cutecatsandvirtualmachines | CheatDriver, SKLib, Sputnik |
| hfiref0x | VBoxHardenedLoader |
| Zero Tang | NoirVisor |
| Alex Ionescu | SimpleVisor |

---

## Recommendations for OmbraHypervisor

### Primary References (CRITICAL)
1. **Voyager** - Boot chain hooking, Hyper-V hijacking
2. **SKLib** - Hardware spoofing, IOMMU protection
3. **EfiGuard** - PatchGuard/DSE defeat patterns
4. **NoirVisor** - Dual-vendor hypervisor architecture
5. **ksm** - Stealth techniques, EPTP switching

### Secondary References (HIGH)
6. **HyperPlatform** - Clean Intel VT-x framework
7. **SimpleSvm** - AMD SVM reference
8. **hvpp** - Modern C++ hypervisor patterns
9. **CheatDriver** - Anticheat evasion techniques
10. **VBoxHardenedLoader** / **VmwareHardenedLoader** - Detection evasion patterns

### Tertiary References (MEDIUM)
11. **InfinityHook** - ETW-based hooking patterns
12. **gbhv** - Educational EPT implementation
13. **Kernel-Bridge** - Kernel utility library
14. **s6_pcie_microblaze** - DMA attack research

---

**End of Repository Inventory**
