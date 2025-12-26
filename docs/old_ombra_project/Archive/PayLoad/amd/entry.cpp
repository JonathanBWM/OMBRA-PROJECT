// PayLoad/amd/entry.cpp
// Extern "C" entry point wrapper for AMD SVM backend
// Required because MSVC linker expects unmangled symbol for DLL entry point

#include "svm_handler.h"

// Entry point for AMD PayLoad DLL
// Wraps the namespaced vmexit_handler with C linkage for linker compatibility
// NOTE: AMD handler returns pgs_base_struct (which is gs_base_struct*, not gs_base_struct**)
extern "C" amd::pgs_base_struct __fastcall OmbraAmdEntry(void* unknown, void* unknown2, amd::pguest_context context)
{
    return amd::vmexit_handler(unknown, unknown2, context);
}
