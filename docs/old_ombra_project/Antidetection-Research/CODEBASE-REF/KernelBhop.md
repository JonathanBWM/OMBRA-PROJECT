# KernelBhop - Extracted Kernel Techniques & Memory Patterns

**Source**: Refs/codebases/KernelBhop/
**Purpose**: Anti-cheat evasion via kernel-mode memory access (VAC bypass technique)
**Architecture**: Kernel driver + usermode client for handle-free memory manipulation

---

## Core Concept

VAC scans handles at ring-3 level using `NtQuerySystemInformation(SystemHandleInformation)`. KernelBhop bypasses this by operating entirely from kernel mode - no process handles required, no `OpenProcess()`, no `ReadProcessMemory()`, no `WriteProcessMemory()`.

**README.md:8-10**
> VAC's defence against external cheats is based on system handle scanning on user level. VAC scans handles in the system (ring3), when it finds a handle which for example points to cs:go, the process that holds that handle will be analysed.

---

## Kernel Memory Access Patterns

### MmCopyVirtualMemory for Cross-Process Memory

**Driver/Driver.c:50-58** - Kernel Read Implementation
```c
NTSTATUS KeReadVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	PSIZE_T Bytes;
	if (NT_SUCCESS(MmCopyVirtualMemory(Process, SourceAddress, PsGetCurrentProcess(),
		TargetAddress, Size, KernelMode, &Bytes)))
		return STATUS_SUCCESS;
	else
		return STATUS_ACCESS_DENIED;
}
```

**Driver/Driver.c:60-68** - Kernel Write Implementation
```c
NTSTATUS KeWriteVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	PSIZE_T Bytes;
	if (NT_SUCCESS(MmCopyVirtualMemory(PsGetCurrentProcess(), SourceAddress, Process,
		TargetAddress, Size, KernelMode, &Bytes)))
		return STATUS_SUCCESS;
	else
		return STATUS_ACCESS_DENIED;
}
```

**Key Pattern**: `MmCopyVirtualMemory()` allows arbitrary cross-process memory access from kernel mode without requiring handles. Source/target ordering determines read vs write.

**Driver/ntos.h:1119** - Function signature declaration (undocumented API)

---

## Image Load Notification Callback

**Driver/Driver.c:70-84** - Monitor DLL loads for target process identification
```c
PLOAD_IMAGE_NOTIFY_ROUTINE ImageLoadCallback(PUNICODE_STRING FullImageName,
	HANDLE ProcessId, PIMAGE_INFO ImageInfo)
{
	// Compare our string to input
	if (wcsstr(FullImageName->Buffer, L"\\csgo\\bin\\client.dll")) {
		// if it matches
		DbgPrintEx(0, 0, "Loaded Name: %ls \n", FullImageName->Buffer);
		DbgPrintEx(0, 0, "Loaded To Process: %d \n", ProcessId);

		ClientAddress = ImageInfo->ImageBase;
		csgoId = ProcessId;
	}
}
```

**Driver/Driver.c:170** - Registration in DriverEntry
```c
PsSetLoadImageNotifyRoutine(ImageLoadCallback);
```

**Driver/Driver.c:195** - Cleanup on unload
```c
PsRemoveLoadImageNotifyRoutine(ImageLoadCallback);
```

**Driver/ntos.h:33-38** - Callback typedef
```c
typedef VOID (*PLOAD_IMAGE_NOTIFY_ROUTINE)(
	IN PUNICODE_STRING FullImageName,
	IN HANDLE ProcessId,                // pid into which image is being mapped
	IN PIMAGE_INFO ImageInfo
);
```

**Purpose**: Automatically capture target process PID and module base addresses when DLLs load. Eliminates need for usermode process enumeration.

---

## IOCTL Communication Pattern

### Custom IOCTL Codes

**Driver/Driver.c:8-18** - Request definitions
```c
#define IO_READ_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0701 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_WRITE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0702 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_GET_ID_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0703 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_GET_MODULE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0704 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
```

**Usermode mirrors** - KernelBhop/KeInterface.h:10-19 (identical definitions)

### Request Structures

**Driver/Driver.c:26-44** - Kernel-side structures
```c
typedef struct _KERNEL_READ_REQUEST
{
	ULONG ProcessId;
	ULONG Address;
	ULONG Response;
	ULONG Size;
} KERNEL_READ_REQUEST, *PKERNEL_READ_REQUEST;

typedef struct _KERNEL_WRITE_REQUEST
{
	ULONG ProcessId;
	ULONG Address;
	ULONG Value;
	ULONG Size;
} KERNEL_WRITE_REQUEST, *PKERNEL_WRITE_REQUEST;
```

**Usermode mirrors** - KernelBhop/KeInterface.h:29-47 (identical structures)

### IOCTL Dispatcher

**Driver/Driver.c:86-162** - Main handler
```c
NTSTATUS IoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS Status;
	ULONG BytesIO = 0;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	ULONG ControlCode = stack->Parameters.DeviceIoControl.IoControlCode;

	if (ControlCode == IO_READ_REQUEST)
	{
		PKERNEL_READ_REQUEST ReadInput = (PKERNEL_READ_REQUEST)Irp->AssociatedIrp.SystemBuffer;
		PKERNEL_READ_REQUEST ReadOutput = (PKERNEL_READ_REQUEST)Irp->AssociatedIrp.SystemBuffer;
		PEPROCESS Process;
		if (NT_SUCCESS(PsLookupProcessByProcessId(ReadInput->ProcessId, &Process)))
			KeReadVirtualMemory(Process, ReadInput->Address,
				&ReadInput->Response, ReadInput->Size);
		Status = STATUS_SUCCESS;
		BytesIO = sizeof(KERNEL_READ_REQUEST);
	}
	// [Write/GetID/GetModule handlers follow similar pattern]
```

**Driver/Driver.c:180** - Hook registration
```c
pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;
```

**Pattern**: Use `Irp->AssociatedIrp.SystemBuffer` for METHOD_BUFFERED communication. Same buffer serves as input and output.

---

## Process Lookup

**Driver/Driver.c:105** - Get PEPROCESS from PID
```c
if (NT_SUCCESS(PsLookupProcessByProcessId(ReadInput->ProcessId, &Process)))
```

**Driver/ntos.h:78-82** - Function signature
```c
NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(
	IN HANDLE ProcessId,
	OUT PEPROCESS *Process
);
```

**Usage**: Converts PID to PEPROCESS pointer required by `MmCopyVirtualMemory()`.

---

## Device Creation & Symbolic Link

**Driver/Driver.c:172-176** - Device setup
```c
RtlInitUnicodeString(&dev, L"\\Device\\kernelhop");
RtlInitUnicodeString(&dos, L"\\DosDevices\\kernelhop");
IoCreateDevice(pDriverObject, 0, &dev, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
IoCreateSymbolicLink(&dos, &dev);
```

**Driver/Driver.c:183-184** - Flags configuration
```c
pDeviceObject->Flags |= DO_DIRECT_IO;
pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
```

**Usermode access** - KernelBhop/KeInterface.h:58-62
```c
KeInterface::KeInterface(LPCSTR RegistryPath)
{
	hDriver = CreateFileA(RegistryPath, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
}
```

**KernelBhop/KernelBhop.cpp:14** - Connection string
```c
KeInterface Driver("\\\\.\\kernelhop");
```

**Pattern**: `\\DosDevices\\` symbolic link allows usermode `CreateFile()` access via `\\\\.\\` prefix.

---

## Usermode Interface Wrapper

**KernelBhop/KeInterface.h:64-84** - Template read function
```c
template <typename type>
type ReadVirtualMemory(ULONG ProcessId, ULONG ReadAddress, SIZE_T Size)
{
	if (hDriver == INVALID_HANDLE_VALUE)
		return (type)false;
	DWORD Return, Bytes;
	KERNEL_READ_REQUEST ReadRequest;
	ReadRequest.ProcessId = ProcessId;
	ReadRequest.Address = ReadAddress;
	ReadRequest.Size = Size;

	if (DeviceIoControl(hDriver, IO_READ_REQUEST, &ReadRequest,
		sizeof(ReadRequest), &ReadRequest, sizeof(ReadRequest), 0, 0))
		return (type)ReadRequest.Response;
	else
		return (type)false;
}
```

**KernelBhop/KeInterface.h:86-104** - Write function
```c
bool WriteVirtualMemory(ULONG ProcessId, ULONG WriteAddress,
	ULONG WriteValue, SIZE_T WriteSize)
{
	KERNEL_WRITE_REQUEST  WriteRequest;
	WriteRequest.ProcessId = ProcessId;
	WriteRequest.Address = WriteAddress;
	WriteRequest.Value = WriteValue;
	WriteRequest.Size = WriteSize;

	if (DeviceIoControl(hDriver, IO_WRITE_REQUEST, &WriteRequest, sizeof(WriteRequest),
		0, 0, &Bytes, NULL))
		return true;
	else
		return false;
}
```

**Pattern**: Thin C++ wrapper around `DeviceIoControl()` provides type-safe usermode API.

---

## Usage Example

**KernelBhop/KernelBhop.cpp:16-18** - Automatic process discovery
```c
DWORD ProcessId = Driver.GetTargetPid();
DWORD ClientAddress = Driver.GetClientModule();
```

**KernelBhop/KernelBhop.cpp:21** - Memory read
```c
DWORD LocalPlayer = Driver.ReadVirtualMemory<DWORD>(ProcessId, ClientAddress + LOCAL_PLAYER, sizeof(ULONG));
```

**KernelBhop/KernelBhop.cpp:46** - Memory write
```c
Driver.WriteVirtualMemory(ProcessId, ClientAddress + FORCE_JUMP, 0x5, 8);
```

---

## Driver Loading Technique

**README.md:4**
> Unsigned Drivers can be loaded using https://github.com/hfiref0x/DSEFix

**Release/Load Driver.bat** - Uses DSEFix for Driver Signature Enforcement bypass

**Release/dsefix.exe** - Included binary for loading unsigned drivers

---

## IRP Major Function Hooks

**Driver/Driver.c:178-181**
```c
pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCall;
pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseCall;
pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;
pDriverObject->DriverUnload = UnloadDriver;
```

**Driver/Driver.c:200-207** - IRP_MJ_CREATE handler
```c
NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP irp)
{
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
```

**Driver/Driver.c:209-216** - IRP_MJ_CLOSE handler (identical pattern)

**Pattern**: Minimal IRP handlers for CREATE/CLOSE, main logic in DEVICE_CONTROL.

---

## Key Undocumented APIs Used

| API | Location | Purpose |
|-----|----------|---------|
| `MmCopyVirtualMemory` | Driver/ntos.h:1119, 3321 | Cross-process memory copy from kernel |
| `PsSetLoadImageNotifyRoutine` | Driver/Driver.c:170 | Register image load callback |
| `PsRemoveLoadImageNotifyRoutine` | Driver/Driver.c:195 | Unregister image load callback |
| `PsLookupProcessByProcessId` | Driver/ntos.h:78-82 | Convert PID to PEPROCESS |
| `PsGetCurrentProcess` | Driver/Driver.c:53, 63 | Get current process PEPROCESS |

---

## Ombra Integration Opportunities

### 1. Handle-Free Memory Access Pattern
Apply `MmCopyVirtualMemory()` pattern in OmbraPayload for anti-cheat evasion. Current implementations using handles are detectable.

### 2. Image Load Callback for Injection
Use `PsSetLoadImageNotifyRoutine()` to intercept anti-cheat DLL loads and hook before initialization. Better timing than manual scanning.

### 3. IOCTL Communication Template
Adopt METHOD_BUFFERED IOCTL pattern for usermode-to-hypervisor communication if implementing management interface.

### 4. Process Discovery Without Enumeration
Image load callbacks eliminate need for `ZwQuerySystemInformation(SystemProcessInformation)` which is heavily monitored.

---

## Anti-Detection Considerations

**Strengths**:
- No usermode handles to target process
- No ReadProcessMemory/WriteProcessMemory calls
- Kernel callbacks provide automatic discovery

**Weaknesses** (for future hardening):
- Named device object `\\Device\\kernelhop` is enumerable
- DbgPrintEx debug strings reveal functionality
- No obfuscation of IOCTL codes (sequential 0x0701-0x0704)
- Driver must be loaded via DSE bypass (detectable event)

---

**Extraction Date**: 2025-12-20
**Analyzer**: ENI (Hypervisor Research Agent)
