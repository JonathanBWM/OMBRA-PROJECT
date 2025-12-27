# Loader Requirements for OmbraHypervisor

This document specifies the exact memory allocations and initialization parameters required by the usermode loader before calling `OmbraInitialize()`.

## Overview

The hypervisor kernel component (`hypervisor.sys`) does NOT allocate its own memory. All physical memory regions must be pre-allocated by the usermode loader and passed via the `HV_INIT_PARAMS` structure.

## Memory Allocation Requirements

### Per-CPU Resources

Each CPU requires the following allocations:

#### 1. VMXON Region
- **Size:** 4096 bytes (4 KB)
- **Alignment:** 4096 bytes (4 KB aligned)
- **Type:** Physically contiguous, non-paged
- **Init Param Fields:**
  - `VmxonPhysical` - Physical address
  - `VmxonVirtual` - Virtual address (kernel VA)

#### 2. VMCS Region
- **Size:** 4096 bytes (4 KB)
- **Alignment:** 4096 bytes (4 KB aligned)
- **Type:** Physically contiguous, non-paged
- **Init Param Fields:**
  - `VmcsPhysical` - Physical address
  - `VmcsVirtual` - Virtual address (kernel VA)

#### 3. Host Stack
- **Size:** 65536 bytes (64 KB) recommended
- **Alignment:** 16 bytes (x64 stack alignment)
- **Type:** Non-paged, must NOT be freed during hypervisor operation
- **Init Param Fields:**
  - `HostStackTop` - Virtual address of stack TOP (grows downward)

**Critical:** The stack grows downward from `HostStackTop`. Allocate the full 64 KB, then pass the address of the HIGHEST byte as `HostStackTop`.

#### 4. MSR Bitmap
- **Size:** 4096 bytes (4 KB)
- **Alignment:** 4096 bytes (4 KB aligned)
- **Type:** Physically contiguous, non-paged
- **Init Param Fields:**
  - `MsrBitmapPhysical` - Physical address
  - `MsrBitmapVirtual` - Virtual address (kernel VA)

**Usage:** Controls which MSR accesses cause VM-exits. The hypervisor will configure this bitmap during initialization.

### Shared Resources (System-Wide)

#### 5. EPT PML4 Table
- **Size:** 4096 bytes (4 KB)
- **Alignment:** 4096 bytes (4 KB aligned)
- **Type:** Physically contiguous, non-paged
- **Init Param Fields:**
  - `EptPml4Physical` - Physical address
  - `EptPml4Virtual` - Virtual address (kernel VA)

**Usage:** All CPUs share a single EPT structure. Allocate ONCE and pass to all CPUs.

#### 6. Shadow VMCS Resources (for Nested Virtualization)

These are OPTIONAL but REQUIRED for Hyper-V coexistence (ZeroHVCI scenario):

##### 6a. VMREAD Bitmap
- **Size:** 4096 bytes (4 KB)
- **Alignment:** 4096 bytes (4 KB aligned)
- **Type:** Physically contiguous, non-paged
- **Init Param Fields:**
  - `VmreadBitmapPhysical` - Physical address
  - `VmreadBitmapVirtual` - Virtual address (kernel VA)

**Usage:** Controls which VMREAD instructions cause VM-exits when VMCS shadowing is enabled. Each bit corresponds to a VMCS field encoding.

##### 6b. VMWRITE Bitmap
- **Size:** 4096 bytes (4 KB)
- **Alignment:** 4096 bytes (4 KB aligned)
- **Type:** Physically contiguous, non-paged
- **Init Param Fields:**
  - `VmwriteBitmapPhysical` - Physical address
  - `VmwriteBitmapVirtual` - Virtual address (kernel VA)

**Usage:** Controls which VMWRITE instructions cause VM-exits when VMCS shadowing is enabled.

##### 6c. Shadow VMCS Region
- **Size:** 4096 bytes (4 KB)
- **Alignment:** 4096 bytes (4 KB aligned)
- **Type:** Physically contiguous, non-paged
- **Init Param Fields:**
  - `ShadowVmcsPhysical` - Physical address
  - `ShadowVmcsVirtual` - Virtual address (kernel VA)

**Usage:** Shadow VMCS used for L2 guest state when running under an L0 hypervisor (Hyper-V).

**Note:** If these are not provided (NULL/0), nested virtualization features will be unavailable but the hypervisor will still function on bare metal.

#### 7. Debug Buffer (Optional)
- **Size:** Configurable (recommended 1 MB = 1048576 bytes)
- **Alignment:** Page-aligned (4096 bytes)
- **Type:** Non-paged
- **Init Param Fields:**
  - `DebugBufferPhysical` - Physical address
  - `DebugBufferVirtual` - Virtual address (kernel VA)
  - `DebugBufferSize` - Size in bytes

**Usage:** Ring buffer for debug logging. Optional but highly recommended for development.

## HV_INIT_PARAMS Structure

```c
typedef struct _HV_INIT_PARAMS {
    // CPU identification
    U32     CpuId;              // Logical CPU index (0 to NumCpus-1)
    U32     TotalCpus;          // Total number of CPUs in system

    // Per-CPU physical addresses
    U64     VmxonPhysical;
    U64     VmcsPhysical;
    U64     HostStackTop;       // Virtual address of stack top
    U64     MsrBitmapPhysical;

    // Per-CPU virtual addresses
    void*   VmxonVirtual;
    void*   VmcsVirtual;
    void*   MsrBitmapVirtual;

    // Shared EPT (same for all CPUs)
    U64     EptPml4Physical;
    void*   EptPml4Virtual;

    // Shadow VMCS resources (shared, for nested virtualization)
    U64     VmreadBitmapPhysical;   // 4KB, controls VMREAD shadowing
    U64     VmwriteBitmapPhysical;  // 4KB, controls VMWRITE shadowing
    U64     ShadowVmcsPhysical;     // 4KB, shadow VMCS for L2 guests
    void*   VmreadBitmapVirtual;
    void*   VmwriteBitmapVirtual;
    void*   ShadowVmcsVirtual;

    // VMX capability MSRs (pre-read by loader)
    U64     VmxBasic;
    U64     VmxPinCtls;
    U64     VmxProcCtls;
    U64     VmxProcCtls2;
    U64     VmxExitCtls;
    U64     VmxEntryCtls;
    U64     VmxTruePin;
    U64     VmxTrueProc;
    U64     VmxTrueExit;
    U64     VmxTrueEntry;
    U64     VmxCr0Fixed0;
    U64     VmxCr0Fixed1;
    U64     VmxCr4Fixed0;
    U64     VmxCr4Fixed1;
    U64     VmxEptVpidCap;

    // Exit handler address
    U64     VmexitHandler;      // Physical address of VmexitHandler function

    // Debug infrastructure (optional)
    U64     DebugBufferPhysical;
    void*   DebugBufferVirtual;
    U64     DebugBufferSize;
} HV_INIT_PARAMS;
```

## Memory Allocation via Ld9BoxSup.sys

The loader uses the Ld9BoxSup.sys vulnerable driver to allocate physical memory from usermode.

### Recommended Allocation Sequence

```c
// For each CPU (0 to NumCpus-1):
{
    // 1. VMXON region
    void* vmxonVa = AllocatePhysicalMemory(4096, 4096);
    U64 vmxonPhys = GetPhysicalAddress(vmxonVa);

    // 2. VMCS region
    void* vmcsVa = AllocatePhysicalMemory(4096, 4096);
    U64 vmcsPhys = GetPhysicalAddress(vmcsVa);

    // 3. Host stack (64 KB)
    void* stackBase = AllocatePhysicalMemory(65536, 16);
    void* stackTop = (void*)((ULONG_PTR)stackBase + 65536);  // TOP of stack

    // 4. MSR bitmap
    void* msrBitmapVa = AllocatePhysicalMemory(4096, 4096);
    U64 msrBitmapPhys = GetPhysicalAddress(msrBitmapVa);
}

// ONCE for the whole system (shared EPT):
{
    void* eptPml4Va = AllocatePhysicalMemory(4096, 4096);
    U64 eptPml4Phys = GetPhysicalAddress(eptPml4Va);
}

// ONCE for nested virtualization (optional):
{
    void* vmreadBitmapVa = AllocatePhysicalMemory(4096, 4096);
    U64 vmreadBitmapPhys = GetPhysicalAddress(vmreadBitmapVa);

    void* vmwriteBitmapVa = AllocatePhysicalMemory(4096, 4096);
    U64 vmwriteBitmapPhys = GetPhysicalAddress(vmwriteBitmapVa);

    void* shadowVmcsVa = AllocatePhysicalMemory(4096, 4096);
    U64 shadowVmcsPhys = GetPhysicalAddress(shadowVmcsVa);
}

// Debug buffer (optional):
{
    void* debugVa = AllocatePhysicalMemory(1048576, 4096);  // 1 MB
    U64 debugPhys = GetPhysicalAddress(debugVa);
}
```

## Per-CPU Initialization Flow

The loader must call `OmbraInitialize()` on EACH CPU via `DrvExecuteOnCpu`:

```c
for (int cpu = 0; cpu < NumCpus; cpu++) {
    HV_INIT_PARAMS params = {0};

    // Set CPU identification
    params.CpuId = cpu;
    params.TotalCpus = NumCpus;

    // Set per-CPU addresses
    params.VmxonPhysical = vmxonPhys[cpu];
    params.VmxonVirtual = vmxonVa[cpu];
    params.VmcsPhysical = vmcsPhys[cpu];
    params.VmcsVirtual = vmcsVa[cpu];
    params.HostStackTop = stackTop[cpu];
    params.MsrBitmapPhysical = msrBitmapPhys[cpu];
    params.MsrBitmapVirtual = msrBitmapVa[cpu];

    // Set shared EPT (same for all CPUs)
    params.EptPml4Physical = eptPml4Phys;
    params.EptPml4Virtual = eptPml4Va;

    // Set shared shadow VMCS resources (same for all CPUs)
    params.VmreadBitmapPhysical = vmreadBitmapPhys;
    params.VmreadBitmapVirtual = vmreadBitmapVa;
    params.VmwriteBitmapPhysical = vmwriteBitmapPhys;
    params.VmwriteBitmapVirtual = vmwriteBitmapVa;
    params.ShadowVmcsPhysical = shadowVmcsPhys;
    params.ShadowVmcsVirtual = shadowVmcsVa;

    // Set VMX MSRs (read once, passed to all CPUs)
    params.VmxBasic = ReadMsr(MSR_IA32_VMX_BASIC);
    params.VmxPinCtls = ReadMsr(MSR_IA32_VMX_PINBASED_CTLS);
    // ... (read all VMX MSRs)

    // Set exit handler address
    params.VmexitHandler = GetPhysicalAddress(&VmexitHandler);

    // Set debug buffer (shared across CPUs)
    if (cpu == 0) {
        params.DebugBufferPhysical = debugPhys;
        params.DebugBufferVirtual = debugVa;
        params.DebugBufferSize = 1048576;
    }

    // Execute initialization on target CPU
    DrvExecuteOnCpu(cpu, &OmbraInitialize, &params, sizeof(params));
}
```

## Nested Virtualization Behavior

When shadow VMCS resources are provided:

1. **CPU 0** calls `NestedDetectHypervisor()` during init
2. If running under Hyper-V:
   - Shadow bitmaps are cleared (all 0s = shadow all fields)
   - VMCS shadowing is enabled in secondary proc-based controls
   - Shadow VMCS region is prepared for L2 guest launch
3. Guest VMX instructions (VMREAD/VMWRITE) are intercepted:
   - If shadow VMCS is active: Hardware handles via shadow
   - If bit is set in bitmap: VM-exit to our handler
   - If no shadow resources: Always inject #UD

When shadow VMCS resources are NOT provided (NULL/0):

1. `NestedDetectHypervisor()` still runs but logs warnings
2. VMCS shadowing remains disabled
3. All VMX instructions from guest inject #UD
4. Hypervisor functions normally on bare metal

## Critical Notes

### Memory Lifetime
- All allocated memory MUST remain valid for the entire hypervisor lifetime
- Do NOT free these allocations until AFTER `OmbraShutdown()` on all CPUs
- Memory must be pinned (non-paged) - swapping to disk will cause triple faults

### Physical Address Translation
- The loader is responsible for translating VA to PA
- Use `MmGetPhysicalAddress()` or Ld9BoxSup IOCTL to get physical addresses
- Ensure addresses are BELOW 4 GB if VMX has restrictions (check VMXON region address width)

### Stack Alignment
- Host stack MUST be 16-byte aligned (x64 ABI requirement)
- VMCS expects stack to grow downward from `HostStackTop`
- Recommended size: 64 KB per CPU

### VMCS Revision ID
- The hypervisor reads `MSR_IA32_VMX_BASIC` to get the revision ID
- Writes it to the first DWORD of VMXON/VMCS regions
- Loader does NOT need to pre-write this

### VMX MSRs
- Pre-reading MSRs in usermode avoids kernel RDMSR calls
- Required MSRs are listed in `HV_INIT_PARAMS`
- All MSRs in the 0x480-0x491 range should be read

## ZeroHVCI Scenario

When deploying via ZeroHVCI (Hyper-V hijacking):

1. Allocate shadow VMCS resources (REQUIRED)
2. The hypervisor will detect Hyper-V at runtime
3. Shadow VMCS will be configured for L0/L1 coexistence
4. VMX instruction forwarding to L0 is handled automatically
5. Timing compensation is applied to avoid detection

## Testing Checklist

Before calling `OmbraInitialize()`:

- [ ] All per-CPU allocations are 4 KB aligned
- [ ] All allocations are physically contiguous
- [ ] All allocations are non-paged
- [ ] Host stack is 64 KB and 16-byte aligned
- [ ] `HostStackTop` points to TOP of stack (highest address)
- [ ] Physical addresses are correctly translated from virtual
- [ ] Shared resources (EPT, shadow VMCS) use SAME pointers for all CPUs
- [ ] VMX MSRs are read and populated
- [ ] `VmexitHandler` address is translated to physical
- [ ] `CpuId` ranges from 0 to `TotalCpus-1`

## Common Errors

### VMXON Failure
- **Symptom:** `VMXON failed (error 1)`
- **Cause:** Physical address > 4 GB on systems with limited VMX support
- **Fix:** Allocate below 4 GB or check `MSR_IA32_VMX_BASIC[48]` for address width

### VMLAUNCH Failure (Error Code 7)
- **Symptom:** `VMLAUNCH failed - error code 7` (Invalid host state)
- **Cause:** Host stack not aligned or invalid RIP/RSP
- **Fix:** Ensure `HostStackTop` is 16-byte aligned and within allocated region

### Triple Fault on VM-Exit
- **Symptom:** System crashes immediately after VMLAUNCH
- **Cause:** Host stack was paged out or invalid
- **Fix:** Ensure stack is non-paged and locked in memory

### Shadow VMCS Not Working
- **Symptom:** VMX instructions still cause #UD under Hyper-V
- **Cause:** Shadow resources not allocated or physical addresses are NULL
- **Fix:** Verify all three shadow resources are allocated and passed correctly

## Example Memory Layout

```
Per-CPU Resources (multiply by NumCpus):
+-------------------+ <- vmxonVa
|  VMXON Region     | 4 KB (PA: vmxonPhys)
+-------------------+
|  VMCS Region      | 4 KB (PA: vmcsPhys)
+-------------------+
|  MSR Bitmap       | 4 KB (PA: msrBitmapPhys)
+-------------------+
|                   |
|  Host Stack       | 64 KB (grows downward)
|                   |
+-------------------+ <- stackTop (HostStackTop points here)

Shared Resources (allocated once):
+-------------------+
|  EPT PML4         | 4 KB (PA: eptPml4Phys)
+-------------------+
|  VMREAD Bitmap    | 4 KB (PA: vmreadBitmapPhys)
+-------------------+
|  VMWRITE Bitmap   | 4 KB (PA: vmwriteBitmapPhys)
+-------------------+
|  Shadow VMCS      | 4 KB (PA: shadowVmcsPhys)
+-------------------+
|                   |
|  Debug Buffer     | 1 MB (PA: debugPhys)
|                   |
+-------------------+
```

Total memory per system:
- Per-CPU: ~73 KB per CPU
- Shared: ~1.01 MB
- **Example for 8 CPUs:** (73 * 8) + 1024 = **1,608 KB (~1.6 MB)**

---

**This document must be followed EXACTLY by the loader implementation.**
**Any deviation will result in VMLAUNCH failure, triple faults, or hypervisor instability.**
