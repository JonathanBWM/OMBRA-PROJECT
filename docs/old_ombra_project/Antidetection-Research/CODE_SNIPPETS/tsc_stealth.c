/**
 * TSC (Time Stamp Counter) Stealth Patterns
 * Extracted from: ksm, hvpp, VBoxHardenedLoader, RDTSC-KVM-Handler
 *
 * These patterns demonstrate timing attack mitigation for
 * hypervisor detection evasion.
 */

#include <stdint.h>

/* VMCS TSC Fields */
#define VMCS_TSC_OFFSET             0x2010
#define VMCS_TSC_MULTIPLIER         0x2032  /* If TSC scaling supported */

/* Execution Controls */
#define CPU_BASED_RDTSC_EXITING     (1 << 12)
#define SECONDARY_TSC_SCALING       (1 << 25)

/**
 * Pattern 1: Hardware TSC Offset (Preferred Method)
 *
 * Source: VBoxHardenedLoader, ksm
 *
 * Use VMCS TSC_OFFSET field to hide VMExit latency.
 * This is the preferred approach - zero runtime overhead.
 */
static void setup_tsc_offset_hardware(void* vmcs, int64_t offset)
{
    /*
     * TSC_OFFSET is added to guest TSC reads automatically.
     * Set to negative value to compensate for VMExit overhead.
     */
    __vmx_vmwrite(VMCS_TSC_OFFSET, offset);

    /*
     * Disable RDTSC exiting - let hardware handle it.
     * This avoids VMExit overhead entirely.
     */
    uint64_t proc_ctls = __vmx_vmread(0x4002);  /* Primary Proc Controls */
    proc_ctls &= ~CPU_BASED_RDTSC_EXITING;
    __vmx_vmwrite(0x4002, proc_ctls);
}

/**
 * Pattern 2: Calibrated TSC Offset
 *
 * Source: ksm/introspect.c
 *
 * Measure VMExit overhead and calibrate offset.
 */
typedef struct {
    uint64_t exit_tsc;
    uint64_t entry_tsc;
    uint64_t accumulated_overhead;
    uint64_t exit_count;
} tsc_tracking_t;

static int64_t calibrate_tsc_offset(void)
{
    int64_t total_overhead = 0;
    int samples = 100;

    for (int i = 0; i < samples; i++) {
        uint64_t start = __rdtsc();

        /*
         * Simulate a minimal VMExit/VMResume cycle.
         * In practice, measure actual VMExit overhead.
         */

        uint64_t end = __rdtsc();
        total_overhead += (end - start);
    }

    /* Return negative average overhead as offset */
    return -(total_overhead / samples);
}

/**
 * Pattern 3: Software RDTSC Handler (from ksm)
 *
 * Source: ksm/introspect.c
 *
 * Handle RDTSC in software when dynamic compensation needed.
 */
static int handle_rdtsc_exit(uint64_t* rax, uint64_t* rdx, tsc_tracking_t* track)
{
    uint64_t tsc = __rdtsc();

    /*
     * Calculate overhead since VMExit occurred.
     * Subtract this from the TSC value we return.
     */
    uint64_t handler_overhead = tsc - track->exit_tsc;

    /*
     * Compensate for:
     * 1. Time spent in VMExit handler
     * 2. Accumulated overhead from previous exits
     */
    uint64_t compensated = tsc - handler_overhead - track->accumulated_overhead;

    /* Split into EAX (low) and EDX (high) */
    *rax = (uint32_t)(compensated & 0xFFFFFFFF);
    *rdx = (uint32_t)(compensated >> 32);

    return 0;
}

/**
 * Pattern 4: RDTSCP Handler (from ksm)
 *
 * Source: ksm/introspect.c
 *
 * RDTSCP also returns TSC_AUX in ECX.
 */
static int handle_rdtscp_exit(uint64_t* rax, uint64_t* rdx, uint64_t* rcx,
                              tsc_tracking_t* track)
{
    /* Get compensated TSC */
    handle_rdtsc_exit(rax, rdx, track);

    /* Get TSC_AUX value (usually processor ID) */
    uint32_t aux;
    unsigned int temp;
    *rcx = __rdtscp(&temp);

    return 0;
}

/**
 * Pattern 5: Per-VCPU TSC Tracking (from hvpp)
 *
 * Source: hvpp/vcpu.cpp
 *
 * Track TSC deltas per virtual CPU for accurate compensation.
 */
typedef struct {
    uint64_t tsc_entry;         /* TSC at VMEntry */
    uint64_t tsc_delta_sum;     /* Accumulated delta */
    uint64_t tsc_delta_previous;
} vcpu_tsc_state_t;

static void on_vmexit(vcpu_tsc_state_t* state)
{
    /* Record TSC at VMExit */
    state->tsc_entry = __rdtsc();
}

static void on_vmentry(vcpu_tsc_state_t* state)
{
    uint64_t now = __rdtsc();

    /* Calculate time spent in hypervisor */
    uint64_t delta = now - state->tsc_entry;

    /* Add to accumulated overhead */
    state->tsc_delta_sum += delta;
    state->tsc_delta_previous = delta;
}

static uint64_t get_compensated_tsc(vcpu_tsc_state_t* state)
{
    return __rdtsc() - state->tsc_delta_sum;
}

/**
 * Pattern 6: AMD SVM TSC Handling
 *
 * Source: SimpleSvm, NoirVisor
 *
 * AMD uses VMCB offset 0x0B0 for TSC_OFFSET.
 */
#define VMCB_TSC_OFFSET     0x0B0
#define VMCB_INTERCEPT_RDTSC (1ULL << 55)

static void setup_amd_tsc_offset(void* vmcb, int64_t offset)
{
    /* Write TSC offset to VMCB */
    *(int64_t*)((uint8_t*)vmcb + VMCB_TSC_OFFSET) = offset;

    /*
     * Clear RDTSC intercept bit to use hardware offsetting.
     * VMCB intercept bits are at offset 0x0C.
     */
    uint64_t* intercepts = (uint64_t*)((uint8_t*)vmcb + 0x0C);
    *intercepts &= ~VMCB_INTERCEPT_RDTSC;
}

/**
 * Pattern 7: Timing Attack Detection Thresholds
 *
 * Common timing checks and their thresholds.
 */
#define BARE_METAL_CPUID_CYCLES     50      /* Typical cycles for CPUID */
#define BARE_METAL_RDTSC_CYCLES     25      /* Typical cycles for RDTSC */
#define VM_CPUID_CYCLES_MIN         500     /* Min cycles that indicate VM */
#define VM_CPUID_CYCLES_TYPICAL     2000    /* Typical unoptimized VM */

/*
 * Detection pattern (what we need to defeat):
 *
 * uint64_t start = __rdtsc();
 * __cpuid(regs, 0);
 * uint64_t end = __rdtsc();
 * uint64_t cycles = end - start;
 *
 * if (cycles > 1000) {
 *     // VM detected!
 * }
 */

/**
 * Pattern 8: Complete TSC Stealth Setup
 *
 * Combined pattern showing initialization and handling.
 */
typedef struct {
    int use_hardware_offset;
    int64_t calibrated_offset;
    vcpu_tsc_state_t vcpu_state;
} tsc_stealth_config_t;

static void init_tsc_stealth(tsc_stealth_config_t* config)
{
    /* Calibrate offset by measuring VMExit overhead */
    config->calibrated_offset = calibrate_tsc_offset();

    /* Prefer hardware offsetting */
    config->use_hardware_offset = 1;

    /* Initialize per-VCPU state */
    memset(&config->vcpu_state, 0, sizeof(vcpu_tsc_state_t));
}

static void apply_tsc_stealth(tsc_stealth_config_t* config, void* vmcs)
{
    if (config->use_hardware_offset) {
        setup_tsc_offset_hardware(vmcs, config->calibrated_offset);
    } else {
        /* Enable RDTSC exiting for software handling */
        uint64_t proc_ctls = __vmx_vmread(0x4002);
        proc_ctls |= CPU_BASED_RDTSC_EXITING;
        __vmx_vmwrite(0x4002, proc_ctls);
    }
}

/*
 * Usage Notes:
 *
 * 1. Hardware TSC offset (preferred):
 *    - Zero runtime overhead
 *    - Set VMCS TSC_OFFSET to negative value
 *    - Disable RDTSC exiting
 *
 * 2. Software compensation (if needed):
 *    - Enable RDTSC exiting
 *    - Track VMExit timing per-VCPU
 *    - Subtract accumulated overhead
 *
 * 3. Calibration:
 *    - Measure typical VMExit overhead on target CPU
 *    - Intel: ~500-2000 cycles
 *    - AMD: ~500-1500 cycles
 *    - Goal: Make CPUID appear to take <200 cycles
 *
 * 4. Testing:
 *    - Use pafish timing checks
 *    - Measure RDTSC around CPUID
 *    - Target: <1000 cycles (ideally <200)
 *
 * Critical: Test on target hardware - overhead varies by CPU model.
 */
