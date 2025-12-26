// PayLoad/amd/svm_handler.h
// AMD SVM backend interface
#pragma once

#include "../include/types.h"
#include "../include/context.h"

// AMD SVM-specific includes
#include <OmbraSELib/Svm.h>

namespace amd {

//===----------------------------------------------------------------------===//
// AMD SVM Types - Aliases from Svm namespace
//===----------------------------------------------------------------------===//

using Vmcb = Svm::Vmcb;
using SvmExitCode = Svm::SvmExitCode;

//===----------------------------------------------------------------------===//
// Guest context structure (from Hyper-V reverse engineering)
//===----------------------------------------------------------------------===//

typedef struct __declspec(align(16)) _guest_context
{
    u8  gap0[8];
    u64 rcx;
    u64 rdx;
    u64 rbx;
    u8  gap20[8];
    u64 rbp;
    u64 rsi;
    u64 rdi;
    u64 r8;
    u64 r9;
    u64 r10;
    u64 r11;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;
    u128 xmm0;
    u128 xmm1;
    u128 xmm2;
    u128 xmm3;
    u128 xmm4;
    u128 xmm5;
    u128 xmm6;
    u128 xmm7;
    u128 xmm8;
    u128 xmm9;
    u128 xmm10;
    u128 xmm11;
    u128 xmm12;
    u128 xmm13;
    u128 xmm14;
    u128 xmm15;
    u8  gap180[8];
    u64 vmcb_physical_address;
} guest_context, *pguest_context;

typedef struct __declspec(align(16)) _vcpu_context
{
    u8 gap0[977];
    u8 byte3D1;
    u8 byte3D2;
    u8 gap3D3[1645];
    guest_context context;
    u8 gapBD0[1];
    u8 byteBD1;
    __declspec(align(16)) u64 dr0;
    u64 dr1;
    u64 dr2;
    u64 dr3;
} vcpu_context, *pvcpu_context;

typedef struct __declspec(align(8)) _gs_base_struct
{
    u8 gap0[64];
    u64* pqword40;
    u8 gap48[66392];
    vcpu_context* pvcpu_context_2;
    u8 gap103A8[8];
    vcpu_context* pvcpu_context;
} gs_base_struct, *pgs_base_struct;

//===----------------------------------------------------------------------===//
// Ombra context structure for AMD
//===----------------------------------------------------------------------===//

#pragma pack(push, 1)
typedef struct _OMBRA_T
{
    u64 vcpu_run_rva;
    u64 hyperv_module_base;
    u64 hyperv_module_size;
    u64 record_base;
    u64 record_size;
    u32 vmcb_base;
    u32 vmcb_link;
    u32 vmcb_off;
} OMBRA_T, * POMBRA_T;
#pragma pack(pop)

// Global Ombra context - exported for boot chain
__declspec(dllexport) inline OMBRA_T ombra_context;

//===----------------------------------------------------------------------===//
// VMCB Access
//===----------------------------------------------------------------------===//

// Get VMCB pointer from GS segment
__forceinline auto get_vmcb() -> Svm::Vmcb*
{
    return *reinterpret_cast<Svm::Vmcb**>(
        *reinterpret_cast<u64*>(
            *reinterpret_cast<u64*>(
                __readgsqword(0) + ombra_context.vmcb_base)
            + ombra_context.vmcb_link) + ombra_context.vmcb_off);
}

// Original handler function type
using vcpu_run_t = pgs_base_struct (__fastcall*)(void*, void*, guest_context*);

//===----------------------------------------------------------------------===//
// EPT Handler Type
//===----------------------------------------------------------------------===//

typedef BOOLEAN (*fnEptHandler)(UINT64 GuestPhysicalAddr);

//===----------------------------------------------------------------------===//
// AMD VMExit Handler
//===----------------------------------------------------------------------===//

// Main entry point - signature must match Hyper-V's hooked handler
// CRITICAL: AMD handler MUST return pgs_base_struct (which is gs_base_struct*)
auto vmexit_handler(void* unknown, void* unknown2, pguest_context context) -> pgs_base_struct;

// Initialize AMD backend (called once on first VMExit)
void Initialize();

} // namespace amd
