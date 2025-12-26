// vmcs.c — VMCS Configuration and Setup
// OmbraHypervisor

#include "vmcs.h"
#include "vmx.h"
#include "ept.h"
#include "debug.h"
#include "../shared/vmcs_fields.h"
#include "../shared/msr_defs.h"
#include "../shared/cpu_defs.h"
#include <intrin.h>

// =============================================================================
// VMCS Initialization
// =============================================================================

OMBRA_STATUS VmcsInitialize(VMX_CPU* cpu, HV_INIT_PARAMS* params) {
    U64 vmxBasic;
    U8 error;

    TRACE("VMCS init: CPU %u, VMCS phys=0x%llx", cpu->CpuId, cpu->VmcsPhysical);

    // Get VMX revision ID from MSR
    vmxBasic = __readmsr(MSR_IA32_VMX_BASIC);
    TRACE("VMX Basic MSR: 0x%llx", vmxBasic);

    // Write revision ID to VMCS region (first 31 bits, bit 31 must be 0)
    *(U32*)cpu->VmcsRegion = (U32)(vmxBasic & 0x7FFFFFFF);

    // Clear the VMCS (makes it inactive and marks as clean)
    error = __vmx_vmclear(&cpu->VmcsPhysical);
    if (error) {
        ERR("VMCLEAR failed: error %u", error);
        return OMBRA_ERROR_VMCS_FAILED;
    }

    // Load the VMCS (makes it current/active on this processor)
    error = __vmx_vmptrld(&cpu->VmcsPhysical);
    if (error) {
        ERR("VMPTRLD failed: error %u", error);
        return OMBRA_ERROR_VMCS_FAILED;
    }

    cpu->VmcsLoaded = true;
    TRACE("VMCS loaded successfully");

    // Setup all VMCS fields
    VmcsSetupControls(cpu, params);
    VmcsSetupGuestState(cpu);
    VmcsSetupHostState(cpu);

    INFO("VMCS setup complete for CPU %u", cpu->CpuId);
    return OMBRA_SUCCESS;
}

// =============================================================================
// Control Field Setup
// =============================================================================

// Check if TRUE controls are supported
static bool UseTrueControls(void) {
    U64 vmxBasic = __readmsr(MSR_IA32_VMX_BASIC);
    return (vmxBasic & (1ULL << 55)) != 0;  // Bit 55 indicates TRUE controls
}

void VmcsSetupControls(VMX_CPU* cpu, HV_INIT_PARAMS* params) {
    U32 pinBased, procBased, procBased2, exitCtls, entryCtls;
    U32 pinMsr, procMsr, exitMsr, entryMsr;
    bool useTrueCtls = UseTrueControls();

    // Select MSRs based on TRUE controls support
    if (useTrueCtls) {
        pinMsr = MSR_IA32_VMX_TRUE_PINBASED;
        procMsr = MSR_IA32_VMX_TRUE_PROCBASED;
        exitMsr = MSR_IA32_VMX_TRUE_EXIT;
        entryMsr = MSR_IA32_VMX_TRUE_ENTRY;
    } else {
        pinMsr = MSR_IA32_VMX_PINBASED_CTLS;
        procMsr = MSR_IA32_VMX_PROCBASED_CTLS;
        exitMsr = MSR_IA32_VMX_EXIT_CTLS;
        entryMsr = MSR_IA32_VMX_ENTRY_CTLS;
    }

    // -------------------------------------------------------------------------
    // Pin-Based Controls
    // -------------------------------------------------------------------------
    // Minimal pin controls - just NMI exiting for safety
    pinBased = 0;
    pinBased |= PIN_NMI_EXIT;           // Exit on NMI for proper handling
    pinBased = AdjustControls(pinBased, pinMsr);
    VmcsWrite(VMCS_CTRL_PIN_BASED, pinBased);

    // -------------------------------------------------------------------------
    // Primary Processor-Based Controls
    // -------------------------------------------------------------------------
    procBased = 0;

    // MSR bitmap to avoid exits on every MSR access
    procBased |= CPU_MSR_BITMAP;

    // CR3 exiting for potential EPT shadowing (can disable later for perf)
    // procBased |= CPU_CR3_LOAD_EXIT;
    // procBased |= CPU_CR3_STORE_EXIT;

    // HLT exiting (optional - for debugging)
    // procBased |= CPU_HLT_EXIT;

    // RDTSC exiting for timing stealth
    procBased |= CPU_RDTSC_EXIT;

    // Enable secondary controls
    procBased |= CPU_SECONDARY_CONTROLS;

    procBased = AdjustControls(procBased, procMsr);
    VmcsWrite(VMCS_CTRL_PROC_BASED, procBased);

    // -------------------------------------------------------------------------
    // Secondary Processor-Based Controls
    // -------------------------------------------------------------------------
    procBased2 = 0;

    // EPT - required for our memory virtualization
    procBased2 |= CPU2_EPT;

    // VPID for TLB performance (assign VPID = 1 to guest)
    procBased2 |= CPU2_VPID;

    // Allow RDTSCP instruction
    procBased2 |= CPU2_RDTSCP;

    // Allow INVPCID instruction
    procBased2 |= CPU2_INVPCID;

    // Enable XSAVES/XRSTORS
    procBased2 |= CPU2_XSAVES;

    // Mode-based execute control for EPT (execute-only pages)
    procBased2 |= CPU2_MODE_BASED_EPT;

    // Unrestricted guest mode (allows real mode, useful for some scenarios)
    // procBased2 |= CPU2_UNRESTRICTED_GUEST;

    procBased2 = AdjustControls(procBased2, MSR_IA32_VMX_PROCBASED_CTLS2);
    VmcsWrite(VMCS_CTRL_PROC_BASED2, procBased2);

    // -------------------------------------------------------------------------
    // VM-Exit Controls
    // -------------------------------------------------------------------------
    exitCtls = 0;

    // 64-bit host (required for x64 Windows)
    exitCtls |= EXIT_HOST_ADDR_SPACE_SIZE;

    // Save/load EFER to handle IA-32e mode properly
    exitCtls |= EXIT_SAVE_EFER;
    exitCtls |= EXIT_LOAD_EFER;

    // Save debug controls
    exitCtls |= EXIT_SAVE_DEBUG;

    // Acknowledge interrupt on exit (for external interrupt handling)
    exitCtls |= EXIT_ACK_INT_ON_EXIT;

    exitCtls = AdjustControls(exitCtls, exitMsr);
    VmcsWrite(VMCS_CTRL_VMEXIT, exitCtls);

    // -------------------------------------------------------------------------
    // VM-Entry Controls
    // -------------------------------------------------------------------------
    entryCtls = 0;

    // IA-32e mode guest (required for x64 Windows)
    entryCtls |= ENTRY_IA32E_MODE;

    // Load EFER on entry
    entryCtls |= ENTRY_LOAD_EFER;

    // Load debug controls
    entryCtls |= ENTRY_LOAD_DEBUG;

    entryCtls = AdjustControls(entryCtls, entryMsr);
    VmcsWrite(VMCS_CTRL_VMENTRY, entryCtls);

    // -------------------------------------------------------------------------
    // Exception Bitmap
    // -------------------------------------------------------------------------
    // Bit set = exit on that exception vector
    // For now, intercept #BP (int3) for debugging
    U32 exceptionBitmap = 0;
    exceptionBitmap |= (1 << 3);    // #BP - breakpoint
    // exceptionBitmap |= (1 << 14); // #PF - page fault (for shadow paging)
    VmcsWrite(VMCS_CTRL_EXCEPTION_BITMAP, exceptionBitmap);

    // -------------------------------------------------------------------------
    // CR0/CR4 Guest/Host Masks and Shadows
    // -------------------------------------------------------------------------
    // Mask: 1 = host owns the bit (exit on write), 0 = guest owns
    // Shadow: what guest sees when reading the masked bits

    // For now, let guest control most CR0 bits
    U64 cr0Mask = 0;
    cr0Mask |= CR0_PE;  // Intercept attempts to disable protection
    cr0Mask |= CR0_PG;  // Intercept attempts to disable paging
    VmcsWrite(VMCS_CTRL_CR0_MASK, cr0Mask);
    VmcsWrite(VMCS_CTRL_CR0_SHADOW, __readcr0());

    // For CR4, intercept VMXE changes
    U64 cr4Mask = 0;
    cr4Mask |= CR4_VMXE;  // Never let guest clear VMXE
    VmcsWrite(VMCS_CTRL_CR4_MASK, cr4Mask);
    VmcsWrite(VMCS_CTRL_CR4_SHADOW, __readcr4() & ~CR4_VMXE);  // Hide VMXE from guest

    // -------------------------------------------------------------------------
    // MSR Bitmap
    // -------------------------------------------------------------------------
    // 4KB bitmap: bits determine which MSRs cause exit on RDMSR/WRMSR
    // Layout:
    //   Bytes 0x000-0x3FF: Read bitmap for MSRs 0x00000000-0x00001FFF
    //   Bytes 0x400-0x7FF: Read bitmap for MSRs 0xC0000000-0xC0001FFF
    //   Bytes 0x800-0xBFF: Write bitmap for MSRs 0x00000000-0x00001FFF
    //   Bytes 0xC00-0xFFF: Write bitmap for MSRs 0xC0000000-0xC0001FFF
    if (cpu->MsrBitmap) {
        U8* bitmap = (U8*)cpu->MsrBitmap;

        // Clear the bitmap (no exits by default)
        __stosq((U64*)bitmap, 0, 4096 / 8);

        // =====================================================================
        // STEALTH: Intercept VMX-related MSRs to hide our presence
        // =====================================================================

        // Helper macro to set bit for an MSR in the low range (0x00000000-0x00001FFF)
        #define SET_MSR_READ(msr)  bitmap[(msr) / 8] |= (1 << ((msr) % 8))
        #define SET_MSR_WRITE(msr) bitmap[0x800 + (msr) / 8] |= (1 << ((msr) % 8))

        // IA32_FEATURE_CONTROL (0x3A) - VMX enable/lock status
        SET_MSR_READ(0x3A);
        SET_MSR_WRITE(0x3A);

        // IA32_VMX_BASIC through IA32_VMX_VMFUNC (0x480-0x491)
        // These reveal VMX capabilities - intercept reads
        for (U32 msr = 0x480; msr <= 0x491; msr++) {
            SET_MSR_READ(msr);
        }

        #undef SET_MSR_READ
        #undef SET_MSR_WRITE

        TRACE("MSR bitmap configured for VMX stealth");
        VmcsWrite(VMCS_CTRL_MSR_BITMAP, cpu->MsrBitmapPhysical);
    }

    // -------------------------------------------------------------------------
    // EPT Pointer
    // -------------------------------------------------------------------------
    if (params->EptPml4Physical) {
        U64 eptp = VmcsConstructEptp(params->EptPml4Physical);
        VmcsWrite(VMCS_CTRL_EPT_POINTER, eptp);

        // Store EPT reference in CPU structure
        cpu->Ept = (struct _EPT_STATE*)params->EptPml4Virtual;
    }

    // -------------------------------------------------------------------------
    // VPID
    // -------------------------------------------------------------------------
    // Assign VPID = 1 to all guests (0 is reserved for hypervisor)
    VmcsWrite(VMCS_CTRL_VPID, 1);

    // -------------------------------------------------------------------------
    // TSC Offset
    // -------------------------------------------------------------------------
    VmcsWrite(VMCS_CTRL_TSC_OFFSET, cpu->TscOffset);

    // -------------------------------------------------------------------------
    // Link Pointer (for VMCS shadowing, not used)
    // -------------------------------------------------------------------------
    VmcsWrite(VMCS_GUEST_VMCS_LINK, 0xFFFFFFFFFFFFFFFF);

    // -------------------------------------------------------------------------
    // MSR Load/Store Counts (not using auto MSR load/store)
    // -------------------------------------------------------------------------
    VmcsWrite(VMCS_CTRL_VMEXIT_MSR_STORE_COUNT, 0);
    VmcsWrite(VMCS_CTRL_VMEXIT_MSR_LOAD_COUNT, 0);
    VmcsWrite(VMCS_CTRL_VMENTRY_MSR_LOAD_COUNT, 0);

    // -------------------------------------------------------------------------
    // CR3 Target Values (not using CR3-target exiting)
    // -------------------------------------------------------------------------
    VmcsWrite(VMCS_CTRL_CR3_TARGET_COUNT, 0);

    // -------------------------------------------------------------------------
    // Page Fault Error Code Mask/Match (for selective #PF interception)
    // -------------------------------------------------------------------------
    VmcsWrite(VMCS_CTRL_PAGE_FAULT_MASK, 0);
    VmcsWrite(VMCS_CTRL_PAGE_FAULT_MATCH, 0);
}

// =============================================================================
// Guest State Setup
// =============================================================================

void VmcsSetupGuestState(VMX_CPU* cpu) {
    SEGMENT_STATE seg;
    DESCRIPTOR_TABLE_REG gdtr, idtr;

    // -------------------------------------------------------------------------
    // Control Registers
    // -------------------------------------------------------------------------
    VmcsWrite(VMCS_GUEST_CR0, __readcr0());
    VmcsWrite(VMCS_GUEST_CR3, __readcr3());
    VmcsWrite(VMCS_GUEST_CR4, __readcr4());

    // -------------------------------------------------------------------------
    // Debug Registers
    // -------------------------------------------------------------------------
    VmcsWrite(VMCS_GUEST_DR7, AsmReadDr7());

    // -------------------------------------------------------------------------
    // RSP and RIP - These will be set by the launch trampoline
    // For now, set placeholders (assembly will update before VMLAUNCH)
    // -------------------------------------------------------------------------
    // These are critical - the launch code must update them
    VmcsWrite(VMCS_GUEST_RSP, 0);  // Will be set by AsmVmxLaunch
    VmcsWrite(VMCS_GUEST_RIP, 0);  // Will be set by AsmVmxLaunch

    // -------------------------------------------------------------------------
    // RFLAGS - Clear IF to prevent immediate interrupts on entry
    // -------------------------------------------------------------------------
    U64 rflags = AsmReadRflags();
    VmcsWrite(VMCS_GUEST_RFLAGS, rflags);

    // -------------------------------------------------------------------------
    // Segment Registers
    // -------------------------------------------------------------------------

    // CS
    GetSegmentState(AsmReadCs(), &seg);
    VmcsWrite(VMCS_GUEST_CS_SEL, seg.Selector);
    VmcsWrite(VMCS_GUEST_CS_BASE, seg.Base);
    VmcsWrite(VMCS_GUEST_CS_LIMIT, seg.Limit);
    VmcsWrite(VMCS_GUEST_CS_ACCESS, seg.AccessRights);

    // SS
    GetSegmentState(AsmReadSs(), &seg);
    VmcsWrite(VMCS_GUEST_SS_SEL, seg.Selector);
    VmcsWrite(VMCS_GUEST_SS_BASE, seg.Base);
    VmcsWrite(VMCS_GUEST_SS_LIMIT, seg.Limit);
    VmcsWrite(VMCS_GUEST_SS_ACCESS, seg.AccessRights);

    // DS
    GetSegmentState(AsmReadDs(), &seg);
    VmcsWrite(VMCS_GUEST_DS_SEL, seg.Selector);
    VmcsWrite(VMCS_GUEST_DS_BASE, seg.Base);
    VmcsWrite(VMCS_GUEST_DS_LIMIT, seg.Limit);
    VmcsWrite(VMCS_GUEST_DS_ACCESS, seg.AccessRights);

    // ES
    GetSegmentState(AsmReadEs(), &seg);
    VmcsWrite(VMCS_GUEST_ES_SEL, seg.Selector);
    VmcsWrite(VMCS_GUEST_ES_BASE, seg.Base);
    VmcsWrite(VMCS_GUEST_ES_LIMIT, seg.Limit);
    VmcsWrite(VMCS_GUEST_ES_ACCESS, seg.AccessRights);

    // FS
    GetSegmentState(AsmReadFs(), &seg);
    VmcsWrite(VMCS_GUEST_FS_SEL, seg.Selector);
    // FS base comes from MSR in 64-bit mode
    VmcsWrite(VMCS_GUEST_FS_BASE, __readmsr(MSR_IA32_FS_BASE));
    VmcsWrite(VMCS_GUEST_FS_LIMIT, seg.Limit);
    VmcsWrite(VMCS_GUEST_FS_ACCESS, seg.AccessRights);

    // GS
    GetSegmentState(AsmReadGs(), &seg);
    VmcsWrite(VMCS_GUEST_GS_SEL, seg.Selector);
    // GS base comes from MSR in 64-bit mode
    VmcsWrite(VMCS_GUEST_GS_BASE, __readmsr(MSR_IA32_GS_BASE));
    VmcsWrite(VMCS_GUEST_GS_LIMIT, seg.Limit);
    VmcsWrite(VMCS_GUEST_GS_ACCESS, seg.AccessRights);

    // LDTR
    GetSegmentState(AsmReadLdtr(), &seg);
    VmcsWrite(VMCS_GUEST_LDTR_SEL, seg.Selector);
    VmcsWrite(VMCS_GUEST_LDTR_BASE, seg.Base);
    VmcsWrite(VMCS_GUEST_LDTR_LIMIT, seg.Limit);
    VmcsWrite(VMCS_GUEST_LDTR_ACCESS, seg.AccessRights);

    // TR (Task Register)
    GetSegmentState(AsmReadTr(), &seg);
    VmcsWrite(VMCS_GUEST_TR_SEL, seg.Selector);
    VmcsWrite(VMCS_GUEST_TR_BASE, seg.Base);
    VmcsWrite(VMCS_GUEST_TR_LIMIT, seg.Limit);
    // TR must be busy TSS
    if (seg.AccessRights != SEG_ACCESS_UNUSABLE) {
        seg.AccessRights |= 0x0B;  // Set to busy TSS type
        seg.AccessRights &= ~0x02; // Clear bit 1 to avoid "not busy" type
    }
    VmcsWrite(VMCS_GUEST_TR_ACCESS, seg.AccessRights);

    // -------------------------------------------------------------------------
    // Descriptor Table Registers
    // -------------------------------------------------------------------------
    AsmReadGdtr(&gdtr);
    VmcsWrite(VMCS_GUEST_GDTR_BASE, gdtr.Base);
    VmcsWrite(VMCS_GUEST_GDTR_LIMIT, gdtr.Limit);

    AsmReadIdtr(&idtr);
    VmcsWrite(VMCS_GUEST_IDTR_BASE, idtr.Base);
    VmcsWrite(VMCS_GUEST_IDTR_LIMIT, idtr.Limit);

    // -------------------------------------------------------------------------
    // MSRs
    // -------------------------------------------------------------------------
    VmcsWrite(VMCS_GUEST_DEBUGCTL, __readmsr(0x1D9));  // IA32_DEBUGCTL
    VmcsWrite(VMCS_GUEST_SYSENTER_CS, __readmsr(MSR_IA32_SYSENTER_CS));
    VmcsWrite(VMCS_GUEST_SYSENTER_ESP, __readmsr(MSR_IA32_SYSENTER_ESP));
    VmcsWrite(VMCS_GUEST_SYSENTER_EIP, __readmsr(MSR_IA32_SYSENTER_EIP));
    VmcsWrite(VMCS_GUEST_EFER, __readmsr(MSR_IA32_EFER));

    // -------------------------------------------------------------------------
    // Activity and Interruptibility State
    // -------------------------------------------------------------------------
    VmcsWrite(VMCS_GUEST_ACTIVITY, GUEST_ACTIVITY_ACTIVE);
    VmcsWrite(VMCS_GUEST_INTERRUPTIBILITY, 0);

    // -------------------------------------------------------------------------
    // Pending Debug Exceptions
    // -------------------------------------------------------------------------
    VmcsWrite(VMCS_GUEST_PENDING_DEBUG, 0);

    // -------------------------------------------------------------------------
    // Cache guest state in CPU structure for quick access
    // -------------------------------------------------------------------------
    cpu->GuestRip = 0;  // Will be set by launch trampoline
    cpu->GuestRsp = 0;  // Will be set by launch trampoline
    cpu->GuestRflags = rflags;
}

// =============================================================================
// Host State Setup
// =============================================================================

void VmcsSetupHostState(VMX_CPU* cpu) {
    DESCRIPTOR_TABLE_REG gdtr, idtr;

    // -------------------------------------------------------------------------
    // Control Registers
    // -------------------------------------------------------------------------
    VmcsWrite(VMCS_HOST_CR0, __readcr0());
    VmcsWrite(VMCS_HOST_CR3, __readcr3());
    VmcsWrite(VMCS_HOST_CR4, __readcr4());

    // -------------------------------------------------------------------------
    // Segment Selectors (must have RPL = 0)
    // -------------------------------------------------------------------------
    // In 64-bit mode, only CS and TR are really meaningful
    // Others are loaded but their bases/limits are ignored
    VmcsWrite(VMCS_HOST_CS_SEL, AsmReadCs() & 0xFFF8);  // Clear RPL
    VmcsWrite(VMCS_HOST_SS_SEL, AsmReadSs() & 0xFFF8);
    VmcsWrite(VMCS_HOST_DS_SEL, AsmReadDs() & 0xFFF8);
    VmcsWrite(VMCS_HOST_ES_SEL, AsmReadEs() & 0xFFF8);
    VmcsWrite(VMCS_HOST_FS_SEL, AsmReadFs() & 0xFFF8);
    VmcsWrite(VMCS_HOST_GS_SEL, AsmReadGs() & 0xFFF8);
    VmcsWrite(VMCS_HOST_TR_SEL, AsmReadTr() & 0xFFF8);

    // -------------------------------------------------------------------------
    // FS/GS/TR Base Addresses
    // -------------------------------------------------------------------------
    VmcsWrite(VMCS_HOST_FS_BASE, __readmsr(MSR_IA32_FS_BASE));
    VmcsWrite(VMCS_HOST_GS_BASE, __readmsr(MSR_IA32_GS_BASE));
    VmcsWrite(VMCS_HOST_TR_BASE, GetSegmentBase(AsmReadTr()));

    // -------------------------------------------------------------------------
    // Descriptor Table Registers
    // -------------------------------------------------------------------------
    AsmReadGdtr(&gdtr);
    VmcsWrite(VMCS_HOST_GDTR_BASE, gdtr.Base);

    AsmReadIdtr(&idtr);
    VmcsWrite(VMCS_HOST_IDTR_BASE, idtr.Base);

    // -------------------------------------------------------------------------
    // MSRs
    // -------------------------------------------------------------------------
    VmcsWrite(VMCS_HOST_SYSENTER_CS, __readmsr(MSR_IA32_SYSENTER_CS));
    VmcsWrite(VMCS_HOST_SYSENTER_ESP, __readmsr(MSR_IA32_SYSENTER_ESP));
    VmcsWrite(VMCS_HOST_SYSENTER_EIP, __readmsr(MSR_IA32_SYSENTER_EIP));
    VmcsWrite(VMCS_HOST_EFER, __readmsr(MSR_IA32_EFER));

    // -------------------------------------------------------------------------
    // Host RSP and RIP
    // -------------------------------------------------------------------------
    // RSP: Top of our host stack (the assembly handler will use this)
    VmcsWrite(VMCS_HOST_RSP, (U64)cpu->HostStackTop);

    // RIP: Address of our VM-exit handler
    VmcsWrite(VMCS_HOST_RIP, (U64)VmexitHandler);
}

// =============================================================================
// EPT Pointer Construction
// =============================================================================

U64 VmcsConstructEptp(U64 pml4Physical) {
    U64 eptp = 0;

    // Memory type: Write-back (6)
    // This goes in bits 2:0
    eptp |= 6;

    // Page-walk length minus 1 (4 levels - 1 = 3)
    // This goes in bits 5:3
    eptp |= (3ULL << 3);

    // Access/dirty flags enable (bit 6)
    // Check if supported first
    U64 eptCap = __readmsr(MSR_IA32_VMX_EPT_VPID_CAP);
    if (eptCap & (1ULL << 21)) {  // Bit 21 = AD flags supported
        eptp |= (1ULL << 6);
    }

    // PML4 physical address (must be 4KB aligned, so bits 11:0 are 0)
    eptp |= (pml4Physical & ~0xFFFULL);

    return eptp;
}
