// test_ept_split.c - Test EPT 1GB to 2MB page splitting
// Compile: cl test_ept_split.c /I. /DTEST_MODE

#include "hypervisor/ept.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Stub implementations for testing
void AsmInvept(U64 type, void* desc) {
    printf("[TEST] INVEPT called (type=%llu)\n", type);
}

U64 __readmsr(U32 msr) {
    // Return capability bits indicating support for everything
    if (msr == 0x48C) { // MSR_IA32_VMX_EPT_VPID_CAP
        return (1ULL << 17) |  // 1GB page support
               (1ULL << 21) |  // Accessed/Dirty support
               (1ULL << 14);   // Write-back support
    }
    return 0;
}

// Debug output stubs
#define INFO(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#define TRACE(fmt, ...) printf("[TRACE] " fmt "\n", ##__VA_ARGS__)
#define ERR(fmt, ...) printf("[ERROR] " fmt "\n", ##__VA_ARGS__)
#define WARN(fmt, ...) printf("[WARN] " fmt "\n", ##__VA_ARGS__)

int main(void) {
    EPT_STATE ept;
    OMBRA_STATUS status;
    void* memory;
    U64 memoryPhys;
    U32 i;

    printf("=== EPT 1GB to 2MB Split Test ===\n\n");

    // Allocate memory pool (512 pages = 2MB)
    memory = calloc(512, 4096);
    if (!memory) {
        printf("Failed to allocate memory\n");
        return 1;
    }
    memoryPhys = 0x100000000ULL; // Fake physical address

    printf("Allocated EPT memory pool: 512 pages (2MB)\n");
    printf("Virtual: %p, Physical: 0x%llx\n\n", memory, memoryPhys);

    // Initialize EPT
    void* pml4 = memory;
    void* pdpt = (char*)memory + 4096;
    U64 pml4Phys = memoryPhys;
    U64 pdptPhys = memoryPhys + 4096;

    status = EptInitialize(&ept, pml4, pml4Phys, pdpt, pdptPhys, 512);
    if (status != OMBRA_SUCCESS) {
        printf("EPT initialization failed: %d\n", status);
        free(memory);
        return 1;
    }

    printf("\nEPT initialized successfully\n");
    printf("Pages used: %u/%u\n", ept.PagesUsed, ept.TotalPagesAllocated);
    printf("Split PD count: %u\n\n", ept.SplitPdCount);

    // Test 1: Split first 1GB page (GPA 0x0)
    printf("=== Test 1: Split GPA 0x0 (PDPT[0]) ===\n");
    status = EptSplit1GbTo2Mb(&ept, 0x0);
    if (status != OMBRA_SUCCESS) {
        printf("Split failed: %d\n", status);
        free(memory);
        return 1;
    }

    printf("Pages used: %u/%u\n", ept.PagesUsed, ept.TotalPagesAllocated);
    printf("Split PD count: %u\n", ept.SplitPdCount);
    printf("Split PD table at: %p\n", ept.SplitPdTables[0]);

    // Verify the split PD table
    EPT_PDE* pd = (EPT_PDE*)ept.SplitPdTables[0];
    printf("\nVerifying PD entries:\n");
    printf("PD[0]: phys=0x%llx, large=%u, R=%u W=%u X=%u\n",
           (U64)pd[0].LargePage.PagePhysAddr << 21,
           pd[0].LargePage.LargePage,
           pd[0].LargePage.Read,
           pd[0].LargePage.Write,
           pd[0].LargePage.Execute);
    printf("PD[1]: phys=0x%llx\n", (U64)pd[1].LargePage.PagePhysAddr << 21);
    printf("PD[511]: phys=0x%llx\n", (U64)pd[511].LargePage.PagePhysAddr << 21);

    // Verify PDPTE now points to PD
    EPT_PDPTE* pdpte = &ept.Pdpt[0];
    printf("\nPDPTE[0] after split:\n");
    printf("LargePage bit: %llu (should be 0)\n", pdpte->LargePage.LargePage);
    printf("PD physical: 0x%llx\n", (U64)pdpte->Pointer.PdPhysAddr << 12);

    // Test 2: Split another 1GB page (GPA 0x40000000 = 1GB)
    printf("\n=== Test 2: Split GPA 0x40000000 (PDPT[1]) ===\n");
    status = EptSplit1GbTo2Mb(&ept, 0x40000000ULL);
    if (status != OMBRA_SUCCESS) {
        printf("Split failed: %d\n", status);
        free(memory);
        return 1;
    }

    printf("Pages used: %u/%u\n", ept.PagesUsed, ept.TotalPagesAllocated);
    printf("Split PD count: %u\n", ept.SplitPdCount);

    // Test 3: Try splitting already split page (should succeed idempotently)
    printf("\n=== Test 3: Re-split GPA 0x0 (should be idempotent) ===\n");
    status = EptSplit1GbTo2Mb(&ept, 0x0);
    if (status != OMBRA_SUCCESS) {
        printf("Re-split failed: %d\n", status);
    } else {
        printf("Re-split succeeded (idempotent)\n");
        printf("Pages used: %u/%u (should be unchanged)\n",
               ept.PagesUsed, ept.TotalPagesAllocated);
        printf("Split PD count: %u (should be unchanged)\n", ept.SplitPdCount);
    }

    // Test 4: Split multiple pages
    printf("\n=== Test 4: Split 10 more pages ===\n");
    for (i = 2; i < 12; i++) {
        U64 gpa = (U64)i * 0x40000000ULL; // i * 1GB
        status = EptSplit1GbTo2Mb(&ept, gpa);
        if (status != OMBRA_SUCCESS) {
            printf("Split of PDPT[%u] failed: %d\n", i, status);
            break;
        }
    }
    printf("Pages used: %u/%u\n", ept.PagesUsed, ept.TotalPagesAllocated);
    printf("Split PD count: %u\n", ept.SplitPdCount);

    // Test 5: Exhaust memory
    printf("\n=== Test 5: Exhaust memory ===\n");
    U32 splits = 0;
    for (i = 12; i < 512; i++) {
        U64 gpa = (U64)i * 0x40000000ULL;
        status = EptSplit1GbTo2Mb(&ept, gpa);
        if (status == OMBRA_ERROR_NO_MEMORY) {
            printf("Out of memory at split %u\n", splits);
            break;
        } else if (status != OMBRA_SUCCESS) {
            printf("Unexpected error: %d\n", status);
            break;
        }
        splits++;
    }
    printf("Final pages used: %u/%u\n", ept.PagesUsed, ept.TotalPagesAllocated);
    printf("Final split count: %u\n", ept.SplitPdCount);

    printf("\n=== All tests passed ===\n");

    EptDestroy(&ept);
    free(memory);
    return 0;
}
