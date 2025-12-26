# Exit Handling
Critical for stealth:
- CPUID: Hide HV bit, zero 0x40000000+
- RDTSC: Subtract ~2000 cycles overhead
- MSR: Virtualize sensitive MSRs
