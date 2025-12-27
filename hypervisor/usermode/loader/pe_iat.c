#include "pe_iat.h"
#include <stdio.h>

bool PatchIAT(void* mappedImage, PE_INFO* peInfo) {
    printf("[*] Patching IAT with %u entries...\n", peInfo->ImportCount);

    uint32_t patched = 0;

    for (uint32_t i = 0; i < peInfo->ImportCount; i++) {
        PE_IMPORT_INFO* imp = &peInfo->Imports[i];

        // Check if import was resolved
        if (imp->ResolvedAddress == 0) {
            printf("[-] Unresolved import: %s!%s\n",
                   imp->ModuleName, imp->FunctionName);
            return false;
        }

        // Patch IAT entry at the specified RVA
        uint64_t* iatEntry = (uint64_t*)((uint8_t*)mappedImage + imp->IatRva);
        *iatEntry = imp->ResolvedAddress;
        patched++;
    }

    printf("[+] Patched %u IAT entries\n", patched);
    return true;
}
