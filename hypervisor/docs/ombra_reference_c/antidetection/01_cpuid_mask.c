/**
 * 01_cpuid_mask.c - CPUID Hypervisor Presence Masking
 * 
 * Pure C implementation of CPUID-based anti-detection.
 * 
 * CONCEPTS COVERED:
 * - Hypervisor presence bit (ECX[31])
 * - VMX capability hiding
 * - Vendor string manipulation
 * - AMD SVM hiding
 */

#include "../common/types.h"
#include "../common/vmcs_defs.h"

/*===========================================================================
 * CONCEPT 1: Detection Vectors via CPUID
 *===========================================================================
 * Anti-cheats and security tools detect hypervisors via:
 * 
 * CPUID.1:ECX[31]     - Hypervisor Present bit (set by all hypervisors)
 * CPUID.1:ECX[5]      - VMX capability (Intel)
 * CPUID.0x40000000    - Hypervisor vendor string
 * CPUID.0x80000001:EDX[2] - SVM capability (AMD)
 * 
 * Evasion strategy depends on context:
 * - Full hiding: Clear all bits, return zeros for 0x40000000
 * - Hyper-V masquerade: Appear as legitimate Hyper-V
 */

/*===========================================================================
 * CONCEPT 2: Cloaking Configuration
 *===========================================================================*/

typedef enum _CPUID_STEALTH_MODE {
    STEALTH_FULL_HIDE,      /* Clear all hypervisor indicators */
    STEALTH_HYPERV_MIMIC,   /* Appear as Microsoft Hyper-V */
    STEALTH_PASSTHROUGH,    /* No modification (debugging) */
} CPUID_STEALTH_MODE;

typedef struct _CPUID_CONFIG {
    CPUID_STEALTH_MODE Mode;
    U32 CustomVendorEbx;    /* Custom vendor string if needed */
    U32 CustomVendorEcx;
    U32 CustomVendorEdx;
} CPUID_CONFIG;

/*===========================================================================
 * CONCEPT 3: CPUID Interception Handler
 *===========================================================================*/

/**
 * Apply CPUID stealth modifications
 * 
 * @param leaf      CPUID leaf (EAX input)
 * @param subleaf   CPUID subleaf (ECX input)
 * @param regs      Output registers [EAX, EBX, ECX, EDX]
 * @param config    Stealth configuration
 */
void CpuidApplyStealth(
    U32 leaf,
    U32 subleaf,
    int regs[4],
    const CPUID_CONFIG* config)
{
    switch (config->Mode) {
    case STEALTH_FULL_HIDE:
        CpuidFullHide(leaf, subleaf, regs);
        break;
        
    case STEALTH_HYPERV_MIMIC:
        CpuidHypervMimic(leaf, subleaf, regs);
        break;
        
    case STEALTH_PASSTHROUGH:
        /* No modification */
        break;
    }
}

/*===========================================================================
 * CONCEPT 4: Full Hiding Implementation
 *===========================================================================*/

/**
 * Full hiding: Remove all hypervisor indicators
 * 
 * Use when:
 * - Targeting software that rejects ANY virtualization
 * - Testing bare-metal detection methods
 * - Maximum stealth required
 */
static void CpuidFullHide(U32 leaf, U32 subleaf, int regs[4])
{
    (void)subleaf;
    
    switch (leaf) {
    case 0x00000001:
        /*
         * Feature Information
         * 
         * ECX[31] - Hypervisor Present
         *   Set by all hypervisors to indicate virtualized environment.
         *   Anti-cheats check this FIRST.
         *   
         * ECX[5] - VMX Capability (Intel)
         *   Indicates CPU supports VMX.
         *   Suspicious if set on consumer hardware without Hyper-V.
         */
        regs[2] &= ~BIT(31);    /* Clear hypervisor present */
        regs[2] &= ~BIT(5);     /* Clear VMX capability */
        break;
        
    case 0x40000000:
        /*
         * Hypervisor Vendor Leaf
         * 
         * If ECX[31] is set, software checks this leaf for vendor string.
         * Common strings: "Microsoft Hv", "KVMKVMKVM", "VMwareVMware"
         * 
         * Return zeros to indicate no hypervisor information.
         */
        regs[0] = 0;
        regs[1] = 0;
        regs[2] = 0;
        regs[3] = 0;
        break;
        
    case 0x40000001:
    case 0x40000002:
    case 0x40000003:
    case 0x40000004:
    case 0x40000005:
    case 0x40000006:
        /*
         * Extended Hypervisor Leaves
         * Return zeros for all.
         */
        regs[0] = 0;
        regs[1] = 0;
        regs[2] = 0;
        regs[3] = 0;
        break;
        
    case 0x80000001:
        /*
         * Extended Feature Information (AMD)
         * 
         * EDX[2] - SVM Capability
         *   AMD's virtualization extension.
         *   Clear to hide SVM support.
         */
        regs[3] &= ~BIT(2);     /* Clear SVM capability */
        break;
    }
}

/*===========================================================================
 * CONCEPT 5: Hyper-V Mimicry
 *===========================================================================*/

/**
 * Hyper-V mimicry: Appear as legitimate Microsoft Hyper-V
 * 
 * Use when:
 * - Hijacking existing Hyper-V installation
 * - Running on systems where Hyper-V is expected
 * - Software whitelists Hyper-V but blocks other hypervisors
 */
static void CpuidHypervMimic(U32 leaf, U32 subleaf, int regs[4])
{
    (void)subleaf;
    
    switch (leaf) {
    case 0x00000001:
        /*
         * Keep hypervisor present bit SET.
         * We're pretending to be Hyper-V, which is legitimate.
         * Still hide VMX capability.
         */
        regs[2] |= BIT(31);     /* Hypervisor present (intentional) */
        regs[2] &= ~BIT(5);     /* Hide VMX */
        break;
        
    case 0x40000000:
        /*
         * Return Hyper-V vendor string: "Microsoft Hv"
         * 
         * EAX = Maximum hypervisor leaf (0x40000006 for Hyper-V)
         * EBX = 'Micr' = 0x7263694D
         * ECX = 'osof' = 0x666F736F
         * EDX = 't Hv' = 0x76482074
         */
        regs[0] = 0x40000006;   /* Max leaf */
        regs[1] = 0x7263694D;   /* "Micr" */
        regs[2] = 0x666F736F;   /* "osof" */
        regs[3] = 0x76482074;   /* "t Hv" */
        break;
        
    case 0x40000001:
        /*
         * Hypervisor Interface Identification
         * Return "Hv#1" signature for Hyper-V interface.
         */
        regs[0] = 0x31237648;   /* "Hv#1" */
        regs[1] = 0;
        regs[2] = 0;
        regs[3] = 0;
        break;
        
    case 0x40000002:
        /*
         * Hyper-V System Identity
         * Return build number, version info.
         */
        regs[0] = 0x00003839;   /* Build number */
        regs[1] = 0x000A0000;   /* Version 10.0 */
        regs[2] = 0;
        regs[3] = 0;
        break;
        
    case 0x40000003:
        /*
         * Hyper-V Feature Identification
         * Minimal features to avoid detection.
         */
        regs[0] = 0x00000001;   /* Minimal features */
        regs[1] = 0;
        regs[2] = 0;
        regs[3] = 0;
        break;
        
    case 0x40000004:
    case 0x40000005:
    case 0x40000006:
        regs[0] = 0;
        regs[1] = 0;
        regs[2] = 0;
        regs[3] = 0;
        break;
    }
}

/*===========================================================================
 * CONCEPT 6: Complete VMExit CPUID Handler
 *===========================================================================*/

/**
 * Handle CPUID VMExit with stealth
 * 
 * @param regs      Guest register context
 * @param config    Stealth configuration
 */
void HandleCpuidStealth(GUEST_REGS* regs, const CPUID_CONFIG* config)
{
    int cpuInfo[4] = {0};
    U32 leaf = (U32)regs->Rax;
    U32 subleaf = (U32)regs->Rcx;
    
    /* Execute real CPUID */
    __cpuidex(cpuInfo, (int)leaf, (int)subleaf);
    
    /* Apply stealth modifications */
    CpuidApplyStealth(leaf, subleaf, cpuInfo, config);
    
    /* Return modified results to guest */
    regs->Rax = (U64)(U32)cpuInfo[0];
    regs->Rbx = (U64)(U32)cpuInfo[1];
    regs->Rcx = (U64)(U32)cpuInfo[2];
    regs->Rdx = (U64)(U32)cpuInfo[3];
}

/*===========================================================================
 * CONCEPT 7: Detection Test Functions
 *===========================================================================
 * These show what anti-cheats look for (use for testing).
 */

/**
 * Check if hypervisor is present (what anti-cheats do)
 */
bool DetectHypervisorPresent(void)
{
    int regs[4];
    __cpuid(regs, 1);
    return (regs[2] & BIT(31)) != 0;
}

/**
 * Get hypervisor vendor string
 */
void DetectHypervisorVendor(char vendor[13])
{
    int regs[4];
    __cpuid(regs, 0x40000000);
    
    *(int*)&vendor[0] = regs[1];
    *(int*)&vendor[4] = regs[2];
    *(int*)&vendor[8] = regs[3];
    vendor[12] = '\0';
}

/**
 * Check for VMX capability
 */
bool DetectVmxCapability(void)
{
    int regs[4];
    __cpuid(regs, 1);
    return (regs[2] & BIT(5)) != 0;
}
