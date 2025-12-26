#pragma once
// OmbraSELib PE wrapper
// Uses standalone vmxroot headers when in vmxroot context

#ifdef _VMXROOT_MODE
#include "vmxroot/pe.h"
#else
#include <PE.h>
#endif
