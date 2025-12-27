/**
 * CPUID Stealth Patterns
 * Extracted from: ksm, hvpp, HyperPlatform, NoirVisor, VBoxHardenedLoader
 *
 * These patterns demonstrate CPUID interception and filtering for
 * hypervisor detection evasion.
 */

#include <stdint.h>

/* CPUID Feature Bits */
#define CPUID_VMX_BIT       (1 << 5)    /* ECX bit 5: VMX support */
#define CPUID_HV_BIT        (1 << 31)   /* ECX bit 31: Hypervisor present */

/* Hypervisor CPUID Leaves */
#define CPUID_HV_VENDOR     0x40000000
#define CPUID_HV_INTERFACE  0x40000001
#define CPUID_HV_MAX_LEAF   0x400000FF

/**
 * Pattern 1: Basic CPUID Handler (from ksm)
 *
 * Source: ksm/introspect.c
 *
 * Intercepts CPUID instruction and filters hypervisor-related leaves.
 */
typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} cpuid_regs_t;

static int handle_cpuid_exit(cpuid_regs_t* regs, uint32_t function, uint32_t subfunction)
{
    /* Execute real CPUID first */
    __cpuidex((int*)regs, function, subfunction);

    /* Hypervisor CPUID space - return zeros */
    if (function >= CPUID_HV_VENDOR && function <= CPUID_HV_MAX_LEAF) {
        regs->eax = 0;
        regs->ebx = 0;
        regs->ecx = 0;
        regs->edx = 0;
        return 0;  /* Handled */
    }

    /* Hide virtualization features in leaf 1 */
    if (function == 1) {
        regs->ecx &= ~CPUID_VMX_BIT;   /* Clear VMX bit */
        regs->ecx &= ~CPUID_HV_BIT;    /* Clear Hypervisor bit */
    }

    return 0;
}

/**
 * Pattern 2: Stealth Mode Toggle (from NoirVisor)
 *
 * Source: NoirVisor/src/vt_core/vt_exit.c
 *
 * Conditional stealth based on configuration flag.
 */
typedef struct {
    int stealth_mode;
    /* ... other fields ... */
} hypervisor_config_t;

static int handle_cpuid_stealth(cpuid_regs_t* regs, uint32_t function,
                                hypervisor_config_t* config)
{
    __cpuidex((int*)regs, function, 0);

    if (!config->stealth_mode) {
        return 0;  /* Stealth disabled, return real values */
    }

    switch (function) {
    case 0:
        /* Optionally hide/modify vendor string */
        break;

    case 1:
        /* Hide hypervisor presence */
        regs->ecx &= ~CPUID_HV_BIT;
        regs->ecx &= ~CPUID_VMX_BIT;
        break;

    case CPUID_HV_VENDOR:
    case CPUID_HV_INTERFACE:
        /* Return zeros for hypervisor leaves */
        regs->eax = regs->ebx = regs->ecx = regs->edx = 0;
        break;

    default:
        if (function >= CPUID_HV_VENDOR && function <= CPUID_HV_MAX_LEAF) {
            regs->eax = regs->ebx = regs->ecx = regs->edx = 0;
        }
        break;
    }

    return 0;
}

/**
 * Pattern 3: VMCALL via CPUID (from Voyager/Sputnik)
 *
 * Source: Voyager/PayLoad/Hv.cpp
 *
 * Uses CPUID with magic value in RCX to trigger hypercall dispatch.
 * This pattern allows guest to communicate with hypervisor while
 * hiding CPUID-based detection.
 */
#define VMCALL_MAGIC_KEY    0xDEADBEEF12345678ULL

typedef enum {
    VMCALL_READ_PHY = 0,
    VMCALL_WRITE_PHY,
    VMCALL_TRANSLATE,
    VMCALL_GET_DIRBASE,
    /* ... more commands ... */
} vmcall_command_t;

static int handle_cpuid_vmcall(cpuid_regs_t* regs, uint64_t rcx_key)
{
    /* Check if this is a VMCALL request */
    if (rcx_key == VMCALL_MAGIC_KEY) {
        uint32_t command = regs->ecx;  /* Command in original ECX low bits */

        switch (command) {
        case VMCALL_READ_PHY:
            /* Handle physical memory read */
            break;
        case VMCALL_WRITE_PHY:
            /* Handle physical memory write */
            break;
        default:
            break;
        }

        return 1;  /* VMCALL handled, don't execute real CPUID */
    }

    /* Normal CPUID handling */
    return 0;
}

/**
 * Pattern 4: AMD CPUID Handling (from SimpleSvm)
 *
 * Source: SimpleSvm/SimpleSvm/SimpleSvm.cpp
 *
 * AMD SVM uses different bits but same concept.
 */
#define CPUID_SVM_BIT       (1 << 2)    /* ECX bit 2: SVM support (leaf 0x80000001) */

static int handle_cpuid_amd(cpuid_regs_t* regs, uint32_t function)
{
    __cpuidex((int*)regs, function, 0);

    switch (function) {
    case 1:
        /* Clear hypervisor bit (same as Intel) */
        regs->ecx &= ~CPUID_HV_BIT;
        break;

    case 0x80000001:
        /* AMD extended features - optionally hide SVM */
        regs->ecx &= ~CPUID_SVM_BIT;
        break;

    default:
        if (function >= CPUID_HV_VENDOR && function <= CPUID_HV_MAX_LEAF) {
            regs->eax = regs->ebx = regs->ecx = regs->edx = 0;
        }
        break;
    }

    return 0;
}

/**
 * Pattern 5: Vendor String Masking (from VBoxHardenedLoader)
 *
 * Source: VBoxHardenedLoader/Source/patterns.h
 *
 * Some detection checks vendor string in CPUID leaf 0.
 */
static int mask_vendor_string(cpuid_regs_t* regs)
{
    /* Leaf 0 returns vendor string in EBX:EDX:ECX */
    /* "GenuineIntel" = 0x756E6547, 0x49656E69, 0x6C65746E */
    /* "AuthenticAMD" = 0x68747541, 0x69746E65, 0x444D4163 */

    /* Don't modify - use real CPU vendor string */
    /* This ensures detection tools see real hardware */

    return 0;
}

/**
 * Pattern 6: Hyper-V Enlightenment Spoofing (Optional)
 *
 * For compatibility with Windows expecting Hyper-V,
 * can optionally spoof Hyper-V vendor string.
 */
#define HYPERV_CPUID_VENDOR_ID      "Microsoft Hv"
#define HYPERV_CPUID_INTERFACE      0x40000001
#define HYPERV_CPUID_VERSION        0x40000002

static int spoof_hyperv_cpuid(cpuid_regs_t* regs, uint32_t function)
{
    /* Only spoof if specifically configured for Hyper-V compat */

    switch (function) {
    case CPUID_HV_VENDOR:
        /* Report as Hyper-V */
        regs->eax = HYPERV_CPUID_VERSION;  /* Max leaf */
        regs->ebx = 0x7263694D;  /* "Micr" */
        regs->ecx = 0x666F736F;  /* "osof" */
        regs->edx = 0x76482074;  /* "t Hv" */
        break;

    default:
        /* Return zeros for other hypervisor leaves */
        regs->eax = regs->ebx = regs->ecx = regs->edx = 0;
        break;
    }

    return 0;
}

/*
 * Usage Notes:
 *
 * 1. Set up VMExit handler to intercept CPUID (reason 10 on Intel, intercept on AMD)
 * 2. Read guest RAX for function, RCX for subfunction
 * 3. Call appropriate handler
 * 4. Write results back to guest RAX, RBX, RCX, RDX
 * 5. Advance guest RIP past CPUID instruction (2 bytes: 0F A2)
 *
 * Critical: Always test with pafish, al-khaser, and VMAware detection tools.
 */
