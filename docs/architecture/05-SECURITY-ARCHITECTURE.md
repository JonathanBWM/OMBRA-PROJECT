# Security Architecture

## Threat Model

PROJECT-OMBRA operates in an adversarial environment where anti-cheat systems actively attempt to detect hypervisor presence. This document details the security architecture and countermeasures.

## Adversaries

| Adversary | Capability | Detection Methods |
|-----------|------------|-------------------|
| **EasyAntiCheat (EAC)** | Kernel driver + usermode | PML4E scan, BigPool scan, timing analysis |
| **BattlEye** | Kernel driver | Process memory scan, driver enumeration |
| **Vanguard** | Kernel driver (boot) | Early boot check, memory integrity |
| **Windows Defender** | Kernel + cloud | Heuristics, behavioral analysis |

## Detection Vectors and Mitigations

### 1. Timing-Based Detection

**Threat**: VM-exits introduce measurable latency (200-2000 cycles) detectable via TSC/RDTSC.

**Detection Pattern**:
```c
// Anti-cheat timing check
u64 start = __rdtsc();
__cpuid(0, &eax, &ebx, &ecx, &edx);  // Causes VM-exit
u64 end = __rdtsc();
u64 delta = end - start;

if (delta > THRESHOLD) {
    // Hypervisor detected
}
```

**Mitigation**: TSC/APERF/MPERF offset compensation in `timing.c`

```c
// Track VM-exit overhead
void RecordExitTime(VMX_CPU* cpu) {
    u64 exit_tsc = __rdtsc();
    u64 overhead = exit_tsc - cpu->LastEntryTsc;
    cpu->TotalOverhead += overhead;
    cpu->TscOffset -= overhead;  // Compensate
}

// Handle RDTSC exit
void HandleRdtsc(GUEST_CONTEXT* ctx) {
    u64 tsc = __rdtsc() + ctx->Cpu->TscOffset;
    ctx->Rax = (u32)tsc;
    ctx->Rdx = (u32)(tsc >> 32);
}
```

**Effectiveness**: Defeats most timing checks (<100 cycle deviation)

### 2. CPUID Enumeration

**Threat**: Hypervisor bit (CPUID.1.ECX[31]) reveals VMX presence. Known hypervisor leaves (0x40000000+) expose vendor.

**Detection Pattern**:
```c
__cpuid(1, &eax, &ebx, &ecx, &edx);
if (ecx & (1 << 31)) {
    // Hypervisor bit set - detected
}

__cpuid(0x40000000, &eax, &ebx, &ecx, &edx);
// Check for "OMBRAHV" or similar signature
```

**Mitigation**: CPUID spoofing in `handlers/cpuid.c`

```c
void HandleCpuid(GUEST_CONTEXT* ctx) {
    u32 leaf = (u32)ctx->Rax;
    u32 subleaf = (u32)ctx->Rcx;

    // Execute real CPUID
    __cpuid(leaf, &eax, &ebx, &ecx, &edx);

    switch (leaf) {
    case 1:
        // Mask hypervisor bit
        ecx &= ~(1 << 31);
        break;

    case 0x40000000:
        // Return 0 (no hypervisor leaves)
        eax = 0;
        ebx = ecx = edx = 0;
        break;
    }

    ctx->Rax = eax;
    ctx->Rbx = ebx;
    ctx->Rcx = ecx;
    ctx->Rdx = edx;
}
```

**Effectiveness**: Hides from leaf-based detection

### 3. Memory Scanning

**Threat**: EAC scans kernel memory for known signatures, including:
- PE headers in non-driver memory
- Known function prologues
- String patterns ("VMCS", "EPT", hypervisor names)

**Detection Pattern**:
```c
// BigPool scan (~20 min into game)
for_each_big_pool_entry(entry) {
    if (IsExecutable(entry) && !IsBackedByDriver(entry)) {
        // Orphaned executable memory - suspicious
    }
}

// PML4E scan (continuous)
for (int i = 256; i < 512; i++) {  // Kernel half
    if (pml4[i].Present && pml4[i].Execute) {
        if (!HasDriverBacking(pml4[i])) {
            // Detected
        }
    }
}
```

**Mitigation**: EPT split-view hiding

```c
// Hide memory from guest reads
void HideMemoryFromGuest(GPA start, u64 size) {
    for (GPA gpa = start; gpa < start + size; gpa += PAGE_SIZE) {
        // Make page non-present to guest
        EptSetPermissions(gpa, EPT_NONE);

        // But keep hypervisor access via VMX root
        // When hypervisor reads, use different EPT
    }
}
```

**Additional mitigations**:
- Wipe PE headers after loading
- Randomize function prologues
- Obfuscate strings at compile time
- Use EPT execute-only for code pages

### 4. PML4E Enumeration

**Threat**: EAC continuously scans PML4 entries for orphaned executable pages.

**Critical**: This is the highest-priority detection vector.

**Detection Pattern**:
```c
// EAC kernel component
CR3 cr3 = __readcr3();
PML4E* pml4 = PhysToVirt(cr3);

for (int i = 256; i < 512; i++) {
    if (pml4[i].Present) {
        if (!IsKnownDriverMemory(pml4[i])) {
            FlagSuspicious(pml4[i]);
        }
    }
}
```

**Mitigation**: EPT-based page table spoofing

```c
// When guest reads CR3/page tables, show clean version
void HandleCrAccess(GUEST_CONTEXT* ctx) {
    if (ctx->CrNumber == 3 && ctx->AccessType == CR_READ) {
        // Return spoofed CR3 pointing to clean page tables
        ctx->Rax = g_SpoofedCr3;
    }
}

// Alternative: Use EPT to redirect page table reads
void SpoofPageTables(void) {
    // Map guest's view of page tables to clean copy
    EptMapForRead(GUEST_PML4_GPA, CLEAN_PML4_HPA);
}
```

### 5. MSR Enumeration

**Threat**: VMX capability MSRs visible to guest reveal hypervisor support.

**Detection Pattern**:
```c
u64 vmx_basic = __readmsr(IA32_VMX_BASIC);
if (vmx_basic != 0) {
    // VMX capable - could be running hypervisor
}

u64 feature_control = __readmsr(IA32_FEATURE_CONTROL);
if (feature_control & VMX_ENABLE) {
    // VMX enabled - likely active
}
```

**Mitigation**: MSR virtualization in `handlers/msr.c`

```c
void HandleRdmsr(GUEST_CONTEXT* ctx) {
    u32 msr = (u32)ctx->Rcx;

    switch (msr) {
    case IA32_VMX_BASIC:
    case IA32_VMX_PINBASED_CTLS:
    case IA32_VMX_PROCBASED_CTLS:
        // Return 0 - hide VMX capability
        ctx->Rax = 0;
        ctx->Rdx = 0;
        return;

    case IA32_FEATURE_CONTROL:
        // Mask VMX enable bits
        u64 value = __readmsr(msr);
        value &= ~(VMX_ENABLE_OUTSIDE_SMX | VMX_ENABLE_INSIDE_SMX);
        ctx->Rax = (u32)value;
        ctx->Rdx = (u32)(value >> 32);
        return;
    }

    // Pass through other MSRs
    u64 value = __readmsr(msr);
    ctx->Rax = (u32)value;
    ctx->Rdx = (u32)(value >> 32);
}
```

### 6. ETW Tracing

**Threat**: Event Tracing for Windows (ETW) logs driver loading, memory operations.

**Detection Pattern**:
- TI (Threat Intelligence) provider logs suspicious activity
- Circular buffers preserve recent events

**Mitigation**: ETW disabling/wiping in `driver/etw.c`

```c
void DisableEtwTi(void) {
    // Find ETW_REG_ENTRY for TI provider
    PETW_REG_ENTRY entry = FindTiProviderEntry();

    // Clear enable callback
    entry->Callback = NULL;
    entry->Enabled = FALSE;
}

void WipeEtwBuffers(u64 start_time, u64 end_time) {
    // Find circular buffer head
    PETW_BUFFER_HEADER header = GetEtwBufferHeader();

    // Zero events in time range
    for (int i = 0; i < header->BufferCount; i++) {
        PETW_BUFFER buffer = header->Buffers[i];
        WipeEventsInRange(buffer, start_time, end_time);
    }
}
```

### 7. Driver Enumeration

**Threat**: Anti-cheat enumerates loaded drivers, checks for unknown entries.

**Detection Pattern**:
```c
// Walk PsLoadedModuleList
PLIST_ENTRY head = &PsLoadedModuleList;
for (PLIST_ENTRY entry = head->Flink; entry != head; entry = entry->Flink) {
    PLDR_DATA_TABLE_ENTRY ldr = CONTAINING_RECORD(entry, ...);
    if (!IsKnownDriver(ldr->BaseDllName)) {
        FlagUnknownDriver(ldr);
    }
}
```

**Mitigation**: Module hiding in `driver/module_lock.c`

```c
void HideFromModuleList(PLDR_DATA_TABLE_ENTRY our_entry) {
    // Unlink from PsLoadedModuleList
    RemoveEntryList(&our_entry->InLoadOrderLinks);

    // Unlink from MmLoadedUserImageList
    RemoveEntryList(&our_entry->InMemoryOrderLinks);

    // Clear MmUnloadedDrivers entry
    ClearUnloadedDriverEntry(our_entry->BaseDllName);

    // Clear PiDDBCacheTable entry
    ClearDdbCacheEntry(our_entry);
}
```

## Security Boundaries

```
┌─────────────────────────────────────────────────────────────────┐
│                        GUEST (Untrusted)                        │
│                                                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │ Game.exe    │  │ EAC.sys     │  │ Guest Kernel            │  │
│  │ (target)    │  │ (adversary) │  │ (semi-trusted)          │  │
│  └─────────────┘  └─────────────┘  └─────────────────────────┘  │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                  EPT Boundary                            │    │
│  │  Guest sees: Clean page tables, normal memory layout     │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │ VM-Exit (hardware enforced)
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                     HYPERVISOR (Trusted)                        │
│                                                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │ VMX Core    │  │ EPT Tables  │  │ Shadow Hooks            │  │
│  │ (hidden)    │  │ (hidden)    │  │ (execute-only)          │  │
│  └─────────────┘  └─────────────┘  └─────────────────────────┘  │
│                                                                 │
│  Hypervisor controls: All guest memory views, all exits        │
└─────────────────────────────────────────────────────────────────┘
```

## BYOVD Security

### Driver Chain Trust

```
Loader.exe (usermode)
    │
    ├── Ld9BoxSup.sys (VirtualBox fork)
    │   ├── Signed by: LDPlayer (code signing cert)
    │   ├── Trust: Medium (legitimate software)
    │   ├── Risk: HIGH (kernel code execution primitive)
    │   └── IOCTLs: SUP_IOCTL_LDR_LOAD allows arbitrary code
    │
    └── ThrottleStop (optional)
        ├── Signed by: Microsoft (kernel signing)
        ├── Trust: High (legitimate utility)
        ├── Risk: MEDIUM (physical memory R/W)
        └── Use: Patch validation flags only
```

### Post-Exploitation Cleanup

After hypervisor loads:

1. **Unlink from driver lists**: PsLoadedModuleList, MmLoadedUserImageList
2. **Clear unload records**: MmUnloadedDrivers linked list
3. **Clear DDB cache**: PiDDBCacheTable hash bucket
4. **Wipe ETW**: Circular buffers in loading timeframe
5. **Delete artifacts**: Prefetch files (if applicable)

## Operational Security

### Development Guidelines

1. **Never commit secrets**: No magic values, keys, or offsets in git
2. **Obfuscate at compile**: Use compile-time string encryption
3. **Randomize signatures**: Function prologues, magic values
4. **Validate inputs**: All VMCALL parameters sanitized
5. **Fail safely**: Invalid operations return error, don't crash

### Deployment Guidelines

1. **Test on clean VM first**: Ensure no crashes
2. **Monitor for detection**: Check for EAC/BE bans
3. **Rotate signatures**: Update magic values periodically
4. **Keep driver updated**: Match game/anti-cheat versions

## Related Documents

- [ADR-010](./adr/ADR-010-timing-compensation.md) - Timing compensation
- [ADR-011](./adr/ADR-011-ept-split-view.md) - EPT split-view
- [ADR-012](./adr/ADR-012-cpuid-spoofing.md) - CPUID spoofing
- [03-C4-COMPONENTS.md](./03-C4-COMPONENTS.md) - Security components
