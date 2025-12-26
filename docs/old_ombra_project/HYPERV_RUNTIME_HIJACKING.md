# HYPER-V RUNTIME HIJACKING
## Stealth VMExit Handler Patching with Timing Compensation

---

## OVERVIEW

Hyper-V runtime hijacking allows injecting custom VMexit handling into an already-running Hyper-V hypervisor. This approach:

1. **Leverages existing virtualization** - No need to fight Hyper-V, work with it
2. **VBS/HVCI compatible** - Works even with Secure Boot and Credential Guard
3. **No reboot required** - Hot-patch at runtime
4. **Inherits Hyper-V's legitimacy** - CPUID already shows Hyper-V present

---

## PREREQUISITES

### ZeroHVCI Kernel R/W
Using CVE-2024-26229 or CVE-2024-35250 for arbitrary kernel read/write:

```cpp
// ZeroHVCI provides these primitives
UINT64 KernelRead64(UINT64 Address);
VOID KernelWrite64(UINT64 Address, UINT64 Value);
VOID KernelReadBuffer(UINT64 Address, PVOID Buffer, SIZE_T Size);
VOID KernelWriteBuffer(UINT64 Address, PVOID Buffer, SIZE_T Size);
```

### Hyper-V Components

| Component | Location | Purpose |
|-----------|----------|---------|
| hvix64.exe | SecureKernel VTL1 | Intel VT-x hypervisor core |
| hvax64.exe | SecureKernel VTL1 | AMD-V hypervisor core |
| hvloader.dll | Boot | Hypervisor loader |
| securekernel.exe | VTL1 | Secure kernel for VBS |

---

## PHASE 1: HYPER-V STRUCTURE DISCOVERY

### Finding hvix64.exe Base

```cpp
// Hyper-V stores info in hypercall page and MSRs
UINT64 FindHvix64Base() {
    // Method 1: Parse MSR 0x40000000 hypercall page
    UINT64 HypercallPage = __readmsr(HV_X64_MSR_HYPERCALL);
    if (HypercallPage & 1) {  // Enabled
        UINT64 Pfn = HypercallPage >> 12;
        // Hypercall page has pointer to hypervisor
    }
    
    // Method 2: Scan for hvix64.exe signature in high memory
    // Hyper-V runs in isolated memory region
    for (UINT64 Addr = 0xFFFFF80000000000; Addr < 0xFFFFFFFFFFFFFFFF; Addr += 0x1000) {
        __try {
            if (IsValidHvix64Header(Addr)) {
                return Addr;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            continue;
        }
    }
    
    // Method 3: Use undocumented hypercall to leak base
    return 0;
}

BOOLEAN IsValidHvix64Header(UINT64 Addr) {
    PIMAGE_DOS_HEADER Dos = (PIMAGE_DOS_HEADER)Addr;
    if (Dos->e_magic != IMAGE_DOS_SIGNATURE) return FALSE;
    
    PIMAGE_NT_HEADERS64 Nt = (PIMAGE_NT_HEADERS64)(Addr + Dos->e_lfanew);
    if (Nt->Signature != IMAGE_NT_SIGNATURE) return FALSE;
    
    // Check for hvix64 specific characteristics
    // TimeDateStamp, SizeOfImage, etc.
    return TRUE;
}
```

### Locating VMexit Handler

The VMexit handler in hvix64.exe can be found by:

1. **Pattern scanning for VMREAD (0F 78)**
2. **Following the VMCS_HOST_RIP value**
3. **Parsing hypercall dispatch table**

```cpp
typedef struct _HYPERV_VMEXIT_HANDLER {
    UINT64 Address;
    UINT32 Size;
    UINT8 OriginalBytes[64];
} HYPERV_VMEXIT_HANDLER;

UINT64 FindVmexitHandler(UINT64 HvBase) {
    // Scan .text section for VMexit handler pattern
    PIMAGE_NT_HEADERS64 Nt = GetNtHeaders(HvBase);
    PIMAGE_SECTION_HEADER Text = FindSection(Nt, ".text");
    
    UINT64 TextStart = HvBase + Text->VirtualAddress;
    UINT64 TextEnd = TextStart + Text->Misc.VirtualSize;
    
    // Pattern: VMexit handler prologue
    // Common pattern: Save registers, read exit reason
    UCHAR Pattern[] = {
        0x0F, 0x78,       // vmread
        0xCC, 0xCC,       // wildcards
        0x48, 0x89,       // mov [reg], ...
    };
    
    for (UINT64 Addr = TextStart; Addr < TextEnd - sizeof(Pattern); Addr++) {
        if (MatchPattern(Addr, Pattern, sizeof(Pattern))) {
            // Validate this is the handler
            if (ValidateVmexitHandler(Addr)) {
                return Addr;
            }
        }
    }
    
    return 0;
}

// Alternative: Read from VMCS directly if we can
UINT64 GetVmexitHandlerFromVmcs() {
    // VMCS_HOST_RIP contains the VMexit entry point
    // We need to execute in VMX root mode to read this
    // Or find where Hyper-V stores its VMCS
    return 0;
}
```

---

## PHASE 2: HOOK INSTALLATION

### Hook Trampoline Design

```cpp
// Our hook handler - runs in VTL1 context!
typedef struct _VMEXIT_HOOK_CONTEXT {
    UINT64 OriginalHandler;
    UINT64 TscOffset;
    UINT64 AccumulatedOverhead;
    BOOLEAN TimingEnabled;
} VMEXIT_HOOK_CONTEXT;

// Shellcode for VMexit hook (must be position-independent)
__declspec(allocate(".hvhook")) UCHAR VmexitHookShellcode[] = {
    // Save registers
    0x50,                               // push rax
    0x51,                               // push rcx
    0x52,                               // push rdx
    0x41, 0x50,                         // push r8
    
    // Read entry TSC
    0x0F, 0x31,                         // rdtsc
    0x48, 0xC1, 0xE2, 0x20,             // shl rdx, 32
    0x48, 0x09, 0xD0,                   // or rax, rdx
    0x49, 0x89, 0xC0,                   // mov r8, rax  (entry TSC in r8)
    
    // Read exit reason
    0xB9, 0x06, 0x44, 0x00, 0x00,       // mov ecx, VMCS_EXIT_REASON
    0x0F, 0x78, 0xC8,                   // vmread rax, rcx
    
    // Check if CPUID exit (reason 10)
    0x3D, 0x0A, 0x00, 0x00, 0x00,       // cmp eax, 10
    0x75, 0x30,                         // jne skip_compensation
    
    // Apply TSC offset compensation for CPUID
    // Read current TSC offset from VMCS
    0xB9, 0x10, 0x20, 0x00, 0x00,       // mov ecx, VMCS_TSC_OFFSET
    0x0F, 0x78, 0xC8,                   // vmread rax, rcx
    
    // Subtract ~1200 cycles (Intel typical overhead)
    0x48, 0x2D, 0xB0, 0x04, 0x00, 0x00, // sub rax, 1200
    
    // Write back adjusted TSC offset
    0x0F, 0x79, 0xC8,                   // vmwrite rcx, rax
    
    // skip_compensation:
    // Restore registers
    0x41, 0x58,                         // pop r8
    0x5A,                               // pop rdx
    0x59,                               // pop rcx
    0x58,                               // pop rax
    
    // Jump to original handler
    0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, // jmp [rip+0]
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // original handler address
};

NTSTATUS InstallVmexitHook(UINT64 HvBase) {
    // Find VMexit handler
    UINT64 Handler = FindVmexitHandler(HvBase);
    if (!Handler) {
        return STATUS_NOT_FOUND;
    }
    
    // Find cave in hvix64.exe for our shellcode
    UINT64 CodeCave = FindCodeCave(HvBase, sizeof(VmexitHookShellcode));
    if (!CodeCave) {
        // Alternative: Allocate in hypervisor memory space
        CodeCave = AllocateHvMemory(sizeof(VmexitHookShellcode));
    }
    
    // Patch shellcode with original handler address
    *(PUINT64)(VmexitHookShellcode + sizeof(VmexitHookShellcode) - 8) = Handler + 14;
    
    // Write shellcode to code cave
    KernelWriteBuffer(CodeCave, VmexitHookShellcode, sizeof(VmexitHookShellcode));
    
    // Save original bytes
    UCHAR OriginalBytes[14];
    KernelReadBuffer(Handler, OriginalBytes, 14);
    
    // Install jump to our hook
    UCHAR JmpHook[14] = {
        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,  // jmp [rip+0]
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    *(PUINT64)(JmpHook + 6) = CodeCave;
    
    // Atomic write hook (must not be interrupted)
    // Use interlocked compare exchange for 16-byte atomicity
    AtomicWrite16(Handler, JmpHook);
    
    return STATUS_SUCCESS;
}
```

---

## PHASE 3: TSC COMPENSATION

### Per-Exit Type Calibration

```cpp
typedef struct _EXIT_TIMING_PROFILE {
    UINT32 ExitReason;
    UINT64 NativeTime;       // Expected native execution time
    UINT64 OverheadEstimate; // Estimated VM-exit overhead
} EXIT_TIMING_PROFILE;

EXIT_TIMING_PROFILE g_TimingProfiles[] = {
    { EXIT_REASON_CPUID,       150,   1200 },  // CPUID: 100-200 cycles native
    { EXIT_REASON_MSR_READ,    30,    800  },  // MSR: ~30 cycles native
    { EXIT_REASON_MSR_WRITE,   30,    800  },
    { EXIT_REASON_EPT_VIOLATION, 0,   1500 },  // Varies widely
    { EXIT_REASON_IO,          100,   1000 },
    { EXIT_REASON_HLT,         0,     500  },
    { EXIT_REASON_MTF,         0,     400  },
    { 0xFFFFFFFF, 0, 0 }  // Sentinel
};

UINT64 GetTimingCompensation(UINT32 ExitReason) {
    for (INT i = 0; g_TimingProfiles[i].ExitReason != 0xFFFFFFFF; i++) {
        if (g_TimingProfiles[i].ExitReason == ExitReason) {
            return g_TimingProfiles[i].OverheadEstimate - 
                   g_TimingProfiles[i].NativeTime;
        }
    }
    return 0;
}

// Enhanced hook with per-exit compensation
__declspec(naked) VOID EnhancedVmexitHook() {
    __asm {
        // Prolog - save all volatile registers
        push rax
        push rcx
        push rdx
        push r8
        push r9
        push r10
        push r11
        
        // Get entry timestamp
        rdtsc
        shl rdx, 32
        or rax, rdx
        mov r10, rax        // r10 = entry TSC
        
        // Read exit reason
        mov ecx, 0x4402     // VMCS_VM_EXIT_REASON
        vmread rax, rcx
        and eax, 0xFFFF     // Mask to get basic exit reason
        mov r11d, eax       // r11 = exit reason
        
        // Look up compensation value
        lea rcx, [g_TimingProfiles]
    lookup_loop:
        mov eax, [rcx]
        cmp eax, 0xFFFFFFFF
        je no_compensation
        cmp eax, r11d
        je found_profile
        add rcx, 24         // sizeof(EXIT_TIMING_PROFILE)
        jmp lookup_loop
        
    found_profile:
        // Get overhead estimate
        mov r8, [rcx + 16]  // OverheadEstimate
        sub r8, [rcx + 8]   // - NativeTime
        
        // Read current TSC offset
        mov ecx, 0x2010     // VMCS_TSC_OFFSET
        vmread rax, rcx
        
        // Subtract compensation
        sub rax, r8
        
        // Write back
        vmwrite rcx, rax
        
    no_compensation:
        // Restore registers
        pop r11
        pop r10
        pop r9
        pop r8
        pop rdx
        pop rcx
        pop rax
        
        // Jump to original handler
        jmp [OriginalHandler]
    }
}
```

---

## PHASE 4: APERF/MPERF COMPENSATION

APERF-based timing detection is harder to defeat. Must intercept MSR reads:

```cpp
// MSR interception for APERF (0xE8) and MPERF (0xE7)
VOID ConfigureMsrInterception(UINT64 HvBase) {
    // Find MSR bitmap in Hyper-V structures
    UINT64 MsrBitmap = FindHyperVMsrBitmap(HvBase);
    
    if (MsrBitmap) {
        // Enable interception of APERF and MPERF reads
        // MSR 0xE7 (MPERF) and 0xE8 (APERF) are in low range
        
        // Read bitmap (bit position = MSR number for low range)
        UINT8 Byte_E7 = KernelRead8(MsrBitmap + (0xE7 / 8));
        UINT8 Byte_E8 = KernelRead8(MsrBitmap + (0xE8 / 8));
        
        // Set bits to intercept reads
        Byte_E7 |= (1 << (0xE7 % 8));
        Byte_E8 |= (1 << (0xE8 % 8));
        
        KernelWrite8(MsrBitmap + (0xE7 / 8), Byte_E7);
        KernelWrite8(MsrBitmap + (0xE8 / 8), Byte_E8);
    }
}

// In VMexit hook, handle APERF/MPERF reads
VOID HandleAperfMperfRead(PVCPU Vcpu) {
    UINT32 Msr = (UINT32)Vcpu->Rcx;
    UINT64 Value;
    
    // Read actual value
    Value = __readmsr(Msr);
    
    // Subtract accumulated VM-exit overhead
    // APERF counts at CPU frequency only in C0 state
    // Our overhead is real work, so subtract it
    if (g_AccumulatedOverhead > 0) {
        if (Value > g_AccumulatedOverhead) {
            Value -= g_AccumulatedOverhead;
        }
    }
    
    Vcpu->Rax = Value & 0xFFFFFFFF;
    Vcpu->Rdx = Value >> 32;
}
```

---

## PHASE 5: INTEGRATION WITH SPUTNIK PAYLOAD

### Memory Read/Write via Hypercall

```cpp
// Define custom hypercall for cheat operations
#define HV_CHEAT_HYPERCALL_BASE 0xC0DE0000
#define HV_CHEAT_READ_MEMORY    (HV_CHEAT_HYPERCALL_BASE + 0)
#define HV_CHEAT_WRITE_MEMORY   (HV_CHEAT_HYPERCALL_BASE + 1)
#define HV_CHEAT_GET_CR3        (HV_CHEAT_HYPERCALL_BASE + 2)
#define HV_CHEAT_GET_EPROCESS   (HV_CHEAT_HYPERCALL_BASE + 3)

typedef struct _CHEAT_REQUEST {
    UINT64 ProcessCr3;
    UINT64 VirtualAddress;
    UINT64 Buffer;
    UINT64 Size;
} CHEAT_REQUEST;

// In VMexit hook, intercept our custom hypercalls
VOID HandleVmcall(PVCPU Vcpu) {
    UINT64 HypercallCode = Vcpu->Rcx;
    
    if ((HypercallCode & 0xFFFF0000) == HV_CHEAT_HYPERCALL_BASE) {
        switch (HypercallCode) {
            case HV_CHEAT_READ_MEMORY: {
                PCHEAT_REQUEST Req = (PCHEAT_REQUEST)Vcpu->Rdx;
                ReadProcessMemory(Req->ProcessCr3, Req->VirtualAddress,
                                 Req->Buffer, Req->Size);
                Vcpu->Rax = STATUS_SUCCESS;
                break;
            }
            
            case HV_CHEAT_WRITE_MEMORY: {
                PCHEAT_REQUEST Req = (PCHEAT_REQUEST)Vcpu->Rdx;
                WriteProcessMemory(Req->ProcessCr3, Req->VirtualAddress,
                                  Req->Buffer, Req->Size);
                Vcpu->Rax = STATUS_SUCCESS;
                break;
            }
            
            case HV_CHEAT_GET_CR3: {
                UINT64 ProcessId = Vcpu->Rdx;
                Vcpu->Rax = GetProcessCr3(ProcessId);
                break;
            }
            
            default:
                // Unknown - pass to real Hyper-V handler
                CallOriginalHandler(Vcpu);
                return;
        }
        
        // Skip original handler, resume guest
        AdvanceGuestRip(Vcpu);
    } else {
        // Not ours - call original
        CallOriginalHandler(Vcpu);
    }
}

// Usermode client
UINT64 HypercallReadMemory(UINT64 TargetCr3, UINT64 Va, PVOID Buffer, SIZE_T Size) {
    CHEAT_REQUEST Req = {
        .ProcessCr3 = TargetCr3,
        .VirtualAddress = Va,
        .Buffer = (UINT64)Buffer,
        .Size = Size
    };
    
    return __vmcall(HV_CHEAT_READ_MEMORY, (UINT64)&Req, 0);
}
```

---

## PHASE 6: ANTI-DETECTION CONSIDERATIONS

### Hyper-V Presence is Expected

Unlike a bare-metal hypervisor that must hide, Hyper-V hijacking has an advantage:

- **CPUID already shows Hyper-V** - Anti-cheat expects this on Windows 10/11 with VBS
- **No timing anomaly from CPUID** - We're not intercepting it, just riding along
- **Legitimate hypervisor bit set** - Bit 31 of CPUID.1.ECX is already 1

### What We Must Hide

1. **Our VMexit handler modifications** - PatchGuard may detect
2. **Custom hypercall codes** - Could be enumerated
3. **Modified MSR bitmap** - If anti-cheat runs at Ring 0

### Mitigation Strategies

```cpp
// 1. Obfuscate hook location
VOID ObfuscateHook(UINT64 HookAddress) {
    // XOR with process-specific key
    // Only decrypt when needed
}

// 2. Validate caller before processing hypercall
VOID ValidateHypercallCaller() {
    // Check caller is our usermode component
    // Use shared secret / signature
    UINT64 CallerRip = VmcsRead64(VMCS_GUEST_RIP);
    if (!IsValidCaller(CallerRip)) {
        // Pretend we don't exist
        InjectGpFault(0);
    }
}

// 3. Time-limited hook activation
VOID ActivateHookTemporarily() {
    // Only hook during gameplay
    // Unhook during anti-cheat scans
}
```

---

## COMPLETE INTEGRATION FLOW

```
┌─────────────────────────────────────────────────────────────┐
│                      BOOT SEQUENCE                           │
│                                                              │
│  1. Windows boots with Hyper-V enabled                      │
│  2. VBS/HVCI active, normal Hyper-V operation               │
│                                                              │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                   ZEROHVCI EXPLOITATION                      │
│                                                              │
│  1. Trigger CVE-2024-26229 or CVE-2024-35250                │
│  2. Gain arbitrary kernel R/W from usermode                 │
│  3. Map hvix64.exe into accessible memory                   │
│                                                              │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                   HYPERVISOR HIJACKING                       │
│                                                              │
│  1. Locate VMexit handler in hvix64.exe                     │
│  2. Find code cave for hook shellcode                       │
│  3. Write TSC-compensating hook                             │
│  4. Patch VMexit entry point to jump to hook                │
│  5. Configure MSR bitmap for APERF/MPERF                    │
│                                                              │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                   CHEAT OPERATION                            │
│                                                              │
│  Usermode Client:                                           │
│  - Call VMCALL with custom hypercall code                   │
│  - Request: read game memory, get process CR3, etc.         │
│                                                              │
│  Hypervisor Hook:                                           │
│  - Intercept VMCALL                                         │
│  - Process cheat request in Ring -1                         │
│  - Return results to usermode                               │
│                                                              │
│  Timing Compensation:                                        │
│  - Track VM-exit overhead                                   │
│  - Adjust TSC offset to hide overhead                       │
│  - Compensate APERF/MPERF reads                             │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## TESTING CHECKLIST

- [ ] ZeroHVCI exploit works reliably
- [ ] hvix64.exe base address located correctly
- [ ] VMexit handler found via pattern scan
- [ ] Hook shellcode executes without crash
- [ ] TSC compensation values calibrated
- [ ] Custom hypercalls work from usermode
- [ ] pafish timing tests pass
- [ ] BattlEye test environment doesn't flag

---

*Reference: Voyager, reversing.info, Intel SDM Vol. 3C, Windows Internals*
