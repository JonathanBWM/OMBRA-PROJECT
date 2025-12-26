#pragma once
// OmbraSELib IDT wrapper
// Uses standalone vmxroot headers when in vmxroot context

#ifdef _VMXROOT_MODE
#include "vmxroot/IDT.h"
#else
#include <IDT.h>
#endif
