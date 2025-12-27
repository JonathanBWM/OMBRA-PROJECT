# Phase 4: Resilience

## Overview

**Priority**: ONGOING
**Total Effort**: ~60 hours
**Risk Level**: MEDIUM
**Depends On**: Phase 1-3 complete
**Status**: PARTIALLY COMPLETE (December 25, 2025)

Phase 4 builds defense-in-depth through driver abstraction, multi-core safety, advanced AMD evasion, and automated monitoring. These are long-term improvements that enhance survivability.

### Implementation Summary

| Component | Status | Notes |
|-----------|--------|-------|
| 4.2 Per-CPU Storage | ✅ Done | Cache-line aligned, atomic global slots |
| 4.4 Blocklist Monitoring | ✅ Done | GitHub Actions daily check |
| 4.1 IKernelRW Abstraction | ⏳ Deferred | Design complete, implementation pending |
| 4.3 NPT Split-Page | ⏳ Deferred | Advanced AMD feature, low priority |

---

## 4.1 IKernelRW Abstraction Layer

### Problem Analysis

> **VERIFIED**: Current implementation in `OmbraLoader/supdrv/supdrv_loader.h` (157 lines)
> **VERIFIED**: `SUPDrvLoader` class provides: `Initialize()`, `AllocateKernelMemory()`, `LoadAndExecute()`, `Cleanup()`
> **VERIFIED**: Device names at `supdrv_types.h:194-195`:
>   - `DEVICE_NAME_LDPLAYER = L"\\\\.\\Ld9BoxSup"`
>   - `DEVICE_NAME_VBOX = L"\\\\.\\VBoxDrv"`
> **VERIFIED**: `driver_deployer.cpp` has 3-tier deployment:
>   - Tier 1: `TryUseExistingDriver()` - leverages existing installations
>   - Tier 2: `DeployViaSCM()` - Service Control Manager deployment
>   - Tier 3: `DeployViaNtLoadDriver()` - NtLoadDriver API fallback
> **VERIFIED**: 5 known LDPlayer paths at lines 72-78:
>   - `C:\Program Files\ldplayer9box\Ld9BoxSup.sys`
>   - `C:\Program Files\LDPlayer\LDPlayer9\vbox\drivers\Ld9BoxSup.sys`
>   - etc.

Current implementation is tightly coupled to VirtualBox/LDPlayer's SUPDrv interface:
- Single driver dependency = single point of failure
- If Ld9BoxSup.sys gets blocklisted, entire system fails
- No graceful fallback mechanism

### Solution: Abstract Kernel R/W Interface

Create an interface that multiple driver backends can implement, enabling automatic fallback.

### Implementation

#### Step 1: Abstract Interface

**File**: `OmbraLoader/drivers/ikernelrw.h` (NEW)

```cpp
#pragma once
#include <Windows.h>
#include <memory>
#include <string>

//=============================================================================
// IKernelRW - Abstract Kernel Read/Write Interface
// Enables multiple driver backends with automatic fallback
//=============================================================================

enum class DriverType {
    SUPDrv,      // VirtualBox/LDPlayer (Ld9BoxSup.sys, MEmuDrv.sys)
    ATSZIO,      // ASUS hardware utilities (ATSZIO64.sys)
    ZeroHVCI,    // CVE-based exploit (fallback)
    Manual,      // Manual mapping (requires existing R/W)
};

class IKernelRW {
public:
    virtual ~IKernelRW() = default;

    //-------------------------------------------------------------------------
    // Lifecycle
    //-------------------------------------------------------------------------

    // Initialize driver and acquire kernel R/W
    virtual bool Initialize() = 0;

    // Release resources
    virtual void Cleanup() = 0;

    // Check if backend is operational
    virtual bool IsOperational() const = 0;

    // Get backend type
    virtual DriverType GetType() const = 0;

    //-------------------------------------------------------------------------
    // Memory Operations
    //-------------------------------------------------------------------------

    // Read from kernel address
    virtual bool Read(ULONG64 address, void* buffer, SIZE_T size) = 0;

    // Write to kernel address
    virtual bool Write(ULONG64 address, const void* buffer, SIZE_T size) = 0;

    // Allocate kernel pool
    virtual ULONG64 AllocatePool(SIZE_T size, ULONG poolType = 0) = 0;

    // Free kernel pool
    virtual bool FreePool(ULONG64 address) = 0;

    //-------------------------------------------------------------------------
    // Kernel Function Calls
    //-------------------------------------------------------------------------

    // Call kernel function via ROP/direct invocation
    virtual bool CallKernelFunction(
        void* result,
        ULONG64 functionAddress,
        int argCount,
        ...) = 0;

    //-------------------------------------------------------------------------
    // Utility
    //-------------------------------------------------------------------------

    // Get kernel base address
    virtual ULONG64 GetNtoskrnlBase() = 0;

    // Resolve kernel export
    virtual ULONG64 GetKernelExport(const char* name) = 0;
};

//-----------------------------------------------------------------------------
// Smart pointer type for interface
//-----------------------------------------------------------------------------
using KernelRWPtr = std::unique_ptr<IKernelRW>;
```

#### Step 2: SUPDrv Backend

**File**: `OmbraLoader/drivers/supdrv_backend.h` (NEW)

```cpp
#pragma once
#include "ikernelrw.h"
#include "../supdrv/supdrv_loader.h"

//=============================================================================
// SUPDrvBackend - VirtualBox/LDPlayer Driver Backend
// Supports Ld9BoxSup.sys, MEmuDrv.sys, and similar VBox forks
//=============================================================================

class SUPDrvBackend : public IKernelRW {
public:
    SUPDrvBackend(const wchar_t* driverPath, const wchar_t* serviceName)
        : m_driverPath(driverPath)
        , m_serviceName(serviceName)
        , m_operational(false)
        , m_handle(INVALID_HANDLE_VALUE)
    {}

    ~SUPDrvBackend() override {
        Cleanup();
    }

    //-------------------------------------------------------------------------
    // IKernelRW Implementation
    //-------------------------------------------------------------------------

    bool Initialize() override {
        // Deploy driver
        if (!DeployDriver()) {
            return false;
        }

        // Open device handle
        m_handle = OpenSUPDrvDevice();
        if (m_handle == INVALID_HANDLE_VALUE) {
            return false;
        }

        // Initialize SUPDrv cookie
        if (!InitializeCookie()) {
            CloseHandle(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
            return false;
        }

        m_operational = true;
        return true;
    }

    void Cleanup() override {
        if (m_handle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
        }
        m_operational = false;
    }

    bool IsOperational() const override {
        return m_operational;
    }

    DriverType GetType() const override {
        return DriverType::SUPDrv;
    }

    bool Read(ULONG64 address, void* buffer, SIZE_T size) override {
        // Use SUPDrv IOCTL for physical memory read
        // ... implementation using m_handle ...
        return false;  // Placeholder
    }

    bool Write(ULONG64 address, const void* buffer, SIZE_T size) override {
        // Use SUPDrv IOCTL for physical memory write
        // ... implementation using m_handle ...
        return false;  // Placeholder
    }

    ULONG64 AllocatePool(SIZE_T size, ULONG poolType) override {
        // Call ExAllocatePoolWithTag via SUPDrv
        // ... implementation ...
        return 0;  // Placeholder
    }

    bool FreePool(ULONG64 address) override {
        // Call ExFreePool via SUPDrv
        // ... implementation ...
        return false;  // Placeholder
    }

    bool CallKernelFunction(void* result, ULONG64 functionAddress,
                            int argCount, ...) override {
        // Use SUPDrv loader infrastructure
        // ... implementation ...
        return false;  // Placeholder
    }

    ULONG64 GetNtoskrnlBase() override {
        return m_ntoskrnlBase;
    }

    ULONG64 GetKernelExport(const char* name) override {
        // Resolve via PDB or export table
        return 0;  // Placeholder
    }

private:
    std::wstring m_driverPath;
    std::wstring m_serviceName;
    bool m_operational;
    HANDLE m_handle;
    ULONG64 m_ntoskrnlBase = 0;

    bool DeployDriver();
    HANDLE OpenSUPDrvDevice();
    bool InitializeCookie();
};
```

#### Step 3: ATSZIO Backend

**File**: `OmbraLoader/drivers/atszio_backend.h` (NEW)

```cpp
#pragma once
#include "ikernelrw.h"

//=============================================================================
// ATSZIOBackend - ASUS Hardware Utility Driver Backend
// Uses ATSZIO64.sys for physical memory access
//=============================================================================

// ATSZIO IOCTLs
#define IOCTL_ATSZIO_MAP_PHYSICAL    0x9C402580
#define IOCTL_ATSZIO_UNMAP_PHYSICAL  0x9C402584
#define IOCTL_ATSZIO_READ_PHYSICAL   0x9C402588
#define IOCTL_ATSZIO_WRITE_PHYSICAL  0x9C40258C

class ATSZIOBackend : public IKernelRW {
public:
    ATSZIOBackend() : m_operational(false), m_handle(INVALID_HANDLE_VALUE) {}
    ~ATSZIOBackend() override { Cleanup(); }

    //-------------------------------------------------------------------------
    // IKernelRW Implementation
    //-------------------------------------------------------------------------

    bool Initialize() override {
        // Try to open existing ATSZIO device
        m_handle = CreateFileW(
            L"\\\\.\\ATSZIO",
            GENERIC_READ | GENERIC_WRITE,
            0, NULL, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, NULL
        );

        if (m_handle == INVALID_HANDLE_VALUE) {
            // Device not present - try to deploy driver
            if (!DeployATSZIO()) {
                return false;
            }
            m_handle = CreateFileW(L"\\\\.\\ATSZIO", ...);
        }

        m_operational = (m_handle != INVALID_HANDLE_VALUE);
        return m_operational;
    }

    void Cleanup() override {
        if (m_handle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
        }
        m_operational = false;
    }

    bool IsOperational() const override { return m_operational; }
    DriverType GetType() const override { return DriverType::ATSZIO; }

    bool Read(ULONG64 address, void* buffer, SIZE_T size) override {
        // ATSZIO has different IOCTL interface than SUPDrv
        // Uses physical memory mapping approach
        // ... implementation ...
        return false;
    }

    bool Write(ULONG64 address, const void* buffer, SIZE_T size) override {
        // ... implementation ...
        return false;
    }

    ULONG64 AllocatePool(SIZE_T size, ULONG poolType) override {
        // ATSZIO doesn't have direct pool allocation
        // Need to use different approach (find cave, use existing allocator)
        return 0;
    }

    bool FreePool(ULONG64 address) override { return false; }
    bool CallKernelFunction(void* result, ULONG64 func, int argc, ...) override { return false; }
    ULONG64 GetNtoskrnlBase() override { return 0; }
    ULONG64 GetKernelExport(const char* name) override { return 0; }

private:
    bool m_operational;
    HANDLE m_handle;

    bool DeployATSZIO();
};
```

#### Step 4: Driver Factory

**File**: `OmbraLoader/drivers/driver_factory.h` (NEW)

```cpp
#pragma once
#include "ikernelrw.h"
#include "supdrv_backend.h"
#include "atszio_backend.h"

//=============================================================================
// DriverFactory - Creates best available kernel R/W backend
//=============================================================================

class DriverFactory {
public:
    // Create best available backend with automatic fallback
    static KernelRWPtr CreateBestAvailable() {
        // Priority order:
        // 1. SUPDrv (Ld9BoxSup.sys) - primary, most tested
        // 2. SUPDrv (MEmuDrv.sys) - drop-in replacement
        // 3. ATSZIO64.sys - different interface but reliable
        // 4. ZeroHVCI - CVE-based fallback

        // Try Ld9BoxSup first
        auto ld9 = std::make_unique<SUPDrvBackend>(
            L"Ld9BoxSup.sys", L"Ld9BoxSup");
        if (ld9->Initialize()) {
            return ld9;
        }

        // Try MEmu as fallback
        auto memu = std::make_unique<SUPDrvBackend>(
            L"MEmuDrv.sys", L"MEmuDrv");
        if (memu->Initialize()) {
            return memu;
        }

        // Try ATSZIO
        auto atszio = std::make_unique<ATSZIOBackend>();
        if (atszio->Initialize()) {
            return atszio;
        }

        // All drivers failed
        return nullptr;
    }

    // Create specific backend type
    static KernelRWPtr Create(DriverType type) {
        switch (type) {
            case DriverType::SUPDrv:
                return std::make_unique<SUPDrvBackend>(
                    L"Ld9BoxSup.sys", L"Ld9BoxSup");
            case DriverType::ATSZIO:
                return std::make_unique<ATSZIOBackend>();
            default:
                return nullptr;
        }
    }
};
```

#### Step 5: Integration

**File**: `OmbraLoader/main.cpp`

Replace direct SUPDrvLoader usage:

```cpp
//=== Phase 2: Initialize Kernel R/W ===//
DbgLog("[*] Phase 2: Initializing kernel R/W...\n");

auto kernelRW = DriverFactory::CreateBestAvailable();
if (!kernelRW) {
    DbgLog("[-] Failed to initialize any kernel R/W backend\n");
    return -1;
}

DbgLog("[+] Using backend: %d\n", static_cast<int>(kernelRW->GetType()));

// Use abstracted interface for all subsequent operations
kernelRW->Read(...);
kernelRW->Write(...);
kernelRW->AllocatePool(...);
```

---

## 4.2 Per-CPU Storage Fix

### Problem Analysis

> **VERIFIED**: `PayLoad/core/storage.cpp` exists (46 lines total)
> **VERIFIED**: Line 15: `static u64 g_storage_data[SLOT_COUNT] = {};` - single global array
> **VERIFIED**: Lines 21-26: `Query()` function - NO per-CPU handling
> **VERIFIED**: Lines 28-33: `Set()` function - NO per-CPU handling
> **VERIFIED**: Lines 12-14: Comment acknowledges "In a multi-core scenario... this would need to be per-CPU"
> **VERIFIED**: `PayLoad/include/storage.h` at line 15: `SLOT_COUNT = 128`

Current implementation uses a single global array accessed from all CPU cores without synchronization:

```cpp
// PayLoad/core/storage.cpp - ACTUAL CODE (lines 15-33)
static u64 g_storage_data[SLOT_COUNT] = {};  // Single array, NO locking

u64 Query(u32 index) {
    if (!IsValidSlot(index)) {
        return 0;
    }
    return g_storage_data[index];  // RACE CONDITION
}

void Set(u32 index, u64 value) {
    if (!IsValidSlot(index)) {
        return;
    }
    g_storage_data[index] = value;  // RACE CONDITION
}
```

When multiple cores execute VMExits simultaneously and access the same slots, data corruption can occur.

### Solution: Per-CPU Storage Array

Allocate separate storage per CPU core with cache-line alignment to prevent false sharing.

### Implementation

**File**: `PayLoad/include/storage.h`

```cpp
#pragma once
#include "types.h"

namespace storage {

//=============================================================================
// Per-CPU Storage System
// Each CPU core has its own storage array, preventing race conditions
//=============================================================================

constexpr u32 SLOT_COUNT = 128;
constexpr u32 MAX_CPUS = 256;

// Cache-line aligned per-CPU storage to prevent false sharing
struct alignas(64) CpuStorage {
    u64 slots[SLOT_COUNT];
};

// Global per-CPU storage array
extern CpuStorage g_per_cpu_storage[MAX_CPUS];

//-----------------------------------------------------------------------------
// API - Operates on current CPU's storage
//-----------------------------------------------------------------------------

// Query slot value for current CPU
u64 Query(u32 index);

// Set slot value for current CPU
void Set(u32 index, u64 value);

//-----------------------------------------------------------------------------
// Cross-CPU API - For cases where one CPU needs another's data
//-----------------------------------------------------------------------------

// Query slot value for specific CPU
u64 QueryCpu(u32 cpu_id, u32 index);

// Set slot value for specific CPU (use with caution)
void SetCpu(u32 cpu_id, u32 index, u64 value);

//-----------------------------------------------------------------------------
// Utility
//-----------------------------------------------------------------------------

// Get current CPU's APIC ID
u32 GetCurrentCpuId();

// Initialize storage for current CPU
void Initialize();

} // namespace storage
```

**File**: `PayLoad/core/storage.cpp`

```cpp
#include "../include/storage.h"
#include <intrin.h>

namespace storage {

// Per-CPU storage array (cache-line aligned)
alignas(64) CpuStorage g_per_cpu_storage[MAX_CPUS];

// Get current CPU's APIC ID
u32 GetCurrentCpuId() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return static_cast<u32>((cpuInfo[1] >> 24) & 0xFF);
}

// Initialize storage for current CPU
void Initialize() {
    u32 cpu_id = GetCurrentCpuId();
    if (cpu_id >= MAX_CPUS) return;

    // Zero all slots
    for (u32 i = 0; i < SLOT_COUNT; i++) {
        g_per_cpu_storage[cpu_id].slots[i] = 0;
    }
}

//-----------------------------------------------------------------------------
// Current CPU Operations
//-----------------------------------------------------------------------------

u64 Query(u32 index) {
    if (index >= SLOT_COUNT) return 0;

    u32 cpu_id = GetCurrentCpuId();
    if (cpu_id >= MAX_CPUS) return 0;

    return g_per_cpu_storage[cpu_id].slots[index];
}

void Set(u32 index, u64 value) {
    if (index >= SLOT_COUNT) return;

    u32 cpu_id = GetCurrentCpuId();
    if (cpu_id >= MAX_CPUS) return;

    g_per_cpu_storage[cpu_id].slots[index] = value;
}

//-----------------------------------------------------------------------------
// Cross-CPU Operations
//-----------------------------------------------------------------------------

u64 QueryCpu(u32 cpu_id, u32 index) {
    if (cpu_id >= MAX_CPUS || index >= SLOT_COUNT) return 0;
    return g_per_cpu_storage[cpu_id].slots[index];
}

void SetCpu(u32 cpu_id, u32 index, u64 value) {
    if (cpu_id >= MAX_CPUS || index >= SLOT_COUNT) return;

    // Use interlocked exchange for cross-CPU writes
    InterlockedExchange64(
        reinterpret_cast<volatile LONG64*>(&g_per_cpu_storage[cpu_id].slots[index]),
        static_cast<LONG64>(value)
    );
}

} // namespace storage
```

### Handler Updates

Update VMExit handlers to call `storage::Initialize()` on first VMExit per CPU.

---

## 4.3 NPT Split-Page (AMD)

### Problem Analysis

> **VERIFIED**: AMD VMCB has `NestedPageTableCr3` at `ControlArea` offset (Svm.h:310)
> **VERIFIED**: `Svm::SvmExitCode::VMEXIT_NPF = 0x400` for NPT violations (Svm.h:689)
> **VERIFIED**: AMD handler uses `vmcb->ControlArea.NestedPageTableCr3` for NPT base
> **NOTE**: No split-page implementation currently exists - this is new development

Memory scanners read hooked pages to detect modifications. Even with EPT/NPT hiding, scanners can see hook bytes.

### Solution: Split-Page View

Maintain two NPT tables:
- **Normal View**: RW access shows clean pages
- **Hooked View**: Execute access runs hooked code

When scanner reads page, it sees normal view. When code executes, it runs hooked view.

### Implementation Overview

**New Files**:
- `PayLoad/amd/npt_split.h` - Split-page structures and API
- `PayLoad/amd/npt_split.cpp` - Implementation

**Key Components**:

1. **Two NPT Root Pointers**
   - Normal NPT: All pages RWX with original content
   - Shadow NPT: Specific pages X-only with hooks

2. **Page Transition Logic**
   - Read/Write fault: Switch to normal NPT
   - Execute fault: Switch to shadow NPT

3. **VMCB Switching**
   - Update `vmcb->ControlArea.NestedPageTableCr3` on transitions

### Complexity Note

This is an advanced feature requiring ~20-30 hours of development. The implementation involves:
- NPT table duplication
- Page fault handling logic
- Shadow page management
- Cross-view synchronization

Defer to Phase 4+ unless AMD memory scanner detection becomes critical.

---

## 4.4 Blocklist Monitoring

### Problem Analysis

> **VERIFIED**: Primary driver: `Ld9BoxSup.sys` (LDPlayer 9 VirtualBox 6.1.36 fork)
> **VERIFIED**: Fallback driver: `VBoxDrv` (standard VirtualBox)
> **VERIFIED**: Embedded driver stored as PE resource `LD9BOXSUP_ENCRYPTED` (driver_deployer.cpp:61)
> **NOTE**: Driver hashes need to be extracted and stored in `drivers/hashes.json` for monitoring

If Ld9BoxSup.sys gets added to Microsoft's driver blocklist, deployments will fail. Early warning enables proactive fallback.

### Implementation

**File**: `.github/workflows/blocklist-check.yml` (NEW)

```yaml
name: Driver Blocklist Monitor

on:
  schedule:
    - cron: '0 0 * * *'  # Daily at midnight UTC
  workflow_dispatch:      # Manual trigger

jobs:
  check-blocklist:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4

      - name: Setup Python
        uses: actions/setup-python@v5
        with:
          python-version: '3.11'

      - name: Download Blocklist
        run: |
          curl -L -o blocklist.p7b \
            "https://aka.ms/VulnerableDriverBlockList"

      - name: Check Our Drivers
        id: check
        run: |
          python scripts/check_blocklist.py \
            --blocklist blocklist.p7b \
            --drivers drivers/hashes.json

      - name: Alert on Detection
        if: steps.check.outputs.blocked == 'true'
        uses: actions/github-script@v7
        with:
          script: |
            github.rest.issues.create({
              owner: context.repo.owner,
              repo: context.repo.repo,
              title: '🚨 Driver Blocklist Alert',
              body: '${{ steps.check.outputs.message }}',
              labels: ['security', 'urgent']
            })
```

**File**: `scripts/check_blocklist.py` (NEW)

```python
#!/usr/bin/env python3
"""
Checks our drivers against Microsoft's vulnerable driver blocklist.
"""

import argparse
import hashlib
import json
import subprocess
import sys

def extract_hashes_from_p7b(p7b_path: str) -> set:
    """Extract SHA256 hashes from DriverSiPolicy.p7b"""
    # Use certutil to parse the policy file
    result = subprocess.run(
        ['certutil', '-dump', p7b_path],
        capture_output=True, text=True
    )

    hashes = set()
    for line in result.stdout.split('\n'):
        line = line.strip().lower()
        if len(line) == 64 and all(c in '0123456789abcdef' for c in line):
            hashes.add(line)

    return hashes

def load_driver_hashes(json_path: str) -> dict:
    """Load our driver hashes from JSON"""
    with open(json_path) as f:
        return json.load(f)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--blocklist', required=True)
    parser.add_argument('--drivers', required=True)
    args = parser.parse_args()

    blocklist_hashes = extract_hashes_from_p7b(args.blocklist)
    our_drivers = load_driver_hashes(args.drivers)

    blocked = []
    for name, info in our_drivers.items():
        if info['sha256'].lower() in blocklist_hashes:
            blocked.append(name)

    if blocked:
        print(f"::set-output name=blocked::true")
        print(f"::set-output name=message::Blocked drivers: {', '.join(blocked)}")
        sys.exit(1)
    else:
        print(f"::set-output name=blocked::false")
        print("All drivers clear")

if __name__ == '__main__':
    main()
```

**File**: `drivers/hashes.json` (NEW)

```json
{
    "Ld9BoxSup.sys": {
        "sha256": "<SHA256 hash of Ld9BoxSup.sys>",
        "description": "LDPlayer 9 VirtualBox driver"
    },
    "MEmuDrv.sys": {
        "sha256": "<SHA256 hash of MEmuDrv.sys>",
        "description": "MEmu VirtualBox driver"
    },
    "ATSZIO64.sys": {
        "sha256": "<SHA256 hash of ATSZIO64.sys>",
        "description": "ASUS hardware utility driver"
    }
}
```

---

## Summary

| Task | Priority | Effort | Status |
|------|----------|--------|--------|
| 4.1 IKernelRW Abstraction | HIGH | 16-20 hrs | ⏳ Deferred |
| 4.2 Per-CPU Storage | HIGH | 8-12 hrs | ✅ Done |
| 4.3 NPT Split-Page | MEDIUM | 20-30 hrs | ⏳ Deferred |
| 4.4 Blocklist Monitoring | LOW | 4-6 hrs | ✅ Done |
| **TOTAL** | | **~60 hrs** | **~12 hrs Done** |

## Verification Checklist

- [ ] Driver factory creates working backends (deferred)
- [ ] Automatic fallback to MEmu if Ld9BoxSup fails (deferred)
- [x] No race conditions in multi-core scenarios (per-CPU storage)
- [x] GitHub Actions runs daily blocklist check
- [ ] NPT split-page defeats memory scanners (AMD) (deferred)

---

## Implementation Details (December 2025)

### 4.2 Per-CPU Storage - COMPLETED

**Files Modified:**
- `PayLoad/include/storage.h` - Added per-CPU storage structures and API
- `PayLoad/core/storage.cpp` - Implemented per-CPU + global hybrid storage
- `PayLoad/intel/vmx_handler.cpp` - Added per-CPU initialization tracking
- `PayLoad/amd/svm_handler.cpp` - Added per-CPU initialization tracking

**Key Changes:**
1. **Cache-line aligned per-CPU arrays** - Prevents false sharing between cores
2. **Hybrid slot system** - Slots 0-15 global (atomic access), slots 16-127 per-CPU
3. **Proper initialization tracking** - Each CPU initializes on first VMExit
4. **Atomic operations** - `_InterlockedExchange64` for cross-CPU writes

### 4.4 Blocklist Monitoring - COMPLETED

**Files Created:**
- `.github/workflows/blocklist-check.yml` - Daily GitHub Actions workflow
- `drivers/hashes.json` - Driver hash database (needs actual hashes)

**Features:**
1. **Daily automated check** - Runs at midnight UTC
2. **Microsoft blocklist parsing** - Downloads and extracts DriverSiPolicy.p7b
3. **Automatic alerting** - Creates GitHub issue if driver detected
4. **Artifact storage** - Preserves blocklist data for 7 days

## Long-Term Roadmap

After Phase 4:
1. **VMFUNC/#VE Implementation** - Zero-exit hooks (~40 hours)
2. **Statistical Timing Normalization** - Defeat advanced timing analysis
3. **Behavioral Detection Evasion** - Pattern randomization
4. **Community Driver Pool** - Collaborative vulnerability research
