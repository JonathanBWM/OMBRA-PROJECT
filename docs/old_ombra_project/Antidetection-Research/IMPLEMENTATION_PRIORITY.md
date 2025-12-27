# OmbraHypervisor Implementation Priority Roadmap

**Generated**: 2025-12-22
**Purpose**: Prioritized implementation guide based on antidetection research

---

## EXECUTIVE SUMMARY

Based on analysis of 28 reference codebases, the following priority order maximizes stealth while minimizing implementation complexity:

| Priority | Category | Techniques | Detection Risk if Missing |
|----------|----------|------------|---------------------------|
| P0 | CPUID Stealth | HV bit, VMX bit, 0x40000000 | CRITICAL - Instant detection |
| P1 | MSR Stealth | VMX MSRs, FEATURE_CONTROL | HIGH - Easy detection |
| P2 | Timing Stealth | TSC offset | HIGH - Timing attacks |
| P3 | Boot Chain | Full hook chain | MEDIUM - Boot-time checks |
| P4 | EPT Stealth | Shadow pages | MEDIUM - Memory scans |
| P5 | Architecture | Dual Intel/AMD | MEDIUM - Platform coverage |

---

## P0: CPUID STEALTH (CRITICAL)

### Implementation Order
1. **CPUID VMExit Handler** - Intercept all CPUID instructions
2. **Hypervisor Present Bit** - Clear CPUID.01H:ECX[31]
3. **VMX Feature Bit** - Clear CPUID.01H:ECX[5] (optional)
4. **Hypervisor Leaves** - Return zeros for 0x40000000-0x400000FF

### Reference Implementation
```c
// From ksm.md - Pattern to adopt
static bool vcpu_handle_cpuid(struct vcpu *vcpu) {
    int cpuid[4];
    int func = ksm_read_reg32(vcpu, STACK_REG_AX);
    int subf = ksm_read_reg32(vcpu, STACK_REG_CX);

    // Hypervisor CPUID space - return zeros
    if (func >= 0x40000000 && func <= 0x400000FF) {
        cpuid[0] = cpuid[1] = cpuid[2] = cpuid[3] = 0;
        goto write_regs;
    }

    __cpuidex(cpuid, func, subf);

    // Hide virtualization features
    if (func == 1) {
        cpuid[2] &= ~(1 << 5);   // Clear VMX bit
        cpuid[2] &= ~(1 << 31);  // Clear HYPERVISOR bit
    }

write_regs:
    // Write to guest registers and advance RIP
}
```

### Files to Modify
- `OmbraPayload/Vmx/VmExit.cpp` - Add CPUID handler
- `OmbraPayload/Svm/VmExit.cpp` - AMD equivalent

### Verification
- Run `cpuid -1` on Linux, check leaf 1 ECX bits
- Run pafish, verify "Checking CPUID hypervisor bit" passes
- Run al-khaser CPUID tests

---

## P1: MSR STEALTH (HIGH)

### Implementation Order
1. **MSR Bitmap Configuration** - 4KB bitmap in VMCS/VMCB
2. **Intercept VMX MSRs** - 0x480-0x491 range
3. **Inject #GP on RDMSR** - Fake "not supported"
4. **IA32_FEATURE_CONTROL** - Clear VMX enable bits

### Reference Implementation
```c
// From ksm.md - MSR bitmap setup
static inline void init_msr_bitmap(struct ksm *k) {
    unsigned long *read_lo = (unsigned long *)k->msr_bitmap;

    // Intercept IA32_FEATURE_CONTROL reads
    set_bit(MSR_IA32_FEATURE_CONTROL, read_lo);

    // Intercept ALL VMX capability MSRs
    for (u32 msr = MSR_IA32_VMX_BASIC; msr <= MSR_IA32_VMX_VMFUNC; ++msr)
        set_bit(msr, read_lo);
}

// RDMSR handler
static bool vcpu_handle_rdmsr(struct vcpu *vcpu) {
    u32 msr = ksm_read_reg32(vcpu, STACK_REG_CX);

    if (msr >= MSR_IA32_VMX_BASIC && msr <= MSR_IA32_VMX_VMFUNC) {
        // Inject #GP - make VMX appear unsupported
        vcpu_inject_hardirq(vcpu, X86_TRAP_GP, 0);
        return true;
    }

    if (msr == MSR_IA32_FEATURE_CONTROL) {
        // Clear VMX enable bits
        u64 val = __readmsr(msr) & ~(FEATURE_CONTROL_VMXON_ENABLED_OUTSIDE_SMX |
                                      FEATURE_CONTROL_VMXON_ENABLED_INSIDE_SMX);
        // Write to guest EAX:EDX
    }
}
```

### Files to Modify
- `OmbraPayload/Vmx/Vmcs.cpp` - MSR bitmap setup
- `OmbraPayload/Vmx/VmExit.cpp` - RDMSR/WRMSR handlers
- `OmbraPayload/Svm/Vmcb.cpp` - AMD MSRPM setup

### Verification
- Attempt to read MSR 0x480 from ring-0, expect #GP
- Check IA32_FEATURE_CONTROL bits 1-2 appear clear

---

## P2: TIMING STEALTH (HIGH)

### Implementation Order
1. **TSC Offset in VMCS** - Hardware-based offset (preferred)
2. **RDTSC Handler** - Optional software compensation
3. **RDTSCP Handler** - Include TSC_AUX handling
4. **Calibration** - Measure and store baseline overhead

### Reference Implementation
```c
// Hardware approach (preferred) - no interception needed
void setup_tsc_offset(struct vcpu *vcpu) {
    // Calculate offset to compensate for any timing drift
    vmcs_write64(TSC_OFFSET, 0);  // Or calculated value

    // Disable RDTSC exiting - let hardware handle
    u64 vm_cpuctl = vmcs_read64(CPU_BASED_VM_EXEC_CONTROL);
    vm_cpuctl &= ~CPU_BASED_RDTSC_EXITING;
    vmcs_write64(CPU_BASED_VM_EXEC_CONTROL, vm_cpuctl);
}

// Software approach (if dynamic adjustment needed)
static bool vcpu_handle_rdtsc(struct vcpu *vcpu) {
    u64 entry_tsc = __rdtsc();
    // Handler overhead measured here
    u64 exit_tsc = __rdtsc();
    u64 overhead = exit_tsc - entry_tsc;

    u64 compensated = exit_tsc - overhead;
    // Write to guest EAX:EDX
}
```

### Files to Modify
- `OmbraPayload/Vmx/Vmcs.cpp` - TSC_OFFSET field
- `OmbraPayload/Vmx/VmExit.cpp` - RDTSC handler (if needed)

### Verification
- Timing attack PoC: measure RDTSC delta around CPUID
- Should be <1000 cycles (bare metal is ~50-200)

---

## P3: BOOT CHAIN (MEDIUM)

### Implementation Order
1. **bootmgfw.efi Hook** - Intercept ArchStartBootApplication
2. **winload.efi Hook** - Intercept BlLdrLoadImage / BlImgLoadPEImageEx
3. **hvloader.efi Hook** - Intercept HvBlImgAllocateImageBuffer
4. **Payload Injection** - Add section to hv.exe
5. **VMExit Hook** - Patch hv.exe VMExit handler
6. **Cleanup** - Delete payload from disk

### Reference Implementation (from Voyager/Sputnik)
```c
// Boot chain flow
EFI_STATUS UefiMain(...) {
    RestoreBootMgfw();           // Restore from backup
    LoadPayLoadFromDisk(&PayLoad);  // Load payload
    InstallBootMgfwHooks(Handle);   // Hook ArchStartBootApplication
    gBS->StartImage(Handle, ...);    // Start bootmgfw
}

// winload hook - detect hv.exe loading
EFI_STATUS BlLdrLoadImage(...) {
    if (!StrCmp(ModuleName, L"hv.exe")) {
        HyperVloading = TRUE;
    }
    // After load, inject payload into "payload" section
    AddSection(ImageBase, "payload", PayLoadSize(), SECTION_RWX);
    MapModule(&Data, PayLoad);
    HookVmExit(HypervBase, HypervSize, VmExitHook);
}
```

### Files to Modify
- `OmbraBoot/UefiMain.cpp` - Entry point
- `OmbraBoot/Hooks/BootMgfw.cpp` - bootmgfw hooks
- `OmbraBoot/Hooks/WinLoad.cpp` - winload hooks
- `OmbraBoot/Hooks/HvLoader.cpp` - hvloader hooks
- `OmbraBoot/PE/SectionAdd.cpp` - PE manipulation
- `OmbraBoot/PE/Mapper.cpp` - Module mapping

### Verification
- Boot with serial debugging enabled
- Verify each hook point triggers
- Verify payload injection succeeds

---

## P4: EPT STEALTH (MEDIUM)

### Implementation Order
1. **EPT Identity Map** - Map all physical memory 1:1
2. **EPT Violation Handler** - Handle access faults
3. **Page Shadowing** - Execute-only vs read shadow pages
4. **Page Splitting** - 2MB→4KB for fine-grained control

### Reference Implementation
```c
// From hvpp.md - EPT entry structure
struct epte_t {
    union {
        uint64_t flags;
        struct {
            uint64_t read_access : 1;
            uint64_t write_access : 1;
            uint64_t execute_access : 1;
            uint64_t memory_type : 3;
            // ...
            uint64_t page_frame_number : 36;
        };
    };
};

// Shadow page setup
void setup_shadow_page(pa_t target, pa_t shadow) {
    epte_t* entry = ept_entry(target, pml::pt);

    // Original page: execute-only
    entry->read_access = 0;
    entry->write_access = 0;
    entry->execute_access = 1;
    entry->page_frame_number = target.pfn();

    // On read violation, swap to shadow page
}
```

### Files to Modify
- `OmbraPayload/Ept/EptManager.cpp` - EPT management
- `OmbraPayload/Ept/EptViolation.cpp` - Violation handler
- `OmbraPayload/Ept/PageShadow.cpp` - Shadow page logic

### Verification
- Read hooked page from usermode, see clean content
- Execute hooked page, run actual code
- No BSODs from EPT violations

---

## P5: DUAL ARCHITECTURE (MEDIUM)

### Implementation Order
1. **CPU Detection** - CPUID leaf 0 vendor string
2. **Runtime Dispatch** - selected_core flag
3. **Common Abstraction** - Unified vCPU interface
4. **Intel Path** - VMX-specific code
5. **AMD Path** - SVM-specific code

### Reference Implementation (from NoirVisor)
```c
// CPU detection and dispatch
noir_status nvc_build_hypervisor() {
    noir_get_vendor_string(hvm_p->vendor_string);
    hvm_p->cpu_manuf = nvc_confirm_cpu_manufacturer(hvm_p->vendor_string);

    switch(hvm_p->cpu_manuf) {
        case intel_processor:
            hvm_p->selected_core = use_vt_core;
            return nvc_vt_subvert_system(hvm_p);
        case amd_processor:
            hvm_p->selected_core = use_svm_core;
            return nvc_svm_subvert_system(hvm_p);
        default:
            return noir_unknown_processor;
    }
}

// Runtime hypercall dispatch
if (hvm_p->selected_core == use_svm_core)
    noir_svm_vmmcall(code, param);
else if (hvm_p->selected_core == use_vt_core)
    noir_vt_vmcall(code, param);
```

### Files to Modify
- `OmbraPayload/OmbraHvm.h` - Central hypervisor structure
- `OmbraPayload/OmbraHvm.cpp` - CPU detection and dispatch
- `OmbraPayload/Intel/` - VMX-specific directory
- `OmbraPayload/Amd/` - SVM-specific directory

### Build System
```makefile
# Intel VMX core
OmbraPayload/Intel/*.cpp: CFLAGS += -D_OMBRA_VMX_CORE

# AMD SVM core
OmbraPayload/Amd/*.cpp: CFLAGS += -D_OMBRA_SVM_CORE

# Cross-platform
OmbraPayload/Core/*.cpp: # No arch defines
```

### Verification
- Boot on Intel system, verify VMX path
- Boot on AMD system, verify SVM path
- Run same detection tests on both

---

## IMPLEMENTATION TIMELINE

### Sprint 1: Core Stealth
- [ ] CPUID handler with HV bit clearing
- [ ] MSR bitmap with VMX MSR interception
- [ ] Basic TSC offset configuration

### Sprint 2: Boot Integration
- [ ] bootmgfw hook installation
- [ ] winload hook chain
- [ ] hvloader payload injection
- [ ] VMExit hook in hv.exe

### Sprint 3: Memory Stealth
- [ ] EPT identity mapping
- [ ] EPT violation handling
- [ ] Page shadow implementation
- [ ] Driver memory hiding

### Sprint 4: Architecture
- [ ] AMD SVM equivalent implementation
- [ ] CPU detection and dispatch
- [ ] Common abstraction layer
- [ ] Cross-platform testing

### Sprint 5: Hardening
- [ ] Timing attack mitigation
- [ ] Advanced MSR filtering
- [ ] SMBIOS/ACPI sanitization (if needed)
- [ ] Full detection tool testing

---

## SUCCESS CRITERIA

### Detection Tool Results
| Tool | Expected Result |
|------|-----------------|
| pafish | All VM checks pass |
| al-khaser | All hypervisor checks pass |
| VMAware | Not detected as VM |
| Custom timing probes | <1000 cycle CPUID overhead |

### Anticheat Compatibility
| Anticheat | Expected Result |
|-----------|-----------------|
| EasyAntiCheat | No hypervisor detection |
| BattlEye | No hypervisor detection |
| Vanguard | No hypervisor detection |

### Stability
- 24-hour stress test without BSOD
- Gaming workload (high CPU/GPU) stable
- Sleep/resume cycle works

---

## RISK MITIGATION

### High Risk Items
| Risk | Mitigation |
|------|------------|
| Timing detection | Use hardware TSC offset, profile overhead |
| Boot chain changes | Maintain multi-version signature database |
| Kernel updates | Test monthly Windows updates |
| Anticheat updates | Monitor anticheat research communities |

### Fallback Strategies
1. If CPUID hiding detected → Add timing normalization
2. If boot chain fails → Alternative injection points
3. If EPT detected → More aggressive shadowing
4. If single CPU fails → Prioritize most common platform

---

**End of Implementation Priority**
