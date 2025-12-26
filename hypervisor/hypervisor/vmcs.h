// vmcs.h — VMCS Management
// OmbraHypervisor

#ifndef OMBRA_VMCS_H
#define OMBRA_VMCS_H

#include "vmx.h"

// =============================================================================
// VMCS Setup Functions
// =============================================================================

// Main initialization - calls all setup functions
OMBRA_STATUS VmcsInitialize(VMX_CPU* cpu, HV_INIT_PARAMS* params);

// Individual setup functions
void VmcsSetupControls(VMX_CPU* cpu, HV_INIT_PARAMS* params);
void VmcsSetupGuestState(VMX_CPU* cpu);
void VmcsSetupHostState(VMX_CPU* cpu);

// EPT pointer construction
U64 VmcsConstructEptp(U64 pml4Physical);

#endif // OMBRA_VMCS_H
