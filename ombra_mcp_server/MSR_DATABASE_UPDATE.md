# MSR Database Update - Critical Anti-Cheat MSRs Added

## Summary

Expanded the MSR database from 35 VMX-only MSRs to **51 comprehensive MSRs** covering all critical areas for hypervisor development and anti-cheat evasion.

**Database:** `/Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server/src/ombra_mcp/data/intel_sdm.db`

## Statistics

- **Original MSR count:** 35 (VMX capability MSRs only)
- **Added:** 16 new MSRs
- **Total MSR count:** 51
- **Categories:** 9 distinct categories

## MSR Distribution by Category

| Category | Count | Purpose |
|----------|-------|---------|
| VMX_Capability | 19 | VMX feature detection and configuration |
| System | 8 | SYSCALL/SYSENTER, EFER, segment registers |
| APIC | 6 | Local APIC and x2APIC control |
| Timing | 5 | **CRITICAL for anti-cheat evasion** |
| Power | 5 | Performance, thermal, clock modulation |
| Segment | 3 | FS/GS base registers |
| Debug | 3 | LBR, PEBS, debug control |
| Memory | 1 | Page Attribute Table |
| Feature | 1 | Feature control lock |

## Critical Anti-Cheat MSRs (Timing Category)

These MSRs are **MANDATORY** to virtualize properly for anti-cheat evasion:

### IA32_TIME_STAMP_COUNTER (0x10)
- Read by RDTSC/RDTSCP instructions
- Most anti-cheats monitor TSC for hypervisor detection via timing inconsistencies
- **Must be compensated on every VM-exit**

### IA32_TSC_ADJUST (0x3B)
- Allows software to adjust TSC without causing discontinuities
- Anti-cheats may monitor this for tampering
- **Should be virtualized to prevent detection**

### IA32_MPERF (0xE7) + IA32_APERF (0xE8)
- **CRITICAL: ESEA anti-cheat specifically monitors APERF/MPERF ratio**
- Detects hypervisors by comparing actual vs maximum performance frequency
- **Must be compensated alongside TSC**
- Ratio deviation indicates VM-exits are stealing CPU time

### IA32_TSC_AUX (0xC0000103)
- Used by RDTSCP instruction
- Contains processor ID on most systems
- **Must be virtualized to appear consistent**

## Other Critical MSRs Added

### System MSRs (8 total)
- **SYSCALL/SYSRET:** IA32_STAR, IA32_LSTAR, IA32_CSTAR, IA32_FMASK
- **SYSENTER/SYSEXIT:** IA32_SYSENTER_CS, IA32_SYSENTER_ESP, IA32_SYSENTER_EIP
- **Extended Features:** IA32_EFER (long mode, NX bit, SYSCALL enable)

### Segment Base MSRs (3 total)
- IA32_FS_BASE (0xC0000100) - Thread Local Storage
- IA32_GS_BASE (0xC0000101) - Per-CPU data structures
- IA32_KERNEL_GS_BASE (0xC0000102) - SWAPGS target

### APIC MSRs (6 total)
- IA32_APIC_BASE (0x1B) - APIC base address and enable
- x2APIC registers: APICID, VERSION, TPR, EOI, SIVR

### Debug MSRs (3 total)
- IA32_DEBUGCTL (0x1D9) - LBR, BTF, BTS control
- IA32_DS_AREA (0x600) - Debug Store buffer
- IA32_PEBS_ENABLE (0x3F1) - Precise Event-Based Sampling

### Power/Thermal MSRs (5 total)
- IA32_MISC_ENABLE (0x1A0) - Fast strings, TCC, SpeedStep
- IA32_PERF_CTL (0x199) - Performance control
- IA32_PERF_STATUS (0x198) - Performance status
- IA32_CLOCK_MODULATION (0x19A) - On-demand clock modulation
- IA32_THERM_STATUS (0x19C) - Thermal monitoring

### Memory MSRs (1 total)
- IA32_PAT (0x277) - Page Attribute Table

## Implementation Notes

### Timing Compensation Strategy

For robust anti-cheat evasion, the hypervisor MUST:

1. **Trap RDTSC/RDTSCP** via Primary Processor-Based VM-Execution Controls
2. **Compensate TSC on every exit:**
   ```c
   // Measure VM-exit overhead
   uint64_t exit_time_tsc = __rdtsc();

   // Handle the exit
   handle_vmexit(vcpu);

   // Compensate TSC before VM-entry
   uint64_t compensation = __rdtsc() - exit_time_tsc;
   vcpu->tsc_offset += compensation;
   vmwrite(VMCS_TSC_OFFSET, vcpu->tsc_offset);
   ```

3. **Synchronize APERF/MPERF with TSC:**
   ```c
   // Maintain consistent ratio
   vcpu->aperf_shadow += compensation;
   vcpu->mperf_shadow += compensation;

   // Trap RDMSR for these MSRs
   if (msr == IA32_APERF) {
       *value = vcpu->aperf_shadow;
   } else if (msr == IA32_MPERF) {
       *value = vcpu->mperf_shadow;
   }
   ```

4. **Virtualize TSC_AUX consistently:**
   - Use MSR bitmap to trap RDMSR of IA32_TSC_AUX
   - Return consistent processor ID
   - Ensure RDTSCP returns same AUX value every time

### MSR Bitmap Configuration

For optimal stealth:

```c
// Trap timing-related MSRs (read and write)
set_msr_intercept(bitmap, IA32_TIME_STAMP_COUNTER, READ | WRITE);
set_msr_intercept(bitmap, IA32_TSC_ADJUST, READ | WRITE);
set_msr_intercept(bitmap, IA32_MPERF, READ | WRITE);
set_msr_intercept(bitmap, IA32_APERF, READ | WRITE);
set_msr_intercept(bitmap, IA32_TSC_AUX, READ | WRITE);

// Passthrough most system MSRs (no VM-exit)
set_msr_passthrough(bitmap, IA32_FS_BASE, READ | WRITE);
set_msr_passthrough(bitmap, IA32_GS_BASE, READ | WRITE);
set_msr_passthrough(bitmap, IA32_KERNEL_GS_BASE, READ | WRITE);

// Trap debug MSRs to hide hypervisor artifacts
set_msr_intercept(bitmap, IA32_DEBUGCTL, READ | WRITE);
```

## Database Schema

Each MSR entry contains:

```sql
CREATE TABLE msrs (
    id INTEGER PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    address INTEGER NOT NULL,
    category TEXT,
    description TEXT,
    bit_fields TEXT,  -- JSON string with bit field layout
    sdm_section TEXT  -- Intel SDM section reference
);
```

### Bit Fields Format

Stored as JSON for structured parsing:

```json
{
    "0": "SCE - SYSCALL Enable",
    "8": "LME - Long Mode Enable",
    "10": "LMA - Long Mode Active (read-only)",
    "11": "NXE - No-Execute Enable",
    "63:16": "Reserved"
}
```

## Usage with OmbraMCP

Query MSR information using the MCP tools:

```bash
# Get complete MSR info
mcp-cli call ombra/get_msr_info '{"msr_name": "IA32_MPERF"}'

# List all timing MSRs
mcp-cli call ombra/list_msrs '{"category": "Timing"}'

# Generate MSR bitmap setup
mcp-cli call ombra/generate_msr_bitmap_setup '{"trap": ["IA32_TIME_STAMP_COUNTER", "IA32_MPERF", "IA32_APERF"]}'
```

## Files Modified

1. **Database:** `src/ombra_mcp/data/intel_sdm.db`
   - Added 16 new MSRs
   - Normalized category capitalization
   - Rebuilt FTS index

2. **Scripts Created:**
   - `scripts/add_critical_msrs.py` - MSR insertion script
   - `scripts/normalize_msr_categories.py` - Category normalization

## Verification

All critical anti-cheat MSRs verified present:

```
✓ IA32_TIME_STAMP_COUNTER (0x00000010) [Timing]
✓ IA32_TSC_ADJUST         (0x0000003B) [Timing]
✓ IA32_MPERF              (0x000000E7) [Timing]
✓ IA32_APERF              (0x000000E8) [Timing]
✓ IA32_TSC_AUX            (0xC0000103) [Timing]
```

## Next Steps

To leverage these MSRs in the hypervisor:

1. **Update `handlers/msr.c`:**
   - Add handlers for all timing MSRs
   - Implement APERF/MPERF ratio maintenance
   - Add TSC compensation logic

2. **Update `vmcs.c`:**
   - Configure MSR bitmap to trap timing MSRs
   - Set TSC_OFFSET in VMCS

3. **Update `timing.c`:**
   - Implement APERF/MPERF synchronization
   - Add TSC_AUX virtualization
   - Track per-VCPU MSR shadow values

4. **Testing:**
   - Verify RDTSC timing consistency
   - Verify RDTSCP returns correct AUX value
   - Verify APERF/MPERF ratio remains consistent
   - Test against ESEA and other timing-based anti-cheats

## References

- Intel SDM Volume 3, Chapter 23-29 (VMX Operation)
- Intel SDM Volume 4, Appendix A (MSR Definitions)
- ESEA anti-cheat timing analysis (APERF/MPERF monitoring)

---

**Database updated:** 2025-12-26
**Total MSRs:** 51
**Critical timing MSRs:** 5
