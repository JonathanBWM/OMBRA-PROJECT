#ifndef OMBRA_TIMING_H
#define OMBRA_TIMING_H
#include "../shared/types.h"

// Timing compensation for VM-exit stealth
// Anti-cheat systems measure RDTSC before/after CPUID, RDMSR, etc to detect hypervisors
// We compensate by adjusting the TSC offset in VMCS to hide VM-exit overhead

// Typical VM-exit overhead ranges (in cycles):
// - Fast exits (CPUID, RDTSC): 500-1500 cycles
// - Medium exits (MSR access): 1000-2500 cycles
// - Slow exits (EPT violation): 2000-5000+ cycles

#define TIMING_CPUID_OVERHEAD       800     // Conservative CPUID exit overhead
#define TIMING_MSR_OVERHEAD         1200    // Conservative MSR exit overhead
#define TIMING_RDTSC_OVERHEAD       600     // Conservative RDTSC exit overhead
#define TIMING_GENERIC_OVERHEAD     1000    // Default for other exits
#define TIMING_EPT_OVERHEAD         2500    // EPT violations are expensive
#define TIMING_CR_OVERHEAD          1000    // CR access overhead
#define TIMING_VMCALL_OVERHEAD      1500    // VMCALL overhead
#define TIMING_MISCONFIG_OVERHEAD   2000    // EPT misconfig (diagnostic heavy)
#define TIMING_POWER_OVERHEAD       500     // MONITOR/MWAIT/PAUSE (lightweight)
#define TIMING_IO_OVERHEAD          900     // I/O instruction overhead

#define CALIBRATION_ITERATIONS      100     // Number of samples for calibration

// =============================================================================
// KUSER_SHARED_DATA Shadowing for Advanced Timing Evasion
// =============================================================================
// Windows keeps system time in KUSER_SHARED_DATA (0xFFFFF78000000000).
// Anti-cheats compare RDTSC with QueryPerformanceCounter/system time.
// If we adjust TSC but not KUSER_SHARED_DATA, the discrepancy reveals us.
//
// Solution: Shadow the KUSER_SHARED_DATA page via EPT and adjust time fields
// to match our TSC offset manipulation.

#define KUSER_SHARED_DATA_VA        0xFFFFF78000000000ULL

// Offsets within KUSER_SHARED_DATA structure (Windows 10/11)
#define KSDATA_INTERRUPT_TIME       0x008   // LARGE_INTEGER InterruptTime
#define KSDATA_SYSTEM_TIME          0x014   // LARGE_INTEGER SystemTime
#define KSDATA_TICK_COUNT_LOW       0x320   // ULONG TickCountLow
#define KSDATA_TICK_COUNT_MULTI     0x004   // ULONG TickCountMultiplier
#define KSDATA_INTERRUPT_TIME_HIGH  0x01C   // For lock-free reads

// State for KUSER_SHARED_DATA shadow page
typedef struct _KUSER_SHADOW_STATE {
    bool    Enabled;                // Shadow system active
    bool    Initialized;            // Setup complete
    U64     KuserGpa;               // Guest physical address of KUSER_SHARED_DATA
    U64     ShadowHpa;              // Host physical of shadow page
    void*   ShadowVa;               // Virtual address of shadow page (for updates)
    void*   OriginalVa;             // Virtual of original (for copying)
    U64     LastUpdateTsc;          // TSC of last shadow update
    I64     AccumulatedOffset100ns; // Total time offset in 100ns units
} KUSER_SHADOW_STATE;

// Convert TSC cycles to 100ns units (Windows system time resolution)
// tscFrequencyHz: CPU TSC frequency (from CPUID 0x15 or calibration)
I64 TscCyclesToTime100ns(U64 cycles, U64 tscFrequencyHz);

// Initialize KUSER_SHARED_DATA shadowing
// Must be called after EPT is initialized, before guest boots
OMBRA_STATUS KuserShadowInit(KUSER_SHADOW_STATE* state, struct _EPT_STATE* ept,
                              U64 kuserGpa, void* shadowPage, U64 shadowHpa);

// Update shadow page with adjusted time values
// Called from TimingApplyCompensation to keep times synchronized
void KuserShadowUpdate(KUSER_SHADOW_STATE* state, U64 hiddenCycles, U64 tscFreqHz);

// Copy original KUSER_SHARED_DATA and apply time adjustments
void AdjustKuserSharedData(U8* shadowPage, const U8* originalPage, I64 hiddenTime100ns);

// TSC offset reset thresholds to prevent wraparound detection
#define TSC_RESET_THRESHOLD         100000  // Reset after 100K VM-exits
#define TSC_RESET_MAX_OFFSET        0x7FFFFFFFFFFF  // Prevent >47-bit offset (sign extension issues)

// Initialize timing compensation for a CPU
void TimingInitialize(struct _VMX_CPU* cpu);

// Reset TSC offset to prevent wraparound (internal, called automatically)
void TimingResetOffset(struct _VMX_CPU* cpu);

// Calibrate actual VM-exit overhead (called during initialization)
void TimingCalibrate(struct _VMX_CPU* cpu);

// Mark the start of timing-sensitive exit handler
// Returns the TSC value at entry - store this and pass to TimingEnd
U64 TimingStart(void);

// Compensate for exit handler overhead
// entryTsc: TSC value captured at handler entry (from TimingStart)
// estimatedOverhead: Expected overhead in cycles for this exit type
void TimingEnd(struct _VMX_CPU* cpu, U64 entryTsc, U64 estimatedOverhead);

// Apply timing compensation by adjusting VMCS TSC offset
// This hides the VM-exit overhead from guest TSC measurements
void TimingApplyCompensation(struct _VMX_CPU* cpu, U64 overhead);

#endif
