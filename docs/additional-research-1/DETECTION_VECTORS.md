# Hypervisor/VM Detection Vectors - Research Documentation

This document catalogs hypervisor and virtual machine detection techniques used by anti-cheat systems, malware, and security products. Research compiled for defensive security purposes.

**Last Updated:** December 22, 2025

---

## Table of Contents

1. [CPUID-Based Detection](#1-cpuid-based-detection)
2. [Timing-Based Detection](#2-timing-based-detection)
3. [Memory-Based Detection](#3-memory-based-detection)
4. [Hardware-Based Detection](#4-hardware-based-detection)
5. [Behavioral Detection](#5-behavioral-detection)
6. [Known Anti-Cheat Detection Methods](#6-known-anti-cheat-detection-methods)
7. [Additional Detection Vectors](#7-additional-detection-vectors)
8. [Summary Table](#8-summary-table)

---

## 1. CPUID-Based Detection

### 1.1 Hypervisor Present Bit (CPUID.1:ECX[31])

**How it works:**
- Intel and AMD CPUs have reserved bit 31 of ECX in CPUID leaf 0x1 as the "hypervisor present bit"
- When executing `CPUID` with EAX=1, bit 31 of ECX indicates virtualization:
  - Physical CPUs: Bit 31 = 0 (always)
  - Virtual/Emulated CPUs: Bit 31 = 1
- This is a hardware-level indication that a hypervisor is managing the system

**Technical implementation:**
```c
// Pseudo-code for detection
cpuid(1, &eax, &ebx, &ecx, &edx);
bool hypervisor_present = (ecx & (1 << 31)) != 0;
```

**Which products use it:**
- Most modern anti-cheat systems (BattlEye, EasyAntiCheat, Vanguard)
- Malware detection/evasion (QakBot, VMProtect-protected binaries)
- All major hypervisors expose this bit (VMware, KVM, Hyper-V, Xen)

**Known bypass techniques:**
- VMware: Configure VM with `cpuid.1.ecx="0---:----:----:----:----:----:----:----"` to mask bit 31
- KVM/QEMU: Use `-cpu host,kvm=off,-hypervisor` flag to hide hypervisor bit
- Hypervisor intercept: Intercept CPUID instruction in hypervisor and return modified values
- Note: Returns value observed on real CPU; malware on re-configured VM gets fooled

**Difficulty rating:** EASY
- Simple to detect from guest
- Well-documented bypass methods
- Most hypervisors provide configuration options to hide this bit

---

### 1.2 Hypervisor Vendor Leaves (0x40000000+)

**How it works:**
- Intel/AMD reserve CPUID leaves 0x40000000 through 0x400000FF for hypervisor software use
- Leaf 0x40000000 returns vendor ID signature in EBX, ECX, EDX registers
- Maximum hypervisor CPUID leaf number also returned in EAX

**Vendor-specific signatures:**
- **VMware:** "VMwareVMware" (stored across EBX, ECX, EDX)
- **Xen:** Returns Xen signature in vendor leaf
- **KVM:** Returns KVM signature
- **Microsoft Hyper-V:** Returns Microsoft Hv signature

**Technical implementation:**
```c
// Check for hypervisor vendor
if (cpuid_hypervisor_bit_set()) {
    cpuid(0x40000000, &eax, &ebx, &ecx, &edx);
    // ebx, ecx, edx contain 12-byte vendor string
    char vendor[13];
    memcpy(vendor, &ebx, 4);
    memcpy(vendor + 4, &ecx, 4);
    memcpy(vendor + 8, &edx, 4);
    vendor[12] = 0;
    // Compare against known signatures
}
```

**Which products use it:**
- Anti-malware sandboxes (Cuckoo, Joe Sandbox)
- Malware evasion mechanisms (checks for analysis environments)
- Proctoring software
- Game anti-cheats (basic checks)

**Known bypass techniques:**
- Intercept CPUID leaf 0x40000000 and return blank/spoofed values
- Return legitimate vendor string (e.g., AuthenticAMD or GenuineIntel)
- Can be neutralized through CPUID instruction interception in hypervisor

**Difficulty rating:** EASY
- Simple string comparison check
- Straightforward to intercept and spoof
- Most hypervisors can modify returned values

---

### 1.3 VMX/SVM Feature Bit Detection

**How it works:**
- Checks for hardware virtualization support flags (Intel VT-x or AMD-V)
- Intel VMX: CPUID.1:ECX[5] = VMX support
- AMD SVM: CPUID.80000001h:ECX[2] = SVM support
- Presence suggests system capable of running nested virtualization
- Can detect if running inside VM by checking IA32_FEATURE_CONTROL MSR

**Technical details:**
- IA32_FEATURE_CONTROL MSR must have:
  - Bit 0 (lock bit) set
  - Bit 1 (enable VMXON outside SMX) or Bit 2 (enable VMXON inside SMX) set
- If VMX is enabled but current privilege level can't use it, suggests VM environment
- Checking if VMXON succeeds or fails reveals virtualization layer

**Which products use it:**
- Hypervisor detection tools (pafish, al-khaser)
- Advanced malware (checks nested virtualization capability)
- Security research tools

**Known bypass techniques:**
- Hide VMX/SVM capability bits via CPUID interception
- Properly configure IA32_FEATURE_CONTROL MSR to avoid detection
- If nested virtualization needed, expose features but ensure consistent behavior
- Can cause VMXON to fault appropriately to mimic physical hardware

**Difficulty rating:** MEDIUM
- Requires MSR access and CPUID interception
- Must maintain consistency between CPUID results and actual CPU behavior
- Nested virtualization scenarios add complexity

---

### 1.4 Brand String Anomalies

**How it works:**
- CPUID leaves 0x80000002 through 0x80000004 return CPU brand string
- Virtual CPUs may expose non-standard or vendor-specific brand strings
- Inconsistencies between reported model and actual behavior detectable
- Some hypervisors append virtualization hints to brand string

**Examples of anomalies:**
- Generic strings like "QEMU Virtual CPU"
- Inconsistent CPU family/model/stepping information
- Missing or altered CPU feature flags
- Brand string doesn't match expected format for physical CPUs

**Which products use it:**
- Malware sandbox evasion
- VM detection libraries (VMAware)
- Anti-cheat systems (secondary check)

**Known bypass techniques:**
- Intercept CPUID leaves 0x80000002-0x80000004 and return authentic CPU brand string
- Copy brand string from actual physical CPU model
- Ensure CPU family/model/stepping values match physical hardware

**Difficulty rating:** EASY
- Simple string comparison
- Easy to intercept and modify CPUID results
- Most hypervisors can configure CPU brand string

---

## 2. Timing-Based Detection

### 2.1 RDTSC/RDTSCP Timing Attacks

**How it works:**
- `RDTSC` (Read Time-Stamp Counter) reads CPU cycle counter
- Virtual machines incur overhead during VM exits, causing timing anomalies
- Anti-cheat measures time between two RDTSC calls around privileged instruction
- Physical hardware: 50-200 cycles typical
- Virtual machine with VM exit: 750-2000+ cycles

**Detection methodology:**
```c
// Standard timing check
uint64_t start = __rdtsc();
__cpuid(0, 0, 0, 0);  // Force VM exit in virtualized environment
uint64_t end = __rdtsc();
uint64_t delta = end - start;
if (delta > 750) {
    // Likely running in VM
}
```

**RDTSCP vs RDTSC:**
- `RDTSC`: Non-serializing, allows instruction reordering
- `RDTSCP`: Serializing instruction, waits for all previous instructions to complete
- `RDTSCP` more reliable for timing measurements
- Sequence `LFENCE; RDTSC` can achieve serialization on older CPUs

**Which products use it:**
- **BattlEye:** Kernel-mode RDTSC timing checks (moved to kernel in recent versions)
- **EasyAntiCheat:** Standard timing attack using rdtsc;cpuid;rdtsc combination
- Malware debugger detection
- Sandbox evasion techniques

**Known bypass techniques:**
1. **TSC offsetting:** Use VMX_TSC_OFFSET to adjust returned TSC values
   ```c
   exec_vmwrite64(VMX_TSC_OFFSET_FULL, val - rdtsc())
   ```
2. **Subtract VM exit overhead:** Track VM exit cycles (1200-2000) and subtract from TSC
3. **Disable RDTSC in ring 3:** Force VM exit and emulate (not ideal, breaks legitimate uses)
4. **Kernel patches:** RDTSC-KVM-Handler project provides Linux kernel patches
5. **QEMU arguments:** `host,rdtscp=off,hv_time,kvm=off,hv_vendor_id=null,-hypervisor`

**Alternative detection via APERF MSR:**
- IA32_APERF MSR (Actual Performance counter) harder to spoof than TSC
- Measures IET (Instruction Execution Time) divergence
- Compares execution time using APERF vs TSC
- Spoofing APERF requires more complex VM exit handling

**Difficulty rating:** MEDIUM-HARD
- Basic TSC offsetting: MEDIUM
- Full evasion including APERF: HARD
- Requires precise timing compensation for VM exits
- Different instructions have different VM exit costs
- Cache state affects timing measurements

---

### 2.2 VM-Exit Latency Measurement

**How it works:**
- Certain instructions force VM exits (transitions from guest to hypervisor)
- Examples: CPUID, VMCALL, IN/OUT port operations, MSR access
- VM exit + VM entry overhead measurable via timing
- Physical execution: consistent, low latency
- Virtual execution: variable, higher latency (hundreds to thousands of cycles)

**Detection pattern:**
```c
// Measure CPUID latency
for (int i = 0; i < 100; i++) {
    uint64_t start = __rdtsc();
    __cpuid(0, 0, 0, 0);
    uint64_t end = __rdtsc();
    timings[i] = end - start;
}
// Analyze statistical distribution
// VM: high variance, occasional spikes
// Physical: consistent, low variance
```

**Which products use it:**
- BattlEye (kernel-level implementation)
- EasyAntiCheat
- VMProtect (anti-debugging/VM detection)
- Academic research on VM detection

**Known bypass techniques:**
- Pre-calculate average VM exit cost and subtract from timing
- Use hardware performance counters to estimate overhead
- Implement TSC deadline timer adjustments
- Cache warming to reduce variance
- Statistical smoothing of returned values

**Challenges in bypassing:**
- Different instructions have different VM exit costs
- Cache state affects measurements
- CPU frequency scaling creates variance
- Nested virtualization adds additional layers of overhead

**Difficulty rating:** HARD
- Requires detailed profiling of VM exit costs
- Must account for variable execution environments
- Statistical analysis can detect smoothing attempts
- Hypervisor must maintain low-latency VM exits

---

### 2.3 Instruction Timing Variance

**How it works:**
- Certain instructions execute faster on physical vs virtual hardware
- Privileged instructions may trap to hypervisor
- Emulated devices have different timing characteristics
- Statistical analysis reveals distribution differences

**Instructions commonly timed:**
- **CPUID:** Already discussed above
- **IN/OUT:** I/O port access (forces VM exit)
- **RDMSR/WRMSR:** Model-specific register access
- **INVD/WBINVD:** Cache invalidation
- **CLI/STI:** Interrupt flag manipulation (may trap in VM)

**Detection approach:**
1. Execute instruction multiple times
2. Collect timing samples
3. Calculate mean, variance, outliers
4. Physical: tight distribution, few outliers
5. Virtual: wider distribution, frequent outliers

**Which products use it:**
- Advanced malware (banking trojans, ransomware)
- VM detection frameworks
- Red team tools

**Known bypass techniques:**
- Intercept and emulate timing-sensitive instructions consistently
- Add artificial jitter to physical-like levels
- Optimize VM exit paths to reduce latency
- Use instruction caching when possible

**Difficulty rating:** MEDIUM-HARD
- Depends on which instructions are tested
- Statistical analysis harder to fool than simple threshold checks
- Requires low-latency hypervisor implementation

---

### 2.4 Cache Timing Attacks

**How it works:**
- CPU caches (L1, L2, L3, TLB) have different performance characteristics in VMs
- Cache hit/miss patterns differ between physical and virtual
- Shared caches between VMs create cross-VM channels
- Eviction patterns reveal hypervisor presence

**Attack methodologies:**

**Flush+Reload:**
- Spatial precision: L1/L2 cache lines (same core) or L3 (cross-core)
- Attacker flushes cache line (CLFLUSH instruction)
- Measures reload time to determine if victim accessed data
- Same-core: 50-100 cycles (hit) vs 200-300 cycles (miss)
- Cross-core: higher latency due to L3

**Prime+Probe:**
- Fill (prime) cache set with attacker's data
- Victim executes
- Probe: measure time to access primed data
- Slower access = victim evicted attacker's data (collision detected)

**Evict+Time:**
- Evict specific cache lines
- Trigger victim operation
- Measure total execution time
- Differences reveal cache usage patterns

**TLB-based attacks:**
- Translation Lookaside Buffer caches virtual-to-physical address translations
- Intel: split L1 TLBs (iTLB for instructions, dTLB for data) + unified L2 (sTLB)
- AMD/Apple: split TLBs at both L1 and L2 levels
- TLB misses have measurable latency differences
- Hypervisor's EPT adds translation layer (guest virtual → guest physical → host physical)
- EPT violations cause VM exits, detectable via timing

**Intel cldemote extension (2025 research):**
- `CLDEMOTE` demotes cache line from upper to lower cache levels
- Facilitates efficient eviction set construction without helper threads
- Execution latency leaks TLB states, page table permissions, translation levels
- Used to derandomize kernel base address in 2.49ms on Linux

**Which products use it:**
- Academic research (USENIX Security 2018, NDSS 2025, ESORICS 2025)
- Advanced persistent threats (APTs)
- Side-channel attack frameworks
- Cloud VM isolation verification ("Home Alone" system)

**Known bypass techniques:**
- Cache partitioning (Intel CAT - Cache Allocation Technology)
- TLB partitioning (secure vs non-secure domains)
- Disable cache timing-sensitive features (CLDEMOTE, CLFLUSH)
- Add noise/jitter to cache access patterns
- Constant-time execution implementations
- Hardware: Intel CET, ARM MTE for memory isolation

**Difficulty rating:** HARD
- Requires deep understanding of cache microarchitecture
- Different CPU models have different cache designs
- Intel sTLB makes split-TLB attacks unreliable
- Hypervisor can't fully hide EPT translation overhead
- Statistical noise difficult to eliminate entirely

---

## 3. Memory-Based Detection

### 3.1 EPT Violation Patterns

**How it works:**
- Extended Page Tables (EPT) / Second Level Address Translation (SLAT) enable nested paging
- Hypervisor controls guest physical to host physical mapping
- EPT access rights: Read, Write, Execute (can be set independently)
- EPT violation occurs when guest accesses page without proper permissions
- Detectable via timing (VM exit overhead) and behavioral patterns

**EPT violation triggers:**
- **Read violation:** Guest reads non-readable page
- **Write violation:** Guest writes to read-only page (e.g., code integrity checks)
- **Execute violation:** Guest executes non-executable page (DEP/NX enforcement)
- **Misconfiguration:** Invalid EPT entry

**Detection vectors:**

1. **Timing-based detection:**
   - Memory access that triggers EPT violation has VM exit overhead
   - Normal memory access: tens of cycles
   - EPT violation: hundreds to thousands of cycles
   - Repeated access to same address with variable timing = EPT monitoring

2. **Behavioral detection:**
   - Write to code section triggers EPT violation for hypervisor hooks
   - Integrity checks (read memory, compute hash) may hit shadowed pages
   - Execution redirection via execute-only EPT pages

**EPT hooking techniques:**

**Single EPT model (Illusion):**
- One EPT per logical processor
- In-place patching with VM-exit instructions (VMCALL)
- Monitor Trap Flag (MTF) stepping to replay original bytes

**Dual EPT model (Matrix):**
- Primary EPT: Read/Write permissions, original code
- Secondary EPT: Execute-only permissions, hook trampolines
- INT3 breakpoints trigger EPTP switching during EPT violations
- Reads/writes see original code, execution sees hooks

**Which products use it:**
- Hypervisor-based game cheats (hypervisor hooks game functions)
- Hypervisor introspection systems (security monitoring)
- Memory forensics tools
- Advanced rootkits

**Known bypass techniques:**
- **For attackers using EPT hooks:**
  - Minimize EPT violations (optimize hook placement)
  - Use MTF single-stepping to hide hooks during integrity checks
  - Shadow pages: present original code on read, hooks on execute

- **For defenders detecting EPT hooks:**
  - Timing analysis of memory accesses
  - Side-channel attacks (cache-based detection of split I/D views)
  - Test for execution vs read discrepancies
  - Monitor for unexplained VM exits

**Performance considerations:**
- EPT violations expensive (VM exit required)
- Page-level granularity (4KB minimum, 2MB/1GB large pages)
- INVEPT required after EPT modifications (TLB invalidation)
- Per-logical-processor TLBs require INVEPT on all cores

**Difficulty rating:** HARD
- EPT hooking itself: MEDIUM-HARD (requires hypervisor development)
- Detecting EPT hooks: HARD (side-channels, timing analysis)
- Hiding EPT hooks perfectly: VERY HARD (cache/TLB timing leaks hard to eliminate)

---

### 3.2 TLB Behavior Anomalies

**How it works:**
- Translation Lookaside Buffer (TLB) caches virtual-to-physical address translations
- Virtualization adds EPT layer: guest virtual → guest physical → host physical
- TLB must cache both guest page tables and EPT translations
- TLB miss patterns differ between physical and virtual systems

**TLB architecture:**

**Intel (typical):**
- L1: Split iTLB (instructions) and dTLB (data)
- L2: Unified sTLB (shared, victim cache for L1 evictions)
- Tagged by PCID (Process Context ID) and VPID (Virtual Processor ID)

**AMD/Apple:**
- L1: Split TLBs
- L2: Split TLBs
- Different tagging mechanisms

**GPU (NVIDIA):**
- L1 TLBs per processing cluster
- L2 TLB (large)
- L3 TLB (even larger for massive multithreading)

**Virtualization impact:**
- VMX transitions may flush TLBs (expensive)
- VPID feature allows TLB retention across VM exits (tags TLB entries)
- EPT adds second translation layer
- TLB must cache EPT translations separately
- TLB miss on EPT walk measurably slower

**Detection techniques:**

1. **Split-TLB attacks (older):**
   - Exploit separate iTLB and dTLB caches
   - Fill dTLB with attacker's translations
   - Execute code: iTLB miss, dTLB hit
   - Timing difference reveals TLB state
   - **Note:** Intel sTLB (victim cache) makes this unreliable on modern CPUs
   - sTLB caches evicted iTLB/dTLB entries; lookups promote back to L1

2. **EPT TLB detection:**
   - Access pattern that causes EPT TLB misses
   - EPT page walk: 4-level walk (or 5-level with LA57)
   - Measurable latency increase
   - Compare against direct physical memory access timing

3. **VPID/PCID analysis:**
   - VPID tags TLB entries per virtual processor
   - PCID tags TLB entries per process
   - VPID=0 or inconsistent VPID usage suggests VM
   - Check IA32_VMX_EPT_VPID_CAP MSR for VPID support

**Which products use it:**
- Security research (USENIX papers on TLB attacks)
- Advanced malware (side-channel information leakage)
- Hypervisor introspection systems
- TLB partitioning for secure computation

**Known bypass techniques:**
- Enable VPID to avoid TLB flushes (improves performance, harder to detect)
- TLB partitioning: separate secure vs non-secure TLB domains
- Consistent TLB management (don't create detectable anomalies)
- Pre-warm TLBs before timing-sensitive operations
- Disable split-TLB exploits by using sTLB (hardware feature)

**Difficulty rating:** HARD
- Requires detailed TLB microarchitecture knowledge
- CPU-specific (Intel vs AMD vs ARM different designs)
- Modern CPUs (sTLB) mitigate split-TLB attacks
- EPT overhead fundamental, cannot be fully hidden
- Statistical timing analysis required

---

### 3.3 Memory Mapping Inconsistencies

**How it works:**
- Virtual machines have simulated memory maps different from physical
- Hypervisor reserves memory regions for its own use
- MMIO (Memory-Mapped I/O) regions may differ
- Guest physical address space != host physical address space

**Detectable inconsistencies:**

1. **Memory size/layout:**
   - VM may report different total RAM than host
   - Memory holes for hypervisor reserved regions
   - Non-standard memory bank configurations
   - NUMA node topology differences

2. **Page table structure:**
   - EPT adds translation layer (detectable via timing)
   - Page table A/D (Accessed/Dirty) bit updates cause EPT violations
   - Hypervisor introspection may monitor guest page tables
   - Unusual VM exit patterns on page table writes

3. **Physical address mapping:**
   - Guest physical addresses don't match host physical
   - IOMMU/VT-d may remap DMA addresses
   - PCI BAR (Base Address Register) locations differ
   - Memory-mapped device regions in unexpected locations

**Which products use it:**
- Hypervisor introspection (monitors guest page tables for attacks)
- Advanced malware (checks for memory map anomalies)
- VM detection tools

**Known bypass techniques:**
- Present consistent memory map to guest
- Hide hypervisor memory regions
- Emulate expected NUMA topology
- Minimize EPT violations on page table accesses (e.g., monitor A/D bits carefully)
- Use 1GB large pages to reduce page table overhead

**Difficulty rating:** MEDIUM
- Memory layout easier to spoof than timing
- Some inconsistencies unavoidable (EPT overhead)
- Depends on sophistication of detection

---

### 3.4 MMIO Region Detection

**How it works:**
- Memory-Mapped I/O uses specific physical address ranges for device access
- Virtual devices have MMIO regions for emulated hardware
- Access to MMIO triggers VM exit (EPT violation or instruction emulation)
- MMIO patterns differ from real hardware

**Common MMIO detection vectors:**

1. **PCI configuration space:**
   - PCI devices have MMIO BARs for configuration
   - Virtual devices: BAR addresses in hypervisor-controlled ranges
   - Physical devices: BAR addresses match chipset/BIOS allocation
   - Inconsistent BAR sizes or alignments

2. **APIC/MSI regions:**
   - Local APIC at 0xFEE00000 (x86)
   - Virtual APIC page may be backed by EPT
   - x2APIC mode uses MSRs instead of MMIO (harder to detect)
   - APIC access timing differences

3. **Device-specific MMIO:**
   - GPU framebuffer regions
   - Network card registers
   - Storage controller registers
   - Virtual devices may have different MMIO access performance

**Detection approach:**
1. Enumerate PCI devices and their BARs
2. Access MMIO regions and measure timing
3. Compare against expected physical hardware behavior
4. Look for EPT violation patterns (timing spikes)

**Which products use it:**
- Driver-level malware (needs to interact with hardware directly)
- VM detection libraries
- Hardware security modules (HSMs) verifying physical presence

**Known bypass techniques:**
- Emulate MMIO access with low latency
- Use passthrough for real devices (SR-IOV, VFIO)
- Present BARs in expected address ranges
- Optimize EPT for MMIO regions (large pages if possible)
- Implement x2APIC to avoid MMIO APIC access

**Difficulty rating:** MEDIUM
- MMIO timing overhead hard to hide completely
- Device passthrough solves problem but limits VM flexibility
- Some MMIO unavoidable (virtual devices)

---

## 4. Hardware-Based Detection

### 4.1 SMBIOS/ACPI Table Strings

**How it works:**
- SMBIOS (System Management BIOS) provides hardware information tables
- ACPI (Advanced Configuration and Power Interface) tables describe system hardware
- Virtual machines populate these tables with VM-identifying strings
- Detection: scan tables for vendor signatures

**SMBIOS detection strings:**

**VirtualBox:**
- System Manufacturer: "innotek GmbH" or "Oracle Corporation"
- Product Name: "VirtualBox"
- BIOS Version: "VBOX"
- Serial numbers: "0" or "VirtualBox"

**VMware:**
- System Manufacturer: "VMware, Inc."
- Product Name: "VMware Virtual Platform" or "VMware7,1"
- BIOS Version: Contains "VMware"
- Serial numbers: "VMware-XX XX XX..."

**QEMU/KVM:**
- System Manufacturer: "QEMU"
- Product Name: "Standard PC (i440FX + PIIX, 1996)" or similar
- BIOS Version: Contains "QEMU"

**Hyper-V:**
- System Manufacturer: "Microsoft Corporation"
- Product Name: "Virtual Machine"
- BIOS Version: May contain "Hyper-V" or "VRTUAL"

**Detection via Windows registry:**
```
HKLM\HARDWARE\Description\System
- SystemBiosVersion = "VBOX" / "QEMU" / etc.

HKLM\SOFTWARE\Oracle\VirtualBox Guest Additions
- Presence indicates VirtualBox
```

**Which products use it:**
- Anti-cheat systems (basic checks)
- Malware sandbox evasion (QakBot, banking trojans)
- Proctoring software
- VM detection tools (pafish, al-khaser, VMAware)
- Type 1 hypervisor detection tools (hvdetecc)

**Known bypass techniques:**
1. **SMBIOS spoofing:**
   - Edit SMBIOS tables to match physical hardware
   - VMware: Use `.vmx` file to set SMBIOS strings
   - QEMU: `-smbios type=X,field=value` arguments
   - VirtualBox: `VBoxManage setextradata` commands

2. **ACPI table modification:**
   - Edit ACPI tables (RSDP, RSDT, XSDT, MADT, FADT, etc.)
   - ACRN Device Model builds ACPI tables at 0xF2400
   - Modify ACPI OEM ID and OEM Table ID fields
   - Ensure consistency across all tables

3. **Registry spoofing (Windows):**
   - Modify registry keys before detection runs
   - Can be done via hypervisor introspection or guest agent
   - Risk: may break guest tools/drivers

4. **Pass-through SMBIOS (security VMs):**
   - ACRN allows passing physical SMBIOS to security VMs
   - Guest sees real hardware information
   - Only works for specific VM types

**Example QEMU SMBIOS spoofing:**
```bash
-smbios type=0,vendor="American Megatrends Inc.",version="2.00",date="04/01/2014" \
-smbios type=1,manufacturer="ASUS",product="Z97-PRO",serial="System-Serial-Number"
```

**Difficulty rating:** EASY
- Simple string checks, trivial to bypass
- All major hypervisors support SMBIOS/ACPI customization
- Must ensure consistency across all tables
- Some guest tools may break with spoofed values

---

### 4.2 PCI Device Fingerprints

**How it works:**
- PCI (Peripheral Component Interconnect) devices identified by Vendor ID (VID) and Device ID (DID)
- Virtual devices use VM-specific VID/DID combinations
- Windows Device Manager, registry, and WMI expose PCI device information
- Detection: enumerate PCI bus and check for virtual device signatures

**Common virtual PCI device signatures:**

**VirtualBox:**
- VBoxVGA: `PCI\VEN_80EE&DEV_BEEF&SUBSYS_00000000`
- VBoxSVGA: `PCI\VEN_80EE&DEV_BEEF&SUBSYS_040515AD`
- VirtualBox Guest Service: VEN_80EE

**VMware:**
- VMware SVGA II: `PCI\VEN_15AD&DEV_0405&SUBSYS_040515AD`
- VMware VMXNET3: VEN_15AD
- VMware paravirtual SCSI: VEN_15AD

**QEMU:**
- QEMU `-vga std`: `PCI\VEN_1234&DEV_1111`
- QEMU `-vga vmware`: `PCI\VEN_15AD&DEV_0405` (same as VMware)
- VirtIO devices: `VEN_1AF4` (VirtIO network, block, etc.)
- Red Hat VirtIO: `VEN_1B36`

**Hyper-V:**
- Hyper-V vPCI devices presented via VMBus
- Do not appear in ACPI tables (dynamically added/removed)
- Device IDs: Microsoft-specific

**Detection strings in registry/WMI:**
```
Common patterns:
- "vbox", "vid_80ee"
- "qemu", "ven_1af4", "ven_1b36", "subsys_11001af4"
- "vmware", "ven_15ad"
- "hyper", "microsoft"
```

**Windows registry locations:**
```
HKLM\SYSTEM\CurrentControlSet\Enum\PCI\
- Subkeys contain VEN_XXXX&DEV_YYYY
```

**Which products use it:**
- Advanced malware (VMProtect, QakBot)
- Anti-cheat systems (Plug and Play device ID analysis)
- VM detection tools (pafish, VMAware)
- Proctoring software

**Known bypass techniques:**

1. **Device passthrough (VFIO, SR-IOV):**
   - Pass real PCI devices to guest
   - Guest sees physical VID/DID
   - Best solution but requires hardware support
   - Limits VM flexibility (device tied to one VM)

2. **PCI ID spoofing:**
   - Modify emulated device VID/DID
   - QEMU: Limited support, may break drivers
   - VirtualBox: Can change some IDs via `VBoxManage`
   - Risk: guest drivers may fail to load

3. **Hide virtual devices:**
   - Use generic PCI devices where possible
   - Example: PS/2 mouse (generic ID, not VM-specific)
   - VMSVGA shares ID with different devices (sloppy software won't detect)

4. **Registry/WMI spoofing (risky):**
   - Modify registry to hide PCI device entries
   - Intercept WMI queries via hypervisor
   - May break legitimate software
   - Inconsistencies detectable

5. **Driver signature spoofing:**
   - Modify device driver .inf files
   - Present as generic device
   - Windows driver signing makes this difficult

**Challenges:**
- VBoxHardenedLoader bypassed by checking PNP device IDs via WMI
- Switching device types (e.g., VMSVGA to VBoxSVGA) may help against sloppy checks
- Full spoofing causes driver issues
- Detection can be very thorough (50+ device IDs checked)

**Difficulty rating:** MEDIUM
- Simple checks: EASY (just enumerate PCI devices)
- Full bypass: MEDIUM-HARD (device passthrough or complex spoofing)
- Driver compatibility issues make perfect spoofing difficult
- Trade-off between stealth and functionality

---

### 4.3 MAC Address Prefixes

**How it works:**
- Network adapters have MAC addresses with vendor-specific prefixes (OUI - Organizationally Unique Identifier)
- First 3 bytes (24 bits) identify manufacturer
- Virtual NICs use VM vendor's OUI
- Detection: check MAC address against known VM prefixes

**VMware MAC prefixes:**
- **00:50:56** - Generated by vCenter
- **00:0C:29** - Generated by ESXi host (not joined to vCenter)
- **00:05:69** - Generated using hash algorithm (legacy)

**VMware MAC generation:**
- Automatic mode with vCenter: 00:50:56 prefix
- Automatic mode standalone ESXi: 00:0C:29 + last 3 bytes of VM UUID
- Manual/static mode: Must start with 00:50:56 (enforced by ESXi)
- Legacy algorithm: 00:05:69 + hash of IP/config

**Other VM platforms:**
- **Microsoft Hyper-V / Virtual Server / Virtual PC:** 00:03:FF
- **Parallels Desktop / Workstation / Server / Virtuozzo:** 00:1C:42
- **Virtual Iron 4:** 00:0F:4B
- **Red Hat Xen:** 00:16:3E
- **Oracle VM:** 00:16:3E
- **XenSource:** 00:16:3E
- **Novell Xen:** 00:16:3E
- **Sun xVM VirtualBox:** 08:00:27
- **KVM (typical):** 52:54:00

**Detection methods:**
1. Query network adapter MAC via API (Windows: GetAdaptersInfo, ipconfig)
2. Read from registry: `HKLM\SYSTEM\CurrentControlSet\Control\Class\{4D36E972-E325-11CE-BFC1-08002BE10318}\`
3. Parse output of system commands (ifconfig, ip link)
4. Compare first 3 bytes against known VM OUI database

**Which products use it:**
- Malware sandbox evasion
- Anti-cheat systems (network fingerprinting)
- VM detection libraries
- Network security tools

**Known bypass techniques:**

1. **Manual MAC assignment:**
   - Set MAC to legitimate vendor OUI (e.g., Intel, Realtek)
   - VMware: Edit .vmx file: `ethernet0.addressType = "static"` and `ethernet0.address = "XX:XX:XX:XX:XX:XX"`
   - VirtualBox: `VBoxManage modifyvm "VM Name" --macaddress1 XXXXXXXXXXXX`
   - QEMU: `-netdev ...,mac=XX:XX:XX:XX:XX:XX`

2. **Locally Administered Addresses (LAA):**
   - Set bit 1 of first byte (locally administered)
   - VMware supports LAA for larger address spaces
   - Example: 02:XX:XX:XX:XX:XX (locally administered unicast)

3. **OUI database poisoning (detection side):**
   - Attackers could register legitimate OUI for VM use
   - Expensive ($3000+ for OUI registration)
   - Unrealistic for most scenarios

4. **Prefix-based allocation (VMware):**
   - Modern VMware uses prefix-based MAC allocation
   - Can configure non-standard prefixes
   - Requires vCenter/NSX-T configuration

**Windows registry spoofing (post-boot):**
- Modify NetworkAddress value in adapter's registry key
- May not survive reboot or network restart
- Inconsistencies detectable (actual vs reported MAC)

**Difficulty rating:** EASY
- Trivial to detect (simple string comparison)
- Trivial to bypass (manual MAC configuration)
- All hypervisors support custom MAC addresses
- No functional downside to spoofing MAC

---

### 4.4 Disk/GPU Vendor Strings

**How it works:**
- Storage devices and GPUs expose vendor strings via various interfaces
- Virtual disks and emulated GPUs have VM-identifying strings
- Detection queries device information via OS APIs, SMART, or direct hardware access

**Disk detection vectors:**

1. **SCSI/IDE controller identification:**
   - Virtual controllers expose vendor names in registry/WMI
   - **QEMU:** `HARDWARE\DEVICEMAP\Scsi\...\Identifier = "QEMU"`
   - **VirtualBox:** `HARDWARE\DEVICEMAP\Scsi\...\Identifier = "VBOX"`
   - **VMware:** `HARDWARE\DEVICEMAP\Scsi\...\Identifier = "VMware"`

2. **Disk vendor/model strings:**
   - Virtual disks: "QEMU HARDDISK", "VBOX HARDDISK", "VMware Virtual disk"
   - Physical disks: "Samsung SSD 970 EVO", "WDC WD10EZEX", etc.
   - Query via WMI: `Win32_DiskDrive.Model`, `Win32_DiskDrive.Manufacturer`

3. **Disk serial numbers:**
   - Virtual disks often have generic/sequential serials
   - Physical disks have unique manufacturer serials
   - Anti-cheat HWID bans track disk serials
   - Query via WMI: `Win32_DiskDrive.SerialNumber`

4. **SMART data (Self-Monitoring, Analysis, and Reporting Technology):**
   - Physical disks expose SMART attributes (temperature, power-on hours, etc.)
   - Virtual disks may not implement SMART or provide fake data
   - Inconsistencies detectable (e.g., 0 power-on hours, no temperature)

**GPU detection vectors:**

1. **GPU vendor/device strings:**
   - Virtual GPUs expose VM vendor names
   - **VirtualBox:** "VirtualBox Graphics Adapter"
   - **VMware:** "VMware SVGA 3D"
   - **QEMU:** "QXL", "VGA compatible controller"
   - Query via WMI: `Win32_VideoController.Name`, `Win32_VideoController.VideoProcessor`

2. **GPU driver version:**
   - Virtual GPUs use VM-provided drivers
   - Driver .inf files contain vendor info
   - Registry: `HKLM\SYSTEM\CurrentControlSet\Control\Class\{4d36e968-e325-11ce-bfc1-08002be10318}\`

3. **GPU BIOS/VBIOS version:**
   - Virtual GPUs may report "VBOX VBIOS" or similar
   - Registry: `HARDWARE\Description\System\VideoBiosVersion`
   - Physical GPUs: manufacturer-specific BIOS version

4. **GPU PCI device ID:**
   - Already covered in section 4.2
   - VEN_80EE (VirtualBox), VEN_1234 (QEMU), VEN_15AD (VMware)

**Which products use it:**
- Anti-cheat HWID bans (disk serial tracking)
- Malware sandbox evasion
- VM detection tools (VMAware, pafish)
- Proctoring software

**Known bypass techniques:**

**Disk spoofing:**
1. **SCSI controller spoofing:**
   - VMware: Edit .vmx to use LSI controller (more generic)
   - QEMU: `-device lsi53c895a` or IDE controller
   - Modify registry Identifier value (risky, may break boot)

2. **Disk model/serial spoofing:**
   - QEMU: `-drive ...,model=Samsung_SSD_870,serial=S5XXXXXXXXXXX`
   - VirtualBox: `VBoxManage setextradata "VM" "VBoxInternal/Devices/*/Config/ModelNumber" "Samsung SSD 870"`
   - VMware: Limited support, may require registry modification in guest

3. **SMART emulation:**
   - Implement fake SMART data with realistic values
   - Report non-zero power-on hours, reasonable temperature
   - Requires hypervisor modification or guest-level spoofing

**GPU spoofing:**
1. **GPU passthrough (best solution):**
   - VFIO/PCI passthrough of physical GPU
   - Guest sees real GPU vendor/model
   - Requires VT-d/AMD-Vi support, IOMMU groups
   - GPU unavailable to host during VM use

2. **vGPU (virtual GPU):**
   - NVIDIA GRID, AMD MxGPU: professional virtualized GPU
   - Presents as physical GPU model
   - Expensive, requires enterprise licenses

3. **Driver/registry modification:**
   - Modify GPU device name in registry (post-boot)
   - Change driver .inf files to present as physical GPU
   - Inconsistencies with PCI IDs detectable

4. **Generic GPU selection:**
   - Use most generic virtual GPU available
   - Example: Standard VGA adapter (less identifiable)
   - Performance penalty, limited features

**HWID spoofer tools:**
- Commercial HWID spoofers target disk/GPU serials
- Kernel-mode drivers intercept WMI/registry queries
- Return spoofed values to anti-cheat
- Detection: kernel callback monitoring, driver signature checks

**Difficulty rating:** MEDIUM
- Disk/GPU model strings: EASY to detect, MEDIUM to bypass (configuration changes)
- Disk serial numbers: EASY to detect, MEDIUM to bypass (HWID spoofers or config)
- SMART data: MEDIUM to detect (requires specific queries), MEDIUM to bypass (emulation)
- GPU passthrough: Best bypass but requires hardware support (HARD to set up, EASY once configured)
- Perfect spoofing without passthrough: HARD (consistency across all query methods)

---

## 5. Behavioral Detection

### 5.1 I/O Port Backdoor Probing

**How it works:**
- VMware uses I/O ports 0x5658 (VX) and 0x5659 (high-bandwidth) for guest-host communication
- Known as "VMware backdoor" - used by VMware Tools
- Hypervisor traps IN/OUT instructions on these ports and services requests
- Detection: attempt I/O port access and check for VMware-specific behavior

**Technical details:**

**Port 0x5658 (traditional backdoor):**
- Guest issues IN/OUT instructions with register convention
- Magic number: 0x564D5868 ("VMXh") in EAX
- Command number in ECX
- Hypervisor intercepts and handles request
- Used for low-bandwidth data transfer

**Port 0x5659 (high-bandwidth backdoor):**
- Allows passing pointers to data buffers
- VMware can read/write guest memory directly
- Faster for large data transfers (RPC mechanism)
- Modern VMware Tools primarily use this port

**Detection code example:**
```c
// VMware detection via backdoor port
#define VMWARE_MAGIC 0x564D5868
#define VMWARE_PORT  0x5658

uint32_t eax = VMWARE_MAGIC;
uint32_t ebx = 0;
uint32_t ecx = 10; // Get VMware version
uint32_t edx = VMWARE_PORT;

__asm__ volatile(
    "in %%dx, %0"
    : "+a"(eax), "+b"(ebx), "+c"(ecx), "+d"(edx)
);

if (ebx == VMWARE_MAGIC) {
    // Running in VMware
}
```

**Which products use it:**
- Malware (QakBot, banking trojans)
- VM detection libraries (pafish, al-khaser)
- Anti-cheat systems (basic VM checks)

**Known bypass techniques:**

1. **Disable backdoor in VMware configuration:**
   - Edit .vmx file: `isolation.tools.getPtrLocation.disable = "TRUE"`
   - Disables backdoor functionality
   - May break VMware Tools features

2. **Intercept I/O port access:**
   - Hypervisor intercepts IN/OUT to 0x5658/0x5659
   - Return values that mimic "port not available" or physical hardware
   - Risk: breaks VMware Tools if they rely on backdoor

3. **Trap and emulate:**
   - Generate exception on backdoor port access from untrusted code
   - VMware Tools still work (trusted)
   - Malware detection code fails (untrusted)
   - Complex to implement (requires ring0 filtering)

**Issues with interception:**
- Activating VM-exit on I/O port 0x5658 can crash vmtoolsd.exe
- VMware Tools depend on backdoor for core functionality
- Disabling completely breaks guest tools
- Selective interception (by process) possible but complex

**Difficulty rating:** EASY
- Trivial to detect (simple I/O port access)
- MEDIUM to bypass without breaking VMware Tools
- Trade-off between stealth and functionality
- Non-VMware hypervisors not affected

---

### 5.2 MSR Access Patterns

**How it works:**
- Model-Specific Registers (MSRs) control CPU features and performance
- Accessed via RDMSR (read) and WRMSR (write) privileged instructions
- Certain MSRs used by hypervisors or reveal virtualization artifacts
- Access patterns and values differ between physical and virtual

**Key MSRs for VM detection:**

1. **IA32_FEATURE_CONTROL (0x3A):**
   - Controls VMX (Intel VT-x) and SMX (Trusted Execution) enablement
   - Bit 0: Lock bit (1 = locked, 0 = unlocked)
   - Bit 1: Enable VMXON outside SMX
   - Bit 2: Enable VMXON inside SMX
   - **Detection:** If lock bit set and VMXON enabled, but VMXON instruction fails → in VM
   - **Physical:** VMXON succeeds if enabled, fails if disabled
   - **Virtual:** VMXON typically faults or triggers VM exit

2. **IA32_VMX_* MSRs (0x480-0x491):**
   - Report VMX capabilities (if VMX supported)
   - **Physical:** Readable if VMX supported, values match CPU model
   - **Virtual:** May not be accessible or return emulated values
   - **Detection:** Inconsistencies between CPUID VMX bit and MSR accessibility

3. **IA32_LSTAR (0xC0000082):**
   - Contains address of syscall handler (KiSystemCall64 on Windows)
   - **Hypervisor use:** Intercept WRMSR to IA32_LSTAR to find kernel base address
   - **Detection:** Unusual access patterns to this MSR (e.g., multiple writes, timing)

4. **IA32_EFER (0xC0000080):**
   - Extended Feature Enable Register
   - Controls long mode, NX bit, SYSCALL/SYSRET enable
   - **Anti-cheat monitoring:** EasyAntiCheat queries IA32_EFER after ~30 minutes
   - **Detection:** If hypervisor intercepts RDMSR, timing overhead detectable

5. **Synthetic MSRs (0x40000000-0x400000FF):**
   - Hypervisor-specific MSRs (Hyper-V, KVM)
   - **Hyper-V:** 0x40000000-0x400000FF for guest-host communication
   - **KVM:** Similar range for paravirtualization features
   - **Detection:** Probing these MSRs succeeds in VM, faults on physical hardware

6. **IA32_APERF (0xE8):**
   - Actual Performance counter (increments at actual CPU frequency)
   - Used for Instruction Execution Time (IET) divergence detection
   - Harder to spoof than TSC (timestamp counter)
   - **Detection:** Compare APERF vs TSC timing for divergence

**Detection patterns:**

1. **MSR accessibility:**
   - Attempt to read MSRs that should/shouldn't exist
   - Physical: Faults on non-existent MSRs (GPF - General Protection Fault)
   - Virtual: May return emulated value or different fault behavior

2. **MSR timing:**
   - RDMSR/WRMSR normally fast (~tens of cycles)
   - Hypervisor intercept: VM exit overhead (hundreds to thousands of cycles)
   - Repeated access with timing measurement reveals VM

3. **Value consistency:**
   - MSR values should match CPUID-reported features
   - Example: CPUID says VMX supported, but IA32_VMX_BASIC unreadable → inconsistency
   - Emulated MSRs may return incorrect values for CPU model

**Which products use it:**
- **EasyAntiCheat:** Queries IA32_EFER MSR
- **BattlEye:** MSR access monitoring
- Advanced malware (hypervisor detection)
- Security research tools

**Known bypass techniques:**

1. **MSR interception and emulation:**
   - Hypervisor intercepts RDMSR/WRMSR on sensitive MSRs
   - Return values consistent with physical CPU
   - Minimize VM exit overhead (optimize hypervisor path)

2. **MSR passthrough:**
   - Allow guest direct access to MSRs (no interception)
   - Risky: guest can read/write host-affecting MSRs
   - Only safe for read-only, non-sensitive MSRs

3. **Synthetic MSR hiding:**
   - Intercept access to 0x40000000+ range
   - Return GPF exception (mimic physical CPU behavior)
   - Hypervisor paravirtualization features disabled

4. **Timing mitigation:**
   - TSC offsetting to hide VM exit overhead
   - Low-latency hypervisor design
   - Pre-calculate and cache MSR values

5. **Fault injection:**
   - Hypervisor uses invalid MSR/CR/XSETBV access as detection method
   - **Counter:** Emulate expected fault behavior precisely

**Example attack (write to IA32_LSTAR):**
- If attacker can write to IA32_LSTAR via WRMSR, they control syscall entry point
- Redirect syscalls to attacker code or ROP gadget
- Arbitrary kernel execution on next syscall
- **Defense:** Hypervisor must intercept WRMSR to IA32_LSTAR

**Difficulty rating:** MEDIUM-HARD
- Basic MSR checks: EASY to detect, MEDIUM to bypass (interception)
- Timing-based MSR detection: MEDIUM to detect, HARD to bypass (low-latency required)
- Consistency checks: MEDIUM to detect, MEDIUM to bypass (accurate emulation)
- IA32_APERF spoofing: HARD (more complex than TSC)

---

### 5.3 Exception Handling Timing

**How it works:**
- Exceptions (faults, traps, aborts) have different handling overhead in VMs
- Hypervisor may intercept certain exceptions (e.g., #GP, #UD, #PF)
- VM exit on exception adds measurable latency
- Statistical analysis of exception timing reveals virtualization

**Exception types and VM behavior:**

1. **#GP (General Protection Fault, vector 13):**
   - Triggered by: Invalid MSR access, protected instruction, segment violation
   - **Physical:** CPU delivers to OS exception handler (fast, ~hundreds of cycles)
   - **Virtual:** May cause VM exit if hypervisor monitors (thousands of cycles)
   - **Detection:** Trigger intentional #GP, measure handling time

2. **#UD (Undefined Opcode, vector 6):**
   - Triggered by: Invalid/undefined instruction
   - **Physical:** Direct to exception handler
   - **Virtual:** Some hypervisors intercept to emulate instructions
   - **Detection:** Execute undefined opcode, time exception delivery

3. **#PF (Page Fault, vector 14):**
   - Triggered by: Invalid page access, page not present
   - **Physical:** Direct to OS page fault handler
   - **Virtual:** EPT violations cause VM exit, then guest #PF injection
   - **Detection:** Cause page fault, measure time to handler
   - **Note:** EPT violations add significant overhead

4. **#DB (Debug Exception, vector 1):**
   - Triggered by: Hardware breakpoints (DR0-DR7), single-step (TF flag)
   - **Physical:** Direct to debugger/exception handler
   - **Virtual:** Hypervisors may intercept to hide their presence or implement features
   - **Detection:** Set hardware breakpoint, measure exception time

**Detection methodology:**
```c
// Measure exception handling time
uint64_t measure_exception_time() {
    uint64_t start = __rdtsc();

    __try {
        // Trigger exception (e.g., access invalid MSR)
        __readmsr(0xFFFFFFFF);  // Non-existent MSR
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        // Exception handled
    }

    uint64_t end = __rdtsc();
    return end - start;
}

// Collect samples and analyze distribution
for (int i = 0; i < 1000; i++) {
    timings[i] = measure_exception_time();
}
// Physical: tight distribution, low mean
// Virtual: wider distribution, occasional spikes
```

**Structured/Vectored Exception Handling (SEH/VEH):**
- Windows exception dispatch mechanism
- SEH: Stack-based exception handlers
- VEH: Vectored exception handlers (registered via AddVectoredExceptionHandler)
- Hypervisors may monitor exception dispatcher (KiUserExceptionDispatcher)
- **Detection:** Timing differences in SEH/VEH dispatch

**Which products use it:**
- VMProtect (anti-debugging, anti-VM)
- Advanced malware (exception timing analysis)
- Rootkits (hypervisor detection before deployment)

**Known bypass techniques:**

1. **Minimize exception interception:**
   - Only intercept exceptions when necessary
   - Pass through most exceptions directly to guest
   - Reduces VM exit overhead

2. **Optimize exception VM exit path:**
   - Fast hypervisor exception injection
   - Pre-calculate exception error codes
   - Minimize context switches

3. **Exception timing normalization:**
   - Artificially delay exception delivery on physical CPU
   - Add jitter to hypervisor exception path
   - Makes timing distributions overlap
   - **Risk:** May degrade performance

4. **Intercept exception timing measurements:**
   - Detect RDTSC around exception triggers
   - Adjust TSC values to hide overhead
   - Complex pattern recognition required

**Challenges:**
- EPT violations fundamentally add overhead (hard to hide)
- Exception dispatch path complex (many steps, hard to optimize all)
- Statistical analysis robust against simple timing adjustments

**Difficulty rating:** HARD
- Detection: MEDIUM (requires exception triggering and timing)
- Bypass: HARD (exception overhead difficult to hide completely)
- EPT-related exceptions especially problematic
- Perfect timing normalization very difficult

---

### 5.4 Debug Register Behavior

**How it works:**
- x86/x64 CPUs have 8 debug registers: DR0-DR7
- DR0-DR3: Debug address registers (hardware breakpoint addresses)
- DR4-DR5: Reserved (mapped to DR6/DR7 if CR4.DE=0)
- DR6: Debug status register (which breakpoint triggered)
- DR7: Debug control register (enable/configure breakpoints)
- Hypervisors and anti-cheats use debug registers for monitoring and protection

**Debug register functionality:**

**DR0-DR3 (Address-Breakpoint Registers):**
- Store linear addresses of breakpoints
- When CPU accesses these addresses, debug exception (#DB) triggered
- Conditions: Read, Write, Execute, I/O access (depending on DR7 configuration)

**DR6 (Debug Status Register):**
- Bits 0-3: Set when corresponding hardware breakpoint (DR0-DR3) triggered
- Bit 13: BD (Debug Register Access Detected) - set if access to DR while GD=1
- Bit 14: BS (Single Step) - set if #DB from single-stepping (TF flag)
- Bit 15: BT (Task Switch) - set if #DB from task switch

**DR7 (Debug Control Register):**
- Bits 0-7: L0-L3 (local enable), G0-G3 (global enable) for DR0-DR3
- Bits 16-31: Breakpoint conditions and sizes for DR0-DR3
- Bit 13: GD (General Detect Enable) - causes #DB on debug register access

**Usage in anti-cheat systems:**

1. **Hardware breakpoint hooking:**
   - Set DR0-DR3 to addresses of important functions
   - DR7 configures trigger condition (e.g., execute)
   - When game executes hooked function → #DB exception → cheat code runs
   - **Advantage:** No .text section modification (stealthier than inline hooks)

2. **Anti-cheat detection of hardware breakpoints:**
   - VMProtect and other anti-cheats detect debug register use
   - Check DR0-DR7 values directly (requires ring 0 access)
   - **Kernel-level anti-cheats:** Can read debug registers from any thread context

**Detection methods:**

1. **Direct DR access (ring 0):**
   ```c
   // Kernel-mode code
   uint64_t dr0 = __readdr(0);  // Read DR0
   uint64_t dr7 = __readdr(7);  // Read DR7
   if (dr0 != 0 || (dr7 & 0xFF) != 0) {
       // Hardware breakpoints detected
   }
   ```

2. **Context structure inspection:**
   - Windows: GetThreadContext/SetThreadContext with CONTEXT_DEBUG_REGISTERS
   - Anti-cheats hook NtGetContextThread and clear debug registers
   - Also hook NtQueryInformationThread (ThreadWow64Context class)

3. **Exception dispatcher monitoring:**
   - Hook KiUserExceptionDispatcher (user-mode exception dispatcher)
   - Check if #DB exceptions occur (suggests hardware breakpoints)
   - Clear debug registers in exception context before delivery

**VM-specific debug register behavior:**

1. **Hypervisor use of debug registers:**
   - Some hypervisors use DR7.GD to detect debug register tampering
   - Nested virtualization: L1 hypervisor may intercept debug register access
   - **Detection:** Attempt to access DR0-DR7, measure timing
   - VM exit on debug register access adds overhead

2. **Debug register interception:**
   - Hypervisor can intercept MOV to/from DR instructions
   - Allows hiding hypervisor's own debug breakpoints
   - Guest sees different DR values than hardware
   - **Detection:** Inconsistencies between reported and actual DR values

**Which products use it:**
- **Game cheats:** Hardware breakpoint hooking (bypasses .text section integrity checks)
- **VMProtect:** Detects hardware breakpoints as anti-debugging measure
- **Kernel anti-cheats:** Monitor debug registers for cheat detection
- **Rootkits:** Use DR for stealthy hooking

**Known bypass techniques:**

**For cheats evading anti-cheat DR detection:**
1. **Hook NtGetContextThread:**
   - Intercept context retrieval
   - Clear CONTEXT_DEBUG_REGISTERS flags or zero out DR values
   - Anti-cheat sees clean debug registers

2. **Hook NtQueryInformationThread:**
   - Intercept ThreadWow64Context queries
   - Clear debug registers in returned context

3. **Hook KiUserExceptionDispatcher:**
   - Intercept exception dispatch
   - Clear DR values in CONTEXT structure before anti-cheat sees it

4. **Kernel-mode DR hiding:**
   - Hypervisor intercepts MOV from DR
   - Returns zero values to anti-cheat
   - Actual hardware breakpoints still functional

**For anti-cheats detecting DR use:**
1. **Kernel-mode direct access:**
   - Read DR values directly in kernel (can't be hooked from user-mode)
   - Compare across threads (consistency check)

2. **DR access timing:**
   - Measure time to read DR values
   - Hypervisor intercept adds VM exit overhead

3. **GD bit trap:**
   - Set DR7.GD (General Detect)
   - Any DR access triggers #DB exception
   - Catches hooking attempts

**2025 architecture status:**
- No major changes to debug registers since early 2000s
- Stability through 2025 CPU models (confirmed in Intel/AMD manuals)
- Focus shifted to newer features (memory tagging, CET)

**Difficulty rating:** MEDIUM-HARD
- For cheats: DR hooking EASY, hiding from detection MEDIUM-HARD
- For anti-cheats: Kernel-level DR detection MEDIUM
- Hypervisor DR interception: MEDIUM (requires VM exit handling)
- Perfect hiding from kernel-level checks: HARD (kernel can read DR directly)

---

## 6. Known Anti-Cheat Detection Methods

### 6.1 EasyAntiCheat (EAC) Techniques

**Overview:**
EasyAntiCheat is a kernel-level anti-cheat system used in games like Fortnite, Rust, Apex Legends, and many others. It employs multiple VM detection techniques, though research suggests they are "lacking and relatively simple to circumvent" compared to more aggressive anti-cheats.

**VM Detection Techniques:**

1. **CPUID hypervisor bit check:**
   - Checks CPUID.1:ECX[31] for hypervisor present bit
   - Standard check, easily bypassed with `-cpu host,-hypervisor,kvm=off`

2. **RDTSC timing attack:**
   - Uses standard `rdtsc;cpuid;rdtsc` combination
   - Measures CPUID execution time
   - Threshold: ~750 cycles (physical) vs 1000+ cycles (virtual)
   - **Bypass:** Proper TSC emulation with offset adjustment

3. **VMX detection via VMREAD:**
   - Performs single VMREAD instruction upon driver initialization
   - Properly wrapped in exception handler
   - If VMREAD succeeds → running in nested virtualization or hypervisor present
   - **Bypass:** Ensure VMREAD faults appropriately (intercept and inject #UD)

4. **MSR access monitoring:**
   - After ~30 minutes of gameplay: queries IA32_EFER MSR (0xC0000080)
   - After 40 minutes: no other MSR reads/writes observed (in one research session)
   - **Bypass:** Ensure MSR access timing consistent with physical hardware

5. **HWID collection (for bans):**
   - Collects: Motherboard UUID, disk serials, MAC address, BIOS UUID
   - Used for hardware bans
   - **Bypass:** HWID spoofers, MAC address spoofing, SMBIOS modification

6. **Registry/WMI checks (suspected):**
   - Likely checks for VM-identifying strings in registry
   - May use WMI to query hardware information
   - **Bypass:** Registry spoofing, SMBIOS/ACPI table modification

**Effectiveness:**
- Research from 2020-2024 suggests EAC's virtualization checks are "lacking"
- Proper TSC emulation and CPUID masking sufficient for most EAC titles
- More effective against simpler VM setups (default VirtualBox/VMware)

**Current bypass status (2024-2025):**
- EAC works with properly configured KVM/QEMU
- Works with Windows Hyper-V + HVCI (Hypervisor-Enforced Code Integrity)
- Community reports successful evasion with:
  - `-cpu host,-hypervisor,kvm=off`
  - SCSI Controller = LSI
  - MAC address set to real Intel vendor ID
  - Deleting EAC registry entries (risky, may flag account)

**Difficulty rating:** MEDIUM
- Detection sophistication: MEDIUM (multiple vectors but not deeply invasive)
- Bypass difficulty: MEDIUM (standard VM hardening sufficient)
- HWID ban evasion: MEDIUM-HARD (requires comprehensive spoofing)

---

### 6.2 BattlEye Approaches

**Overview:**
BattlEye is a kernel-level anti-cheat used in games like PUBG, Rainbow Six Siege, Arma 3, and DayZ. It employs more aggressive VM detection than EAC, including kernel-level timing checks.

**VM Detection Techniques:**

1. **RDTSC timing attack (kernel-level):**
   - Uses standard `rdtsc;cpuid;rdtsc` method
   - **Key difference:** Check moved to kernel-mode
   - Harder to intercept than user-mode checks
   - Threshold: ~200 cycles (physical) vs 750+ cycles (virtual)

2. **Generic hypervisor detection via timing:**
   - Targets "abnormal time values in the instruction CPUID"
   - Not specific to any VM vendor
   - Statistical analysis of timing distributions
   - **Bypass:** TSC emulation with careful offset calculation

3. **Improved VM detection (post-update):**
   - Updates in ~2020 improved VM detection mechanisms
   - Users report error: "Virtual Machine process detected"
   - More aggressive enumeration of VM artifacts

4. **HWID collection (for bans):**
   - Collects: Motherboard UUID, disk serials, MAC address, GPU device IDs
   - Persistent tracking across reinstalls
   - **Bypass:** Comprehensive HWID spoofing required

5. **Kernel callback monitoring (suspected):**
   - Likely uses PsSetCreateProcessNotifyRoutine, ObRegisterCallbacks
   - Monitors for hypervisor-related processes/drivers
   - Checks for suspicious kernel modules

**Effectiveness:**
- More effective than EAC at detecting VMs
- Kernel-level implementation harder to hook/bypass
- Still described as "relatively simple to circumvent" by advanced users
- Less aggressive than FACEIT, ESEA, or Vanguard

**Current bypass status (2024-2025):**
- BattlEye works with Windows Hyper-V + HVCI
- Community reports mixed success with KVM/QEMU
- Some users report detection even with:
  - QEMU anti-detection patches
  - CPUID masking
  - TSC offsetting

**Challenges in bypassing:**
- Kernel-level checks harder to intercept
- Multiple redundant detection vectors
- Frequent updates improve detection
- HWID bans require comprehensive spoofing

**Difficulty rating:** MEDIUM-HARD
- Detection sophistication: MEDIUM-HARD (kernel-level, multiple vectors)
- Bypass difficulty: MEDIUM-HARD (requires advanced VM hardening)
- HWID ban evasion: HARD (comprehensive, persistent tracking)

---

### 6.3 Vanguard (Riot Games) Methods

**Overview:**
Vanguard is Riot Games' kernel-level anti-cheat used in Valorant and League of Legends. It's one of the most aggressive anti-cheat systems, starting at boot time and running continuously.

**Key Characteristics:**

1. **Boot-time initialization:**
   - Vanguard driver (vgk.sys) loads during Windows boot
   - Starts before user login, before most other drivers
   - Collects HWID data before game even launches

2. **Kernel-level operation:**
   - Full kernel-mode driver with SYSTEM privileges
   - Can inspect all processes, memory, drivers
   - Monitors kernel callbacks, system calls, hardware access
   - More invasive than EAC or BattlEye

**VM Detection Techniques:**

1. **CPUID checks:**
   - Standard hypervisor bit detection
   - Vendor leaf enumeration
   - **Bypass:** CPUID masking

2. **Timing-based detection:**
   - RDTSC/RDTSCP timing measurements
   - Likely includes statistical analysis (multiple samples)
   - **Bypass:** TSC offsetting, low-latency hypervisor

3. **Hardware enumeration:**
   - Comprehensive PCI device enumeration
   - SMBIOS/ACPI table inspection
   - MAC address checking
   - Disk/GPU vendor strings
   - **Bypass:** SMBIOS spoofing, PCI device hiding/spoofing

4. **Driver scanning:**
   - Enumerates all loaded kernel drivers
   - Checks for hypervisor-related drivers (e.g., vmmemctl, vmhgfs, VBoxGuest)
   - Signature-based detection of known virtualization drivers
   - **Bypass:** Hide VM drivers, rename/remove non-essential drivers

5. **Kernel callback monitoring:**
   - Likely uses ObRegisterCallbacks, PsSetCreateProcessNotifyRoutine
   - Monitors process/thread creation for suspicious patterns
   - Detects hypervisor-based cheats loading drivers

6. **HWID tracking:**
   - Collects 50+ hardware fingerprints (industry-leading)
   - CPU ID, GPU serial, disk volumes, motherboard, BIOS, even monitor serials
   - Used for hardware bans
   - **Bypass:** Comprehensive HWID spoofing tools

**Boot-time requirement implications:**
- Must hide VM before Vanguard loads
- Drivers must be loaded before Vanguard initializes
- Post-boot spoofing may be detected (Vanguard captures state early)

**Effectiveness:**
- One of most aggressive anti-cheats
- Boot-time loading makes evasion difficult
- Comprehensive hardware fingerprinting
- Frequent updates improve detection

**Current bypass status (2024-2025):**
- Vanguard works with Windows Hyper-V + HVCI (officially supported for Windows 11 24H2+)
- KVM/QEMU: Community reports mixed results
- Requires extremely thorough VM hardening:
  - CPUID masking
  - TSC offsetting
  - SMBIOS/ACPI spoofing
  - PCI device hiding
  - Driver removal/hiding
  - HWID spoofing

**Challenges:**
- Boot-time initialization captures system state early
- Kernel-level operation hard to fool
- Comprehensive hardware enumeration
- Hardware bans require thorough spoofing
- Officially supports Hyper-V (may detect other hypervisors more aggressively)

**Difficulty rating:** HARD
- Detection sophistication: HARD (boot-time, kernel-level, comprehensive)
- Bypass difficulty: HARD (requires advanced techniques, thorough spoofing)
- HWID ban evasion: HARD (50+ fingerprints tracked)
- Hyper-V exception: EASY (officially supported on Windows 11)

---

### 6.4 Kernel-Level AC Detection Methods

**Overview:**
Modern kernel-level anti-cheats (EAC, BattlEye, Vanguard, FACEIT, ESEA) operate with SYSTEM privileges and use Windows kernel APIs for monitoring. This section covers common kernel-level detection techniques.

**Kernel Callback Mechanisms:**

1. **PsSetCreateProcessNotifyRoutine / PsSetCreateProcessNotifyRoutineEx:**
   - Registers callback for process creation/termination events
   - Anti-cheat receives notification when any process starts/exits
   - **Use case:** Detect suspicious processes (debuggers, cheat tools, VM utilities)
   - **Example:** Detect vmtoolsd.exe, VBoxService.exe, qemu-ga.exe

2. **PsSetCreateThreadNotifyRoutine / PsSetCreateThreadNotifyRoutineEx:**
   - Registers callback for thread creation/termination
   - **Use case:** Detect thread injection (common cheat technique)
   - Monitor for suspicious thread start addresses

3. **PsSetLoadImageNotifyRoutine / PsSetLoadImageNotifyRoutineEx:**
   - Registers callback when image (DLL, EXE) loaded into memory
   - **Use case:** Detect cheat DLL injection
   - Monitor for unsigned/suspicious modules

4. **ObRegisterCallbacks:**
   - Registers pre/post-operation callbacks for object handle operations
   - Monitors: Process handle opening, thread handle opening, desktop handle operations
   - **Use case:** Block handle access to anti-cheat processes (prevents termination, memory reading)
   - **Example:** OpenProcess(PROCESS_ALL_ACCESS) on anti-cheat → handle returned with limited permissions

5. **CmRegisterCallback / CmRegisterCallbackEx:**
   - Registers callback for registry operations
   - **Use case:** Detect registry modifications (e.g., HWID spoofing via registry)
   - Monitor for VM-related registry keys being hidden

**Kernel-Level VM Detection:**

1. **CPUID execution from kernel:**
   - Kernel-mode code can execute CPUID directly
   - No user-mode hooks can intercept
   - **Bypass:** Hypervisor must intercept at CPU level

2. **MSR access:**
   - RDMSR/WRMSR from kernel-mode
   - Can probe synthetic MSRs (0x40000000+)
   - Can read IA32_FEATURE_CONTROL, IA32_EFER, etc.
   - **Bypass:** Hypervisor intercepts MSR access

3. **Direct hardware access:**
   - Kernel can access I/O ports directly
   - Can probe VMware backdoor (0x5658)
   - Can access PCI configuration space
   - **Bypass:** Hypervisor intercepts I/O port access

4. **Timing from kernel:**
   - RDTSC timing measurements from kernel-mode
   - User-mode hooks ineffective
   - **Bypass:** Hypervisor TSC offsetting

5. **Driver enumeration:**
   - Enumerate loaded drivers via ZwQuerySystemInformation
   - Check for VM-related drivers (VBoxGuest.sys, vmhgfs.sys, etc.)
   - **Bypass:** Hide/rename VM drivers

**Difficulty rating:** HARD
- Detection from kernel: HARD to bypass (kernel-level hooks ineffective)
- Callback-based monitoring: MEDIUM-HARD to bypass (requires kernel exploitation or hypervisor)
- Perfect evasion: VERY HARD (kernel-level anti-cheat has extensive visibility)

---

## 7. Additional Detection Vectors

### 7.1 VirtualBox/QEMU Specific Artifacts

**VirtualBox-Specific Detection:**

**Registry artifacts:**
```
HKLM\HARDWARE\DEVICEMAP\Scsi\Scsi Port 0\Scsi Bus 0\Target Id 0\Logical Unit Id 0
  Identifier = "VBOX"

HKLM\HARDWARE\Description\System
  SystemBiosVersion = "VBOX"
  VideoBiosVersion = "VIRTUALBOX"

HKLM\SOFTWARE\Oracle\VirtualBox Guest Additions
  (Presence indicates VirtualBox)
```

**PCI devices:**
- VBoxVGA: `PCI\VEN_80EE&DEV_BEEF&SUBSYS_00000000`
- VBoxSVGA: `PCI\VEN_80EE&DEV_BEEF&SUBSYS_040515AD`
- VirtualBox Guest Service: VEN_80EE

**File system artifacts:**
- VBoxGuest.sys, VBoxMouse.sys, VBoxSF.sys (drivers)
- VBoxService.exe, VBoxTray.exe (Guest Additions processes)
- C:\Program Files\Oracle\VirtualBox Guest Additions\

**Process names:**
- VBoxService.exe
- VBoxTray.exe

**MAC address:**
- 08:00:27:XX:XX:XX

**SMBIOS strings:**
- Manufacturer: "innotek GmbH" or "Oracle Corporation"
- Product: "VirtualBox"
- Version: "1.2" or similar

**Bypass techniques:**
- VBoxHardenedLoader: Patches VirtualBox to hide artifacts
- Issue: Bypassed by WMI PNP device ID checks
- VBoxManage setextradata commands to customize SMBIOS
- Remove/rename Guest Additions (breaks functionality)
- Manual registry editing (risky, may break boot)

---

**QEMU/KVM-Specific Detection:**

**Registry artifacts:**
```
HKLM\HARDWARE\DEVICEMAP\Scsi\Scsi Port 0\Scsi Bus 0\Target Id 0\Logical Unit Id 0
  Identifier = "QEMU"

HKLM\HARDWARE\Description\System
  SystemBiosVersion = "QEMU"
```

**PCI devices:**
- QEMU `-vga std`: `PCI\VEN_1234&DEV_1111`
- QEMU `-vga vmware`: `PCI\VEN_15AD&DEV_0405`
- VirtIO network: `PCI\VEN_1AF4&...`
- Red Hat VirtIO: `PCI\VEN_1B36&...`

**File system artifacts:**
- qemu-ga.exe (QEMU Guest Agent)
- virtio drivers (viostor.sys, netkvm.sys, etc.)

**Process names:**
- qemu-ga.exe

**SMBIOS strings:**
- Manufacturer: "QEMU"
- Product: "Standard PC (i440FX + PIIX, 1996)" or similar
- Version: Contains "QEMU"

**Disk model:**
- "QEMU HARDDISK"

**Bypass techniques:**
- QEMU command-line arguments:
  - `-cpu host,-hypervisor,kvm=off,hv_vendor_id=null`
  - `-smbios type=X,field=value` for SMBIOS spoofing
  - `-device ...` for custom PCI devices
- qemu-anti-detection patches (GitHub projects)
- Remove QEMU Guest Agent (breaks some features)
- Use IDE/SCSI controllers with generic IDs

**Detection tools:**
- **VMAware:** Advanced VM detection library (GitHub: kernelwernel/VMAware)
- **SEMS:** Detects VirtualBox, QEMU, Cuckoo, Anubis (GitHub: AlicanAkyol/sems)

**Difficulty rating:**
- VirtualBox detection: EASY (many artifacts)
- VirtualBox bypass: MEDIUM (VBoxHardenedLoader partially effective, WMI checks harder)
- QEMU/KVM detection: MEDIUM (fewer artifacts, more configurable)
- QEMU/KVM bypass: MEDIUM (comprehensive spoofing required, anti-detection patches available)

---

## 8. Summary Table

| Detection Vector | Difficulty to Detect | Difficulty to Bypass | Used By | Notes |
|-----------------|---------------------|---------------------|---------|-------|
| **CPUID-Based** |
| Hypervisor bit (CPUID.1:ECX[31]) | EASY | EASY | All anti-cheats, malware | Simple config change to hide |
| Vendor leaves (0x40000000+) | EASY | EASY | All VM detection tools | Intercept CPUID, return blank |
| VMX/SVM feature bits | MEDIUM | MEDIUM | Advanced malware | Must maintain consistency |
| Brand string anomalies | EASY | EASY | VM detection libraries | Copy real CPU brand string |
| **Timing-Based** |
| RDTSC/RDTSCP attacks | MEDIUM | MEDIUM-HARD | BattlEye, EAC, malware | TSC offsetting required |
| VM-exit latency | MEDIUM | HARD | Kernel anti-cheats | Low-latency hypervisor needed |
| Instruction timing variance | MEDIUM | MEDIUM-HARD | Advanced malware | Statistical analysis robust |
| Cache timing attacks | HARD | HARD | Academic research, APTs | Fundamental overhead hard to hide |
| **Memory-Based** |
| EPT violation patterns | HARD | HARD | Hypervisor-based cheats/security | EPT overhead unavoidable |
| TLB behavior anomalies | HARD | HARD | Research, advanced malware | CPU-specific, complex |
| Memory mapping inconsistencies | MEDIUM | MEDIUM | Malware, introspection | Some inconsistencies unavoidable |
| MMIO region detection | MEDIUM | MEDIUM | Driver-level malware | Device passthrough solves |
| **Hardware-Based** |
| SMBIOS/ACPI strings | EASY | EASY | All anti-cheats, malware | All hypervisors support spoofing |
| PCI device fingerprints | EASY | MEDIUM-HARD | Anti-cheats, malware | Full bypass needs passthrough |
| MAC address prefixes | EASY | EASY | Network tools, malware | Trivial to configure |
| Disk/GPU vendor strings | EASY-MEDIUM | MEDIUM | HWID bans, malware | Passthrough best, spoofing medium |
| **Behavioral** |
| I/O port backdoor (VMware) | EASY | MEDIUM | Malware | Only VMware, breaks Tools |
| MSR access patterns | MEDIUM | MEDIUM-HARD | EAC, advanced malware | MSR interception required |
| Exception handling timing | MEDIUM | HARD | VMProtect, malware | Exception overhead hard to hide |
| Debug register behavior | MEDIUM | MEDIUM-HARD | VMProtect, cheats | Kernel-level checks harder |
| **Anti-Cheat Specific** |
| EasyAntiCheat | MEDIUM | MEDIUM | Fortnite, Rust, Apex | Standard VM hardening sufficient |
| BattlEye | MEDIUM-HARD | MEDIUM-HARD | PUBG, R6 Siege | Kernel-level, more aggressive |
| Vanguard (Riot) | HARD | HARD | Valorant, LoL | Boot-time, comprehensive, Hyper-V supported |
| Kernel callbacks | HARD | HARD | All kernel ACs | Kernel-level hooks ineffective |

---

## Methodology and Ethical Considerations

**Research Purpose:**
This documentation is compiled for **defensive security research** purposes:
- Understanding anti-cheat detection mechanisms
- Hardening virtualized development/testing environments
- Security research and malware analysis in isolated VMs
- Improving hypervisor stealth for legitimate use cases

**Responsible Use:**
- This research should not be used to violate Terms of Service agreements
- Bypassing anti-cheat for competitive advantage is unethical and often illegal
- Information provided for educational and defensive purposes only
- Users are responsible for compliance with applicable laws and agreements

**Disclosure:**
- Anti-cheat vendors continually improve detection mechanisms
- This document represents a point-in-time analysis (December 2025)
- Specific bypasses may be detected/patched by time of reading
- Always verify current state of detections before implementation

---

## References and Sources

**CPUID-Based Detection:**
- [Evasions: CPU - Check Point](https://evasions.checkpoint.com/src/Evasions/techniques/cpu.html)
- [Analyzing and countering Windows anti-VM techniques - eShard](https://eshard.com/posts/windows-anti-vm-detection-bypass)
- [Protecting VMware from CPUID hypervisor detection - Hexacorn](https://www.hexacorn.com/blog/2014/08/25/protecting-vmware-from-cpuid-hypervisor-detection/)
- [Feature and Interface Discovery - Microsoft Learn](https://learn.microsoft.com/en-us/virtualization/hyper-v-on-windows/tlfs/feature-discovery)
- [Defeating malware's Anti-VM techniques (CPUID-Based Instructions) - Rayanfam](https://rayanfam.com/topics/defeating-malware-anti-vm-techniques-cpuid-based-instructions/)

**Timing-Based Detection:**
- [Preventing VM detection from RDTSC VM EXIT checks - Proxmox Forum](https://forum.proxmox.com/threads/preventing-vm-detection-from-rdtsc-vm-exit-checks.73669/)
- [My Ticks Don't Lie: New Timing Attacks for Hypervisor Detection - Black Hat EU 2020](https://i.blackhat.com/eu-20/Thursday/eu-20-DElia-My-Ticks-Dont-Lie-New-Timing-Attacks-For-Hypervisor-Detection.pdf)
- [Detecting Hardware-Assisted Virtualization - DIMVA 2016](https://www.christian-rossow.de/publications/detectvt-dimva2016.pdf)
- [Understanding RDTSC Timing Checks - RetroArch](https://retroarchemu.gitlab.io/home/understanding-rdtsc-timing-checks-the-technical-reality-of-vm-gaming/)
- [RDTSCP – a recooked AntiRe trick - Hexacorn](https://www.hexacorn.com/blog/2014/06/15/rdtscp-a-recooked-antire-trick/)
- [RDTSC-KVM-Handler - GitHub](https://github.com/WCharacter/RDTSC-KVM-Handler)

**Memory-Based Detection:**
- [Hypervisors for Memory Introspection and Reverse Engineering - secret club](https://secret.club/2025/06/02/hypervisors-for-memory-introspection-and-reverse-engineering.html)
- [Hardware assisted hypervisor introspection - PMC](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4870477/)
- [Hypervisor From Scratch – Part 7: Using EPT - Rayanfam](https://rayanfam.com/topics/hypervisor-from-scratch-part-7/)
- [MMU Virtualization via Intel EPT: Technical Details - Reverse Engineering](https://revers.engineering/mmu-ept-technical-details/)
- [Secure and Scalable TLB Partitioning - SpringerLink](https://link.springer.com/chapter/10.1007/978-981-95-3537-8_8)
- [Defeating Cache Side-channel Protections with TLB Attacks - USENIX 2018](https://www.usenix.org/system/files/conference/usenixsecurity18/sec18-gras.pdf)

**Hardware-Based Detection:**
- [hvdetecc - Collection of hypervisor detections - GitHub](https://github.com/can1357/hvdetecc)
- [VMware hypervisor fingerprinting - ResearchGate](https://www.researchgate.net/publication/327816711_VMware_hypervisor_fingerprinting)
- [How to recognise a VMware VM by MAC address - macaddress.io](https://macaddress.io/faq/how-to-recognise-a-vmwares-virtual-machine-by-its-mac-address)
- [VMware Virtual Machines MAC addresses - Virtual Simon](https://virtual-simon.co.uk/vmware-virtual-machines-mac-addresses/)

**Behavioral Detection:**
- [VMware backdoor I/O ports - HyperDbg Documentation](https://docs.hyperdbg.org/tips-and-tricks/nested-virtualization-environments/vmware-backdoor-io-ports)
- [How does malware know the difference between VMs and real world - Cisco Talos](https://blog.talosintelligence.com/how-does-malware-know-difference/)
- [Bypassing Qakbot Anti-Analysis - Lab52](https://lab52.io/blog/bypassing-qakbot-anti-analysis-tactics/)
- [Hardware breakpoints and exceptions on Windows - LingSec](https://ling.re/hardware-breakpoints/)
- [Hardware Breakpoint Detection - Guided Hacking](https://guidedhacking.com/threads/hardware-breakpoint-detection.16383/)

**Anti-Cheat Systems:**
- [How anti-cheats detect system emulation - secret club](https://secret.club/2020/04/13/how-anti-cheats-detect-system-emulation.html)
- [BattlEye hypervisor detection - secret club](https://secret.club/2020/01/12/battleye-hypervisor-detection.html)
- [Understanding Kernel-Level Anticheats in Online Games - MeekoLab](https://research.meekolab.com/understanding-kernel-level-anticheats-in-online-games)
- [Hypervisor-Phantom - GitHub](https://github.com/Scrut1ny/Hypervisor-Phantom)
- [HWID Spoofer Guide 2025 - Medium](https://medium.com/@synctop/hwid-spoofer-best-hardware-id-spoofer-2025-guide-89510094fa0e)

**Additional Vectors:**
- [Detecting Virtual Environment Artefacts - Unprotect Project](https://unprotect.it/technique/detecting-virtual-environment-artefacts/)
- [VMAware - Advanced VM detection library - GitHub](https://github.com/kernelwernel/VMAware)
- [Hide Artifacts: Run Virtual Instance - MITRE ATT&CK](https://attack.mitre.org/techniques/T1564/006/)
- [Fast and Furious: Outrunning Windows Kernel Notification Routines - SpringerLink](https://link.springer.com/chapter/10.1007/978-3-030-52683-2_4)
- [RealBlindingEDR - GitHub](https://github.com/myzxcg/RealBlindingEDR)

**Hypervisor Evasion:**
- [How to hide a hook: A hypervisor for rootkits - Phrack](https://phrack.org/issues/69/15)
- [Detecting Hypervisor-assisted Hooking - Maurice's Blog](https://momo5502.com/posts/2022-05-02-detecting-hypervisor-assisted-hooking/)
- [gbhv - Simple x86-64 VT-x Hypervisor with EPT Hooking - GitHub](https://github.com/Gbps/gbhv)
- [Hypervisor From Scratch – Part 8 - Rayanfam](https://rayanfam.com/topics/hypervisor-from-scratch-part-8/)

---

**Document Version:** 2.0 (Comprehensive Research Update)
**Compiled:** December 22, 2025
**For:** Ombra Hypervisor Anti-Detection Research
**Classification:** Security Research - Educational Use Only
