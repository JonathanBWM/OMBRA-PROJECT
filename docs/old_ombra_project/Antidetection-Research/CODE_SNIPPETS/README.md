# Code Snippets for OmbraHypervisor

This directory contains extracted and synthesized code patterns from the 28+ reference codebases analyzed during the antidetection research phase.

## Files

| File | Purpose | Key Patterns |
|------|---------|--------------|
| `cpuid_stealth.c` | CPUID interception and filtering | HV bit clearing, vendor leaf suppression, VMCALL dispatch |
| `msr_stealth.c` | MSR interception and hiding | VMX MSR #GP injection, FEATURE_CONTROL filtering |
| `tsc_stealth.c` | Timing attack mitigation | TSC offset, RDTSC/RDTSCP handling, calibration |
| `ept_shadow.c` | EPT-based memory hiding | Execute-only pages, shadow hooks, MTF handling |
| `boot_chain.c` | Boot chain hooking | bootmgfw→winload→hvloader→hv.exe injection |
| `dual_cpu.c` | Intel/AMD dual architecture | CPU detection, unified vCPU, runtime dispatch |

## Usage Notes

These patterns are extracted for **educational and research purposes**. They demonstrate techniques found across multiple open-source hypervisor projects.

### Pattern Sources

- **ksm** - CPUID/MSR filtering, timing
- **hvpp** - C++ EPT implementation, VCPU state
- **HyperPlatform** - VMCS setup, MSR bitmap
- **NoirVisor** - Dual CPU architecture
- **DdiMon** - EPT shadow hooks
- **Voyager/Sputnik** - Boot chain hooks
- **EfiGuard** - UEFI boot patching
- **CheatDriver** - EPT violation handling

### Implementation Priority

1. **P0 (Critical)**: CPUID HV bit + VMX bit + vendor leaves
2. **P1 (High)**: MSR bitmap + VMX MSR hiding
3. **P2 (High)**: TSC offset configuration
4. **P3 (Medium)**: Boot chain integration
5. **P4 (Medium)**: EPT shadow pages
6. **P5 (Medium)**: Dual Intel/AMD support

### Testing Requirements

All patterns should be tested against:
- pafish (Paranoid Fish)
- al-khaser (Anti-analysis toolkit)
- VMAware (VM detection library)
- Custom timing probes

### Platform Support

| Pattern | Intel VT-x | AMD-V/SVM |
|---------|-----------|-----------|
| CPUID | Yes | Yes |
| MSR | Yes | Yes (different format) |
| TSC | Yes | Yes |
| EPT/NPT | Yes | Yes (no execute-only) |
| Boot Chain | Yes | Yes |

**Note**: AMD NPT does not support execute-only pages. Use alternative hook strategies for AMD.

## File Structure

```
CODE_SNIPPETS/
├── README.md           # This file
├── cpuid_stealth.c     # CPUID patterns
├── msr_stealth.c       # MSR patterns
├── tsc_stealth.c       # Timing patterns
├── ept_shadow.c        # EPT patterns
├── boot_chain.c        # Boot patterns
└── dual_cpu.c          # Architecture patterns
```

---

**Generated**: 2025-12-22
**Research Phase**: Antidetection Technique Extraction
