#include "VMCSCheck.h"

CR0 AdjustCr0(CR0 Cr0) {
    CR0 newCr0, fixed0Cr0, fixed1Cr0;

    newCr0 = Cr0;
    fixed0Cr0.Flags = __readmsr(IA32_VMX_CR0_FIXED0);
    fixed1Cr0.Flags = __readmsr(IA32_VMX_CR0_FIXED1);
    newCr0.Flags &= fixed1Cr0.Flags;
    newCr0.Flags |= fixed0Cr0.Flags;
    return newCr0;
}

CR4 AdjustCr4(CR4 Cr4) {
    CR4 newCr4, fixed0Cr4, fixed1Cr4;

    newCr4 = Cr4;
    fixed0Cr4.Flags = __readmsr(IA32_VMX_CR4_FIXED0);
    fixed1Cr4.Flags = __readmsr(IA32_VMX_CR4_FIXED1);
    newCr4.Flags &= fixed1Cr4.Flags;
    newCr4.Flags |= fixed0Cr4.Flags;
    return newCr4;
}

CR0 AdjustGuestCr0(CR0 Cr0) {
    CR0 newCr0;
    IA32_VMX_PROCBASED_CTLS2_REGISTER secondaryProcBasedControls;

    newCr0 = AdjustCr0(Cr0);

    //
    // When the UnrestrictedGuest bit is set, ProtectionEnable and PagingEnable
    // bits are allowed to be zero. Make this adjustment, by setting them 1 only
    // when the guest did indeed requested them to be 1 (ie,
    // Cr0.ProtectionEnable == 1) and the FIXED0 MSR indicated them to be 1 (ie,
    // newCr0.ProtectionEnable == 1).
    //
    __vmx_vmread(VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, &secondaryProcBasedControls.Flags);
    if (secondaryProcBasedControls.UnrestrictedGuest != FALSE)
    {
        newCr0.ProtectionEnable &= Cr0.ProtectionEnable;
        newCr0.PagingEnable &= Cr0.PagingEnable;
    }
    return newCr0;
}

CR4 AdjustGuestCr4(CR4 Cr4) {
    return AdjustCr4(Cr4);
}

BOOLEAN IsValidGuestPat(UINT64 Pat) {
    return ((Pat == MEMORY_TYPE_UNCACHEABLE) ||
        (Pat == MEMORY_TYPE_WRITE_COMBINING) ||
        (Pat == MEMORY_TYPE_WRITE_THROUGH) ||
        (Pat == MEMORY_TYPE_WRITE_PROTECTED) ||
        (Pat == MEMORY_TYPE_WRITE_BACK) ||
        (Pat == MEMORY_TYPE_UNCACHEABLE_MINUS));
}

void ValidateSegmentAccessRightsHelper(
    SEGMENT_TYPE SegmentType,
    UINT32 AccessRightsAsUInt32,
    UINT32 segmentLimit,
    UINT16 SegmentSelectorAsUInt16,
    BOOLEAN Ia32EModeGuest,
    BOOLEAN UnrestrictedGuest) {

    SEGMENT_SELECTOR selector;
    VMX_SEGMENT_ACCESS_RIGHTS accessRights;
    VMX_SEGMENT_ACCESS_RIGHTS accessRightsSs;
    VMX_SEGMENT_ACCESS_RIGHTS accessRightsCs;
    CR0 cr0;

    selector.Flags = SegmentSelectorAsUInt16;
    accessRights.Flags = AccessRightsAsUInt32;
    __vmx_vmread(VMCS_GUEST_SS_ACCESS_RIGHTS, (size_t*)&accessRightsSs.Flags);
    __vmx_vmread(VMCS_GUEST_CS_ACCESS_RIGHTS, (size_t*)&accessRightsCs.Flags);
    __vmx_vmread(VMCS_GUEST_CR0, &cr0.Flags);

    //
    // Bits 3:0 (Type)
    //
    switch (SegmentType)
    {
    case SegmentCs:
        if (UnrestrictedGuest == FALSE)
        {
            ASSERT((accessRights.Type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_ONLY_ACCESSED) ||
                (accessRights.Type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_READ_ACCESSED) ||
                (accessRights.Type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_ONLY_CONFORMING_ACCESSED) ||
                (accessRights.Type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_READ_CONFORMING_ACCESSED));
        }
        else
        {
            ASSERT((accessRights.Type == SEGMENT_DESCRIPTOR_TYPE_DATA_READ_WRITE_ACCESSED) ||
                (accessRights.Type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_ONLY_ACCESSED) ||
                (accessRights.Type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_READ_ACCESSED) ||
                (accessRights.Type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_ONLY_CONFORMING_ACCESSED) ||
                (accessRights.Type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_READ_CONFORMING_ACCESSED));
        }
        break;

    case SegmentSs:
        if (accessRights.Unusable == 0)
        {
            ASSERT((accessRights.Type == SEGMENT_DESCRIPTOR_TYPE_DATA_READ_WRITE_ACCESSED) ||
                (accessRights.Type == SEGMENT_DESCRIPTOR_TYPE_DATA_READ_WRITE_EXPAND_DOWN_ACCESSED));
        }
        break;

    default:
        if (accessRights.Unusable == 0)
        {
            ASSERT(IS_FLAG_SET(accessRights.Type, (1 << 0) /* accessed */));
            if (IS_FLAG_SET(accessRights.Type, (1 << 3) /* code segment */))
            {
                ASSERT(IS_FLAG_SET(accessRights.Type, (1 << 1) /* readable */));
            }
        }
        break;
    }

    //
    // Bit 4 (S)
    //
    if ((SegmentType == SegmentCs) ||
        (accessRights.Unusable == 0))
    {
        ASSERT(accessRights.DescriptorType == 1);
    }

    //
    // Bits 6:5 (DPL)
    //
    switch (SegmentType)
    {
    case SegmentCs:
        switch (accessRights.Type)
        {
        case SEGMENT_DESCRIPTOR_TYPE_DATA_READ_WRITE_ACCESSED:
            ASSERT(accessRights.DescriptorPrivilegeLevel == 0);
            break;
        case SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_ONLY_ACCESSED:
        case SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_READ_ACCESSED:
            ASSERT(accessRights.DescriptorPrivilegeLevel == accessRightsSs.DescriptorPrivilegeLevel);
            break;
        case SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_ONLY_CONFORMING_ACCESSED:
        case SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_READ_CONFORMING_ACCESSED:
            ASSERT(accessRights.DescriptorPrivilegeLevel <= accessRightsSs.DescriptorPrivilegeLevel);
            break;
        default:
            ASSERT(FALSE);
        }
        break;

    case SegmentSs:
        if (UnrestrictedGuest == FALSE)
        {
            ASSERT(accessRights.DescriptorPrivilegeLevel == selector.RequestPrivilegeLevel);
        }
        if ((accessRightsCs.Type == SEGMENT_DESCRIPTOR_TYPE_DATA_READ_WRITE_ACCESSED) ||
            (cr0.ProtectionEnable == 1))
        {
            ASSERT(accessRights.DescriptorPrivilegeLevel == 0);
        }
        break;

    default:
        if ((UnrestrictedGuest == FALSE) &&
            (accessRights.Unusable == 0) &&
            (/*(accessRights.Type >= 0) &&*/
                (accessRights.Type <= 11)))
        {
            ASSERT(accessRights.DescriptorPrivilegeLevel >= selector.RequestPrivilegeLevel);
        }
        break;
    }

    //
    // Bit 7 (P)
    //
    if ((SegmentType == SegmentCs) ||
        (accessRights.Unusable == 0))
    {
        ASSERT(accessRights.Present == 1);
    }

    //
    // Bits 11:8 (reserved) and bits 31:17 (reserved)
    //
    if ((SegmentType == SegmentCs) ||
        (accessRights.Unusable == 0))
    {
        ASSERT(accessRights.Reserved1 == 0);
        ASSERT(accessRights.Reserved2 == 0);
    }

    //
    // Bit 14 (D/B)
    //
    if (SegmentType == SegmentCs)
    {
        if ((Ia32EModeGuest != FALSE) &&
            (accessRights.LongMode == 1))
        {
            ASSERT(accessRights.DefaultBig == 0);
        }
    }

    //
    // Bit 15 (G)
    //
    if ((SegmentType == SegmentCs) ||
        (accessRights.Unusable == 0))
    {
        if (!IS_FLAG_SET(segmentLimit, 0xfff))
        {
            ASSERT(accessRights.Granularity == 0);
        }
        if (IS_FLAG_SET(segmentLimit, 0xfff00000))
        {
            ASSERT(accessRights.Granularity == 1);
        }
    }
}

DWORD32 VmRead32(DWORD32 dwVal) {
    size_t tmp;
    __vmx_vmread(dwVal, &tmp);
    return tmp;
}

void Checks::CheckGuestVmcsFieldsForVmEntry()
{
    VMENTRY_INTERRUPT_INFORMATION interruptInfo;
    IA32_VMX_ENTRY_CTLS_REGISTER vmEntryControls;
    IA32_VMX_PINBASED_CTLS_REGISTER pinBasedControls;
    IA32_VMX_PROCBASED_CTLS_REGISTER primaryProcBasedControls;
    IA32_VMX_PROCBASED_CTLS2_REGISTER secondaryProcBasedControls;
    RFLAGS rflags;
    BOOLEAN unrestrictedGuest;
    BOOLEAN bResult = 0;

    bResult |= __vmx_vmread(VMCS_GUEST_RFLAGS, &rflags.Flags);

    bResult |= __vmx_vmread(VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD, (size_t*)&interruptInfo.Flags);
    bResult |= __vmx_vmread(VMCS_CTRL_VMENTRY_CONTROLS, &vmEntryControls.Flags);
    bResult |= __vmx_vmread(VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS, &pinBasedControls.Flags);
    bResult |= __vmx_vmread(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, &primaryProcBasedControls.Flags);
    bResult |= __vmx_vmread(VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, &secondaryProcBasedControls.Flags);

    unrestrictedGuest = ((primaryProcBasedControls.ActivateSecondaryControls == 1) &&
        (secondaryProcBasedControls.UnrestrictedGuest == 1));

    //
    // 26.3.1.1 Checks on Guest Control Registers, Debug Registers, and MSRs
    //
    CR0 cr0;
    CR4 cr4;
    IA32_DEBUGCTL_REGISTER debugControl;

    bResult |= __vmx_vmread(VMCS_GUEST_CR0, &cr0.Flags);
    bResult |= __vmx_vmread(VMCS_GUEST_CR4, &cr4.Flags);

    ASSERT(cr0.Flags == AdjustGuestCr0(cr0).Flags);
    if ((cr0.PagingEnable == 1) &&
        (unrestrictedGuest == FALSE))
    {
        ASSERT(cr0.ProtectionEnable == 1);
    }
    ASSERT(cr4.Flags == AdjustGuestCr4(cr4).Flags);

    //
    // If bit 23 in the CR4 field (corresponding to CET) is 1, bit 16 in the
    // CR0 field (WP) must also be 1.
    //

    if (vmEntryControls.LoadDebugControls == 1)
    {
        bResult |= __vmx_vmread(VMCS_GUEST_DEBUGCTL, &debugControl.Flags);
        ASSERT(debugControl.Reserved1 == 0);
        ASSERT(debugControl.Reserved2 == 0);
    }
    if (vmEntryControls.Ia32EModeGuest == 1)
    {
        ASSERT(cr0.PagingEnable == 1);
        ASSERT(cr4.PhysicalAddressExtension == 1);
    }
    if (vmEntryControls.LoadDebugControls == 1)
    {
        DR7 dr7;

        bResult |= __vmx_vmread(VMCS_GUEST_DR7, &dr7.Flags);
        ASSERT(dr7.Reserved4 == 0);
    }
    //
    // The IA32_SYSENTER_ESP field and the IA32_SYSENTER_EIP field must each
