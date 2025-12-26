// PayLoad/intel/entry.cpp
// Extern "C" entry point wrapper for Intel VMX backend
// Required because MSVC linker expects unmangled symbol for DLL entry point

#include "vmx_handler.h"

// Entry point for Intel PayLoad DLL
// Wraps the namespaced vmexit_handler with C linkage for linker compatibility
extern "C" void __fastcall OmbraIntelEntry(intel::pcontext_t* context, void* unknown)
{
    intel::vmexit_handler(context, unknown);
}
