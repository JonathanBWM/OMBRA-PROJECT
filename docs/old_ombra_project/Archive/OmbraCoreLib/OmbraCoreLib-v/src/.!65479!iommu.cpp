#include "iommu.h"

/**
 * @brief Collects relevant information of each DMA-remapping hardware units.
 */
EFI_STATUS
ProcessDmarTable(
    IN EFI_ACPI_DMAR_HEADER* DmarTable,
    IN OUT DMAR_UNIT_INFORMATION* DmarUnits,
    IN UINT64 MaxDmarUnitCount,
    OUT UINT64* DetectedUnitCount
)
{
    if (!MmIsAddressValid(DmarTable)) {
        DbgMsg("[VT-d] DMAR table ptr is invalid: %p", DmarTable);
        return STATUS_NOT_MAPPED_DATA;
    }
    UINT64 endOfDmar;
    EFI_ACPI_DMAR_STRUCTURE_HEADER* dmarHeader;
    UINT64 discoveredUnitCount;

    RtlZeroMemory(DmarUnits, sizeof(*DmarUnits) * MaxDmarUnitCount);

    //
    // Walk through the DMAR table, find all DMA-remapping hardware unit
    // definition structures in it, and gather relevant information into DmarUnits.
    //
    discoveredUnitCount = 0;
    endOfDmar = (UINT64)Add2Ptr(DmarTable, DmarTable->Header.Length);
    dmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER*)(DmarTable + 1);
    while (dmarHeader && (UINT64)dmarHeader < endOfDmar)
    {
        if (dmarHeader->Type == EFI_ACPI_DMAR_TYPE_DRHD)
        {
            if (discoveredUnitCount < MaxDmarUnitCount)
            {
                EFI_ACPI_DMAR_DRHD_HEADER* dmarUnit;

                dmarUnit = (EFI_ACPI_DMAR_DRHD_HEADER*)dmarHeader;
                DmarUnits[discoveredUnitCount].RegisterBasePa = dmarUnit->RegisterBaseAddress;

                DmarUnits[discoveredUnitCount].Capability.Uint64 =
                    CPU::MmIoRead<DWORD64>(DmarUnits[discoveredUnitCount].RegisterBasePa + R_CAP_REG);
                DmarUnits[discoveredUnitCount].ExtendedCapability.Uint64 =
                    CPU::MmIoRead<DWORD64>(DmarUnits[discoveredUnitCount].RegisterBasePa + R_ECAP_REG);

                dmarUnit->RegisterBaseAddress = 0;
            }
            else {
                DbgMsg("[VT-d] Already found max amount of ACPI tables: 0x%x", MaxDmarUnitCount);
                break;
            }
            discoveredUnitCount++;
        }
        dmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER*)Add2Ptr(dmarHeader, dmarHeader->Length);
    }

    //
    // Processed all structures. It is an error if nothing found, or found too many.
    //
    *DetectedUnitCount = discoveredUnitCount;

    for (UINT64 i = 0; i < discoveredUnitCount; ++i)
    {
        DbgMsg("[VT-d] Unit %d at %p - Cap: %llx, ExCap: %llx",
            i,
            DmarUnits[i].RegisterBasePa,
            DmarUnits[i].Capability.Uint64,
            DmarUnits[i].ExtendedCapability.Uint64);
    }
    if (discoveredUnitCount == 0)
    {
        DbgMsg("[VT-d] No DMA remapping hardware unit found");
        return STATUS_UNSUCCESSFUL;
    }
    if (discoveredUnitCount > MaxDmarUnitCount)
    {
        DbgMsg("[VT-d] Too many DMA remapping hardware units found (%llu)",
            discoveredUnitCount);
        return STATUS_RESOURCE_NOT_OWNED;
    }
    return STATUS_SUCCESS;
}

BOOLEAN b2MBCompatible = FALSE;

/**
 * @brief Tests whether all hardware units are compatible with this project.
 */
BOOLEAN
bAllDmaRemappingUnitsCompatible(
    IN DMAR_UNIT_INFORMATION* DmarUnits,
    IN UINT64 DmarUnitsCount
)
{
    for (UINT64 i = 0; i < DmarUnitsCount; ++i)
    {
        //
        // This project does not handle 3-level page-table for simplicity.
        //
        if ((DmarUnits[i].Capability.Bits.SAGAW & BIT2) == 0)
        {
            DbgMsg(
                "[VT-d] Unit %lld does not support 48-bit AGAW (4-level page-table) : %016llx",
                i,
                DmarUnits[i].Capability.Uint64);
            return FALSE;
        }

        //
        // This project requires 2MB large pages for simple second-level table
        // implementation.
        //
        if ((DmarUnits[i].Capability.Bits.SLLPS & BIT0) == 0)
        {
            DbgMsg(
                "[VT-d] Unit %lld does not support 2MB second level large pages : %016llx",
                i,
                DmarUnits[i].Capability.Uint64);
            b2MBCompatible = false;
        }

        //
        // Earlier implementation of DMA-remapping required explicit write buffer
        // flushing. The author have not encounter with such implementation. As
        // such, this project does not support it. See 6.8 Write Buffer Flushing.
        //
        if (DmarUnits[i].Capability.Bits.RWBF != FALSE)
        {
            DbgMsg(
                "[VT-d] Unit %lld requires explicit write buffer flushing : %016llx",
                i,
                DmarUnits[i].Capability.Uint64);
            //return FALSE;
        }

        if ((CPU::MmIoRead<DWORD64>(DmarUnits[i].RegisterBasePa + R_GSTS_REG) & B_GSTS_REG_TE) != 0)
        {
            DbgMsg(
                "[VT-d] Unit %lld already enabled DMA remapping : %016llx",
                i,
                CPU::MmIoRead<DWORD64>(DmarUnits[i].RegisterBasePa + R_GSTS_REG));
            DmarUnits[i].bEnabled = true;
        }

        //
        // Looks good. Dump physical address of where translation fault logs are saved.
        //
        DbgMsg("[VT-d] Fault-recording register at %p",
            DmarUnits[i].RegisterBasePa + (UINT64)DmarUnits[i].Capability.Bits.FRO * 16);
    }
    return TRUE;
}

/**
 * @brief Builds identity mapping for all PCI devices, up to 512GB.
 */
VOID
BuildPassthroughTranslations(
    OUT DMAR_TRANSLATIONS* Translations
)
{
    VTD_ROOT_ENTRY defaultRootValue;
    VTD_CONTEXT_ENTRY defaultContextValue;
    VTD_SECOND_LEVEL_PAGING_ENTRY* pdpt;
    VTD_SECOND_LEVEL_PAGING_ENTRY* pd;
    VTD_SECOND_LEVEL_PAGING_ENTRY* pml4e;
    VTD_SECOND_LEVEL_PAGING_ENTRY* pdpte;
    VTD_SECOND_LEVEL_PAGING_ENTRY* pde;
    UINT64 pml4Index;
    UINT64 destinationPa;

    ASSERT(((UINT64)Translations % PAGE_SIZE) == 0);

    RtlZeroMemory(Translations, sizeof(*Translations));

    void* pSubstitutePage = cpp::kMalloc(PAGE_SIZE, PAGE_READWRITE);
    RtlCopyMemory(pSubstitutePage, Translations->RootTable, PAGE_SIZE);

    //
    // Fill out the root table. All root entries point to the same context table.
    //
    defaultRootValue.Uint128.Uint64Hi = defaultRootValue.Uint128.Uint64Lo = 0;
    UINT64 contextTable = (UINT64)Memory::VirtToPhy(Translations->ContextTable);
    defaultRootValue.Bits.ContextTablePointerLo = (UINT32)(contextTable >> 12);
    defaultRootValue.Bits.ContextTablePointerHi = (UINT32)(contextTable >> 32);
    defaultRootValue.Bits.Present = TRUE;
    for (UINT64 bus = 0; bus < ARRAY_SIZE(Translations->RootTable); bus++)
    {
        Translations->RootTable[bus] = defaultRootValue;
    }

    //
    // Fill out the context table. All context entries point to the same
    // second-level PML4.
    //
    // Note that pass-through translations can also be archived by setting 10b to
    // the TT: Translation Type field, instead of using the second-level page
    // tables.
    //
    defaultContextValue.Uint128.Uint64Hi = defaultContextValue.Uint128.Uint64Lo = 0;
    defaultContextValue.Bits.DomainIdentifier = 2;
    defaultContextValue.Bits.AddressWidth = BIT1;  // 010b: 48-bit AGAW (4-level page table)
    UINT64 Pml4 = (UINT64)Memory::VirtToPhy(Translations->SlPml4);
    defaultContextValue.Bits.SecondLevelPageTranslationPointerLo = (UINT32)(Pml4 >> 12);
    defaultContextValue.Bits.SecondLevelPageTranslationPointerHi = (UINT32)(Pml4 >> 32);
    defaultContextValue.Bits.Present = TRUE;
    for (UINT64 i = 0; i < ARRAY_SIZE(Translations->ContextTable); i++)
    {
        Translations->ContextTable[i] = defaultContextValue;
    }

    //
    // Fill out the second level page tables. All entries indicates readable and
    // writable, and translations are identity mapping. No second-level page table
    // is used to save space. All PDEs are configured for 2MB large pages.
    //
    destinationPa = 0;

    //
    // SL-PML4. Only the first entry (ie, translation up to 512GB) is initialized.
    //
    pml4Index = 0;
    pdpt = Translations->SlPdpt[pml4Index];
    pml4e = &Translations->SlPml4[pml4Index];
    pml4e->Uint64 = (UINT64)Memory::VirtToPhy(pdpt);
    pml4e->Bits.Read = TRUE;
    pml4e->Bits.Write = TRUE;

    for (UINT64 pdptIndex = 0; pdptIndex < 512; pdptIndex++)
    {
        //
        // SL-PDPT
        //
        pd = Translations->SlPd[pml4Index][pdptIndex];
        pdpte = &pdpt[pdptIndex];
        pdpte->Uint64 = (UINT64)Memory::VirtToPhy(pd);
        pdpte->Bits.Read = TRUE;
        pdpte->Bits.Write = TRUE;

        for (UINT64 pdIndex = 0; pdIndex < 512; pdIndex++)
        {
            //
            // SL-PD.
            //
            if (b2MBCompatible) {
                pde = &pd[pdIndex];
                pde->Uint64 = destinationPa;
                pde->Bits.Read = TRUE;
                pde->Bits.Write = TRUE;
                pde->Bits.PageSize = TRUE;
                destinationPa += SIZE_2MB;
            }
            else {
                pde = (VTD_SECOND_LEVEL_PAGING_ENTRY*)cpp::kMallocZero(PAGE_SIZE);
                pde->Uint64 = (UINT64)Memory::VirtToPhy(pd);
                pde->Bits.Read = TRUE;
                pde->Bits.Write = TRUE;
                pde->Bits.PageSize = TRUE;

                for (UINT64 ptIndex = 0; ptIndex < 512; ptIndex++) {
                    auto pte = &pde[ptIndex];
                    pte->Uint64 = destinationPa;
                    pte->Bits.Read = TRUE;
                    pte->Bits.Write = TRUE;
                    pte->Bits.PageSize = TRUE;
                    destinationPa += PAGE_SIZE;
                }
            }
        }
    }

    //
    // Write-back the whole range of the translations object to RAM. This flushing
    // cache line is not required if the C: Page-walk Coherency bit is set. Same
    // as other flush in this project. All author's units did not set this bit.
    //
    CPU::WriteBackDataCacheRange(Translations, sizeof(*Translations));
}

/**
* @brief Splits the PDE to a new PT.
*/
VTD_SECOND_LEVEL_PAGING_ENTRY*
Split2MbPage(
    IN OUT VTD_SECOND_LEVEL_PAGING_ENTRY* PageDirectoryEntry
)
{
    UINT64 baseAddress;
    VTD_SECOND_LEVEL_PAGING_ENTRY* pageTable;
    BOOLEAN readable;
    BOOLEAN writable;

    ASSERT(PageDirectoryEntry->Bits.PageSize == TRUE);

    pageTable = (VTD_SECOND_LEVEL_PAGING_ENTRY*)cpp::kMalloc(PAGE_SIZE, PAGE_READWRITE);
    if (pageTable == NULL)
    {
        goto Exit;
    }
    RtlZeroMemory(pageTable, PAGE_SIZE);

    //
    // Those fields should inherit from the PDE.
    //
    readable = (PageDirectoryEntry->Bits.Read != FALSE);
    writable = (PageDirectoryEntry->Bits.Write != FALSE);

    //
    // Fill out the page table.
    //
    baseAddress = ((UINT64)PageDirectoryEntry->Bits.AddressLo << 12) |
        ((UINT64)PageDirectoryEntry->Bits.AddressHi << 32);
    for (UINT64 ptIndex = 0; ptIndex < 512; ++ptIndex)
    {
        pageTable[ptIndex].Uint64 = baseAddress;
        pageTable[ptIndex].Bits.Read = readable;
        pageTable[ptIndex].Bits.Write = writable;
        baseAddress += PAGE_SIZE;
    }

    //
    // The PDE should no longer indicates 2MB large page.
    //
    PageDirectoryEntry->Uint64 = (UINT64)Memory::VirtToPhy(pageTable);
    PageDirectoryEntry->Bits.PageSize = FALSE;
    PageDirectoryEntry->Bits.Read = TRUE;
    PageDirectoryEntry->Bits.Write = TRUE;

    //
    // Write back changes to RAM. Also, invalidation of IOTLB would be required
    // if the DMA-remapping is already enabled. Not the case in this project.
    //
    CPU::WriteBackDataCacheRange(PageDirectoryEntry, sizeof(*PageDirectoryEntry));
    CPU::WriteBackDataCacheRange(pageTable, PAGE_SIZE);

    baseAddress = ((UINT64)PageDirectoryEntry->Bits.AddressLo << 12) |
        ((UINT64)PageDirectoryEntry->Bits.AddressHi << 32);
    DbgMsg("[VT-d] Split page at %p to 0x%llx", PageDirectoryEntry, baseAddress);
Exit:
    return pageTable;
}

/**
 * @brief Updates the access permissions in the translations for the given address.
 *
 * @note As the name suggests, this change is applied for all devices, ie, you
 *       may not specify a source-id (ie, bus:device:function). This is purely
 *       for overall simplicity of this project.
 */
EFI_STATUS
ChangePermissionOfPageForAllDevices(
    IN OUT DMAR_TRANSLATIONS* Translations,
    IN UINT64 Address,
    IN BOOLEAN AllowReadWrite,
    OUT VTD_SECOND_LEVEL_PAGING_ENTRY** AllocatedPageTable
)
{
    PHYSICAL_ADDRESS pa = { 0 };
    EFI_STATUS status;
    ADDRESS_TRANSLATION_HELPER helper;
    VTD_SECOND_LEVEL_PAGING_ENTRY* pde;
    VTD_SECOND_LEVEL_PAGING_ENTRY* pt;
    VTD_SECOND_LEVEL_PAGING_ENTRY* pte;

    *AllocatedPageTable = NULL;

    helper.AsUInt64 = Address;
    DbgMsg("[VT-d] Target 0x%llx at pml4: 0x%llx, pdpt: 0x%llx, pdt: 0x%llx, pt: 0x%llx",
        helper.AsUInt64,
        helper.AsIndex.Pml4, helper.AsIndex.Pdpt, helper.AsIndex.Pd, helper.AsIndex.Pt);

    //
    // Locate the second-level PDE for the given address. If that entry indicates
    // the page is 2MB large page, split it into 512 PTEs so that the exactly
    // specified page (4KB) only is updated.
    //
    if (helper.AsIndex.Pml4) {
        DbgMsg("[VT-d] This is not a physical address: 0x%llx", Address);
        return STATUS_UNSUCCESSFUL;
    }

    pde = &Translations->SlPd[helper.AsIndex.Pml4][helper.AsIndex.Pdpt][helper.AsIndex.Pd];
    if (pde->Bits.PageSize != FALSE)
    {
        *AllocatedPageTable = Split2MbPage(pde);
        if (*AllocatedPageTable == NULL)
        {
            status = STATUS_RESOURCE_NOT_OWNED;
            goto Exit;
        }
    }

    //
    // Then, update the single PTE that corresponds to the given address.
    //
    pt = (VTD_SECOND_LEVEL_PAGING_ENTRY*)(((UINT64)pde->Bits.AddressLo << 12) |
        ((UINT64)pde->Bits.AddressHi << 32));
    pa.QuadPart = (ULONGLONG)pt;
    pt = (VTD_SECOND_LEVEL_PAGING_ENTRY*)MmMapIoSpaceEx(pa, PAGE_SIZE, PAGE_READWRITE | PAGE_NOCACHE);
    pte = &pt[helper.AsIndex.Pt];
    pte->Bits.Read = AllowReadWrite;
    pte->Bits.Write = AllowReadWrite;
    CPU::WriteBackDataCacheRange(pte, sizeof(*pte));

    //
    // We are good. Note that any of page table updates would require invalidation
    // of IOTLB if DMA-remapping is already enabled. In our case, not yet.
    //
    status = STATUS_SUCCESS;
    MmUnmapIoSpace((PVOID)pt, PAGE_SIZE);

Exit:
    return status;
}

EFI_STATUS
ChangePointerOfPageForAllDevices(
    IN OUT DMAR_TRANSLATIONS* Translations,
    IN UINT64 Address,
    IN UINT64 SubstituteAddress,
    OUT VTD_SECOND_LEVEL_PAGING_ENTRY** AllocatedPageTable
)
{
    PHYSICAL_ADDRESS pa = { 0 };
    EFI_STATUS status;
    ADDRESS_TRANSLATION_HELPER helper;
    VTD_SECOND_LEVEL_PAGING_ENTRY* pde;
    VTD_SECOND_LEVEL_PAGING_ENTRY* pt;
    VTD_SECOND_LEVEL_PAGING_ENTRY* pte;

    *AllocatedPageTable = NULL;

    if (MmIsAddressValid((PVOID)Address))
        Address = paging::VirtToPhy((PVOID)Address);

    helper.AsUInt64 = Address;
    DbgMsg("[VT-d] Target 0x%llx at pml4: 0x%llx, pdpt: 0x%llx, pdt: 0x%llx, pt: 0x%llx",
        helper.AsUInt64,
        helper.AsIndex.Pml4, helper.AsIndex.Pdpt, helper.AsIndex.Pd, helper.AsIndex.Pt);

    //
    // Locate the second-level PDE for the given address. If that entry indicates
    // the page is 2MB large page, split it into 512 PTEs so that the exactly
    // specified page (4KB) only is updated.
    //
    if (helper.AsIndex.Pml4) {
        DbgMsg("[VT-d] Physical address outside boundaries: 0x%llx", Address);
        return STATUS_UNSUCCESSFUL;
    }

    pde = &Translations->SlPd[helper.AsIndex.Pml4][helper.AsIndex.Pdpt][helper.AsIndex.Pd];
    if (pde->Bits.PageSize != FALSE)
    {
        *AllocatedPageTable = Split2MbPage(pde);
        if (*AllocatedPageTable == NULL)
        {
            status = STATUS_RESOURCE_NOT_OWNED;
            goto Exit;
        }
    }

    //
    // Then, update the single PTE that corresponds to the given address.
    //
    pt = (VTD_SECOND_LEVEL_PAGING_ENTRY*)(((UINT64)pde->Bits.AddressLo << 12) |
        ((UINT64)pde->Bits.AddressHi << 32));
    pa.QuadPart = (ULONGLONG)pt;
    pt = (VTD_SECOND_LEVEL_PAGING_ENTRY*)MmMapIoSpaceEx(pa, PAGE_SIZE, PAGE_READWRITE | PAGE_NOCACHE);
    pte = &pt[helper.AsIndex.Pt];

    if (MmIsAddressValid((PVOID)SubstituteAddress))
        SubstituteAddress = paging::VirtToPhy((PVOID)SubstituteAddress);

    pte->Bits.AddressLo = SubstituteAddress >> 12;
    pte->Bits.AddressHi = SubstituteAddress >> 32;
    CPU::WriteBackDataCacheRange(pte, sizeof(*pte));

    //
    // We are good. Note that any of page table updates would require invalidation
    // of IOTLB if DMA-remapping is already enabled. In our case, not yet.
    //
    status = STATUS_SUCCESS;
    MmUnmapIoSpace((PVOID)pt, PAGE_SIZE);

Exit:
    return status;
}

/**
 * @brief Enables DMA-remapping for the hardware unit using the given translation.
 */
VOID
EnableDmaRemapping(
    IN CONST DMAR_UNIT_INFORMATION* DmarUnit,
    IN CONST DMAR_TRANSLATIONS* Translations
)
{
    UINT64 iotlbRegOffset;

    DbgMsg("[VT-d] Working with the remapping unit at %p", DmarUnit->RegisterBasePa);

    acpi::DmarRegister reg(DmarUnit->RegisterBasePa);

    //
    // Set the Root Table Pointer. This is equivalent to setting CR3 conceptually.
    // After setting the "SRTP: Set Root Table Pointer" bit, software must wait
    // completion of it. See 10.4.5 Global Status Register.
    //
    DbgMsg("[VT-d] Setting the root table pointer to %p", Translations->RootTable);
    reg.SetRootTable((PVOID)Translations->RootTable);

    //
    // Then, invalidate cache that may exists as requested by the specification.
    //
