# Shellcode Signature Audit

**Audit Date:** December 25, 2025
**Auditor:** Subagent 3 - Shellcode Analysis Specialist

---

## Executive Summary

The OmbraHypervisor codebase contains **multiple high-signature shellcode patterns** that anti-cheat systems can detect. These patterns span across hooking, ROP gadget signatures, VMExit handler scanning, and embedded driver resources. The most critical finding is the embedded Intel NAL driver byte array, which is a well-known publicly signaturable artifact.

---

## Critical Findings

### 1. Embedded Intel Driver Resource (HIGHEST PRIORITY)

**File:** `OmbraCoreLib/kdmapper_lib/kdmapper/include/intel_driver_resource.hpp:6-871+`

**Signature Pattern:**
```cpp
0x4D, 0x5A, 0x90, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xFF, 0xFF...
```

| Property | Value |
|----------|-------|
| Detection Risk | **CRITICAL** |
| Public Match | YES - kdmapper exact binary |
| Size | ~630KB embedded |
| Obfuscation | NONE |

**Why Critical:** This is the Intel NAL driver used by every public kdmapper implementation. Anti-cheat databases specifically include this binary's signatures.

---

### 2. Hook Trampolines (HIGH PRIORITY)

**Files:**
- `libombra/mapper/hook.hpp:97-123`
- `OmbraCoreLib/phymeme_lib/util/hook.hpp:97-123`

**Signature Pattern (x64):**
```cpp
0x48, 0xB8,                    // mov rax, imm64
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xFF, 0xE0                     // jmp rax
```

| Property | Value |
|----------|-------|
| Detection Risk | **HIGH** |
| Public Match | YES - physmeme, kdmapper |
| Pattern | `48 B8 ?? ?? ?? ?? ?? ?? ?? ?? FF E0` |
| Size | 12 bytes |

**Register Alternatives:**
```cpp
// RCX variant
0x48, 0xB9, <addr>, 0xFF, 0xE1

// RDX variant
0x48, 0xBA, <addr>, 0xFF, 0xE2

// R8 variant
0x49, 0xB8, <addr>, 0x41, 0xFF, 0xE0

// R15 variant
0x49, 0xBF, <addr>, 0x41, 0xFF, 0xE7
```

---

### 3. VMExit Handler Signatures (HIGH PRIORITY)

**File:** `OmbraLoader/zerohvci/version_detect.h:100-189`

**Intel Pattern (Build 16299+):**
```cpp
0x65, 0xC6, 0x04, 0x25, 0x6D, 0x00, 0x00, 0x00, 0x00,  // mov byte ptr gs:[0x6D], 0
0x48, 0x8B, 0x4C, 0x24, 0x00,                            // mov rcx, [rsp+?]
0x48, 0x8B, 0x54, 0x24, 0x00,                            // mov rdx, [rsp+?]
0xE8, 0x00, 0x00, 0x00, 0x00,                            // call <offset>
0xE9                                                      // jmp
```

| Property | Value |
|----------|-------|
| Detection Risk | **HIGH** |
| Public Match | NO (Hyper-V specific) |
| Pattern Length | 25 bytes |
| Version Specific | YES |

---

### 4. ROP Gadget Signatures (MEDIUM PRIORITY)

**File:** `OmbraLoader/zerohvci/kforge.h:123-168`

**Guard Retpoline Exit Gadget:**
```cpp
0x48, 0x8B, 0x44, 0x24, 0x20,  // mov rax, [rsp+0x20]
0x48, 0x8B, 0x4C, 0x24, 0x28,  // mov rcx, [rsp+0x28]
0x48, 0x8B, 0x54, 0x24, 0x30,  // mov rdx, [rsp+0x30]
0x4C, 0x8B, 0x44, 0x24, 0x38,  // mov r8, [rsp+0x38]
0x4C, 0x8B, 0x4C, 0x24, 0x40,  // mov r9, [rsp+0x40]
0x48, 0x83, 0xC4, 0x48,        // add rsp, 0x48
0x48, 0xFF, 0xE0               // jmp rax
```

| Property | Value |
|----------|-------|
| Detection Risk | MEDIUM |
| Size | 38 bytes |
| Public Match | PARTIAL |

**Other Gadgets:**
- Stack alignment: `48 83 C4 68 C3` (add rsp, 0x68; ret)
- RCX load: `59 C3` (pop rcx; ret)
- Store RAX: `48 89 01 C3` (mov [rcx], rax; ret)

---

### 5. Syscall Shellcode (HIGH PRIORITY)

**File:** `OmbraLoader/zerohvci/utils.h:500-525`

**Pattern:**
```cpp
0x48, 0x8B, 0xC4,             // mov rax, rsp
0xFA,                          // cli
0x48, 0x83, 0xEC, 0x10,       // sub rsp, 0x10
0x50,                          // push rax
0x9C,                          // pushfq
0x6A, 0x10,                    // push 0x10
0x48, 0x8D, 0x05, 0xFF, 0xFF, 0xFF, 0xFF,  // lea rax, [rip+offset]
0x50,                          // push rax
0xB8, 0x00, 0x00, 0x00, 0x00, // mov eax, <syscall_number>
0xE9, 0xFF, 0xFF, 0xFF, 0xFF  // jmp <KiServiceInternal>
```

| Property | Value |
|----------|-------|
| Detection Risk | HIGH |
| Size | 35 bytes |
| Distinctive | CLI instruction rare in user code |

---

### 6. Trampoline Generation (MEDIUM PRIORITY)

**File:** `OmbraLoader/zerohvci/trampoline.h:36-93`

**Structure:**
```cpp
struct TrampolineCode {
    uint8_t MovRaxOpcode[2];      // 0x48, 0xB8
    uint64_t TargetAddress;       // 8-byte address
    uint8_t JmpRaxOpcode[2];      // 0xFF, 0xE0
};
```

| Property | Value |
|----------|-------|
| Detection Risk | MEDIUM |
| Generated At | Runtime |
| Size | 12 bytes per trampoline |

---

## Summary Table

| Pattern | File | Lines | Risk | Public Match | Evadable |
|---------|------|-------|------|--------------|----------|
| Intel Driver Binary | intel_driver_resource.hpp | 6-871 | **CRITICAL** | YES (kdmapper) | NO |
| Hook Trampoline (x64) | hook.hpp | 97-123 | **HIGH** | YES (physmeme) | YES |
| VMExit Intel Signature | version_detect.h | 108-189 | **HIGH** | NO | YES |
| Syscall Shellcode | utils.h | 500-525 | **HIGH** | PARTIAL | YES |
| ROP Gadget Signatures | kforge.h | 123-168 | MEDIUM | PARTIAL | YES |
| Trampoline Generation | trampoline.h | 36-93 | MEDIUM | MEDIUM | YES |

---

## Polymorphic Alternatives

### Hook Trampoline Polymorphism

**Current (Static RAX):**
```asm
mov rax, 0xFFFF000000000000
jmp rax
```

**Polymorphic Option 1 (Random Register):**
```cpp
void BuildPolymorphicTrampoline(uint8_t* buffer, uint64_t target) {
    static const struct {
        uint8_t mov_op[2];
        uint8_t jmp_op[2];
    } variants[] = {
        {{0x48, 0xB8}, {0xFF, 0xE0}},  // RAX
        {{0x48, 0xB9}, {0xFF, 0xE1}},  // RCX
        {{0x48, 0xBA}, {0xFF, 0xE2}},  // RDX
        {{0x49, 0xB8}, {0x41, 0xFF, 0xE0}},  // R8
    };

    auto& v = variants[__rdtsc() % 4];
    memcpy(buffer, v.mov_op, 2);
    memcpy(buffer + 2, &target, 8);
    memcpy(buffer + 10, v.jmp_op, 2);
}
```

**Polymorphic Option 2 (Split Address):**
```asm
xor rax, rax
mov eax, 0xFFFF0000       ; Low 32 bits
shl rax, 32
or eax, 0x00000000        ; High 32 bits
jmp rax
```

**Polymorphic Option 3 (Indirect):**
```asm
lea rax, [rip + target_table]
mov rax, [rax]
jmp rax
```

---

## Pool Tag Rotation (Positive Finding)

**File:** `OmbraLoader/zerohvci/kforge.h:19-45`

**Status:** IMPLEMENTED - Uses legitimate Windows tags:
- 'sftN' (Ntfs.sys)
- 'eliF' (File objects)
- 'pRI ' (IRP allocations)
- 'looP' (Pool allocations)
- 'dteR' (Registry)
- 'gaTI' (I/O tag)
- 'kroW' (Work items)
- 'truC' (Current allocations)
- 'dmI ' (Image loader)
- 'aeSK' (Ksec security)

**Effectiveness:** HIGH - Good anti-forensic technique for pool allocations.

---

## Recommendations

### Priority 1: Remove Intel Driver Binary
```bash
rm OmbraCoreLib/kdmapper_lib/kdmapper/include/intel_driver_resource.hpp
```
Or encrypt at compile time, decrypt at runtime.

### Priority 2: Implement Hook Polymorphism
Random register selection at runtime. See polymorphic options above.

### Priority 3: Obfuscate VMExit Signatures
Use hash-based pattern matching instead of raw byte sequences.

### Priority 4: Vary ROP Gadgets
Generate gadgets with different register combinations.

---

## Overall Assessment

**Detection Surface:** LARGE

The Intel driver alone is instantly signaturable. Hook trampolines match public projects. VMExit patterns are version-specific but detectable. ROP gadgets are more generic but recognizable in context.

**Most Impactful Improvement:** Replace or encrypt the Intel driver binary - it represents the single largest detection risk.
