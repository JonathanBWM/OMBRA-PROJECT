# Rainbow - Memory Analysis Patterns

**Source**: `Refs/codebases/rainbow/`
**Type**: UEFI Bootkit / HWID Spoofer
**Focus**: Boot-time memory analysis, pattern scanning, LOADER_PARAMETER_BLOCK manipulation

---

## Core Concept

HWID spoofer that operates during boot transition phase before kernel drivers load. Uses pattern scanning to locate runtime structures in Windows loader memory space.

---

## Pattern Scanning

### FindPattern - Raw Memory Scan
**Source**: `rainbow/UefiDriver/utils.cpp:6-31`

```cpp
UINT64 Utils::FindPattern(VOID* baseAddress, UINT64 size, const CHAR8* pattern)
{
    UINT8* firstMatch = nullptr;
    const CHAR8* currentPattern = pattern;

    UINT8* start = static_cast<UINT8*>(baseAddress);
    UINT8* end = start + size;

    for (UINT8* current = start; current < end; current++)
    {
        UINT8 byte = currentPattern[0];
        if (!byte) return reinterpret_cast<UINT64>(firstMatch);
        if (byte == '\?' || *static_cast<UINT8*>(current) == GET_BYTE(byte, currentPattern[1]))
        {
            if (!firstMatch) firstMatch = current;
            if (!currentPattern[2]) return reinterpret_cast<UINT64>(firstMatch);
            ((byte == '\?') ? (currentPattern += 2) : (currentPattern += 3));
        }
        else
        {
            currentPattern = pattern;
            firstMatch = nullptr;
        }
    }

    return 0;
}
```

**Pattern**:
- Wildcard support with `\?` character
- Hex byte pattern matching `GET_BYTE(a, b)` - converts ASCII hex to byte
- Linear scan with pattern state tracking
- Returns first match address or 0

**Macros**: `utils.cpp:3-5`
```cpp
#define IN_RANGE(x, a, b) (x >= a && x <= b)
#define GET_BITS(x) (IN_RANGE((x&(~0x20)),'A','F')?((x&(~0x20))-'A'+0xA):(IN_RANGE(x,'0','9')?x-'0':0))
#define GET_BYTE(a, b) (GET_BITS(a) << 4 | GET_BITS(b))
```

### FindPatternImage - PE Section Scan
**Source**: `rainbow/UefiDriver/utils.cpp:33-51`

```cpp
UINT64 Utils::FindPatternImage(VOID* base, const CHAR8* pattern)
{
    UINT64 match = 0;

    PIMAGE_NT_HEADERS64 headers = reinterpret_cast<PIMAGE_NT_HEADERS64>(
        reinterpret_cast<UINT64>(base) + static_cast<PIMAGE_DOS_HEADER>(base)->e_lfanew);
    PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(headers);

    for (INTN i = 0; i < headers->FileHeader.NumberOfSections; ++i)
    {
        PIMAGE_SECTION_HEADER section = &sections[i];
        if (*reinterpret_cast<UINT32*>(section->Name) == 'EGAP' ||
            CompareMem(section->Name, ".text", 5) == 0)
        {
            match = FindPattern(
                reinterpret_cast<void*>(reinterpret_cast<UINT64>(base) + section->VirtualAddress),
                section->Misc.VirtualSize,
                pattern);
            if (match) break;
        }
    }

    return match;
}
```

**Pattern**:
- Parses PE header to find sections
- Scans only `.text` or `PAGE` sections (reversed check: `'EGAP'`)
- Uses base + section RVA for scan range
- Early exit on first match

---

## Boot Transition Analysis

### OslLoaderBlock Location
**Source**: `rainbow/UefiDriver/main.cpp:85-97`

```cpp
// In HookedExitBootServices - return address points into OslFwpKernelSetupPhase1

// Find OslExecuteTransition
UINT64 loaderBlockScan = Utils::FindPattern(
    reinterpret_cast<VOID*>(returnAddress),
    SCAN_MAX,
    E("48 8B 3D ? ? ? ? 48 8B 8F ? ? ? ?"));

// Pattern: mov rdi, [OslLoaderBlock]  mov rcx, [rdi+offset]
// Resolve RIP-relative address
UINT64 resolvedAddress = *reinterpret_cast<UINT64*>(
    (loaderBlockScan + 7) + *reinterpret_cast<int*>(loaderBlockScan + 3));

Print(EW(L"OslLoaderBlock -> (virt) 0x%p\n"), resolvedAddress);
```

**Pattern**:
- ExitBootServices called from `OslFwpKernelSetupPhase1`
- Scan forward from return address (SCAN_MAX = `0x5f5e100` / ~100MB window)
- Find instruction loading global `OslLoaderBlock` pointer
- Resolve RIP-relative addressing: `(instruction_end + displacement)`

**Structure**: `ntdef.h:47-60`
```cpp
typedef struct _LOADER_PARAMETER_BLOCK
{
    UINT32 OsMajorVersion;
    UINT32 OsMinorVersion;
    UINT32 Size;
    UINT32 OsLoaderSecurityVersion;
    struct _LIST_ENTRY LoadOrderListHead;
    struct _LIST_ENTRY MemoryDescriptorListHead;
    struct _LIST_ENTRY BootDriverListHead;
    struct _LIST_ENTRY EarlyLaunchListHead;
    struct _LIST_ENTRY CoreDriverListHead;
    struct _LIST_ENTRY CoreExtensionsDriverListHead;
    struct _LIST_ENTRY TpmCoreDriverListHead;
} LOADER_PARAMETER_BLOCK, * PLOADER_PARAMETER_BLOCK;
```

### BlpArchSwitchContext Location
**Source**: `rainbow/UefiDriver/main.cpp:102-108`

```cpp
BlpArchSwitchContext = reinterpret_cast<BlpArchSwitchContext_t>(
    Utils::FindPattern(
        reinterpret_cast<VOID*>(returnAddress),
        SCAN_MAX,
        E("40 53 48 83 EC 20 48 8B 15")));
// Pattern: push rbx; sub rsp, 20h; mov rdx, [...]
```

**Usage**: `main.cpp:14-22`
```cpp
typedef void(__stdcall* BlpArchSwitchContext_t)(int target);

enum WinloadContext { ApplicationContext, FirmwareContext };

#define ContextPrint(x, ...) \
    BlpArchSwitchContext(FirmwareContext); \
    Print(x, __VA_ARGS__); \
    BlpArchSwitchContext(ApplicationContext);
```

**Pattern**:
- Winload maintains two address contexts during boot
- ApplicationContext: Virtual addressing (post-virtual memory init)
- FirmwareContext: Physical/identity mapping (EFI services accessible)
- Must switch to firmware context to use EFI Print/RT services

---

## Module Enumeration

### GetModule - Walk LoadOrderListHead
**Source**: `rainbow/UefiDriver/utils.cpp:83-94`

```cpp
KLDR_DATA_TABLE_ENTRY Utils::GetModule(LIST_ENTRY* list, const CHAR16* name)
{
    for (LIST_ENTRY* entry = list->ForwardLink; entry != list; entry = entry->ForwardLink)
    {
        PKLDR_DATA_TABLE_ENTRY module = CONTAINING_RECORD(entry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        if (module && StrnCmp(name, module->BaseDllName.Buffer, module->BaseDllName.Length) == 0)
            return *module;
    }

    return {};
}
```

**Usage**: `main.cpp:125-131`
```cpp
auto kernelModule = Utils::GetModule(&loaderBlock->LoadOrderListHead, EW(L"ntoskrnl.exe"));
if (!kernelModule.DllBase)
{
    ContextPrint(EW(L"Failed to find ntoskrnl.exe in OslLoaderBlock!\n"));
    INFINITE_LOOP();
}
ContextPrint(EW(L"ntoskrnl.exe -> (virt) 0x%p\n"), kernelModule.DllBase);
```

**Structure**: `ntdef.h:16-45`
```cpp
typedef struct _KLDR_DATA_TABLE_ENTRY
{
    struct _LIST_ENTRY InLoadOrderLinks;
    VOID* ExceptionTable;
    UINT32 ExceptionTableSize;
    VOID* GpValue;
    struct _NON_PAGED_DEBUG_INFO* NonPagedDebugInfo;
    VOID* DllBase;                          // <-- Module base
    VOID* EntryPoint;
    UINT32 SizeOfImage;
    struct _UNICODE_STRING FullDllName;
    struct _UNICODE_STRING BaseDllName;     // <-- Module name
    UINT32 Flags;
    // ...
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
```

**Pattern**:
- Iterate circular linked list (`entry->ForwardLink`)
- Use `CONTAINING_RECORD` to get structure from embedded list entry
- String compare on `BaseDllName.Buffer` (Unicode)

---

## Export Resolution

### GetExport - EAT Parsing
**Source**: `rainbow/UefiDriver/utils.cpp:53-81`

```cpp
UINT64 Utils::GetExport(VOID* base, const CHAR8* functionName)
{
    PIMAGE_DOS_HEADER dosHeader = static_cast<PIMAGE_DOS_HEADER>(base);
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return 0;

    PIMAGE_NT_HEADERS64 ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS64>(
        reinterpret_cast<UINT64>(base) + dosHeader->e_lfanew);

    UINT32 exportsRva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (!exportsRva) return 0;

    PIMAGE_EXPORT_DIRECTORY exports = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
        reinterpret_cast<UINT64>(base) + exportsRva);
    UINT32* nameRva = reinterpret_cast<UINT32*>(
        reinterpret_cast<UINT64>(base) + exports->AddressOfNames);

    for (UINT32 i = 0; i < exports->NumberOfNames; ++i)
    {
        CHAR8* func = reinterpret_cast<CHAR8*>(reinterpret_cast<UINT64>(base) + nameRva[i]);
        if (AsciiStrCmp(func, functionName) == 0)
        {
            UINT32* funcRva = (UINT32*)(reinterpret_cast<UINT64>(base) + exports->AddressOfFunctions);
            UINT16* ordinalRva = (UINT16*)(reinterpret_cast<UINT64>(base) + exports->AddressOfNameOrdinals);

            return reinterpret_cast<UINT64>(base) + funcRva[ordinalRva[i]];
        }
    }

    return 0;
}
```

**Pattern**:
- DOS header validation (`e_magic == 0x5A4D`)
- Navigate: base → NT headers → data directory[0] → export directory
- Arrays: `AddressOfNames[i]` → name string, `AddressOfNameOrdinals[i]` → ordinal index
- Lookup: `AddressOfFunctions[ordinal]` → function RVA
- Return: base + RVA

---

## Kernel Function Location

### IopLoadDriver Hook Target
**Source**: `rainbow/UefiDriver/main.cpp:133-147`

```cpp
UINT64 loadDriverScan = Utils::FindPatternImage(
    kernelModule.DllBase,
    E("E8 ? ? ? ? 33 D2 8B D8 44 8B FA"));
// Pattern: call IopLoadDriver; xor edx, edx; mov ebx, eax; mov r15d, edx

if (!loadDriverScan)
{
    ContextPrint(EW(L"Failed to find reference to IopLoadDriver!\n"));
    INFINITE_LOOP();
}

// Resolve call target: (instruction_end + displacement)
resolvedAddress = (loadDriverScan + 5) + *reinterpret_cast<int*>(loadDriverScan + 1);

Hooks::function = reinterpret_cast<Hooks::IopLoadDriver_t>(resolvedAddress);
Hooks::kernelBase = kernelModule.DllBase;
ContextPrint(EW(L"IopLoadDriver -> (virt) 0x%p\n"), Hooks::function);
```

**Pattern**:
- Find call instruction to `IopLoadDriver` in ntoskrnl
- E8 = relative call instruction (5 bytes total)
- Displacement at offset +1 (4 bytes)
- Target = (call_end_address + signed_displacement)

---

## Virtual Address Mapping

### NotifySetVirtualAddressMap
**Source**: `rainbow/UefiDriver/main.cpp:36-55`

```cpp
EFI_EVENT virtualEvent = NULL;
VOID EFIAPI NotifySetVirtualAddressMap(EFI_EVENT Event, VOID* Context)
{
    // Called when winload sets up virtual addressing
    // Convert physical hook address to virtual using EFI runtime services

    Utils::CopyMemory(Hooks::originalData, Hooks::function, 15);

    // Absolute jump to hook
    UINT8 jump[] = {
        0x48, 0x31, 0xc0,             // xor rax, rax
        0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // movabs rax, imm64
        0xff, 0xe0                     // jmp rax
    };
    *reinterpret_cast<UINT64*>(reinterpret_cast<UINT64>(jump) + 5) =
        reinterpret_cast<UINT64>(GetVirtual(Hooks::HookedIopLoadDriver));

    Utils::CopyMemory(Hooks::newData, jump, 15);
    Hooks::Hook();
}

__forceinline VOID* GetVirtual(VOID* physical)
{
    VOID* address = physical;
    gRT->ConvertPointer(0, &address);  // EFI runtime service
    return address;
}
```

**Pattern**:
- Register for `EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE` in main
- Callback fires when winload calls `SetVirtualAddressMap`
- Use `ConvertPointer` to translate physical addresses to virtual
- Build absolute jump using virtual hook address

**Registration**: `main.cpp:181-182`
```cpp
EFI_STATUS status = gBS->CreateEvent(
    EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE, TPL_NOTIFY, NotifySetVirtualAddressMap, NULL, &virtualEvent);
```

---

## Memory Protection Bypass

### CopyToReadOnly - CR0 WP Bypass
**Source**: `rainbow/UefiDriver/utils.cpp:120-137`

```cpp
VOID Utils::CopyToReadOnly(VOID* destination, VOID* source, UINTN size)
{
    _disable();                     // Disable interrupts (CLI)
    UINT64 cr0 = __readcr0();
    UINT64 oldCr0 = cr0;
    cr0 &= ~(1UL << 16);            // Clear WP bit (bit 16)
    __writecr0(cr0);

    CopyMemory(destination, source, size);

    __writecr0(oldCr0);             // Restore WP bit
    _enable();                      // Enable interrupts (STI)
}
```

**Pattern**:
- CR0.WP (bit 16) = Write Protect supervisor pages
- Clearing WP allows kernel mode writes to read-only pages
- Interrupt disable prevents context switch with WP cleared
- Used for inline hooking of kernel functions

**Usage**: `hooks.cpp:4-6`
```cpp
void Hooks::Hook()
{
    Utils::CopyToReadOnly(function, newData, 15);  // Install hook
}
```

---

## SMBIOS Memory Manipulation

### ZeroSmbiosData - Physical Address Zeroing
**Source**: `rainbow/UefiDriver/hooks.cpp:106-122`

```cpp
EFI_STATUS Hooks::ZeroSmbiosData()
{
    // WmipFindSMBiosStructure -> WmipSMBiosTablePhysicalAddress
    auto* physicalAddress = reinterpret_cast<LARGE_INTEGER*>(
        Utils::FindPatternImage(kernelBase, E("48 8B 0D ? ? ? ? 48 85 C9 74 ? 8B 15")));
    if (!physicalAddress) return EFI_NOT_FOUND;

    // Resolve RIP-relative load
    physicalAddress = reinterpret_cast<LARGE_INTEGER*>(
        reinterpret_cast<char*>(physicalAddress) + 7 +
        *reinterpret_cast<int*>(reinterpret_cast<char*>(physicalAddress) + 3));
    if (!physicalAddress) return EFI_NOT_FOUND;

    Utils::SetMemory(physicalAddress, 0, sizeof(LARGE_INTEGER));

    return EFI_SUCCESS;
}
```

**Pattern**:
- Find instruction loading `WmipSMBiosTablePhysicalAddress` global
- Pattern: `mov rcx, [rip+offset]; test rcx, rcx; je ...`
- Resolve pointer to LARGE_INTEGER holding SMBIOS physical address
- Zero it to prevent SMBIOS table enumeration

---

## Driver Dispatch Manipulation

### ChangeDiskDispatch - Point to Dummy Handler
**Source**: `rainbow/UefiDriver/hooks.cpp:23-65`

```cpp
EFI_STATUS Hooks::ChangeDiskDispatch()
{
    VOID* base = GetModuleBase(EW(L"CLASSPNP.SYS"));
    if (!base) return EFI_NOT_FOUND;

    // ClassMpdevInternalDeviceControl - instantly returns error
    UINT64 scan = Utils::FindPatternImage(base,
        E("40 53 48 83 EC 20 48 8B 41 40 48 8B DA 4C 8B C1 80 B8 ? ? ? ? ? 74 57"));
    if (!scan) return EFI_NOT_FOUND;

    // Get \Driver\Disk object
    static RtlInitUnicodeString_t RtlInitUnicodeString =
        reinterpret_cast<RtlInitUnicodeString_t>(Utils::GetExport(kernelBase, E("RtlInitUnicodeString")));

    UNICODE_STRING targetName;
    RtlInitUnicodeString(&targetName, EW(L"\\Driver\\Disk"));

    static ObReferenceObjectByName_t ObReferenceObjectByName =
        reinterpret_cast<ObReferenceObjectByName_t>(Utils::GetExport(kernelBase, E("ObReferenceObjectByName")));
    static POBJECT_TYPE* IoDriverObjectType =
        reinterpret_cast<POBJECT_TYPE*>(Utils::GetExport(kernelBase, E("IoDriverObjectType")));

    PDRIVER_OBJECT driverObject;
    auto status = ObReferenceObjectByName(&targetName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        nullptr, 0, *IoDriverObjectType, KernelMode, nullptr,
        reinterpret_cast<VOID**>(&driverObject));
    if (status != 0) return EFI_NOT_READY;

    // Replace IRP_MJ_DEVICE_CONTROL with dummy that returns error
    *reinterpret_cast<UINT64*>(&driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]) = scan;

    static ObfDereferenceObject_t ObfDereferenceObject =
        reinterpret_cast<ObfDereferenceObject_t>(Utils::GetExport(kernelBase, E("ObfDereferenceObject")));
    ObfDereferenceObject(driverObject);
    return EFI_SUCCESS;
}
```

**Pattern**:
- Find benign function that immediately returns error (`ClassMpdevInternalDeviceControl`)
- Get reference to `\Driver\Disk` object via `ObReferenceObjectByName`
- Replace `MajorFunction[IRP_MJ_DEVICE_CONTROL]` pointer to dummy handler
- Makes disk serial queries instantly fail (no serial returned)

**Structure**: `ntdef.h:257-274`
```cpp
typedef struct _DRIVER_OBJECT
{
    INT16 Type;
    INT16 Size;
    struct _DEVICE_OBJECT* DeviceObject;
    UINT32 Flags;
    VOID* DriverStart;
    UINT32 DriverSize;
    VOID* DriverSection;
    struct _DRIVER_EXTENSION* DriverExtension;
    struct _UNICODE_STRING DriverName;
    struct _UNICODE_STRING* HardwareDatabase;
    struct _FAST_IO_DISPATCH* FastIoDispatch;
    UINT32(*DriverInit)(struct _DRIVER_OBJECT* arg1, struct _UNICODE_STRING* arg2);
    VOID(*DriverStartIo)(struct _DEVICE_OBJECT* arg1, struct _IRP* arg2);
    VOID(*DriverUnload)(struct _DRIVER_OBJECT* arg1);
    UINT32(*MajorFunction[28])(struct _DEVICE_OBJECT* arg1, struct _IRP* arg2);
} DRIVER_OBJECT, * PDRIVER_OBJECT;
```

### ChangeNetworkDispatch - Walk NDIS Driver List
**Source**: `rainbow/UefiDriver/hooks.cpp:67-104`

```cpp
EFI_STATUS Hooks::ChangeNetworkDispatch()
{
    VOID* base = GetModuleBase(EW(L"ndis.sys"));
    if (!base) return EFI_NOT_FOUND;

    // ndisDummyIrpHandler
    UINT64 scan = Utils::FindPatternImage(base, E("48 8D 05 ? ? ? ? B9 ? ? ? ? 49 8D 7E 70"));
    if (!scan) return EFI_NOT_FOUND;
    scan = reinterpret_cast<UINT64>(RELATIVE_ADDRESS((UINT8*)scan, 7));

    // _NDIS_M_DRIVER_BLOCK* ndisMiniDriverList
    UINT64 listScan = Utils::FindPatternImage(base, E("48 8B 35 ? ? ? ? 44 0F B6 E0"));
    if (!listScan) return EFI_NOT_FOUND;

    bool found = false;
    PNDIS_M_DRIVER_BLOCK block = *static_cast<PNDIS_M_DRIVER_BLOCK*>(RELATIVE_ADDRESS(listScan, 7));
    for (PNDIS_M_DRIVER_BLOCK currentDriver = block; currentDriver;
         currentDriver = static_cast<PNDIS_M_DRIVER_BLOCK>(currentDriver->NextDriver))
    {
        if (!currentDriver->DriverObject) continue;
        if (!currentDriver->DriverObject->MajorFunction) continue;

        *reinterpret_cast<UINT64*>(&currentDriver->DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]) = scan;
        found = true;
    }

    return found ? EFI_SUCCESS : EFI_NOT_FOUND;
}
```

**Pattern**:
- Find `ndisDummyIrpHandler` (existing error-returning dispatch)
- Find `ndisMiniDriverList` global pointer
- Walk linked list of all network drivers
- Replace each driver's `IRP_MJ_DEVICE_CONTROL` with dummy handler

**Structure**: `ntdef.h:278-293`
```cpp
typedef struct _NDIS_M_DRIVER_BLOCK
{
    union
    {
        struct
        {
            VOID* Header;
            VOID* NextDriver;  // Linked list
        };
        struct
        {
            char Space[0x028];
            PDRIVER_OBJECT DriverObject;  // At offset +0x28
        };
    };
} NDIS_M_DRIVER_BLOCK, * PNDIS_M_DRIVER_BLOCK;
```

**Macro**: `ntdef.h:276`
```cpp
#define RELATIVE_ADDRESS(address, size) \
    ((VOID*)((UINT8*)(address) + *(INT32*)((UINT8*)(address) + ((size) - (INT32)sizeof(INT32))) + (size)))
```

---

## Runtime Module Resolution

### GetModuleBase - PsLoadedModuleList Walk
**Source**: `rainbow/UefiDriver/hooks.cpp:13-21`

```cpp
VOID* Hooks::GetModuleBase(const CHAR16* moduleName)
{
    static LIST_ENTRY* PsLoadedModuleList = nullptr;
    if (!PsLoadedModuleList)
        PsLoadedModuleList = *reinterpret_cast<LIST_ENTRY**>(
            Utils::GetExport(kernelBase, E("PsLoadedModuleList")));

    auto moduleInfo = Utils::GetModule(PsLoadedModuleList, moduleName);
    return moduleInfo.DllBase;
}
```

**Pattern**:
- Cache `PsLoadedModuleList` pointer from ntoskrnl exports
- `PsLoadedModuleList` points to kernel's global loaded module list
- Walk list with `GetModule` to find specific driver by name
- Returns base address for later pattern scanning

---

## Hook Lifecycle

### Hook Installation
**Source**: `rainbow/UefiDriver/hooks.cpp:3-11`

```cpp
void Hooks::Hook()
{
    Utils::CopyToReadOnly(function, newData, 15);
}

void Hooks::Unhook()
{
    Utils::CopyToReadOnly(function, originalData, 15);
}
```

**State**: `hooks.h:5-9`
```cpp
inline INT8 originalData[15];  // Original function bytes
inline INT8 newData[15];       // Jump shellcode
typedef INTN(__stdcall* IopLoadDriver_t)(VOID* KeyHandle);
inline IopLoadDriver_t function;  // Hook target
inline VOID* kernelBase;
```

### Hook Handler
**Source**: `rainbow/UefiDriver/hooks.cpp:124-233`

```cpp
INTN Hooks::HookedIopLoadDriver(VOID* KeyHandle)
{
    Unhook();                    // Remove hook temporarily
    INTN status = function(KeyHandle);  // Call original
    Hook();                      // Reinstall hook

    if (status != 0) return status;  // Driver load failed, ignore

    static DbgPrintEx_t DbgPrintEx = nullptr;
    if (!DbgPrintEx)
        DbgPrintEx = reinterpret_cast<DbgPrintEx_t>(Utils::GetExport(kernelBase, E("DbgPrintEx")));

    static bool diskDone = false;
    static bool nicDone = false;
    static bool smbiosDone = false;

    if (!diskDone && ChangeDiskDispatch() == EFI_SUCCESS)
        diskDone = true;

    if (!nicDone && ChangeNetworkDispatch() == EFI_SUCCESS)
        nicDone = true;

    if (!smbiosDone && ZeroSmbiosData() == EFI_SUCCESS)
        smbiosDone = true;

    if (diskDone && nicDone && smbiosDone)
    {
        DbgPrintEx(0, 0, "[efi] All set, exiting\n");
        Unhook();  // Permanent unhook when all targets patched
    }

    return 0;
}
```

**Pattern**:
- Unhook-Call-Rehook for original function execution
- After each driver load, attempt to patch target drivers
- Static flags prevent repeated patching
- Once all targets patched, permanently unhook and exit
- Called during early boot as each driver loads

---

## Key Memory Analysis Techniques

### 1. RIP-Relative Address Resolution
- Pattern: `instruction_end + signed_displacement`
- Common in x64 position-independent code
- Examples: `mov rax, [rip+offset]`, `call [rip+offset]`

### 2. LoaderBlock Structure Walking
- `LOADER_PARAMETER_BLOCK` → module lists
- `LoadOrderListHead` → `KLDR_DATA_TABLE_ENTRY` chain
- Each entry has `DllBase`, `BaseDllName`

### 3. PE Export Table Parsing
- DOS header → NT headers → data directory[0] → exports
- `AddressOfNames` array → name strings
- `AddressOfNameOrdinals` array → ordinal lookup
- `AddressOfFunctions` array → RVA of function

### 4. Pattern Scanning with Wildcards
- Hex byte string with `\?` wildcards
- Linear scan through memory regions
- Used when exact offsets unknown (version independence)

### 5. Physical → Virtual Transition Handling
- Boot starts with physical addressing (EFI firmware context)
- Winload switches to virtual addressing (application context)
- `BlpArchSwitchContext` toggles between contexts
- `SetVirtualAddressMap` event converts addresses

### 6. CR0.WP Bypass for Kernel Patching
- Clear CR0 bit 16 to allow writes to read-only pages
- Required for inline hooks in kernel code
- Must disable interrupts during CR0 manipulation

---

## Relevance to Ombra

**Applicable Patterns**:
1. Boot-time pattern scanning for Hyper-V structures
2. LOADER_PARAMETER_BLOCK analysis for early module location
3. Physical → virtual address translation during boot
4. Export table parsing for runtime function resolution
5. PE section enumeration for targeted scanning
6. Context switching for EFI service access during winload phase

**Not Directly Applicable**:
- HWID spoofing specific logic (disk/NIC dispatch replacement)
- IopLoadDriver hooking (Ombra hooks different targets)

**Key Takeaway**:
Rainbow demonstrates production-quality memory analysis during boot transition phase, showing how to locate and manipulate kernel structures before kernel security features activate. Pattern scanning and address resolution techniques are directly applicable to locating Hyper-V loader structures.
