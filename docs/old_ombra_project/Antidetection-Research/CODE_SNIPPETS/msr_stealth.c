/**
 * MSR Stealth Patterns
 * Extracted from: ksm, HyperPlatform, hvpp, NoirVisor
 *
 * These patterns demonstrate MSR interception and filtering for
 * hypervisor detection evasion.
 */

#include <stdint.h>

/* VMX Capability MSRs */
#define MSR_IA32_VMX_BASIC              0x480
#define MSR_IA32_VMX_PINBASED_CTLS      0x481
#define MSR_IA32_VMX_PROCBASED_CTLS     0x482
#define MSR_IA32_VMX_EXIT_CTLS          0x483
#define MSR_IA32_VMX_ENTRY_CTLS         0x484
#define MSR_IA32_VMX_MISC               0x485
#define MSR_IA32_VMX_CR0_FIXED0         0x486
#define MSR_IA32_VMX_CR0_FIXED1         0x487
#define MSR_IA32_VMX_CR4_FIXED0         0x488
#define MSR_IA32_VMX_CR4_FIXED1         0x489
#define MSR_IA32_VMX_VMCS_ENUM          0x48A
#define MSR_IA32_VMX_PROCBASED_CTLS2    0x48B
#define MSR_IA32_VMX_EPT_VPID_CAP       0x48C
#define MSR_IA32_VMX_TRUE_PINBASED_CTLS 0x48D
#define MSR_IA32_VMX_TRUE_PROCBASED_CTLS 0x48E
#define MSR_IA32_VMX_TRUE_EXIT_CTLS     0x48F
#define MSR_IA32_VMX_TRUE_ENTRY_CTLS    0x490
#define MSR_IA32_VMX_VMFUNC             0x491

/* Feature Control MSR */
#define MSR_IA32_FEATURE_CONTROL        0x3A
#define FEATURE_CONTROL_LOCK            (1 << 0)
#define FEATURE_CONTROL_VMXON_SMX       (1 << 1)
#define FEATURE_CONTROL_VMXON_OUTSIDE_SMX (1 << 2)

/* TSC Related MSRs */
#define MSR_IA32_TSC_ADJUST             0x3B

/* Other Sensitive MSRs */
#define MSR_IA32_DEBUGCTL               0x1D9
#define MSR_LSTAR                       0xC0000082
#define MSR_IA32_SYSENTER_CS            0x174
#define MSR_IA32_SYSENTER_ESP           0x175
#define MSR_IA32_SYSENTER_EIP           0x176

/* Exception Vector */
#define X86_TRAP_GP                     13

/**
 * Pattern 1: MSR Bitmap Setup (from ksm)
 *
 * Source: ksm/vmx.c
 *
 * 4KB MSR bitmap controls which MSRs cause VMExit.
 * Bits are set for MSRs that should trigger VMExit on RDMSR/WRMSR.
 */
#define MSR_BITMAP_SIZE     0x1000  /* 4KB */

typedef struct {
    uint8_t read_low[1024];     /* MSRs 0x00000000 - 0x00001FFF */
    uint8_t read_high[1024];    /* MSRs 0xC0000000 - 0xC0001FFF */
    uint8_t write_low[1024];    /* MSRs 0x00000000 - 0x00001FFF */
    uint8_t write_high[1024];   /* MSRs 0xC0000000 - 0xC0001FFF */
} msr_bitmap_t;

static inline void set_msr_bit(uint8_t* bitmap, uint32_t msr)
{
    uint32_t byte_offset = (msr & 0x1FFF) / 8;
    uint32_t bit_offset = (msr & 0x1FFF) % 8;
    bitmap[byte_offset] |= (1 << bit_offset);
}

static void init_msr_bitmap_stealth(msr_bitmap_t* bitmap)
{
    /* Clear all bits first (no interception by default) */
    memset(bitmap, 0, MSR_BITMAP_SIZE);

    /* Intercept IA32_FEATURE_CONTROL reads */
    set_msr_bit(bitmap->read_low, MSR_IA32_FEATURE_CONTROL);

    /* Intercept ALL VMX capability MSR reads */
    for (uint32_t msr = MSR_IA32_VMX_BASIC; msr <= MSR_IA32_VMX_VMFUNC; msr++) {
        set_msr_bit(bitmap->read_low, msr);
    }

    /* Intercept TSC_ADJUST reads (can detect TSC manipulation) */
    set_msr_bit(bitmap->read_low, MSR_IA32_TSC_ADJUST);

    /* Optionally intercept DEBUGCTL (LBR can leak addresses) */
    set_msr_bit(bitmap->read_low, MSR_IA32_DEBUGCTL);
}

/**
 * Pattern 2: RDMSR Handler with #GP Injection (from ksm)
 *
 * Source: ksm/introspect.c
 *
 * When guest reads VMX MSRs, inject #GP to make VMX appear unsupported.
 */
static int handle_rdmsr_stealth(uint32_t msr, uint64_t* value, int* inject_gp)
{
    *inject_gp = 0;

    /* VMX capability MSRs - inject #GP (make VMX appear unsupported) */
    if (msr >= MSR_IA32_VMX_BASIC && msr <= MSR_IA32_VMX_VMFUNC) {
        *inject_gp = 1;
        return 1;  /* Don't provide a value */
    }

    /* IA32_FEATURE_CONTROL - clear VMX enable bits */
    if (msr == MSR_IA32_FEATURE_CONTROL) {
        uint64_t real_value = __readmsr(MSR_IA32_FEATURE_CONTROL);

        /* Clear VMX enable bits while keeping lock bit */
        *value = real_value & ~(FEATURE_CONTROL_VMXON_SMX |
                                FEATURE_CONTROL_VMXON_OUTSIDE_SMX);
        return 0;  /* Provide modified value */
    }

    /* TSC_ADJUST - return 0 to hide TSC manipulation */
    if (msr == MSR_IA32_TSC_ADJUST) {
        *value = 0;
        return 0;
    }

    /* Other MSRs - read real value */
    *value = __readmsr(msr);
    return 0;
}

/**
 * Pattern 3: #GP Exception Injection
 *
 * Source: HyperPlatform/vm.cpp
 *
 * Inject #GP(0) to make guest think MSR doesn't exist.
 */
typedef struct {
    uint32_t vector : 8;
    uint32_t type : 3;
    uint32_t error_code_valid : 1;
    uint32_t reserved : 19;
    uint32_t valid : 1;
} vmentry_interrupt_info_t;

#define INTERRUPT_TYPE_HARDWARE_EXCEPTION   3

static void inject_gp_fault(void* vmcs_ptr)
{
    vmentry_interrupt_info_t info = {0};

    info.vector = X86_TRAP_GP;
    info.type = INTERRUPT_TYPE_HARDWARE_EXCEPTION;
    info.error_code_valid = 1;
    info.valid = 1;

    /* Write to VMCS VM-Entry Interruption-Information field */
    __vmx_vmwrite(0x4016, *(uint32_t*)&info);

    /* Write error code 0 */
    __vmx_vmwrite(0x4018, 0);
}

/**
 * Pattern 4: AMD MSR Handling (from SimpleSvm/NoirVisor)
 *
 * Source: SimpleSvm/SimpleSvm.cpp
 *
 * AMD SVM uses MSRPM (MSR Permission Map) instead of bitmap.
 */
#define SVM_MSRPM_SIZE      0x2000  /* 8KB for AMD */

typedef struct {
    uint8_t map[0x2000];
} svm_msrpm_t;

/*
 * MSRPM format for AMD:
 * 0x0000 - 0x17FF: MSRs 0x00000000 - 0x00001FFF
 * 0x1800 - 0x1FFF: MSRs 0xC0000000 - 0xC0001FFF
 *
 * Each MSR gets 2 bits: [1] = intercept write, [0] = intercept read
 */
static void set_svm_msr_intercept(svm_msrpm_t* msrpm, uint32_t msr, int read, int write)
{
    uint32_t base_offset, bit_offset;

    if (msr <= 0x1FFF) {
        base_offset = (msr * 2) / 8;
        bit_offset = (msr * 2) % 8;
    } else if (msr >= 0xC0000000 && msr <= 0xC0001FFF) {
        base_offset = 0x1800 + ((msr - 0xC0000000) * 2) / 8;
        bit_offset = ((msr - 0xC0000000) * 2) % 8;
    } else {
        return;  /* MSR out of range */
    }

    if (read)
        msrpm->map[base_offset] |= (1 << bit_offset);
    if (write)
        msrpm->map[base_offset] |= (1 << (bit_offset + 1));
}

/**
 * Pattern 5: WRMSR Handler (from HyperPlatform)
 *
 * Source: HyperPlatform/vm.cpp
 *
 * Handle writes to sensitive MSRs.
 */
static int handle_wrmsr_stealth(uint32_t msr, uint64_t value)
{
    switch (msr) {
    case MSR_IA32_FEATURE_CONTROL:
        /* Ignore writes to FEATURE_CONTROL */
        /* Guest thinks it succeeded, but we don't change anything */
        return 0;

    case MSR_IA32_TSC_ADJUST:
        /* Ignore TSC_ADJUST writes */
        return 0;

    case MSR_IA32_DEBUGCTL:
        /* Optionally filter DEBUGCTL to prevent LBR use */
        __writemsr(MSR_IA32_DEBUGCTL, value & ~0x3);  /* Clear LBR bits */
        return 0;

    default:
        /* Pass through other writes */
        __writemsr(msr, value);
        return 0;
    }
}

/**
 * Pattern 6: Complete MSR Exit Handler
 *
 * Combined pattern showing full RDMSR/WRMSR handling.
 */
typedef struct {
    uint32_t exit_reason;
    uint64_t exit_qualification;
    /* ... other fields ... */
} vmexit_info_t;

#define EXIT_REASON_RDMSR   31
#define EXIT_REASON_WRMSR   32

static int handle_msr_exit(vmexit_info_t* info, uint32_t msr,
                           uint64_t* rax, uint64_t* rdx, uint64_t* rcx)
{
    if (info->exit_reason == EXIT_REASON_RDMSR) {
        uint64_t value;
        int inject_gp = 0;

        handle_rdmsr_stealth(msr, &value, &inject_gp);

        if (inject_gp) {
            inject_gp_fault(NULL);  /* Will cause guest to see #GP */
            return 1;  /* Exception injected */
        }

        /* Return value in EAX:EDX */
        *rax = (uint32_t)(value & 0xFFFFFFFF);
        *rdx = (uint32_t)(value >> 32);

    } else if (info->exit_reason == EXIT_REASON_WRMSR) {
        uint64_t value = (*rax & 0xFFFFFFFF) | ((*rdx & 0xFFFFFFFF) << 32);
        handle_wrmsr_stealth(msr, value);
    }

    /* Advance RIP past RDMSR/WRMSR (2 bytes) */
    return 0;
}

/*
 * Usage Notes:
 *
 * 1. Allocate 4KB page-aligned MSR bitmap (Intel) or 8KB MSRPM (AMD)
 * 2. Initialize bitmap to intercept VMX-related MSRs
 * 3. Set bitmap address in VMCS/VMCB
 * 4. Handle RDMSR/WRMSR exits in VMExit handler
 * 5. Inject #GP for VMX MSRs to make VMX appear unsupported
 * 6. Filter IA32_FEATURE_CONTROL to hide VMX enable bits
 *
 * Critical: Test that Windows still boots (some MSRs are needed).
 */
