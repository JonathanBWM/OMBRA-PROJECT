# Internal Cheat Detection Vectors: BattlEye & EAC Deep Analysis

## Executive Summary

This document consolidates research from secret.club, unknowncheats, GuidedHacking, and various GitHub repositories to provide a comprehensive analysis of how modern anti-cheats detect internal cheats living inside the game process.

**Key Finding:** For a hypervisor-backed internal cheat (like Ombra), detection falls into two categories:
1. **Hypervisor-level detection** (timing attacks, CPUID analysis) - addressed in timing bypass
2. **Process-level detection** (memory scanning, thread analysis, hook detection) - THIS DOCUMENT

---

## 1. Memory Scanning Detection

### 1.1 Executable Page Enumeration

**How it works:**
```cpp
// BattlEye's shellcode enumerates the entire address space
for (PVOID addr = 0; addr < MAX_USER_ADDRESS; addr += mbi.RegionSize) {
    VirtualQuery(addr, &mbi, sizeof(mbi));
    
    // Flag executable pages outside known modules
    if ((mbi.Protect & PAGE_EXECUTE_*) && mbi.Type != MEM_IMAGE) {
        battleye::report(&report, sizeof(report), false);
    }
}
```

**Detection criteria:**
| Field | Legitimate | Suspicious |
|-------|------------|------------|
| `Type` | `MEM_IMAGE` | `MEM_PRIVATE` or `MEM_MAPPED` |
| `Protect` | `PAGE_EXECUTE_READ` | `PAGE_EXECUTE_READWRITE` |
| `AllocationBase` | Points to valid module | Points to unknown allocation |

**Your cheat is flagged if:**
- Executable memory (`PAGE_EXECUTE_*`) exists outside loaded modules
- Memory type is `MEM_PRIVATE` instead of `MEM_IMAGE`
- Memory region doesn't correspond to any PE header

### 1.2 VAD Tree Manipulation Detection

**From secret.club (BattlEye reverse engineer tracking):**
```cpp
// BattlEye spams IsBadReadPtr on reserved/freed memory
// to detect VAD manipulation
for (search_index = 0; ; ++search_index) {
    // If memory is readable when it shouldn't be,
    // the VAD tree has been modified
    if (!IsBadReadPtr(reserved_address, 1)) {
        // DETECTION: VAD tree manipulated to hide memory
    }
}
```

**Why this matters:**
- Some cheats modify the VAD tree to hide allocations from VirtualQuery
- BattlEye probes "should be empty" addresses to catch this
- If your hidden memory is readable, you're caught

### 1.3 Copy-on-Write Detection (QueryWorkingSetEx)

**Critical detection method:**
```cpp
PSAPI_WORKING_SET_EX_INFORMATION info = {};
info.VirtualAddress = module_text_section;
QueryWorkingSetEx(GetCurrentProcess(), &info, sizeof(info));

// If Shared bit is clear, page was modified (CoW occurred)
if (!info.VirtualAttributes.Shared) {
    // DETECTION: Code section was modified (inline hook placed)
}
```

**Why this is dangerous:**
- VirtualQuery reports hooked pages as `MEM_IMAGE` even after modification
- QueryWorkingSetEx reveals the truth via the `Shared` bit
- Any inline hook in a shared DLL triggers copy-on-write
- This detects even "cleaned up" hooks where bytes were restored

---

## 2. Thread Detection

### 2.1 Thread Start Address Verification

**From EAC/EAAC analysis:**
```cpp
// Anti-cheat queries thread start address
NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, 
                         &start_addr, sizeof(start_addr), NULL);

// Check if start address is inside a known module
if (!IsAddressInsideModule(start_addr, &module_list)) {
    // DETECTION: Thread executing from non-backed memory
}
```

**Detection triggers:**
- Thread start address in `MEM_PRIVATE` memory
- Thread start address in "ghost" region (manually mapped)
- Thread start address spoofed but stack walk reveals truth

### 2.2 Stack Walking

**Most dangerous detection vector:**
```cpp
// Anti-cheat walks the thread's call stack
CONTEXT ctx;
GetThreadContext(hThread, &ctx);

while (RtlVirtualUnwind(..., &ctx, ...)) {
    if (!IsAddressInsideModule(ctx.Rip, &module_list)) {
        // DETECTION: Return address points to unbacked memory
    }
}
```

**What this catches:**
- Any function call from your cheat to game code
- Hook trampolines with return addresses in MEM_PRIVATE
- Thread hijacking where execution temporarily leaves modules
- Asynchronous shellcode execution

**From overlayhack.com:**
> "After you've hijacked a thread, that thread executes code outside of modules (non backed) and this is going to get you either flagged or banned after the anti-cheat stackwalk threads."

### 2.3 Thread Creation Monitoring

**Kernel callbacks:**
```cpp
// Anti-cheat registers thread notify callback
PsSetCreateThreadNotifyRoutine(ThreadNotifyCallback);

void ThreadNotifyCallback(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create) {
    if (Create) {
        PETHREAD Thread;
        PsLookupThreadByThreadId(ThreadId, &Thread);
        
        // Get thread start address
        PVOID StartAddress = PsGetThreadStartAddress(Thread);
        
        // Flag if not in valid module
        if (!IsValidModuleAddress(StartAddress)) {
            // DETECTION: Suspicious thread created
        }
    }
}
```

### 2.4 Thread Start Address Spoofing Detection

**From Elastic Security Labs:**
```cpp
// Even if start address is spoofed, the stack reveals truth
// Four classes of trampolines exist:
// 1. Direct entrypoint hooking
// 2. Context manipulation (SetThreadContext)
// 3. APC-based early bird
// 4. Return address manipulation

// Detection: Check if first stack frames are consistent
// with the declared start address
```

---

## 3. Module/DLL Detection

### 3.1 PEB Module List Enumeration

**Standard approach:**
```cpp
// Iterate InLoadOrderModuleList
PPEB_LDR_DATA Ldr = NtCurrentPeb()->Ldr;
for (PLIST_ENTRY Entry = Ldr->InLoadOrderModuleList.Flink;
     Entry != &Ldr->InLoadOrderModuleList;
     Entry = Entry->Flink) {
    // Check each module...
}
```

**Bypassing PEB:**
- Unlinking from PEB lists doesn't help
- NtQueryVirtualMemory with `MemorySectionName` reveals file mapping path
- Memory scanning finds PE headers regardless of PEB state

### 3.2 PE Header Detection

**From secret.club:**
```cpp
// Scan for PE headers in suspicious memory
for (each executable page) {
    if (*(WORD*)page == 'ZM' || *(WORD*)page == 'MZ') {
        // Found DOS header
        PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(page + dos->e_lfanew);
        if (nt->Signature == 'EP') {
            // DETECTION: Manually mapped module found
        }
    }
}
```

**Common evasion (and why it fails):**
- Zeroing PE header: Detected by checking if first page is zeroed
- BattlEye checks for `PoolTag == 'enoN'` (NonPagedPool) + size > 0x1000

### 3.3 Module Integrity Verification

**BattlEye's approach:**
```cpp
// Compare loaded module against disk copy
auto disk_image = LoadLibraryEx(path, NULL, DONT_RESOLVE_DLL_REFERENCES);
auto mem_image = GetModuleHandle(name);

// Compare .text sections
if (memcmp(mem_image->text, disk_image->text, text_size) != 0) {
    // DETECTION: Module has been tampered
}
```

**What this catches:**
- IAT hooks (modified import table)
- Inline hooks (modified code)
- Trampoline hooks

---

## 4. Hook Detection

### 4.1 IAT Hook Detection

**Standard detection:**
```cpp
// For each import, verify it points to the right module
for (each IAT entry) {
    PVOID func_addr = IAT[i];
    
    // Get module containing this address
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(func_addr, &mbi, sizeof(mbi));
    
    // Check if it's in the expected module
    if (mbi.AllocationBase != expected_module_base) {
        // DETECTION: IAT hooked
    }
}
```

### 4.2 Inline Hook Detection

**Methods used:**
1. **Byte pattern comparison:** Compare first bytes against known signatures
2. **Copy-on-write check:** QueryWorkingSetEx `Shared` bit
3. **Disassembly:** Check if first instruction is JMP/CALL to outside

**BattlEye specific checks:**
```cpp
// Check for common hook patterns
if (*(BYTE*)func == 0xE9 ||      // JMP rel32
    *(BYTE*)func == 0xFF ||      // JMP/CALL indirect
    *(WORD*)func == 0x25FF) {    // JMP [rip+disp]
    // Possible inline hook detected
}
```

### 4.3 EAT Hook Detection

```cpp
// Verify export table entries point within module bounds
PIMAGE_EXPORT_DIRECTORY exports = get_export_dir(module);
PDWORD functions = (PDWORD)(base + exports->AddressOfFunctions);

for (DWORD i = 0; i < exports->NumberOfFunctions; i++) {
    PVOID func = (PVOID)(base + functions[i]);
    if (func < module_start || func >= module_end) {
        // DETECTION: EAT forwarding to external code
    }
}
```

---

## 5. Game-Specific Detections (Escape from Tarkov)

### 5.1 Unity/Mono Assembly Integrity

**From secret.club (mono integrity):**
```cpp
// Tarkov-specific: Scan managed assemblies
auto paths = {
    "EscapeFromTarkov_Data\\Managed",
    "EscapeFromTarkov_Data",
    "EscapeFromTarkov_Data\\StreamingAssets\\..."
};

// Upload file hashes to BattlEye servers
for (each file in paths) {
    report.filename = file.name;
    report.filesize = file.size;
    report.hash = compute_hash(file);
    battleye::report(&report);
}
```

### 5.2 Dynamic Shellcode Streaming

**BattlEye streams detection shellcode:**
- Downloaded from server via BEService
- Executed in game process as position-independent code
- No function calls (all inlined) - hard to analyze
- Changes frequently server-side

**What shellcode checks:**
- Window enumeration (titles, classes)
- TCP connections (known cheat IPs/ports)
- Driver list (blacklisted vulnerable drivers)
- Process list (known cheat processes)
- Certificate enumeration (cheat signing certs)

### 5.3 Reverse Engineer Detection

**From BattlEye RE tracking article:**
- Special shellcode pushed to machines with RE tools installed
- Detects: x64dbg, IDA, Ghidra, Cheat Engine
- Uploads: All visible window titles, driver list, certificates
- Specifically targets cheat developers

---

## 6. Kernel-Level Detection (BEDaisy/EAC.sys)

### 6.1 Driver Enumeration

```cpp
// Scan PiDDBCacheTable for mapped drivers
NtQuerySystemInformation(SystemModuleInformation, ...);

// Check MmUnloadedDrivers for evidence of unloaded vulnerable drivers
for (each entry in MmUnloadedDrivers) {
    if (is_blacklisted(entry.Name)) {
        // DETECTION: Vulnerable driver was loaded
    }
}
```

### 6.2 Handle Table Monitoring

```cpp
// Enumerate all handles system-wide
NtQuerySystemInformation(SystemHandleInformation, ...);

for (each handle) {
    if (handle.ObjectType == PROCESS_HANDLE && 
        handle.TargetPID == GamePID) {
        // Check handle permissions
        if (handle.AccessMask & (PROCESS_VM_READ | PROCESS_VM_WRITE)) {
            // DETECTION: External process has game handle
        }
    }
}
```

### 6.3 Callback Registration

**BEDaisy registers:**
- `ObRegisterCallbacks` - Strip handle access
- `PsSetCreateProcessNotifyRoutine` - Monitor process creation
- `PsSetCreateThreadNotifyRoutine` - Monitor thread creation
- `PsSetLoadImageNotifyRoutine` - Monitor DLL/driver loading
- Minifilter - Monitor file operations

---

## 7. What Hypervisor + EPT Hides

### Successfully Hidden:
| Vector | Hidden by EPT? | Notes |
|--------|----------------|-------|
| Memory scanning | ✅ Yes | EPT maps cheat pages as not-present to guest |
| VirtualQuery | ✅ Yes | Returns fabricated data if hooked |
| PE header detection | ✅ Yes | Header page hidden from scans |
| NtQueryVirtualMemory | ✅ Yes | Can be intercepted at VMExit |

### NOT Hidden:
| Vector | Hidden by EPT? | Why Not |
|--------|----------------|---------|
| Thread start address | ❌ No | Kernel structure, not memory scan |
| Stack walking | ❌ No | Walks guest memory, return addrs in cheat |
| Thread creation callback | ❌ No | Kernel-level, pre-EPT |
| Copy-on-write detection | ❌ No | Working set is kernel-managed |
| CPUID timing | ❌ No | Measures VMExit latency |
| Handle enumeration | ❌ No | Kernel object, not memory |

---

## 8. Complete Detection Evasion Strategy

### For Ombra V3 Internal Cheat:

**Tier 1: Already Handled by Hypervisor**
- ✅ Memory read/write (EPT remapping)
- ✅ Driver hiding (EPT not-present)
- ⚠️ CPUID timing (NEEDS timing bypass)

**Tier 2: Needs Internal Cheat Design**
```
┌─────────────────────────────────────────────────────────────┐
│                    THREAD SAFETY                             │
├─────────────────────────────────────────────────────────────┤
│ • Never create new threads - hijack existing                │
│ • Spoof thread start address in ETHREAD                     │
│ • Use thread pool threads (legitimate start addr)           │
│ • Execute only during safe windows (hook callbacks)         │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                    STACK SAFETY                              │
├─────────────────────────────────────────────────────────────┤
│ • Return address spoofing on all game calls                 │
│ • Place cheat in executable module (not MEM_PRIVATE)        │
│ • Use gadgets inside legitimate modules                     │
│ • Clean stack before anti-cheat can walk it                 │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                    HOOK SAFETY                               │
├─────────────────────────────────────────────────────────────┤
│ • EPT execute-only hooks (no CoW trigger)                   │
│ • Don't modify shared DLL pages directly                    │
│ • Use VMExit-based hooks (CPUID, VMCALL)                    │
│ • Hardware breakpoints via hypervisor                       │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                   MEMORY LAYOUT                              │
├─────────────────────────────────────────────────────────────┤
│ • Map cheat as MEM_IMAGE (fake PE header)                   │
│ • Or: Hide in existing module's code cave                   │
│ • Or: Use hypervisor to intercept all memory queries        │
│ • Clear PE header after initialization                      │
└─────────────────────────────────────────────────────────────┘
```

**Tier 3: Kernel-Level (OmbraDriver handles)**
- ✅ PiDDBCache cleared
- ✅ MmUnloadedDrivers cleared  
- ✅ Driver hidden via EPT
- ⚠️ Handle protection (needs callback bypass)

---

## 9. Priority Detection Vectors for EFT

Based on BattlEye's known focus for Escape from Tarkov:

| Priority | Vector | Risk Level | Mitigation |
|----------|--------|------------|------------|
| 1 | CPUID timing | CRITICAL | Timing bypass (Ombra fix) |
| 2 | Stack walking | HIGH | Return address spoofing |
| 3 | Thread start addr | HIGH | Thread hijacking + spoof |
| 4 | Memory type scan | MEDIUM | EPT hiding |
| 5 | Mono integrity | MEDIUM | Don't touch managed DLLs |
| 6 | Module blacklist | LOW | Avoid known names |
| 7 | Window enum | LOW | No visible windows |

---

## 10. Recommended Ombra SDK Enhancements

### Phase 1: Core Stealth (Hypervisor Level)
```
[x] EPT memory hiding
[x] CPUID spoofing
[ ] TSC compensation ← CRITICAL
[ ] APERF/MPERF interception
```

### Phase 2: Thread Safety (Internal Module)
```
[ ] Return address spoofer utility
[ ] Thread hijack helpers
[ ] Safe callback execution framework
[ ] Stack cleanup utilities
```

### Phase 3: Memory Layout (Internal Module)
```
[ ] MEM_IMAGE allocation wrapper
[ ] Code cave finder
[ ] PE header management
[ ] Safe section helpers
```

### Phase 4: Hook Safety
```
[x] EPT execute-only hooks
[ ] Hardware breakpoint manager
[ ] Callback-based execution
[ ] CoW-safe patching utilities
```

---

## Sources

1. secret.club - BattlEye analysis articles (2019-2020)
2. secret.club - EAC integrity check bypass (2020)
3. secret.club - Hypervisor detection methods (2020)
4. secret.club - RE tracking (2020)
5. GuidedHacking - EAC bypass guide
6. GuidedHacking - Thread detection
7. overlayhack.com - EAC/EAAC bypass
8. MDSec - Beacon detection techniques
9. Elastic Security Labs - Thread trampoline detection
10. Various GitHub anti-cheat implementations

---

*Document compiled from comprehensive research on modern anti-cheat internals*
*Focus: Internal cheat detection in BattlEye-protected games (EFT)*
