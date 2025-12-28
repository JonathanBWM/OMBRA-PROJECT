/**
 * 03_timing.c - Timing Attack Mitigation
 * 
 * Pure C implementation of TSC offsetting and timing evasion.
 * 
 * CONCEPTS COVERED:
 * - VMExit timing overhead
 * - TSC offsetting
 * - RDTSC/RDTSCP interception
 * - Timing ratio attacks
 */

#include "../common/types.h"
#include "../common/vmcs_defs.h"
#include "../common/msr_defs.h"

/*===========================================================================
 * CONCEPT 1: The Timing Detection Problem
 *===========================================================================
 * VMExits take time (~500-2000 cycles depending on exit reason).
 * Detection method:
 * 
 *   t1 = RDTSC
 *   CPUID (causes VMExit)
 *   t2 = RDTSC
 *   delta = t2 - t1
 *   
 *   if (delta > threshold)
 *       // Likely virtualized
 * 
 * Native CPUID: ~100-200 cycles
 * Virtualized CPUID: ~1000-3000 cycles
 * 
 * The difference is detectable.
 */

/*===========================================================================
 * CONCEPT 2: TSC Offsetting
 *===========================================================================
 * Intel VMX provides TSC_OFFSET field in VMCS.
 * When guest reads TSC:
 *   returned_value = real_tsc + TSC_OFFSET
 * 
 * Strategy:
 * 1. Before VMExit handler runs, record TSC
 * 2. After VMExit handler finishes, calculate elapsed time
 * 3. Subtract elapsed time from TSC_OFFSET
 * 4. Guest sees no time passed during VMExit
 * 
 * VMCS field: VMCS_CTRL_TSC_OFFSET (0x2010)
 * Control bit: CPU_TSC_OFFSETTING in primary proc-based controls
 */

extern VMX_RESULT AsmVmread(U64 field, U64* value);
extern VMX_RESULT AsmVmwrite(U64 field, U64 value);

/*===========================================================================
 * CONCEPT 3: Per-CPU Timing State
 *===========================================================================*/

typedef struct _TIMING_STATE {
    U64     VmexitStartTsc;     /* TSC when VMExit started */
    U64     CurrentOffset;       /* Current TSC offset value */
    U64     TotalHiddenCycles;   /* Total cycles hidden from guest */
    bool    OffsetEnabled;       /* TSC offsetting active */
} TIMING_STATE;

/*===========================================================================
 * CONCEPT 4: VMExit Timing Compensation
 *===========================================================================*/

/**
 * Call at the START of VMExit handler
 * Records timestamp for later compensation
 */
static inline void TimingVmexitStart(TIMING_STATE* state)
{
    state->VmexitStartTsc = __rdtsc();
}

/**
 * Call at the END of VMExit handler (before VMRESUME)
 * Adjusts TSC offset to hide elapsed time
 */
static inline void TimingVmexitEnd(TIMING_STATE* state)
{
    U64 vmexitEndTsc;
    U64 elapsedCycles;
    
    vmexitEndTsc = __rdtsc();
    elapsedCycles = vmexitEndTsc - state->VmexitStartTsc;
    
    /* Subtract elapsed time from offset */
    state->CurrentOffset -= elapsedCycles;
    state->TotalHiddenCycles += elapsedCycles;
    
    /* Write new offset to VMCS */
    AsmVmwrite(VMCS_CTRL_TSC_OFFSET, state->CurrentOffset);
}

/*===========================================================================
 * CONCEPT 5: Complete VMExit Wrapper Pattern
 *===========================================================================*/

/**
 * Wrapper that adds timing compensation to any VMExit handler
 * 
 * Usage pattern:
 *   TimingVmexitStart(&timing);
 *   action = ActualVmexitHandler(regs);
 *   TimingVmexitEnd(&timing);
 *   // VMRESUME
 */

typedef int (*VMEXIT_HANDLER)(GUEST_REGS* regs);

int TimingCompensatedHandler(
    GUEST_REGS* regs,
    TIMING_STATE* timing,
    VMEXIT_HANDLER handler)
{
    int action;
    
    /* Record entry time */
    TimingVmexitStart(timing);
    
    /* Run actual handler */
    action = handler(regs);
    
    /* Compensate for elapsed time */
    TimingVmexitEnd(timing);
    
    return action;
}

/*===========================================================================
 * CONCEPT 6: RDTSC Interception (Alternative)
 *===========================================================================
 * Instead of TSC offsetting, intercept RDTSC directly.
 * More control but causes VMExit on every RDTSC (performance hit).
 * 
 * Enable: Set CPU_RDTSC_EXIT in primary proc-based controls
 * Exit reason: EXIT_REASON_RDTSC (17)
 */

/**
 * Handle RDTSC VMExit
 * Returns compensated TSC value
 */
void HandleRdtscExit(GUEST_REGS* regs, TIMING_STATE* timing)
{
    U64 tsc;
    
    /* Read real TSC */
    tsc = __rdtsc();
    
    /* Apply offset */
    if (timing->OffsetEnabled) {
        tsc += timing->CurrentOffset;
    }
    
    /* Return in EDX:EAX */
    regs->Rax = (U32)(tsc & 0xFFFFFFFF);
    regs->Rdx = (U32)(tsc >> 32);
}

/**
 * Handle RDTSCP VMExit
 * Same as RDTSC but also returns processor ID
 */
void HandleRdtscpExit(GUEST_REGS* regs, TIMING_STATE* timing)
{
    U64 tsc;
    U32 aux;
    
    /* Read real TSC and TSC_AUX */
    tsc = __rdtsc();
    aux = (U32)__readmsr(MSR_IA32_TSC_AUX);
    
    /* Apply offset */
    if (timing->OffsetEnabled) {
        tsc += timing->CurrentOffset;
    }
    
    /* Return in EDX:EAX:ECX */
    regs->Rax = (U32)(tsc & 0xFFFFFFFF);
    regs->Rdx = (U32)(tsc >> 32);
    regs->Rcx = aux;
}

/*===========================================================================
 * CONCEPT 7: Timing Ratio Attack Mitigation
 *===========================================================================
 * Advanced detection compares ratios:
 * 
 *   t1 = RDTSC
 *   loop 1000 times: NOP
 *   t2 = RDTSC
 *   t3 = RDTSC  
 *   CPUID
 *   t4 = RDTSC
 *   
 *   nop_time = t2 - t1
 *   cpuid_time = t4 - t3
 *   ratio = cpuid_time / nop_time
 *   
 *   if (ratio > expected_ratio)
 *       // Virtualized
 * 
 * This is harder to defeat because we can't just subtract time.
 * Solution: Also virtualize loop timing OR accept detection risk.
 */

/*===========================================================================
 * CONCEPT 8: KUSER_SHARED_DATA Time
 *===========================================================================
 * Windows keeps system time in KUSER_SHARED_DATA (0xFFFFF78000000000).
 * Anti-cheats compare RDTSC with this:
 * 
 *   t1 = RDTSC
 *   kernel_time = KUSER_SHARED_DATA->SystemTime
 *   t2 = RDTSC
 *   
 *   // Compare against expected relationship
 * 
 * Solution: Use EPT to shadow KUSER_SHARED_DATA page.
 * Return modified time values that match our TSC offset.
 */

#define KUSER_SHARED_DATA_VA    0xFFFFF78000000000ULL

/* Offsets within KUSER_SHARED_DATA */
#define KSDATA_SYSTEM_TIME      0x014   /* LARGE_INTEGER SystemTime */
#define KSDATA_INTERRUPT_TIME   0x008   /* LARGE_INTEGER InterruptTime */
#define KSDATA_TICK_COUNT       0x320   /* ULONG TickCount */

/**
 * Adjust KUSER_SHARED_DATA times to match TSC offset
 * Called before guest reads the page (via EPT read handler)
 * 
 * @param shadowPage    Shadow copy of KUSER_SHARED_DATA
 * @param originalPage  Original KUSER_SHARED_DATA
 * @param hiddenTime100ns  Time to subtract (in 100ns units)
 */
void AdjustKuserSharedData(
    U8* shadowPage,
    const U8* originalPage,
    I64 hiddenTime100ns)
{
    U64 systemTime;
    U64 interruptTime;
    
    /* Copy original page */
    for (int i = 0; i < PAGE_SIZE; i++) {
        shadowPage[i] = originalPage[i];
    }
    
    /* Adjust SystemTime */
    systemTime = *(U64*)(originalPage + KSDATA_SYSTEM_TIME);
    systemTime -= hiddenTime100ns;
    *(U64*)(shadowPage + KSDATA_SYSTEM_TIME) = systemTime;
    
    /* Adjust InterruptTime */
    interruptTime = *(U64*)(originalPage + KSDATA_INTERRUPT_TIME);
    interruptTime -= hiddenTime100ns;
    *(U64*)(shadowPage + KSDATA_INTERRUPT_TIME) = interruptTime;
}

/*===========================================================================
 * CONCEPT 9: Initialize Timing Protection
 *===========================================================================*/

/**
 * Initialize timing state for a CPU
 */
void TimingInit(TIMING_STATE* state)
{
    state->VmexitStartTsc = 0;
    state->CurrentOffset = 0;
    state->TotalHiddenCycles = 0;
    state->OffsetEnabled = true;
    
    /* Write initial offset to VMCS (0 = no adjustment) */
    AsmVmwrite(VMCS_CTRL_TSC_OFFSET, 0);
}

/**
 * Convert TSC cycles to 100ns units (approximate)
 * Uses CPU frequency - get from CPUID or calibrate
 */
I64 TscCyclesToTime100ns(U64 cycles, U64 tscFrequencyHz)
{
    /* 100ns = 10,000,000 per second */
    return (I64)((cycles * 10000000ULL) / tscFrequencyHz);
}
