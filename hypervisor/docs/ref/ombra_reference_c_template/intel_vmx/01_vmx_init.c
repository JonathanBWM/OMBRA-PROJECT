/**
 * 01_vmx_init.c - Intel VMX Initialization Reference
 * 
 * Pure C implementation of VMX initialization sequence.
 * Reference: Intel SDM Volume 3, Chapter 23-24
 * 
 * CONCEPTS COVERED:
 * - VMX capability detection
 * - CR0/CR4 fixed bit application
 * - VMXON region setup
 * - VMXON execution
 */

#include "../common/types.h"
#include "../common/vmcs_defs.h"
#include "../common/msr_defs.h"

/*===========================================================================
 * CONCEPT 1: VMX Support Detection
 *===========================================================================
 * Before using VMX, we must verify:
 * 1. CPU supports VMX (CPUID.1:ECX[5])
 * 2. VMX is enabled in BIOS (IA32_FEATURE_CONTROL)
 * 3. Required features are available
 */

typedef struct _VMX_CAPS {
    /* From IA32_VMX_BASIC */
    U32 VmcsRevisionId;
    U32 VmcsSize;
    bool TrueControlsSupported;
    U8  MemoryType;
    
    /* From IA32_VMX_EPT_VPID_CAP */
    bool EptSupported;
    bool Ept1GbPages;
    bool Ept2MbPages;
    bool EptExecuteOnly;
    bool InveptSupported;
    bool VpidSupported;
    
} VMX_CAPS;

/**
 * Check if CPU and BIOS support VMX
 */
static bool VmxCheckSupport(void)
{
    int regs[4];
    U64 featureControl;
    
    /* Check CPUID.1:ECX[5] - VMX support bit */
    __cpuid(regs, 1);
    if (!(regs[2] & BIT(5))) {
        /* VMX not supported by CPU */
        return false;
    }
    
    /* Check IA32_FEATURE_CONTROL MSR */
    featureControl = __readmsr(MSR_IA32_FEATURE_CONTROL);
    
    /* Bit 0: Lock bit - MUST be set by BIOS */
    if (!(featureControl & FEATURE_CONTROL_LOCK)) {
        /* MSR not locked - BIOS didn't configure VMX properly */
        /* Note: We could try to set it ourselves before it's locked */
        return false;
    }
    
    /* Bit 2: Enable VMX outside SMX (required for normal operation) */
    if (!(featureControl & FEATURE_CONTROL_VMX_OUTSIDE_SMX)) {
        /* VMX disabled by BIOS */
        return false;
    }
    
    return true;
}

/**
 * Read VMX capabilities from MSRs
 */
static void VmxReadCapabilities(VMX_CAPS* caps)
{
    U64 basic, eptVpid;
    
    /* Read IA32_VMX_BASIC */
    basic = __readmsr(MSR_IA32_VMX_BASIC);
    
    caps->VmcsRevisionId = (U32)(basic & VMX_BASIC_REVISION_MASK);
    caps->VmcsSize = (U32)((basic >> VMX_BASIC_VMCS_SIZE_SHIFT) & VMX_BASIC_VMCS_SIZE_MASK);
    caps->TrueControlsSupported = (basic & VMX_BASIC_TRUE_CTLS) != 0;
    caps->MemoryType = (U8)((basic >> VMX_BASIC_MEMORY_TYPE_SHIFT) & VMX_BASIC_MEMORY_TYPE_MASK);
    
    /* Read IA32_VMX_EPT_VPID_CAP */
    eptVpid = __readmsr(MSR_IA32_VMX_EPT_VPID_CAP);
    
    caps->EptSupported = (eptVpid & (EPT_CAP_PAGE_WALK_4 | EPT_CAP_WB_MEMORY)) != 0;
    caps->Ept1GbPages = (eptVpid & EPT_CAP_1GB_PAGE) != 0;
    caps->Ept2MbPages = (eptVpid & EPT_CAP_2MB_PAGE) != 0;
    caps->EptExecuteOnly = (eptVpid & EPT_CAP_EXEC_ONLY) != 0;
    caps->InveptSupported = (eptVpid & EPT_CAP_INVEPT) != 0;
    caps->VpidSupported = (eptVpid & VPID_CAP_INVVPID) != 0;
}

/*===========================================================================
 * CONCEPT 2: CR0/CR4 Fixed Bit Application
 *===========================================================================
 * Intel mandates certain CR0/CR4 bits be set or clear for VMX operation.
 * These requirements are encoded in MSRs:
 * - IA32_VMX_CR0_FIXED0: bits that must be 1
 * - IA32_VMX_CR0_FIXED1: bits that must be 0 (inverted)
 * Same pattern for CR4.
 */

/**
 * Apply CR0 fixed bits required for VMX
 */
static void VmxApplyCr0FixedBits(void)
{
    U64 cr0, fixed0, fixed1;
    
    cr0 = __readcr0();
    fixed0 = __readmsr(MSR_IA32_VMX_CR0_FIXED0);  /* Must be 1 */
    fixed1 = __readmsr(MSR_IA32_VMX_CR0_FIXED1);  /* Can be 1 (inverted: must be 0) */
    
    /* Set bits that must be 1 */
    cr0 |= fixed0;
    
    /* Clear bits that must be 0 */
    cr0 &= fixed1;
    
    __writecr0(cr0);
}

/**
 * Apply CR4 fixed bits required for VMX
 * NOTE: CR4.VMXE must be set BEFORE VMXON
 */
static void VmxApplyCr4FixedBits(void)
{
    U64 cr4, fixed0, fixed1;
    
    cr4 = __readcr4();
    fixed0 = __readmsr(MSR_IA32_VMX_CR4_FIXED0);  /* Must be 1 */
    fixed1 = __readmsr(MSR_IA32_VMX_CR4_FIXED1);  /* Can be 1 */
    
    /* Set bits that must be 1 */
    cr4 |= fixed0;
    
    /* Clear bits that must be 0 */
    cr4 &= fixed1;
    
    /* CR4.VMXE MUST be set before VMXON */
    cr4 |= CR4_VMXE;
    
    __writecr4(cr4);
}

/*===========================================================================
 * CONCEPT 3: VMXON Region Setup
 *===========================================================================
 * VMXON region is a 4KB page that must:
 * 1. Be 4KB aligned
 * 2. Have VMCS revision ID in first 4 bytes (bit 31 = 0)
 * 3. Be in write-back cacheable memory
 * 4. Use physical address for VMXON instruction
 */

/**
 * Initialize VMXON region
 * 
 * @param vmxonVirtual   Virtual address of 4KB aligned region
 * @param vmxonPhysical  Physical address of the region
 * @param revisionId     VMCS revision ID from IA32_VMX_BASIC
 */
static void VmxInitVmxonRegion(void* vmxonVirtual, U64 vmxonPhysical, U32 revisionId)
{
    U8* region = (U8*)vmxonVirtual;
    U32 i;
    
    /* Zero the entire 4KB region */
    for (i = 0; i < PAGE_SIZE; i++) {
        region[i] = 0;
    }
    
    /* Write revision ID to first 4 bytes (bit 31 must be 0) */
    *(U32*)vmxonVirtual = revisionId & 0x7FFFFFFF;
    
    (void)vmxonPhysical; /* Used by caller for VMXON instruction */
}

/*===========================================================================
 * CONCEPT 4: Execute VMXON
 *===========================================================================
 * VMXON puts the processor in VMX root operation.
 * - Requires CR4.VMXE = 1
 * - Requires VMXON region physical address as operand
 * - Returns success/failure via RFLAGS
 */

/* Declared in asm_stubs/vmx_ops.asm */
extern VMX_RESULT AsmVmxon(U64 vmxonPhysical);

/**
 * Enter VMX operation
 * 
 * @param vmxonPhysical  Physical address of initialized VMXON region
 * @return OMBRA_SUCCESS or error
 */
static OMBRA_STATUS VmxEnterVmxOperation(U64 vmxonPhysical)
{
    VMX_RESULT result;
    
    result = AsmVmxon(vmxonPhysical);
    
    if (result == VMX_OK) {
        return OMBRA_SUCCESS;
    } else if (result == VMX_FAIL_INVALID) {
        /* Invalid operand or VMX not properly configured */
        return OMBRA_ERROR_VMXON_FAILED;
    } else {
        /* VMX_FAIL_VALID - check VM-instruction error field */
        return OMBRA_ERROR_VMXON_FAILED;
    }
}

/*===========================================================================
 * CONCEPT 5: Complete Initialization Sequence
 *===========================================================================
 * Full initialization for one CPU:
 * 1. Check VMX support
 * 2. Read capabilities
 * 3. Apply CR0/CR4 fixed bits
 * 4. Initialize VMXON region
 * 5. Execute VMXON
 * 6. (Continue to VMCS setup in 02_vmcs_setup.c)
 */

/**
 * Initialize VMX on current CPU
 * 
 * @param cpu   Per-CPU state structure to populate
 * @return OMBRA_SUCCESS or error
 */
OMBRA_STATUS VmxInitializeCpu(VMX_CPU* cpu)
{
    VMX_CAPS caps;
    OMBRA_STATUS status;
    
    /* Step 1: Check support */
    if (!VmxCheckSupport()) {
        return OMBRA_ERROR_VMX_DISABLED;
    }
    
    /* Step 2: Read capabilities */
    VmxReadCapabilities(&caps);
    
    /* Verify EPT support (required for our use case) */
    if (!caps.EptSupported) {
        return OMBRA_ERROR_NOT_SUPPORTED;
    }
    
    /* Step 3: Apply CR fixed bits */
    VmxApplyCr0FixedBits();
    VmxApplyCr4FixedBits();
    
    /* Step 4: Initialize VMXON region */
    /* NOTE: Caller must have allocated VmxonRegion and VmxonPhysical */
    if (!cpu->VmxonRegion || cpu->VmxonPhysical == 0) {
        return OMBRA_ERROR_INVALID_PARAM;
    }
    
    VmxInitVmxonRegion(cpu->VmxonRegion, cpu->VmxonPhysical, caps.VmcsRevisionId);
    
    /* Step 5: Execute VMXON */
    status = VmxEnterVmxOperation(cpu->VmxonPhysical);
    if (OMBRA_FAILED(status)) {
        return status;
    }
    
    cpu->VmxEnabled = true;
    
    /* Continue to VMCS setup (see 02_vmcs_setup.c) */
    return OMBRA_SUCCESS;
}

/*===========================================================================
 * CONCEPT 6: VMX Shutdown
 *===========================================================================
 * To exit VMX operation:
 * 1. Execute VMXOFF on each CPU
 * 2. Clear CR4.VMXE
 * 
 * IMPORTANT: Each CPU must execute VMXOFF on itself!
 */

/* Declared in asm_stubs/vmx_ops.asm */
extern VMX_RESULT AsmVmxoff(void);

/**
 * Exit VMX operation on current CPU
 * 
 * @param cpu   Per-CPU state
 */
void VmxShutdownCpu(VMX_CPU* cpu)
{
    U64 cr4;
    
    if (!cpu->VmxEnabled) {
        return;
    }
    
    /* Execute VMXOFF */
    AsmVmxoff();
    
    /* Clear CR4.VMXE */
    cr4 = __readcr4();
    cr4 &= ~CR4_VMXE;
    __writecr4(cr4);
    
    cpu->VmxEnabled = false;
    cpu->VmcsActive = false;
    cpu->Launched = false;
}
