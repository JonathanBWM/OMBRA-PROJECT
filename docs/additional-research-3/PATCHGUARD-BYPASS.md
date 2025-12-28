# PatchGuard Bypass Documentation

## Educational Security Research - Ombra Hypervisor

This document explains how Extended Page Tables (EPT) operate at a privilege level below Windows kernel protections, making EPT-based hooks architecturally invisible to PatchGuard without requiring explicit bypass code.

---

## 1. Overview of PatchGuard (Kernel Patch Protection)

### Purpose

PatchGuard (internally called KPP - Kernel Patch Protection) is Microsoft's kernel integrity verification system introduced in Windows XP x64. Its goal: prevent unauthorized modification of critical kernel structures and code.

### Protected Structures

PatchGuard monitors multiple kernel components:

- **System Service Descriptor Table (SSDT)** - Syscall dispatch table
- **Interrupt Descriptor Table (IDT)** - Hardware/software interrupt handlers
- **Global Descriptor Table (GDT)** - CPU segment descriptors
- **Kernel code sections** - ntoskrnl.exe .text segment
- **Critical kernel structures** - KPRCB, KPCR, driver objects
- **MSR values** - LSTAR, CSTAR, SYSCALL entry points
- **Callback lists** - PsSetCreateProcessNotifyRoutine, etc.

### Verification Mechanism

PatchGuard uses cryptographic hashing and obfuscated validation routines:

1. **Initial State Capture** - During boot, PG records checksums of protected structures
2. **Random Timer Execution** - DPC timers and work items trigger at irregular intervals
3. **Hash Verification** - Compare current state against stored checksums
4. **BSOD on Mismatch** - `CRITICAL_STRUCTURE_CORRUPTION` (0x109) bugcheck

### Implementation Characteristics

- **Self-Decrypting Code** - PG routines are encrypted in memory, decrypt on execution
- **Obfuscated Entry Points** - Function pointers scrambled to prevent discovery
- **Context Validation** - Checks IRQL, stack location before verification runs
- **Timer-Based Triggers** - Uses `KiDpcWatchdog`, `ExpWorkerThread`, `ExAcquireResourceExclusiveLite`
- **Ring 0 Only** - Operates entirely within kernel virtual address space

---

## 2. How PatchGuard Works

### Initialization Phase

During kernel startup (`Phase1InitializationDiscard`), PatchGuard:

1. Allocates encrypted context structures
2. Captures baseline checksums of protected areas
3. Schedules first verification timer (randomized 5-15 minute interval)
4. Encrypts verification routine code in memory

### Execution Flow

```
Timer Expires
    ↓
DPC Routine Scheduled
    ↓
Decrypt PatchGuard Code
    ↓
Verify Context (IRQL, stack)
    ↓
Hash Protected Structures
    ↓
Compare Against Baseline
    ↓
Re-encrypt Code
    ↓
Schedule Next Timer (random interval)
```

### Verification Algorithm (Simplified)

```cpp
void PatchGuardVerify() {
    // Context validation
    if (KeGetCurrentIrql() != DISPATCH_LEVEL) return;
    if (!IsStackValid()) return;

    // Decrypt verification routine
    DecryptPgContext(&g_PgContext);

    // Check SSDT integrity
    u64 ssdt_hash = SHA256(KeServiceDescriptorTable, sizeof(KSERVICE_TABLE));
    if (ssdt_hash != g_PgContext.SsdtBaseline) {
        KeBugCheckEx(CRITICAL_STRUCTURE_CORRUPTION, 0x109, ...);
    }

    // Check IDT integrity
    u64 idt_hash = SHA256(GetIdtBase(), 256 * sizeof(KIDTENTRY64));
    if (idt_hash != g_PgContext.IdtBaseline) {
        KeBugCheckEx(CRITICAL_STRUCTURE_CORRUPTION, 0x109, ...);
    }

    // Re-encrypt and reschedule
    EncryptPgContext(&g_PgContext);
    ScheduleNextVerification(Random(300000, 900000)); // 5-15 minutes
}
```

### Entry Point Discovery (Historical)

Before Windows 10, researchers used pattern scanning:

```cpp
// Find PatchGuard context structures
u8 pattern[] = { 0x48, 0x8B, 0x05, 0xCC, 0xCC, 0xCC, 0xCC }; // MOV RAX, [RIP+offset]
ScanKernelForPattern(pattern, sizeof(pattern));
```

Modern PatchGuard uses dynamic obfuscation, making signature-based detection unreliable.

---

## 3. Why EPT Hooks Are Invisible to PatchGuard

### Privilege Level Hierarchy

CPU privilege rings define execution authority:

```
Ring 3  - User Mode (applications)
Ring 0  - Kernel Mode (ntoskrnl.exe, drivers)
Ring -1 - Hypervisor Mode (VMX root operation)
Ring -2 - System Management Mode (SMM firmware)
```

**Critical Insight:** PatchGuard executes at Ring 0. It can only observe and verify Ring 0 memory. Hypervisor structures (EPT tables, VMCS) exist at Ring -1 and are architecturally invisible.

### EPT Translation Layer

Extended Page Tables add a second layer of address translation:

```
Guest Virtual Address (GVA)
    ↓ [Guest Page Tables - Ring 0 controlled]
Guest Physical Address (GPA)
    ↓ [EPT Tables - Ring -1 controlled]
Host Physical Address (HPA)
```

PatchGuard verifies the GVA→GPA mapping (guest page tables). It **cannot** access or verify the GPA→HPA mapping (EPT tables).

### Hook Mechanism Comparison

#### Traditional Inline Hook (Detected by PG)

```cpp
// Original kernel function
u8 OriginalCode[] = {
    0x48, 0x89, 0x5C, 0x24, 0x08,  // MOV [RSP+8], RBX
    0x48, 0x89, 0x74, 0x24, 0x10,  // MOV [RSP+10], RSI
    // ... rest of function
};

// Inline hook (MODIFIES kernel memory)
u8 HookedCode[] = {
    0xE9, 0x12, 0x34, 0x56, 0x78,  // JMP HookFunction
    0x90, 0x90, 0x90, 0x90, 0x90,  // NOP padding
    // ... rest overwritten
};

// PatchGuard detection
u64 current_hash = SHA256(FunctionAddress, 64);
if (current_hash != baseline_hash) {
    BSOD(CRITICAL_STRUCTURE_CORRUPTION);  // DETECTED
}
```

#### EPT Hook (Invisible to PG)

```cpp
// Guest view: Original code UNCHANGED
ReadGuestMemory(FunctionAddress) == OriginalCode  // TRUE

// Hypervisor view: Shadow page with hook
ReadShadowPage(FunctionAddress) == HookedCode    // TRUE

// PatchGuard verification (runs in guest context)
u64 current_hash = SHA256(FunctionAddress, 64);  // Reads original!
if (current_hash != baseline_hash) {
    // NEVER triggers - guest memory is unmodified
}
```

### Execute-Only EPT Hook Architecture

Ombra uses execute-only permissions for maximum stealth:

```
EPT Entry for Hooked Page:
    Read  = 0  (redirect to original)
    Write = 0  (redirect to original)
    Exec  = 1  (redirect to shadow)
```

When PatchGuard reads the hooked function to hash it:
1. CPU checks EPT Read permission → ALLOWED (points to original)
2. Data comes from original unmodified page
3. Hash matches baseline

When CPU executes the hooked function:
1. CPU checks EPT Execute permission → ALLOWED (points to shadow)
2. Code runs from shadow page with hook installed
3. Guest remains unaware of redirection

### Memory View Comparison

```
┌─────────────────────────────────────────────────────────────────┐
│                    PatchGuard's View (Ring 0)                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  Guest Virtual Memory (Kernel Space)                             │
│  ┌─────────────────────────────────────┐                        │
│  │ NtCreateFile @ 0xFFFFF80012345678    │                        │
│  │                                      │                        │
│  │  48 89 5C 24 08  MOV [RSP+8], RBX   │ ← Original code        │
│  │  48 89 74 24 10  MOV [RSP+10], RSI  │   UNMODIFIED           │
│  │  ...                                 │   Hash = 0xABCDEF...   │
│  └─────────────────────────────────────┘                        │
│              ↓                                                   │
│  Guest Page Tables (CR3)                                         │
│  ┌─────────────────────────────────────┐                        │
│  │ GVA 0xFFFFF80012345678               │                        │
│  │  → GPA 0x12345000 (Read/Write/Exec) │                        │
│  └─────────────────────────────────────┘                        │
│                                                                  │
│  PatchGuard Verification:                                        │
│    ✓ Original code present                                      │
│    ✓ Hash matches baseline                                      │
│    ✓ No modifications detected                                  │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
                              ↓
                   [EPT Translation - INVISIBLE]
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│                  Hypervisor's View (Ring -1)                     │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  EPT Tables (VMCS.EPT_POINTER)                                   │
│  ┌─────────────────────────────────────┐                        │
│  │ GPA 0x12345000                       │                        │
│  │  Read:  → HPA 0x20000000 (original) │ ← PG reads this        │
│  │  Write: → HPA 0x20000000 (original) │                        │
│  │  Exec:  → HPA 0x30000000 (shadow)   │ ← Code executes this   │
│  └─────────────────────────────────────┘                        │
│                                                                  │
│  Host Physical Memory                                            │
│  ┌─────────────────────────────────────┐                        │
│  │ HPA 0x20000000 (Original Page)       │                        │
│  │  48 89 5C 24 08  MOV [RSP+8], RBX   │                        │
│  │  48 89 74 24 10  MOV [RSP+10], RSI  │                        │
│  └─────────────────────────────────────┘                        │
│  ┌─────────────────────────────────────┐                        │
│  │ HPA 0x30000000 (Shadow Page)         │                        │
│  │  E9 12 34 56 78  JMP HookFunction   │ ← Hooked code          │
│  │  90 90 90 90 90  NOP NOP NOP ...    │   Executes here        │
│  └─────────────────────────────────────┘                        │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### Why This Works

1. **PatchGuard Executes in Guest Context**
   - All PG verification code runs at Ring 0
   - Guest page tables control what PG sees
   - EPT translation happens transparently below

2. **EPT Tables Are Hypervisor-Private**
   - Located in hypervisor memory (Ring -1)
   - No guest virtual address mapping
   - Not accessible via CR3-based page tables

3. **Read/Execute Split**
   - Reads return original page (hash verification passes)
   - Executes run shadow page (hook active)
   - No single memory view shows both simultaneously

4. **No Kernel Modification Required**
   - Original kernel .text section untouched
   - Guest CR3 page tables unchanged
   - SSDT/IDT remain at original addresses

---

## 4. Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        Guest OS (Ring 0)                         │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │                      PatchGuard                             ││
│  │  - Timer-based integrity verification                       ││
│  │  - Hashes kernel code, SSDT, IDT                           ││
│  │  - Operates in guest virtual memory                         ││
│  └──────────────────────┬──────────────────────────────────────┘│
│                         │                                        │
│                         │ Verifies                               │
│                         ↓                                        │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │                  Kernel Virtual Memory                      ││
│  │  ┌───────────┐  ┌───────────┐  ┌───────────┐              ││
│  │  │   SSDT    │  │    IDT    │  │ ntoskrnl  │              ││
│  │  │ (original)│  │ (original)│  │ (original)│              ││
│  │  └───────────┘  └───────────┘  └───────────┘              ││
│  │                                                             ││
│  │  All structures appear UNMODIFIED                           ││
│  │  Hash verification: ✓ PASS                                  ││
│  └─────────────────────────────────────────────────────────────┘│
│                         │                                        │
│                         │ Guest Page Tables (CR3)                │
│                         ↓                                        │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │           Guest Physical Address Space (GPA)                ││
│  │  ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐              ││
│  │  │ Page 0 │ │ Page 1 │ │ Page 2 │ │ Page 3 │              ││
│  │  └────────┘ └────────┘ └────────┘ └────────┘              ││
│  └─────────────────────────────────────────────────────────────┘│
└────────────────────────┬────────────────────────────────────────┘
                         │
                         │ EPT Translation Layer
                         │ (PatchGuard CANNOT see this)
                         ↓
┌─────────────────────────────────────────────────────────────────┐
│                    Hypervisor (Ring -1)                          │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │                  EPT Page Tables                            ││
│  │  ┌──────────────────────────────────────────────────────┐  ││
│  │  │ GPA 0x12345000 (NtCreateFile page)                   │  ││
│  │  │   Read  Permission: → HPA 0x20000000 (original)      │  ││
│  │  │   Write Permission: → HPA 0x20000000 (original)      │  ││
│  │  │   Exec  Permission: → HPA 0x30000000 (shadow/hook)   │  ││
│  │  └──────────────────────────────────────────────────────┘  ││
│  │                                                             ││
│  │  EPT Violation Handler:                                     ││
│  │   - Monitors guest memory access                            ││
│  │   - Enforces read/write/execute split                       ││
│  │   - Manages shadow page swapping                            ││
│  └─────────────────────────────────────────────────────────────┘│
│                         │                                        │
│                         ↓                                        │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │          Host Physical Address Space (HPA)                  ││
│  │  ┌────────────────────┐  ┌────────────────────┐            ││
│  │  │ HPA 0x20000000     │  │ HPA 0x30000000     │            ││
│  │  │ (Original Page)    │  │ (Shadow Page)      │            ││
│  │  │                    │  │                    │            ││
│  │  │ 48 89 5C 24 08    │  │ E9 12 34 56 78    │            ││
│  │  │ MOV [RSP+8],RBX   │  │ JMP HookFunction  │            ││
│  │  │ ... original ...  │  │ ... hook code ... │            ││
│  │  └────────────────────┘  └────────────────────┘            ││
│  │         ↑                        ↑                          ││
│  │         │                        │                          ││
│  │    Read/Write access        Execute access                  ││
│  │    returns this             runs this                       ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘

Execution Flow:
─────────────

1. Guest calls NtCreateFile()
   → CPU fetches instruction from GVA 0xFFFFF80012345678

2. Guest page tables translate GVA → GPA
   → GPA 0x12345000

3. EPT checks execute permission
   → EPT redirects to HPA 0x30000000 (shadow page)

4. CPU executes hooked code
   → Ombra hook handler runs, VMCALL to hypervisor

5. PatchGuard timer fires, reads same GVA for verification
   → Guest page tables translate GVA → GPA 0x12345000

6. EPT checks read permission
   → EPT redirects to HPA 0x20000000 (original page)

7. PatchGuard hashes original code
   → Hash matches baseline, verification PASSES
```

---

## 5. Comparison: EPT Hooks vs Traditional Patches

| Aspect | EPT Hooks (Ombra) | Inline Patches (SSDT Hook) |
|--------|-------------------|----------------------------|
| **PatchGuard Detection** | Immune | Detected immediately |
| **Kernel Memory Modified** | No | Yes (overwrites code) |
| **Guest View** | Original code intact | Modified code visible |
| **Implementation Level** | Ring -1 (hypervisor) | Ring 0 (kernel driver) |
| **Requires Bypass Code** | No | Yes (PG disable exploit) |
| **CPU Privilege** | VMX root mode | Kernel mode |
| **Page Table Modified** | No (only EPT) | No (hooks function directly) |
| **Atomic Installation** | Yes (MTF single-step) | No (vulnerable during write) |
| **Performance Overhead** | ~200 cycles (VMExit) | ~10 cycles (direct jump) |
| **Uninstallation** | Instant (EPT update) | Risky (restore during execution) |
| **Multi-Core Safety** | Built-in (IPI sync) | Manual (requires IRQL raise) |
| **Debug Detection** | None (no code change) | Debuggers see JMP |

### Code Comparison

#### Traditional SSDT Hook (Detected)

```cpp
// Driver code (Ring 0)
PVOID OriginalNtCreateFile;

NTSTATUS HookedNtCreateFile(...) {
    // Custom logic
    return ((NtCreateFileFunc)OriginalNtCreateFile)(...);
}

void InstallSsdtHook() {
    PSERVICE_DESCRIPTOR_TABLE ssdt = GetSSDT();

    // Disable write protection
    CR0_WP_DISABLE();

    // Replace SSDT entry (MODIFIES KERNEL STRUCTURE)
    OriginalNtCreateFile = ssdt->ServiceTable[NtCreateFileIndex];
    ssdt->ServiceTable[NtCreateFileIndex] = (PVOID)HookedNtCreateFile;

    CR0_WP_ENABLE();

    // PatchGuard will detect SSDT modification within 5-15 minutes
    // BSOD: CRITICAL_STRUCTURE_CORRUPTION
}
```

#### EPT Hook (Immune)

```cpp
// Hypervisor code (Ring -1)
void InstallEptHook(u64 gpa_function, u64 hpa_original, u64 hpa_shadow) {
    EPT_PTE* pte = EptGetPteForGpa(gpa_function);

    // Split large page to 4KB (if needed)
    if (pte->LargePage) {
        EptSplitLargePage(pte);
    }

    // Configure read/write to return original
    pte->ReadAccess = 1;
    pte->WriteAccess = 1;
    pte->PageFrameNumber = hpa_original >> 12;

    // Configure execute to run shadow
    pte->ExecuteAccess = 1;
    pte->ExecutePageFrameNumber = hpa_shadow >> 12;  // Intel only

    // Flush EPT TLB
    InveptSingleContext(g_Context.EptPml4);

    // Guest kernel NEVER sees modification
    // PatchGuard NEVER detects hook
}
```

### Detection Surface

**Inline Patch Detection:**
- PatchGuard hash verification detects modified code
- Kernel debuggers see JMP instruction
- Code integrity checks fail
- ETW events log kernel modification
- Driver signature verification fails

**EPT Hook Detection:**
- No hash mismatch (original code reads correctly)
- No debugger artifacts (guest sees original)
- No code integrity failures
- No ETW events (no kernel change)
- Requires hypervisor-aware detection:
  - CPUID hypervisor bit check
  - MSR timing attacks
  - Cache-timing analysis
  - TLB desync detection

---

## 6. Implementation Details

### Ombra's EPT Hook Installation

```cpp
/**
 * @brief   Install EPT execute hook with shadow page
 *
 * Creates execute-only split where reads/writes return original page
 * but execution runs from shadow page containing hook trampoline.
 *
 * @param   gpa         Guest physical address to hook
 * @param   hook_func   Hypervisor hook handler function
 * @return  Status code
 */
STATUS EptInstallHook(u64 gpa, void* hook_func) {
    // Allocate shadow page
    u64 hpa_shadow = PoolAllocatePage(&g_Context.Pool);
    if (!hpa_shadow) return STATUS_NO_MEMORY;

    // Copy original code to shadow
    u64 hpa_original = EptGpaToHpa(gpa);
    memcpy((void*)hpa_shadow, (void*)hpa_original, PAGE_SIZE);

    // Write hook trampoline at function entry
    u8* shadow_entry = (u8*)hpa_shadow + (gpa & 0xFFF);
    WriteJmpTrampoline(shadow_entry, hook_func);

    // Configure EPT PTE for execute-only split
    EPT_PTE* pte = EptGetPteForGpa(gpa);
    EptSplitLargePage(pte);  // Ensure 4KB granularity

    // Intel: Use separate execute PFN
    pte->ReadAccess = 1;
    pte->WriteAccess = 1;
    pte->ExecuteAccess = 1;
    pte->PageFrameNumber = hpa_original >> 12;       // Read/write
    pte->ExecutePageFrameNumber = hpa_shadow >> 12;  // Execute

    // Invalidate EPT TLB
    InveptSingleContext(g_Context.EptPml4);

    // IPI sync on all cores
    IpiSyncAllCores();

    return STATUS_SUCCESS;
}
```

### AMD NPT Dual-Table Approach

AMD NPT lacks Intel's execute-only split, requiring dual table switching:

```cpp
/**
 * @brief   Install NPT hook using table switching (AMD)
 *
 * Maintains two NPT table sets:
 *   - Table A: Original mappings (for guest reads/writes)
 *   - Table B: Shadow mappings (for guest execution)
 *
 * #VMEXIT on page fault switches between tables.
 */
STATUS NptInstallHook(u64 gpa, void* hook_func) {
    // Allocate shadow page
    u64 hpa_shadow = PoolAllocatePage(&g_Context.Pool);
    memcpy((void*)hpa_shadow, (void*)NptGpaToHpa(gpa), PAGE_SIZE);

    // Write hook trampoline
    u8* shadow_entry = (u8*)hpa_shadow + (gpa & 0xFFF);
    WriteJmpTrampoline(shadow_entry, hook_func);

    // Configure Table A (original - for reads/writes)
    NPT_PTE* pte_a = NptGetPte(g_NptTableA, gpa);
    pte_a->Read = 1;
    pte_a->Write = 1;
    pte_a->Execute = 0;  // Trigger #NPF on execute
    pte_a->PageFrameNumber = NptGpaToHpa(gpa) >> 12;

    // Configure Table B (shadow - for executes)
    NPT_PTE* pte_b = NptGetPte(g_NptTableB, gpa);
    pte_b->Read = 0;  // Trigger #NPF on read
    pte_b->Write = 0;
    pte_b->Execute = 1;
    pte_b->PageFrameNumber = hpa_shadow >> 12;

    // Start with Table A active
    g_Vmcb->NCr3 = g_NptTableA;

    return STATUS_SUCCESS;
}

void HandleNptViolation(u64 gpa, u64 exit_info) {
    if (exit_info & NPT_EXIT_READ || exit_info & NPT_EXIT_WRITE) {
        // Switch to Table A (original)
        g_Vmcb->NCr3 = g_NptTableA;
    } else if (exit_info & NPT_EXIT_EXEC) {
        // Switch to Table B (shadow)
        g_Vmcb->NCr3 = g_NptTableB;
    }
}
```

### Hook Handler Example

```cpp
/**
 * @brief   Hook handler for NtCreateFile
 *
 * Intercepts file creation, logs parameters, enforces access policy.
 * Runs at Ring -1 in hypervisor context via VMCALL.
 */
void HookNtCreateFile(GUEST_CONTEXT* guest) {
    // Extract parameters from guest stack
    UNICODE_STRING* file_path = (UNICODE_STRING*)guest->Rdx;
    ACCESS_MASK desired_access = (ACCESS_MASK)guest->R8;

    // Translate guest virtual to host physical
    u64 hpa_path = TranslateGuestVA((u64)file_path->Buffer, guest->Cr3);
    wchar_t* path = (wchar_t*)hpa_path;

    // Log access attempt
    LogInfo("NtCreateFile: %ls (Access: 0x%X)", path, desired_access);

    // Policy enforcement
    if (wcsstr(path, L"\\System32\\drivers\\")) {
        // Block driver loading
        guest->Rax = STATUS_ACCESS_DENIED;
        guest->Rip += 5;  // Skip CALL instruction
        return;
    }

    // Allow original function to execute
    // (Return from VMCALL, continue to original NtCreateFile)
}
```

---

## 7. Why No PatchGuard Bypass Code Needed

### Traditional Bypass Approaches (Obsolete for Ombra)

**Historical Methods:**
1. **PG Context Corruption** - Locate and zero PG context structures
2. **Timer Unhooking** - Remove DPC timers from kernel timer list
3. **Infinite Loop Injection** - Redirect PG routine to infinite loop
4. **Entry Point Patching** - NOP out PG initialization code

**All require:**
- Pattern scanning to find obfuscated PG code
- Kernel memory modification (triggers what you're trying to bypass)
- Maintenance across Windows updates
- Potential instability (PG is integral to kernel operation)

### Ombra's Approach: Architectural Invisibility

**No bypass needed because:**

1. **Hypervisor Exists Below Kernel**
   - PatchGuard operates at Ring 0
   - EPT operates at Ring -1
   - Ring 0 cannot observe Ring -1 structures

2. **Guest Kernel Remains Pristine**
   - Zero bytes modified in ntoskrnl.exe
   - SSDT, IDT, GDT unchanged
   - All hashes match baselines

3. **Memory Redirection Is Transparent**
   - EPT translation happens in CPU microcode
   - No software visibility into EPT tables
   - Guest page tables show correct mappings

4. **No Code Injection Required**
   - Hooks live in hypervisor memory (Ring -1)
   - No guest virtual address for hook code
   - Kernel memory scanners find nothing

### Verification: PatchGuard Still Active

Ombra does NOT disable PatchGuard. PG continues running and verifying:

```cpp
void VerifyPatchGuardActive() {
    // PatchGuard timer is still scheduled
    LIST_ENTRY* timer_list = GetDpcTimerList();
    BOOL pg_timer_found = FALSE;

    for (LIST_ENTRY* entry = timer_list->Flink; entry != timer_list; entry = entry->Flink) {
        KTIMER* timer = CONTAINING_RECORD(entry, KTIMER, TimerListEntry);
        if (IsPatchGuardTimer(timer)) {
            pg_timer_found = TRUE;
            break;
        }
    }

    // PatchGuard context is intact
    PPATCHGUARD_CONTEXT pg_ctx = FindPatchGuardContext();
    assert(pg_ctx != NULL);
    assert(pg_ctx->Magic == PATCHGUARD_MAGIC);

    // Verification routines still execute
    // (Check ETW events for PG activity)
}
```

PatchGuard sees:
- ✓ SSDT entries point to original functions
- ✓ IDT entries have correct handler addresses
- ✓ Kernel code hashes match baselines
- ✓ No suspicious memory regions

PatchGuard does NOT see:
- EPT tables redirecting execution
- Shadow pages with hook code
- VMCS controlling guest execution
- Hypervisor memory allocations

---

## 8. Detection Resistance

### What Anti-Cheat Cannot Detect via PatchGuard

- **Code Integrity**: Guest kernel code unchanged
- **Hash Verification**: All checksums pass
- **Structure Inspection**: SSDT/IDT/GDT unmodified
- **Driver Enumeration**: No suspicious driver objects
- **Memory Scanning**: Hook code not in guest memory

### What Anti-Cheat CAN Detect (Hypervisor Presence)

**CPUID Hypervisor Bit:**
```cpp
void CheckHypervisor() {
    int cpuid_result[4];
    __cpuid(cpuid_result, 1);

    if (cpuid_result[2] & (1 << 31)) {
        // Hypervisor present bit set
        // Could be VMware, VirtualBox, Hyper-V, or Ombra
    }
}
```

**Ombra Mitigation:** Emulate Hyper-V signature ("Microsoft Hv"), appear as legitimate Windows component.

**Timing Attacks:**
```cpp
void DetectHypervisorTiming() {
    u64 start = __rdtsc();
    __cpuid(cpuid_result, 0);
    u64 end = __rdtsc();

    if ((end - start) > 1000) {
        // CPUID took too long (VMExit overhead)
        // Likely running under hypervisor
    }
}
```

**Ombra Mitigation:** TSC offsetting to hide VMExit latency, RDTSC emulation.

**TLB Desynchronization:**
```cpp
void DetectEptHook() {
    void* func = GetProcAddress(GetModuleHandle(L"ntdll"), "NtCreateFile");

    // Execute and measure
    u64 exec_time = MeasureExecutionTime(func);

    // Read and hash
    u64 code_hash = HashMemory(func, 64);

    // Flush TLB
    InvalidatePage(func);

    // Re-measure
    u64 exec_time2 = MeasureExecutionTime(func);

    if (exec_time != exec_time2 || code_hash != EXPECTED_HASH) {
        // TLB flush changed behavior - EPT hook likely
    }
}
```

**Ombra Mitigation:** EPT execute-only split maintains consistency, MTF prevents TLB flush detection.

---

## 9. Conclusion

### Key Takeaways

1. **EPT Hooks Operate Below PatchGuard**
   - Ring -1 hypervisor vs Ring 0 kernel protection
   - Architecturally invisible, not detected

2. **Guest Kernel Remains Unmodified**
   - No code changes trigger PatchGuard
   - All integrity checks pass naturally

3. **No Bypass Code Required**
   - Ombra does not disable, corrupt, or evade PatchGuard
   - PG continues operating normally, simply cannot see hypervisor layer

4. **Detection Requires Hypervisor-Aware Techniques**
   - CPUID checks, timing analysis, cache attacks
   - Cannot rely on kernel integrity verification alone

### Architectural Principle

```
The hypervisor is not "evading" PatchGuard.
PatchGuard is structurally unable to verify hypervisor memory.
This is by design in CPU architecture, not a vulnerability.
```

### Implications for Security Research

**For Defenders:**
- Kernel-level integrity tools insufficient against hypervisor-based threats
- Require hypervisor-aware detection (HVCI, VBS, TPM attestation)
- Boot-time validation critical (Secure Boot, Measured Boot)

**For Researchers:**
- EPT-based instrumentation ideal for transparent kernel analysis
- No interference with guest kernel operation
- Stable across Windows updates (no signature dependence)

**For Ombra:**
- Hypervisor architecture provides inherent stealth
- Focus shifts from PatchGuard bypass to hypervisor presence hiding
- CPUID emulation, timing normalization, TSC offsetting become critical

---

## References

- Intel SDM Vol. 3C: VMX Operation and EPT
- AMD APM Vol. 2: Nested Paging
- Microsoft: Kernel Patch Protection Design
- Alex Ionescu: PatchGuard Analysis (2006-2023)
- Ombra Implementation: `OmbraPayload/Hooks/EptHooks.cpp`

---

*Educational documentation for Ombra Hypervisor security research project.*
*EPT-based hooks demonstrate architectural separation between hypervisor and kernel privilege levels.*
