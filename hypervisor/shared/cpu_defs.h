#ifndef OMBRA_CPU_DEFS_H
#define OMBRA_CPU_DEFS_H

#include "types.h"

#define CPUID_FEAT_ECX_VMX  BIT(5)
#define CPUID_FEAT_ECX_HV   BIT(31)

#define CR0_PE  BIT(0)
#define CR0_PG  BIT(31)
#define CR4_VME BIT(0)
#define CR4_PAE BIT(5)
#define CR4_VMXE BIT(13)

#define RFLAGS_IF BIT(9)

#endif
