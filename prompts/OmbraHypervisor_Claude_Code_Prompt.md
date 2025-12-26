# OmbraHypervisor — Claude Code Implementation Prompt

## Project Overview

You are building **OmbraHypervisor**, a custom Intel VT-x hypervisor from scratch for Windows 10 x64. This hypervisor must be:

1. **Completely original** — No code copied from public GitHub repos (HyperPlatform, SimpleSvm, Voyager, etc.) as their shellcode patterns are signature-detected by anti-cheats
2. **Undetected** — Must spoof CPUID hypervisor presence, fix RDTSC/RDTSCP timing to under 150 cycles, hide all VMX artifacts
3. **Written in C + Assembly** — No C++, no STL, minimal runtime
4. **Loaded via vulnerable driver** — Uses Ld9BoxSup.sys (LDPlayer's rebranded VirtualBox SUPDrv) for kernel access without mapping a custom driver

---

## Architecture Decision: Custom Hypervisor (NOT Hyper-V Hijacking)

We chose a custom "blue-pill" hypervisor approach because:
- Boot-chain EFI modification is detected by anti-cheats (EAC, Vanguard, BattlEye)
- Hyper-V hijacking at runtime requires breaking into SLAT-protected memory (extremely difficult)
- Custom hypervisor works when Hyper-V/VBS is DISABLED on target systems
- Target audience has Hyper-V disabled (gaming PCs, not enterprise)

**Tradeoff accepted:** This approach fails on VBS-enabled systems, but avoids boot-chain detection.

---

## Vulnerable Driver: Ld9BoxSup.sys

This is a rebranded VirtualBox SUPDrv (version 6.1.36) signed by Shanghai Yiyu Network Technology Co., Ltd.

### Available Primitives via IOCTL

```c
// From VirtualBox sup.h — these IOCTLs are available:

// Session establishment
SUP_IOCTL_COOKIE                    // Establish driver session

// Memory allocation (kernel pool)
SUP_IOCTL_CONT_ALLOC                // Contiguous physical memory
SUP_IOCTL_PAGE_ALLOC_EX             // Non-paged pool allocation
SUP_IOCTL_PAGE_MAP_KERNEL           // Map to kernel VA
SUP_IOCTL_PAGE_LOCK                 // Lock pages
SUP_IOCTL_LOW_ALLOC                 // Low physical memory (<4GB)

// Code execution
SUP_IOCTL_CALL_VMMR0                // Execute function at Ring 0
SUP_IOCTL_LDR_LOAD                  // Load Ring 0 module

// MSR access
SUP_IOCTL_MSR_PROBER                // Read/Write arbitrary MSRs

// Virtualization
SUP_IOCTL_VT_CAPS                   // Query VT-x capabilities
SUP_IOCTL_GET_HWVIRT_MSRS           // Get VMX MSRs
```

### Key Structures

```c
typedef struct SUPCOOKIE {
    uint32_t u32Cookie;
    uint32_t u32SessionCookie;
    uint32_t u32SessionVersion;
    uint32_t u32DriverVersion;
    uint32_t fFlags;
    char     szMagic[16];           // "SUP_COOKIE"
} SUPCOOKIE;

typedef struct SUPCONTALLOC {
    void*    pvR3;                  // Ring-3 mapping
    void*    pvR0;                  // Ring-0 mapping
    uint64_t HCPhys;                // Physical address
    uint32_t cPages;                // Page count
} SUPCONTALLOC;

typedef struct SUPMSRPROBE {
    uint32_t uMsr;                  // MSR number
    uint32_t idCpu;                 // Target CPU
    uint64_t uValue;                // Value for write
} SUPMSRPROBE;
```

---

## Project Structure

```
OmbraHypervisor/
├── usermode/
│   ├── main.c                      # Entry point
│   ├── driver_interface.c          # Ld9BoxSup.sys IOCTL wrapper
│   ├── driver_interface.h
│   └── payload_loader.c            # Copy hypervisor to kernel, trigger
│
├── hypervisor/
│   ├── entry.c                     # DPC/IPI broadcast, per-CPU virtualization
│   ├── vmx.c                       # VMXON, VMCS setup
│   ├── vmx.h
│   ├── vmcs.c                      # VMCS field configuration
│   ├── vmcs.h
│   ├── ept.c                       # EPT identity map builder
│   ├── ept.h
│   ├── exit_dispatch.c             # VM-exit main handler
│   ├── exit_dispatch.h
│   ├── handlers/
│   │   ├── cpuid.c                 # CPUID spoofing
│   │   ├── rdtsc.c                 # RDTSC/RDTSCP timing fix
│   │   ├── msr.c                   # MSR read/write filtering
│   │   ├── cr_access.c             # CR0/CR3/CR4 shadowing
│   │   ├── ept_violation.c         # EPT R/W/X violations
│   │   ├── vmcall.c                # Hypercall interface
│   │   ├── exception.c             # Exception interception
│   │   ├── io.c                    # I/O port access
│   │   ├── xsetbv.c                # XSETBV handling
│   │   └── invd.c                  # INVD/WBINVD
│   ├── timing.c                    # TSC offset calculation
│   ├── timing.h
│   ├── mem.c                       # Memory utilities
│   ├── mem.h
│   ├── cpu.c                       # CPU enumeration, feature detection
│   ├── cpu.h
│   └── asm/
│       ├── vmenter.asm             # VM-entry stub
│       ├── vmexit.asm              # VM-exit handler entry
│       ├── intrinsics.asm          # VMREAD/VMWRITE/VMXON wrappers
│       └── segment.asm             # Segment descriptor helpers
│
├── shared/
│   ├── vmcs_fields.h               # All VMCS field encodings
│   ├── msr_defs.h                  # MSR numbers
│   ├── exit_reasons.h              # VM-exit reason codes
│   ├── ept_defs.h                  # EPT structure definitions
│   ├── cpu_defs.h                  # CR bits, CPUID definitions
│   └── types.h                     # Basic type definitions
│
└── docs/
    ├── vmcs_reference.md           # VMCS field documentation
    ├── exit_handling.md            # Exit reason handling guide
    └── detection_vectors.md        # What to avoid for stealth
```

---

## Execution Flow

```
USERMODE                                 KERNEL (via Ld9BoxSup.sys)
────────                                 ─────────────────────────────
1. Open \\.\Ld9BoxSup
2. SUP_IOCTL_COOKIE (establish session)
3. SUP_IOCTL_CONT_ALLOC:
   - VMXON regions (4KB × NumCPUs, 4KB aligned)
   - VMCS regions (4KB × NumCPUs, 4KB aligned)
   - EPT tables (~2MB)
   - Hypervisor code + stacks (64KB+)
4. Copy hypervisor payload to allocated memory
5. SUP_IOCTL_CALL_VMMR0 → entry()
                                         6. entry() executes at Ring 0
                                         7. For each CPU (via KeIpiGenericCall):
                                            a. Set CR4.VMXE = 1
                                            b. VMXON(vmxon_region[cpu])
                                            c. VMPTRLD(vmcs[cpu])
                                            d. Configure all VMCS fields
                                            e. Setup EPT identity map
                                            f. Capture guest state
                                            g. VMLAUNCH
                                         8. CPU now runs as guest under hypervisor
                                         
RUNTIME (every VM-exit):
─────────────────────────
vmexit.asm entry point
  → Save all GPRs/XMM to stack
  → Call exit_dispatch(vcpu_context)
  → exit_dispatch reads EXIT_REASON
  → Dispatches to appropriate handler
  → Handler processes, may modify guest state
  → Advances guest RIP if needed
  → Returns
  → Restore GPRs/XMM
  → VMRESUME
```

---

## Critical Implementation Details

### 1. VMCS Setup Requirements

```c
// Guest state must capture current CPU state exactly
VMWRITE(GUEST_CR0, __readcr0());
VMWRITE(GUEST_CR3, __readcr3());
VMWRITE(GUEST_CR4, __readcr4() | CR4_VMXE);  // VMXE already set
VMWRITE(GUEST_DR7, __readdr(7));
VMWRITE(GUEST_RSP, current_rsp);
VMWRITE(GUEST_RIP, address_after_vmlaunch);
VMWRITE(GUEST_RFLAGS, __readeflags());

// Segment registers — must read from GDT properly
VMWRITE(GUEST_CS_SELECTOR, __read_cs());
VMWRITE(GUEST_CS_BASE, 0);  // Flat model
VMWRITE(GUEST_CS_LIMIT, 0xFFFFFFFF);
VMWRITE(GUEST_CS_ACCESS_RIGHTS, read_segment_access_rights(cs));
// ... repeat for DS, ES, FS, GS, SS, TR, LDTR

// Host state — where VM-exits land
VMWRITE(HOST_CR0, __readcr0());
VMWRITE(HOST_CR3, __readcr3());  // Or dedicated host CR3
VMWRITE(HOST_CR4, __readcr4());
VMWRITE(HOST_RSP, host_stack_top);
VMWRITE(HOST_RIP, vmexit_handler_address);

// VM-execution controls
VMWRITE(PIN_BASED_VM_EXEC_CONTROL, adjust_controls(desired_pin, MSR_IA32_VMX_PINBASED_CTLS));
VMWRITE(CPU_BASED_VM_EXEC_CONTROL, adjust_controls(desired_proc, MSR_IA32_VMX_PROCBASED_CTLS));
VMWRITE(SECONDARY_VM_EXEC_CONTROL, adjust_controls(desired_proc2, MSR_IA32_VMX_PROCBASED_CTLS2));
VMWRITE(VM_EXIT_CONTROLS, adjust_controls(desired_exit, MSR_IA32_VMX_EXIT_CTLS));
VMWRITE(VM_ENTRY_CONTROLS, adjust_controls(desired_entry, MSR_IA32_VMX_ENTRY_CTLS));
```

### 2. EPT Identity Map

```c
// EPT maps Guest Physical → Host Physical (1:1 identity)
// Structure: PML4 → PDPT → PD → PT (or 2MB large pages)

typedef struct {
    uint64_t read : 1;
    uint64_t write : 1;
    uint64_t execute : 1;
    uint64_t memory_type : 3;
    uint64_t ignore_pat : 1;
    uint64_t large_page : 1;      // 1 for 2MB pages at PD level
    uint64_t accessed : 1;
    uint64_t dirty : 1;
    uint64_t user_execute : 1;
    uint64_t reserved : 1;
    uint64_t pfn : 40;            // Physical frame number
    uint64_t reserved2 : 12;
} EPT_PTE;

// Build identity map:
// - Use 2MB large pages for simplicity (512 entries × 512 entries = 512GB coverage)
// - Set RWX permissions
// - Memory type = WB (6) for normal memory, UC (0) for MMIO
```

### 3. Timing Evasion (CRITICAL)

```c
// Anti-cheats measure CPUID/RDTSC timing:
//   start = rdtsc()
//   cpuid()
//   end = rdtsc()
//   if (end - start > 150) → hypervisor detected

// Solution: Track accumulated VM-exit overhead, subtract from TSC

typedef struct {
    uint64_t tsc_offset;          // Accumulated exit overhead
    uint64_t last_exit_tsc;       // TSC when exit occurred
} TIMING_STATE;

void handle_rdtsc(VCPU* vcpu) {
    uint64_t real_tsc = __rdtsc();
    uint64_t adjusted = real_tsc - vcpu->timing.tsc_offset;
    
    vcpu->guest_regs.rax = (uint32_t)adjusted;
    vcpu->guest_regs.rdx = (uint32_t)(adjusted >> 32);
    vcpu->guest_rip += 2;  // RDTSC = 0F 31
}

void handle_cpuid(VCPU* vcpu) {
    uint64_t tsc_start = __rdtsc();
    
    int regs[4];
    __cpuidex(regs, vcpu->guest_regs.rax, vcpu->guest_regs.rcx);
    
    // Spoof hypervisor presence
    if (vcpu->guest_regs.rax == 1) {
        regs[2] &= ~(1 << 31);    // Clear CPUID.1:ECX.HV bit
        regs[2] &= ~(1 << 5);     // Clear VMX available bit
    }
    
    // Hide hypervisor vendor string
    if (vcpu->guest_regs.rax == 0x40000000) {
        regs[0] = 0;              // Max hypervisor leaf = 0
        regs[1] = regs[2] = regs[3] = 0;
    }
    
    vcpu->guest_regs.rax = regs[0];
    vcpu->guest_regs.rbx = regs[1];
    vcpu->guest_regs.rcx = regs[2];
    vcpu->guest_regs.rdx = regs[3];
    
    uint64_t tsc_end = __rdtsc();
    vcpu->timing.tsc_offset += (tsc_end - tsc_start);
    
    vcpu->guest_rip += 2;  // CPUID = 0F A2
}
```

### 4. VM-Exit Handler Dispatch

```c
// Must handle ALL exit reasons — unhandled = BSOD

void exit_dispatch(VCPU* vcpu) {
    uint32_t reason = (uint32_t)__vmx_vmread(VM_EXIT_REASON) & 0xFFFF;
    
    switch (reason) {
        case EXIT_REASON_CPUID:           handle_cpuid(vcpu); break;
        case EXIT_REASON_RDTSC:           handle_rdtsc(vcpu); break;
        case EXIT_REASON_RDTSCP:          handle_rdtscp(vcpu); break;
        case EXIT_REASON_RDMSR:           handle_rdmsr(vcpu); break;
        case EXIT_REASON_WRMSR:           handle_wrmsr(vcpu); break;
        case EXIT_REASON_CR_ACCESS:       handle_cr_access(vcpu); break;
        case EXIT_REASON_EPT_VIOLATION:   handle_ept_violation(vcpu); break;
        case EXIT_REASON_EPT_MISCONFIG:   handle_ept_misconfig(vcpu); break;
        case EXIT_REASON_VMCALL:          handle_vmcall(vcpu); break;
        case EXIT_REASON_INVD:            handle_invd(vcpu); break;
        case EXIT_REASON_XSETBV:          handle_xsetbv(vcpu); break;
        case EXIT_REASON_IO_INSTRUCTION:  handle_io(vcpu); break;
        case EXIT_REASON_HLT:             handle_hlt(vcpu); break;
        case EXIT_REASON_INVLPG:          handle_invlpg(vcpu); break;
        case EXIT_REASON_VMCLEAR:
        case EXIT_REASON_VMLAUNCH:
        case EXIT_REASON_VMPTRLD:
        case EXIT_REASON_VMPTRST:
        case EXIT_REASON_VMREAD:
        case EXIT_REASON_VMRESUME:
        case EXIT_REASON_VMWRITE:
        case EXIT_REASON_VMXOFF:
        case EXIT_REASON_VMXON:           handle_vmx_instruction(vcpu); break;
        case EXIT_REASON_EXCEPTION_NMI:   handle_exception(vcpu); break;
        case EXIT_REASON_EXTERNAL_INT:    handle_external_interrupt(vcpu); break;
        case EXIT_REASON_TRIPLE_FAULT:    handle_triple_fault(vcpu); break;
        // ... ALL other reasons must be handled
        default:
            // Unhandled — inject #UD or log and continue
            inject_exception(vcpu, EXCEPTION_UD);
            break;
    }
}
```

### 5. Assembly VM-Exit Stub (vmexit.asm)

```asm
; vmexit.asm — MASM x64 syntax
; This is the HOST_RIP target — called on every VM-exit

.code

EXTERN exit_dispatch:PROC

VmExitHandler PROC
    ; Save all general-purpose registers
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rdi
    push rsi
    push rbp
    push rbp        ; Placeholder for RSP
    push rbx
    push rdx
    push rcx
    push rax
    
    ; Save XMM registers (anti-cheat may check these)
    sub rsp, 100h
    movaps [rsp + 00h], xmm0
    movaps [rsp + 10h], xmm1
    movaps [rsp + 20h], xmm2
    movaps [rsp + 30h], xmm3
    movaps [rsp + 40h], xmm4
    movaps [rsp + 50h], xmm5
    ; ... xmm6-xmm15 if needed
    
    ; RCX = pointer to saved context (VCPU*)
    mov rcx, rsp
    sub rsp, 28h            ; Shadow space for x64 ABI
    
    call exit_dispatch
    
    add rsp, 28h
    
    ; Restore XMM
    movaps xmm0, [rsp + 00h]
    movaps xmm1, [rsp + 10h]
    movaps xmm2, [rsp + 20h]
    movaps xmm3, [rsp + 30h]
    movaps xmm4, [rsp + 40h]
    movaps xmm5, [rsp + 50h]
    add rsp, 100h
    
    ; Restore GPRs
    pop rax
    pop rcx
    pop rdx
    pop rbx
    add rsp, 8              ; Skip RSP placeholder
    pop rbp
    pop rsi
    pop rdi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    
    ; Resume guest
    vmresume
    
    ; If VMRESUME fails, we reach here
    jmp VmResumeFailed
    
VmExitHandler ENDP

VmResumeFailed PROC
    ; VMRESUME failed — read error from VMCS
    ; This is fatal, trigger controlled crash
    int 3
    hlt
VmResumeFailed ENDP

END
```

---

## Detection Vectors to Avoid

| Detection Method | Mitigation |
|------------------|------------|
| CPUID.1:ECX[31] hypervisor bit | Clear it in CPUID handler |
| CPUID.0x40000000 vendor string | Return zeros |
| RDTSC/RDTSCP timing > 150 cycles | Accumulate TSC offset, subtract on read |
| CR4.VMXE visible | Shadow CR4 reads, hide VMXE bit |
| IA32_VMX_* MSRs readable | Inject #GP on read attempts |
| EPT at predictable addresses | Randomize PML4 base |
| Known pool tags | Use random/custom pool tags |
| Public shellcode signatures | Write all code from scratch, unique patterns |
| Sequential "Protecting CPU X" logs | Silent operation, no debug output |
| VMCS region in known locations | Randomize allocation |

---

## Build Configuration

```makefile
# Windows Driver Kit build
CC = cl.exe
AS = ml64.exe
LINK = link.exe

CFLAGS = /kernel /GS- /Oi /W4 /WX /O2 /Zl /Zp8 \
         /D_AMD64_ /DAMD64 /D_WIN64 \
         /Fd"$(OUTDIR)\" /Fo"$(OUTDIR)\"

ASFLAGS = /c /Cx /Zi

KERNEL_LIBS = ntoskrnl.lib hal.lib

# No CRT, no exceptions, no RTTI
CFLAGS += /GR- /EHs-c-

hypervisor.sys: $(OBJS)
    $(LINK) /DRIVER /ENTRY:DriverEntry /SUBSYSTEM:NATIVE \
            /NODEFAULTLIB $(KERNEL_LIBS) $(OBJS) /OUT:$@
```

---

## Implementation Order

**Phase 1: Infrastructure**
1. `shared/*.h` — All definitions (VMCS fields, MSRs, exit reasons)
2. `usermode/driver_interface.c` — IOCTL wrappers for Ld9BoxSup.sys
3. `hypervisor/mem.c` — Memory allocation helpers
4. `hypervisor/cpu.c` — CPU feature detection

**Phase 2: Core VMX**
5. `hypervisor/asm/intrinsics.asm` — VMXON/VMREAD/VMWRITE wrappers
6. `hypervisor/vmx.c` — VMXON sequence, feature adjustment
7. `hypervisor/vmcs.c` — Full VMCS configuration
8. `hypervisor/asm/vmexit.asm` — Exit handler entry stub

**Phase 3: EPT**
9. `hypervisor/ept.c` — Identity map builder

**Phase 4: Exit Handlers**
10. `hypervisor/exit_dispatch.c` — Main dispatcher
11. `hypervisor/handlers/cpuid.c` — With timing fix
12. `hypervisor/handlers/rdtsc.c` — With offset tracking
13. `hypervisor/handlers/msr.c` — Filter sensitive MSRs
14. `hypervisor/handlers/*.c` — All remaining handlers

**Phase 5: Integration**
15. `hypervisor/entry.c` — Per-CPU virtualization via IPI
16. `usermode/payload_loader.c` — Load and trigger
17. `usermode/main.c` — Entry point

**Phase 6: Stealth & Testing**
18. `hypervisor/timing.c` — Fine-tune TSC offsets
19. Test against timing detection tools
20. Verify all CPUID leaves properly spoofed

---

## Reference Documentation Strategy

Since you cannot paste the entire Intel SDM (4000+ pages), I will provide:

1. **Pre-extracted reference files** in `/docs`:
   - `vmcs_reference.md` — All VMCS fields with encodings
   - `exit_handling.md` — Every exit reason and handling requirements
   - `msr_reference.md` — Relevant MSRs

2. **C headers with all magic numbers** in `/shared`:
   - `vmcs_fields.h` — `#define VMCS_GUEST_RIP 0x681E` etc.
   - `msr_defs.h` — `#define MSR_IA32_VMX_BASIC 0x480` etc.
   - `exit_reasons.h` — `#define EXIT_REASON_CPUID 10` etc.

3. **Code skeletons** with structure in place, ready for logic implementation

If you need specific Intel SDM details during implementation (e.g., "what are the exact bits in VM_EXIT_QUALIFICATION for EPT violations"), ask and I'll provide the relevant excerpt.

---

## Success Criteria

The hypervisor is complete when:

1. ✅ All CPUs virtualized without BSOD
2. ✅ `systeminfo` shows "Virtualization: Not capable" (VT-x consumed)
3. ✅ Task Manager CPU tab shows "Virtualization: Not capable"
4. ✅ CPUID.1:ECX[31] returns 0 (hypervisor hidden)
5. ✅ RDTSC timing across CPUID < 150 cycles
6. ✅ No detectable signatures matching public hypervisors
7. ✅ Can intercept and log all VM-exits
8. ✅ Hypercall interface working from usermode

---

## Begin Implementation

Start with Phase 1. Generate the header files first (`shared/vmcs_fields.h`, `shared/msr_defs.h`, `shared/exit_reasons.h`) with all the magic numbers extracted from Intel SDM. Then proceed to the driver interface wrapper for Ld9BoxSup.sys.

Ask clarifying questions if any specification is ambiguous. Reference Intel SDM Vol 3C Chapters 23-33 for VMX details — request specific sections as needed rather than loading entire chapters.
