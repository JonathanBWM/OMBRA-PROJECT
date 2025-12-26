#pragma once
// OmbraSELib Vmcall wrapper
// Uses standalone vmxroot headers when in vmxroot context

#ifdef _VMXROOT_MODE
#include "vmxroot/vmcall.h"
#else
#include <Vmcall.h>
#endif
