# Voyager Architecture Extraction

**PRIMARY REFERENCE FOR OMBRA HYPERVISOR**

---

## 1. BOOT CHAIN FLOW

### Entry Point (UefiMain.c:12)
```c
EFI_STATUS EFIAPI UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
```
- **Location**: `Voyager/UefiMain.c:12-72`
- **Flow**:
  1. Restore bootmgfw.efi from backup (line 24)
  2. Load payload.dll from disk into memory (line 32)
  3. Get bootmgfw device path (line 40)
  4. Load bootmgfw with `LoadImage()` (line 48)
  5. Install hooks on bootmgfw (line 56)
  6. Start bootmgfw execution (line 65)

### bootmgfw.efi Restoration (BootMgfw.c:5-132)
```c
EFI_STATUS EFIAPI RestoreBootMgfw(VOID)
```
- **File**: `Voyager/BootMgfw.c:5-132`
- **Paths**:
  - `WINDOWS_BOOTMGFW_PATH`: `L"\\efi\\microsoft\\boot\\bootmgfw.efi"` (BootMgfw.h:19)
  - `WINDOWS_BOOTMGFW_BACKUP_PATH`: `L"\\efi\\microsoft\\boot\\bootmgfw.efi.backup"` (BootMgfw.h:21)
  - `PAYLOAD_PATH`: `L"\\efi\\microsoft\\boot\\payload.dll"` (BootMgfw.h:20)
- **Process**:
  1. Delete current bootmgfw.efi (line 47)
  2. Read bootmgfw.efi.backup into buffer (lines 88-92)
  3. Delete backup file (line 95)
  4. Create new bootmgfw.efi from backup data (lines 102-115)

### bootmgfw Hook Installation (BootMgfw.c:179-209)
```c
EFI_STATUS EFIAPI InstallBootMgfwHooks(EFI_HANDLE ImageHandle)
```
- **File**: `Voyager/BootMgfw.c:179-209`
- **Target Function**: `ArchStartBootApplication` (aka `BlImgStartBootApplication`)
- **Pattern Scan**: Version-specific signatures (BootMgfw.h:4-16)
  - Win10 2004+: `"\x48\x8B\xC4\x48\x89\x58\x20..."`
  - Win10 1709: `"\x48\x8B\xC4\x48\x89\x58\x00\x44\x89\x40..."`
  - Win10 1703: Different signature
  - Win10 <1703: `"\xE8\x00\x00\x00\x00\x48\x8B\xCE\x8B\xD8..."`
- **Hook Type**: Inline hook via `MakeInlineHook()` (line 203/206)

### bootmgfw → winload Transition (BootMgfw.c:211-258)
```c
EFI_STATUS EFIAPI ArchStartBootApplicationHook(VOID* AppEntry, VOID* ImageBase, UINT32 ImageSize, UINT8 BootOption, VOID* ReturnArgs)
```
- **File**: `Voyager/BootMgfw.c:211-258`
- **Process**:
  1. Disable bootmgfw hook (line 214)
  2. Check if winload exports `BlLdrLoadImage` (line 217) - version detection
  3. **For Windows ≤1703** (no exports):
     - Find `BlImgLoadPEImageEx` via pattern (lines 219-224)
     - Hook it to intercept hvloader.efi loading (line 233)
     - Signature: `LOAD_PE_IMG_SIG` (WinLoad.h:17)
  4. **For Windows 1709-2004** (has exports):
     - Hook `BlLdrLoadImage` to detect hv.exe (line 254)
     - Hook `BlImgAllocateImageBuffer` to extend allocation (line 255)
     - Signatures: `ALLOCATE_IMAGE_BUFFER_SIG` (WinLoad.h:9)

### winload → hvloader/hv.exe Flow

#### Path 1: Windows 1709-2004 (WinLoad.c:13-97)
```c
EFI_STATUS EFIAPI BlLdrLoadImage(...)
```
- **File**: `Voyager/WinLoad.c:13-97`
- **Triggers**: When `ModuleName == L"hv.exe"` (line 33)
- **Actions**:
  1. Set `HyperVloading = TRUE` flag (line 34)
  2. Wait for allocation hook to extend hv.exe memory
  3. After load completes (line 62), inject payload:
     - Add new PE section named "payload" (lines 76-81)
     - Map payload module into section (line 89)
     - Hook VMExit handler (lines 85-90)
     - Extend `SizeOfImage` in PE header (line 94)

#### Path 2: Windows ≤1703 via hvloader.efi (WinLoad.c:99-180)
```c
EFI_STATUS EFIAPI BlImgLoadPEImageEx(...)
```
- **File**: `Voyager/WinLoad.c:99-180`
- **Triggers**: When `ImagePath` contains `L"hvloader.efi"` (line 141)
- **Actions**:
  1. Find hvloader functions via patterns (lines 144-168):
     - `HvBlImgLoadPEImageFromSourceBuffer` (Win10 1703) or
     - `HvBlImgLoadPEImageEx` (Win10 ≤1607)
     - `HvBlImgAllocateImageBuffer` (all versions)
  2. Install hooks in hvloader (lines 170-177)
  3. Wait for hvloader to load hv.exe

### hvloader → hv.exe (HvLoader.c:12-204)

#### Allocation Extension (HvLoader.c:168-204)
```c
UINT64 EFIAPI HvBlImgAllocateImageBuffer(...)
```
- **Condition**: `imageSize >= HV_ALLOC_SIZE` (0x1400000) (line 178)
- **Extension**: `imageSize += PayLoadSize()` (line 181)
- **Memory Type**: Change to `BL_MEMORY_ATTRIBUTE_RWX` (0x424000) (line 184)

#### Payload Injection (HvLoader.c:92-165)
```c
EFI_STATUS EFIAPI HvBlImgLoadPEImageEx(...)
```
- **File**: `Voyager/HvLoader.c:92-165`
- **After hv.exe loaded** (line 132):
  1. Add "payload" section to hv.exe PE (lines 144-150)
  2. Populate `VOYAGER_T` structure (lines 139-152)
  3. Map payload module (line 158)
  4. Hook VMExit handler (lines 154-159)
  5. Update `ImageSize` with new `SizeOfImage` (line 163)

---

## 2. PAYLOAD INJECTION

### Payload Storage (PayLoad.c:5)
```c
VOID* PayLoad = NULL;  // Set at runtime
```
- **Loaded from disk**: `L"\\efi\\microsoft\\boot\\payload.dll"` (PayLoad.h:34)
- **Loading function**: `LoadPayLoadFromDisk()` (PayLoad.c:75-162)
- **Cleanup**: Payload deleted from disk after load (line 148)

### Payload Size Calculation (PayLoad.c:7-18)
```c
UINT32 PayLoadSize(VOID)
{
    // Returns PE SizeOfImage + 0x1000 padding
    return RecordNtHeaders->OptionalHeader.SizeOfImage + 0x1000;
}
```

### PE Section Addition (PayLoad.c:35-73)
```c
VOID* AddSection(VOID* ImageBase, CHAR8* SectionName, UINT32 VirtualSize, UINT32 Characteristics)
```
- **File**: `Voyager/PayLoad.c:35-73`
- **Section Name**: `"payload"` (hardcoded in calls)
- **Characteristics**: `SECTION_RWX` (PayLoad.h:8-13)
  - `EFI_IMAGE_SCN_MEM_WRITE | EFI_IMAGE_SCN_CNT_CODE | EFI_IMAGE_SCN_MEM_EXECUTE | EFI_IMAGE_SCN_MEM_READ`
- **Process**:
  1. Calculate new section's `VirtualAddress` via alignment (lines 56-58)
  2. Set `SizeOfRawData` aligned to `FileAlignment` (line 60)
  3. Set `PointerToRawData` after last section (lines 63-65)
  4. Increment `NumberOfSections` (line 67)
  5. Update `SizeOfImage` in OptionalHeader (lines 68-70)
  6. Return virtual address of new section (line 72)

### Module Mapping (Hv.c:4-87)
```c
VOID* MapModule(PVOYAGER_T VoyagerData, UINT8* ImageBase)
```
- **File**: `Voyager/Hv.c:4-87`
- **Process**:
  1. Copy PE headers (line 17)
  2. Copy all sections to `VoyagerData->ModuleBase` (lines 19-31)
  3. Find `voyager_context` export and write `VOYAGER_T` structure (lines 33-47)
  4. Process relocations (DIR64 type) (lines 49-84)
  5. Return entry point RVA (line 86)

---

## 3. HYPER-V INTEGRATION

### VOYAGER_T Structure (PayLoad.h:23-30)
```c
typedef struct _VOYAGER_T
{
    UINT64 VmExitHandlerRva;      // Offset to original VMExit handler
    UINT64 HypervModuleBase;      // hv.exe base address
    UINT64 HypervModuleSize;      // hv.exe module size
    UINT64 ModuleBase;            // Payload base (in hv.exe section)
    UINT64 ModuleSize;            // Payload size
} VOYAGER_T, *PVOYAGER_T;
```
- **Populated in**: `MakeVoyagerData()` (Hv.c:89-139)
- **Passed to payload**: Via `voyager_context` export (Hv.c:42-47)

### VMExit Handler Hook (Hv.c:141-183)
```c
VOID* HookVmExit(VOID* HypervBase, VOID* HypervSize, VOID* VmExitHook)
```
- **File**: `Voyager/Hv.c:141-183`

#### Intel VMExit Hook (lines 143-166)
- **Pattern**: `INTEL_VMEXIT_HANDLER_SIG` (Hv.h:6-26, version-specific)
- **Hook Location**: Call to `vmexit_c_handler` (offset +19 from pattern match)
- **Code Structure** (lines 113-118):
  ```asm
  mov rcx, [rsp+arg_18]    ; Register context
  mov rdx, [rsp+arg_28]
  call vmexit_c_handler    ; ← Hook this (RIP-relative call)
  jmp  loc_...
  ```
- **RVA Calculation** (lines 160-163):
  ```c
  VmExitHandlerCall = pattern + 19;                    // Points to call instruction
  VmExitHandlerCallRip = VmExitHandlerCall + 5;        // RIP after call
  VmExitFunction = VmExitHandlerCallRip + *(INT32*)(VmExitHandlerCall + 1);
  NewVmExitRVA = VmExitHook - VmExitHandlerCallRip;
  *(INT32*)(VmExitHandlerCall + 1) = NewVmExitRVA;     // Patch RVA
  ```

#### AMD VMExit Hook (lines 168-182)
- **Pattern**: `AMD_VMEXIT_HANDLER_SIG` (Hv.h:28-29)
  - `"\xE8\x00\x00\x00\x00\x48\x89\x04\x24\xE9"` (call instruction)
- **Same RVA patching technique** (lines 177-180)

### VMExit Handler RVA Storage (Hv.c:89-139)
```c
VOID MakeVoyagerData(...)
{
    // Calculate offset from payload entry to original handler
    VoyagerData->VmExitHandlerRva = PayLoadEntry(PayLoadBase) - VmExitFunction;
}
```
- **Intel calculation** (line 123)
- **AMD calculation** (line 137)

### Payload VMExit Implementation (vmexit_handler.cpp:1-141)
```c
void vmexit_handler(pcontext_t* context, void* unknown)
```
- **File**: `PayLoad (Intel)/vmexit_handler.cpp:1-141`
- **VMCALL Interface**: CPUID-based hypercall
  - **Trigger**: Exit reason == `VMX_EXIT_REASON_EXECUTE_CPUID` (line 18)
  - **Key**: `guest_registers->rcx == VMEXIT_KEY` (line 20)
  - **Command**: `guest_registers->rdx` (line 22)
  - **Data Pointer**: `guest_registers->r8` (lines 32, 50, etc.)
  - **Return Value**: `guest_registers->rax` (lines 26, 41, etc.)

### VMCALL Commands (vmexit_handler.cpp:24-126)
1. **init_page_tables** (line 24): Initialize VMX root page tables
2. **get_dirbase** (line 29): Read guest CR3
3. **read_guest_phys** (line 47): Physical memory read
4. **write_guest_phys** (line 68): Physical memory write
5. **copy_guest_virt** (line 89): Virtual memory copy
6. **translate** (line 104): Virtual → Physical translation

### Original Handler Invocation (vmexit_handler.cpp:137-140)
```c
reinterpret_cast<vmexit_handler_t>(
    reinterpret_cast<u64>(&vmexit_handler) - voyager_context.vmexit_handler_rva
)(context, unknown);
```
- **Calculation**: Current function address - stored RVA = original handler

---

## 4. UEFI RUNTIME

### File System Access Pattern
- **Protocol**: `gEfiSimpleFileSystemProtocolGuid`
- **Usage**: `LocateHandleBuffer()` → `OpenProtocol()` → `OpenVolume()` (BootMgfw.c:14-32)
- **File Operations**:
  - Open with `VolumeHandle->Open()` (BootMgfw.c:34)
  - Read with `BootMgfwFile->Read()` (BootMgfw.c:88)
  - Delete with `BootMgfwFile->Delete()` (BootMgfw.c:47, 95)
  - Write with `BootMgfwFile->Write()` (BootMgfw.c:111)

### Memory Allocation
- **Type**: `EfiBootServicesData` (BootMgfw.c:69, 85)
- **Functions**:
  - `gBS->AllocatePool()` (BootMgfw.c:69, 85, 140)
  - `gBS->FreePool()` (BootMgfw.c:118, 130)

### SetVirtualAddressMap Considerations
- **Not explicitly handled** in Voyager (payload mapped before OS boot)
- **Memory Type**: Uses boot services memory (freed after ExitBootServices)
- **Payload lives in hv.exe's extended section** (kernel memory, not UEFI runtime)

### Image Loading Protocol
- **LoadImage**: `gBS->LoadImage()` (UefiMain.c:48)
- **StartImage**: `gBS->StartImage()` (UefiMain.c:65)
- **HandleProtocol**: `gBS->HandleProtocol()` with `gEfiLoadedImageProtocolGuid` (BootMgfw.c:184)
  - Retrieves `EFI_LOADED_IMAGE` structure containing `ImageBase` and `ImageSize`

---

## 5. SERIAL DEBUGGING

### Debug Output Macro (Utils.h:27-29)
```c
#define DBG_PRINT(...) \
    AsciiSPrint(dbg_buffer, sizeof dbg_buffer, __VA_ARGS__); \
    __outbytestring(PORT_NUM, dbg_buffer, AsciiStrLen(dbg_buffer))
```

### Serial Port Configuration (Utils.h:15)
```c
#define PORT_NUM 0x2F8  // COM2
```

### Intrinsics (Utils.h:20-24)
```c
void __outdword(unsigned short, unsigned long);
VOID __outbytestring(UINT16 Port, UINT8* Buffer, UINT32 Count);
void __outbyte(unsigned short Port, unsigned char Data);
#pragma intrinsic(__outbytestring)
#pragma intrinsic(__outbyte)
```

### Debug Usage Examples
- Print module info: `Print(L"BootMgfw Image Base -> 0x%p\n", BootMgfw->ImageBase);` (BootMgfw.c:187)
- Pattern match results: `Print(L"BootMgfw.BlImgStartBootApplication -> 0x%p\n", ArchStartBootApplication);` (BootMgfw.c:202)
- Payload size: `Print(L"Hyper-V PayLoad Size -> 0x%x\n", PayLoadSize());` (BootMgfw.c:231)

---

## 6. PE MANIPULATION

### Pattern Scanning (Utils.c - referenced)
```c
VOID* FindPattern(CHAR8* base, UINTN size, CHAR8* pattern, CHAR8* mask);
BOOLEAN CheckMask(CHAR8* base, CHAR8* pattern, CHAR8* mask);
```
- **Used for**: Finding functions in bootmgfw, winload, hvloader (BootMgfw.c:190-196)

### RVA Resolution Macro (Utils.h:31-32)
```c
#define RESOLVE_RVA(SIG_RESULT, RIP_OFFSET, RVA_OFFSET) \
    (*(INT32*)(((UINT64)SIG_RESULT) + RVA_OFFSET)) + ((UINT64)SIG_RESULT) + RIP_OFFSET
```
- **Purpose**: Resolve RIP-relative calls/jumps from pattern matches
- **Example**: `RESOLVE_RVA(ImgLoadPEImageEx, 10, 6)` (BootMgfw.c:232)
  - Offset 6 bytes into instruction, read 4-byte RVA, add to RIP (sig + 10)

### Export Resolution (Utils.c - referenced)
```c
VOID* GetExport(UINT8* base, CHAR8* export);
```
- **Used for**: `GetExport(ImageBase, "BlLdrLoadImage")` (BootMgfw.c:217, 237)

### Entry Point Calculation (PayLoad.c:20-31)
```c
VOID* PayLoadEntry(VOID* ModuleBase)
{
    // Returns ModuleBase + AddressOfEntryPoint
    return (UINT64)ModuleBase + RecordNtHeaders->OptionalHeader.AddressOfEntryPoint;
}
```

### Relocation Processing (Hv.c:49-84)
```c
// Process base relocations
EFI_IMAGE_DATA_DIRECTORY* baseRelocDir = &ntHeaders->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC];
if (baseRelocDir->VirtualAddress) {
    // Iterate relocation blocks
    for (UINT32 currentSize = 0; currentSize < baseRelocDir->Size; ) {
        // Process each relocation entry
        switch (type) {
            case EFI_IMAGE_REL_BASED_DIR64:
                *rva = VoyagerData->ModuleBase + (*rva - ntHeaders->OptionalHeader.ImageBase);
                break;
        }
    }
}
```
- **Only DIR64 relocations supported** (line 70)
- **Purpose**: Rebase payload to actual load address in hv.exe section

### Section Alignment (PayLoad.c:35-73)
```c
// Align to section alignment
#define P2ALIGNUP(x, align) (-(-(x) & -(align)))

newSectionHeader->VirtualAddress =
    P2ALIGNUP(lastSectionHeader->VirtualAddress + lastSectionHeader->Misc.VirtualSize, sectionAlignment);
```
- **Section Alignment**: From PE OptionalHeader (typically 0x1000)
- **File Alignment**: From PE OptionalHeader (typically 0x200)

### SizeOfImage Update
- **Critical**: MUST update after section addition (PayLoad.c:68-70)
  ```c
  ntHeaders->OptionalHeader.SizeOfImage =
      P2ALIGNUP(newSectionHeader->VirtualAddress + newSectionHeader->Misc.VirtualSize, sectionAlignment);
  ```
- **Also updated in LDR entry**: `TableEntry->SizeOfImage = NT_HEADER(...)->OptionalHeader.SizeOfImage;` (WinLoad.c:94)

---

## 7. INLINE HOOK MECHANISM

### Hook Structure (InlineHook.h:4-11)
```c
typedef struct _INLINE_HOOK
{
    unsigned char Code[14];        // Original bytes
    unsigned char JmpCode[14];     // Hook trampoline
    void* Address;                 // Hook location
    void* HookAddress;             // Destination function
} INLINE_HOOK, *PINLINE_HOOK_T;
```

### Hook Creation (InlineHook.c:3-23)
```c
VOID MakeInlineHook(PINLINE_HOOK_T Hook, VOID* HookFrom, VOID* HookTo, BOOLEAN Install)
{
    unsigned char JmpCode[14] = {
        0xff, 0x25, 0x0, 0x0, 0x0, 0x0,  // jmp QWORD PTR[rip + 0x0]
        0x0, 0x0, 0x0, 0x0,              // Address bytes 0-3
        0x0, 0x0, 0x0, 0x0               // Address bytes 4-7
    };

    Hook->Address = HookFrom;
    Hook->HookAddress = HookTo;
    MemCopy(Hook->Code, HookFrom, sizeof Hook->Code);  // Save original
    MemCopy(JmpCode + 6, &HookTo, sizeof HookTo);      // Embed target
    MemCopy(Hook->JmpCode, JmpCode, sizeof JmpCode);
    if (Install) EnableInlineHook(Hook);
}
```
- **Trampoline Type**: Absolute indirect jump (`jmp [rip+0]` with address inline)
- **Size**: 14 bytes total (6 byte instruction + 8 byte address)

### Hook Enable/Disable (InlineHook.c:25-33)
```c
VOID EnableInlineHook(PINLINE_HOOK_T Hook) {
    MemCopy(Hook->Address, Hook->JmpCode, sizeof Hook->JmpCode);
}

VOID DisableInlineHook(PINLINE_HOOK_T Hook) {
    MemCopy(Hook->Address, Hook->Code, sizeof Hook->Code);  // Restore original
}
```

### Hook Usage Pattern
1. **Create hook**: `MakeInlineHook(&HookStruct, TargetFunc, MyHook, TRUE);`
2. **In hook function**:
   ```c
   DisableInlineHook(&HookStruct);          // Disable to call original
   Result = ((FUNC_TYPE)HookStruct.Address)(...);  // Call original
   EnableInlineHook(&HookStruct);           // Re-enable if needed
   ```

---

## KEY INSIGHTS FOR OMBRA

### Boot Chain Hook Points
1. **bootmgfw.efi**: Hook `ArchStartBootApplication` (called when loading winload)
2. **winload.efi/exe**: Hook `BlLdrLoadImage` (2004-1709) OR `BlImgLoadPEImageEx` (<1703)
3. **hvloader.efi/exe**: Hook `HvBlImgAllocateImageBuffer` + load functions
4. **hv.exe/hvix64.exe/hvax64.exe**: Modify VMExit handler call instruction

### Memory Extension Strategy
- **Allocation hook**: Intercept `BlImgAllocateImageBuffer` when `imageSize >= HV_ALLOC_SIZE`
- **Extension amount**: `PayLoadSize()` (SizeOfImage + 0x1000)
- **Memory attributes**: Change to `BL_MEMORY_ATTRIBUTE_RWX` (0x424000)
- **Critical**: Must happen BEFORE image is loaded into allocated buffer

### PE Injection Flow
1. Wait for allocation to be extended
2. After load completes, add new PE section to hv.exe
3. Map payload into new section (headers + sections + relocations)
4. Populate `VOYAGER_T` structure via export
5. Hook VMExit handler with RVA patch
6. Update `SizeOfImage` in PE header AND LDR entry

### VMCALL Interface Design
- **Trigger**: CPUID VMExit (harder to detect than dedicated VMCALL)
- **Key in RCX**: Identifies Voyager VMCalls vs legitimate CPUID
- **Command in RDX**: Dispatch value
- **Data pointer in R8**: Physical address of command structure
- **Return in RAX**: Error code or result
- **Critical**: Advance RIP by instruction length after handling (vmexit_handler.cpp:129-132)

### Version-Specific Handling
- **Signature scanning required** for each Windows version
- **Export availability**: Windows ≥1709 exports functions, ≤1703 doesn't
- **hvloader path**: ≤1703 requires hvloader.efi hooking, ≥1709 can hook winload directly
- **Context pointer**: Win10 >1803 passes `pcontext_t*`, ≤1803 passes `pcontext_t`

---

## FILES TO MODIFY IN OMBRA

### OmbraBoot/
- `UefiMain.cpp` - Implement Voyager's UefiMain flow
- `Hooks/BootMgfw.cpp` - Port BootMgfw.c hook logic
- `Hooks/WinLoad.cpp` - Port WinLoad.c hook logic
- `Hooks/HvLoader.cpp` - Port HvLoader.c hook logic
- `Utils/PatternScan.cpp` - Port FindPattern implementation
- `Utils/InlineHook.cpp` - Port inline hook mechanism
- `PE/SectionAdd.cpp` - Port AddSection logic
- `PE/Mapper.cpp` - Port MapModule logic

### OmbraPayload/
- `Entry.cpp` - Implement vmexit_handler pattern
- `VmExit/Handler.cpp` - VMCALL dispatch table
- `VmExit/Commands.cpp` - Individual command implementations
- `VmExit/Intel.cpp` - Intel VMX-specific handling
- `VmExit/Amd.cpp` - AMD SVM-specific handling

---

## RISKS & MITIGATIONS

| Risk | Mitigation |
|------|------------|
| **Signature changes across Windows versions** | Maintain multi-version signature database, test on all target OS versions |
| **PatchGuard detection of VMExit hook** | Hook is in hv.exe space (before PG init), use RVA patch not inline hook |
| **Memory allocation size mismatch** | Verify HV_ALLOC_SIZE (0x1400000) per Windows version, check second allocation |
| **SizeOfImage not updated** | Update in BOTH PE header AND LDR_DATA_TABLE_ENTRY or hv.exe won't load |
| **Relocations fail** | Ensure all DIR64 relocations processed, verify ImageBase delta calculation |
| **VMCALL key collision** | Use cryptographically random VMEXIT_KEY value |
| **RIP not advanced after VMCALL** | Always `__vmx_vmread(VMCS_VMEXIT_INSTRUCTION_LENGTH)` and add to RIP |

---

**Extraction Date**: 2025-12-20
**Voyager Commit**: Latest as of Refs/codebases/Voyager
**Windows Versions Covered**: 1507, 1607, 1703, 1709, 1803, 1809, 1903, 1909, 2004
**Architecture**: x64 only (Intel VMX + AMD SVM)
