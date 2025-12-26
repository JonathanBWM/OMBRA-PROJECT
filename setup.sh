#!/bin/bash
#
# generate_ombra_project.sh
# OmbraHypervisor Project Generator
#
# Creates the hypervisor skeleton matching PROJECT_STRUCTURE.md exactly.
# Designed for: macOS development → GitHub Actions Windows build
#
# Usage: ./generate_ombra_project.sh [target_directory]
#

set -e

TARGET_DIR="${1:-.}"
PROJECT_ROOT="$TARGET_DIR/OmbraHypervisor"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; CYAN='\033[0;36m'; NC='\033[0m'

echo -e "${CYAN}"
echo "   ╔═══════════════════════════════════════╗"
echo "   ║        OmbraHypervisor Generator      ║"
echo "   ╚═══════════════════════════════════════╝"
echo -e "${NC}"
echo -e "${YELLOW}Target: $PROJECT_ROOT${NC}"
echo ""

[ -d "$PROJECT_ROOT/hypervisor" ] && { echo -e "${RED}Error: hypervisor/ exists${NC}"; exit 1; }

mkdir -p "$PROJECT_ROOT"

# =============================================================================
echo -e "${GREEN}[1/6]${NC} Creating directories..."
# =============================================================================

mkdir -p "$PROJECT_ROOT/hypervisor/usermode"
mkdir -p "$PROJECT_ROOT/hypervisor/hypervisor/handlers"
mkdir -p "$PROJECT_ROOT/hypervisor/hypervisor/asm"
mkdir -p "$PROJECT_ROOT/hypervisor/shared"
mkdir -p "$PROJECT_ROOT/hypervisor/build"
mkdir -p "$PROJECT_ROOT/hypervisor/docs"
mkdir -p "$PROJECT_ROOT/hypervisor/.vscode"
mkdir -p "$PROJECT_ROOT/hypervisor/.github/workflows"

# =============================================================================
echo -e "${GREEN}[2/6]${NC} Creating GitHub Actions..."
# =============================================================================

cat > "$PROJECT_ROOT/hypervisor/.github/workflows/build.yml" << 'GHACTION'
name: Build OmbraHypervisor

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]
  workflow_dispatch:

jobs:
  build:
    runs-on: windows-latest
    steps:
    - uses: actions/checkout@v4
    
    - name: Setup MSBuild
      uses: microsoft/setup-msbuild@v2

    - name: Build usermode
      shell: cmd
      run: |
        call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
        cd hypervisor\usermode
        cl.exe /O2 /W4 /Fe:loader.exe main.c driver_interface.c payload_loader.c /link

    - name: Build hypervisor core  
      shell: cmd
      run: |
        call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
        cd hypervisor\hypervisor
        ml64.exe /c /Fo:asm\vmexit.obj asm\vmexit.asm
        ml64.exe /c /Fo:asm\intrinsics.obj asm\intrinsics.asm  
        ml64.exe /c /Fo:asm\segment.obj asm\segment.asm
        cl.exe /c /O2 /W4 /I..\shared entry.c vmx.c vmcs.c ept.c exit_dispatch.c timing.c
        cl.exe /c /O2 /W4 /I..\shared handlers\*.c
        lib.exe /OUT:hypervisor.lib *.obj asm\*.obj

    - name: Upload artifacts
      uses: actions/upload-artifact@v4
      with:
        name: ombra-${{ github.sha }}
        path: |
          hypervisor/usermode/*.exe
          hypervisor/hypervisor/*.lib
GHACTION

# =============================================================================
echo -e "${GREEN}[3/6]${NC} Creating shared headers..."
# =============================================================================

# types.h
cat > "$PROJECT_ROOT/hypervisor/shared/types.h" << 'EOF'
#ifndef OMBRA_TYPES_H
#define OMBRA_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t  U8;  typedef int8_t  I8;
typedef uint16_t U16; typedef int16_t I16;
typedef uint32_t U32; typedef int32_t I32;
typedef uint64_t U64; typedef int64_t I64;

typedef U64 GPA, GVA, HPA, HVA;

#define PAGE_SIZE   0x1000ULL
#define PAGE_SHIFT  12
#define BIT(n)      (1ULL << (n))

#define ALIGN_UP(x, a)   (((x) + ((a)-1)) & ~((a)-1))
#define ALIGN_DOWN(x, a) ((x) & ~((a)-1))

typedef enum {
    OMBRA_SUCCESS = 0,
    OMBRA_ERROR_NOT_SUPPORTED,
    OMBRA_ERROR_VMX_DISABLED,
    OMBRA_ERROR_ALREADY_RUNNING,
    OMBRA_ERROR_VMXON_FAILED,
    OMBRA_ERROR_VMCS_FAILED,
    OMBRA_ERROR_VMLAUNCH_FAILED,
} OMBRA_STATUS;

#define OMBRA_SUCCESS(s) ((s) == OMBRA_SUCCESS)
#define OMBRA_FAILED(s)  ((s) != OMBRA_SUCCESS)

#endif
EOF

# vmcs_fields.h
cat > "$PROJECT_ROOT/hypervisor/shared/vmcs_fields.h" << 'EOF'
#ifndef OMBRA_VMCS_FIELDS_H
#define OMBRA_VMCS_FIELDS_H

// 16-bit Guest
#define VMCS_GUEST_ES_SEL     0x0800
#define VMCS_GUEST_CS_SEL     0x0802
#define VMCS_GUEST_SS_SEL     0x0804
#define VMCS_GUEST_DS_SEL     0x0806
#define VMCS_GUEST_FS_SEL     0x0808
#define VMCS_GUEST_GS_SEL     0x080A
#define VMCS_GUEST_LDTR_SEL   0x080C
#define VMCS_GUEST_TR_SEL     0x080E

// 16-bit Host
#define VMCS_HOST_ES_SEL      0x0C00
#define VMCS_HOST_CS_SEL      0x0C02
#define VMCS_HOST_SS_SEL      0x0C04
#define VMCS_HOST_DS_SEL      0x0C06
#define VMCS_HOST_FS_SEL      0x0C08
#define VMCS_HOST_GS_SEL      0x0C0A
#define VMCS_HOST_TR_SEL      0x0C0C

// 64-bit Control
#define VMCS_CTRL_IO_BITMAP_A       0x2000
#define VMCS_CTRL_IO_BITMAP_B       0x2002
#define VMCS_CTRL_MSR_BITMAP        0x2004
#define VMCS_CTRL_TSC_OFFSET        0x2010
#define VMCS_CTRL_EPT_POINTER       0x201A

// 64-bit Guest
#define VMCS_GUEST_VMCS_LINK        0x2800
#define VMCS_GUEST_DEBUGCTL         0x2802
#define VMCS_GUEST_EFER             0x2806

// 32-bit Control
#define VMCS_CTRL_PIN_BASED         0x4000
#define VMCS_CTRL_PROC_BASED        0x4002
#define VMCS_CTRL_EXCEPTION_BITMAP  0x4004
#define VMCS_CTRL_VMEXIT            0x400C
#define VMCS_CTRL_VMENTRY           0x4012
#define VMCS_CTRL_PROC_BASED2       0x401E

// 32-bit Exit Info
#define VMCS_EXIT_REASON            0x4402
#define VMCS_EXIT_INT_INFO          0x4404
#define VMCS_EXIT_INSTR_LEN         0x440C
#define VMCS_EXIT_INSTR_INFO        0x440E

// 32-bit Guest
#define VMCS_GUEST_ES_LIMIT         0x4800
#define VMCS_GUEST_CS_LIMIT         0x4802
#define VMCS_GUEST_INTERRUPTIBILITY 0x4824
#define VMCS_GUEST_ACTIVITY         0x4826
#define VMCS_GUEST_SYSENTER_CS      0x482A

// Natural Control
#define VMCS_CTRL_CR0_MASK          0x6000
#define VMCS_CTRL_CR4_MASK          0x6002
#define VMCS_CTRL_CR0_SHADOW        0x6004
#define VMCS_CTRL_CR4_SHADOW        0x6006

// Natural Exit
#define VMCS_EXIT_QUALIFICATION     0x6400
#define VMCS_EXIT_GUEST_LINEAR      0x640A

// Natural Guest
#define VMCS_GUEST_CR0              0x6800
#define VMCS_GUEST_CR3              0x6802
#define VMCS_GUEST_CR4              0x6804
#define VMCS_GUEST_ES_BASE          0x6806
#define VMCS_GUEST_CS_BASE          0x6808
#define VMCS_GUEST_SS_BASE          0x680A
#define VMCS_GUEST_DS_BASE          0x680C
#define VMCS_GUEST_FS_BASE          0x680E
#define VMCS_GUEST_GS_BASE          0x6810
#define VMCS_GUEST_GDTR_BASE        0x6816
#define VMCS_GUEST_IDTR_BASE        0x6818
#define VMCS_GUEST_DR7              0x681A
#define VMCS_GUEST_RSP              0x681C
#define VMCS_GUEST_RIP              0x681E
#define VMCS_GUEST_RFLAGS           0x6820
#define VMCS_GUEST_SYSENTER_ESP     0x6824
#define VMCS_GUEST_SYSENTER_EIP     0x6826

// Natural Host
#define VMCS_HOST_CR0               0x6C00
#define VMCS_HOST_CR3               0x6C02
#define VMCS_HOST_CR4               0x6C04
#define VMCS_HOST_FS_BASE           0x6C06
#define VMCS_HOST_GS_BASE           0x6C08
#define VMCS_HOST_TR_BASE           0x6C0A
#define VMCS_HOST_GDTR_BASE         0x6C0C
#define VMCS_HOST_IDTR_BASE         0x6C0E
#define VMCS_HOST_RSP               0x6C14
#define VMCS_HOST_RIP               0x6C16

#endif
EOF

# msr_defs.h
cat > "$PROJECT_ROOT/hypervisor/shared/msr_defs.h" << 'EOF'
#ifndef OMBRA_MSR_DEFS_H
#define OMBRA_MSR_DEFS_H

#include "types.h"

#define MSR_IA32_FEATURE_CONTROL    0x3A
#define MSR_IA32_VMX_BASIC          0x480
#define MSR_IA32_VMX_PINBASED_CTLS  0x481
#define MSR_IA32_VMX_PROCBASED_CTLS 0x482
#define MSR_IA32_VMX_EXIT_CTLS      0x483
#define MSR_IA32_VMX_ENTRY_CTLS     0x484
#define MSR_IA32_VMX_CR0_FIXED0     0x486
#define MSR_IA32_VMX_CR0_FIXED1     0x487
#define MSR_IA32_VMX_CR4_FIXED0     0x488
#define MSR_IA32_VMX_CR4_FIXED1     0x489
#define MSR_IA32_VMX_PROCBASED_CTLS2 0x48B
#define MSR_IA32_VMX_EPT_VPID_CAP   0x48C

#define MSR_IA32_SYSENTER_CS        0x174
#define MSR_IA32_SYSENTER_ESP       0x175
#define MSR_IA32_SYSENTER_EIP       0x176
#define MSR_IA32_EFER               0xC0000080
#define MSR_IA32_FS_BASE            0xC0000100
#define MSR_IA32_GS_BASE            0xC0000101
#define MSR_IA32_KERNEL_GS_BASE     0xC0000102

#define FEATURE_CONTROL_LOCK        BIT(0)
#define FEATURE_CONTROL_VMX         BIT(2)

#define EFER_LME                    BIT(8)
#define EFER_LMA                    BIT(10)

#endif
EOF

# exit_reasons.h
cat > "$PROJECT_ROOT/hypervisor/shared/exit_reasons.h" << 'EOF'
#ifndef OMBRA_EXIT_REASONS_H
#define OMBRA_EXIT_REASONS_H

#define EXIT_REASON_EXCEPTION_NMI   0
#define EXIT_REASON_EXTERNAL_INT    1
#define EXIT_REASON_TRIPLE_FAULT    2
#define EXIT_REASON_CPUID           10
#define EXIT_REASON_HLT             12
#define EXIT_REASON_RDTSC           16
#define EXIT_REASON_VMCALL          18
#define EXIT_REASON_CR_ACCESS       28
#define EXIT_REASON_IO              30
#define EXIT_REASON_RDMSR           31
#define EXIT_REASON_WRMSR           32
#define EXIT_REASON_EPT_VIOLATION   48
#define EXIT_REASON_EPT_MISCONFIG   49
#define EXIT_REASON_RDTSCP          51
#define EXIT_REASON_XSETBV          55

#endif
EOF

# ept_defs.h
cat > "$PROJECT_ROOT/hypervisor/shared/ept_defs.h" << 'EOF'
#ifndef OMBRA_EPT_DEFS_H
#define OMBRA_EPT_DEFS_H

#include "types.h"

#define EPT_READ        BIT(0)
#define EPT_WRITE       BIT(1)
#define EPT_EXECUTE     BIT(2)
#define EPT_RWX         (EPT_READ | EPT_WRITE | EPT_EXECUTE)
#define EPT_LARGE_PAGE  BIT(7)

#define EPT_MEMORY_TYPE_WB  6

#define EPT_PML4_INDEX(gpa) (((gpa) >> 39) & 0x1FF)
#define EPT_PDPT_INDEX(gpa) (((gpa) >> 30) & 0x1FF)
#define EPT_PD_INDEX(gpa)   (((gpa) >> 21) & 0x1FF)
#define EPT_PT_INDEX(gpa)   (((gpa) >> 12) & 0x1FF)

typedef union {
    U64 Value;
    struct {
        U64 Read : 1;
        U64 Write : 1;
        U64 Execute : 1;
        U64 Reserved1 : 5;
        U64 Accessed : 1;
        U64 Ignored : 1;
        U64 ExecuteUser : 1;
        U64 Ignored2 : 1;
        U64 PhysAddr : 40;
        U64 Ignored3 : 12;
    };
} EPT_PML4E, EPT_PDPTE, EPT_PDE, EPT_PTE;

#endif
EOF

# cpu_defs.h
cat > "$PROJECT_ROOT/hypervisor/shared/cpu_defs.h" << 'EOF'
#ifndef OMBRA_CPU_DEFS_H
#define OMBRA_CPU_DEFS_H

#include "types.h"

#define CPUID_FEAT_ECX_VMX  BIT(5)
#define CPUID_FEAT_ECX_HV   BIT(31)

#define CR0_PE  BIT(0)
#define CR0_PG  BIT(31)
#define CR4_VME BIT(0)
#define CR4_PAE BIT(5)
#define CR4_VMXE BIT(13)

#define RFLAGS_IF BIT(9)

#endif
EOF
#!/bin/bash
# generate_ombra_project_part2.sh - Appended to main script

# =============================================================================
echo -e "${GREEN}[4/6]${NC} Creating hypervisor core..."
# =============================================================================

# vmx.h
cat > "$PROJECT_ROOT/hypervisor/hypervisor/vmx.h" << 'EOF'
#ifndef OMBRA_VMX_H
#define OMBRA_VMX_H

#include "../shared/types.h"

typedef struct {
    void* VmxonRegion;
    U64   VmxonPhysical;
    void* VmcsRegion;
    U64   VmcsPhysical;
    void* HostStack;
    void* MsrBitmap;
    U64   MsrBitmapPhysical;
    bool  VmxEnabled;
} VMX_CPU;

OMBRA_STATUS VmxCheckSupport(void);
OMBRA_STATUS VmxEnable(VMX_CPU* cpu);
OMBRA_STATUS VmxDisable(VMX_CPU* cpu);
U64 VmcsRead(U32 field);
void VmcsWrite(U32 field, U64 value);
U32 AdjustControls(U32 requested, U32 msr);

#endif
EOF

# vmx.c
cat > "$PROJECT_ROOT/hypervisor/hypervisor/vmx.c" << 'EOF'
#include "vmx.h"
#include "../shared/msr_defs.h"
#include "../shared/cpu_defs.h"
#include <intrin.h>

OMBRA_STATUS VmxCheckSupport(void) {
    int info[4];
    __cpuid(info, 1);
    if (!(info[2] & CPUID_FEAT_ECX_VMX)) return OMBRA_ERROR_NOT_SUPPORTED;
    if (info[2] & CPUID_FEAT_ECX_HV) return OMBRA_ERROR_ALREADY_RUNNING;
    
    U64 fc = __readmsr(MSR_IA32_FEATURE_CONTROL);
    if ((fc & FEATURE_CONTROL_LOCK) && !(fc & FEATURE_CONTROL_VMX))
        return OMBRA_ERROR_VMX_DISABLED;
    return OMBRA_SUCCESS;
}

U32 AdjustControls(U32 req, U32 msr) {
    U64 cap = __readmsr(msr);
    req |= (U32)cap;
    req &= (U32)(cap >> 32);
    return req;
}

OMBRA_STATUS VmxEnable(VMX_CPU* cpu) {
    __writecr4(__readcr4() | CR4_VMXE);
    U64 cr0 = __readcr0();
    cr0 |= __readmsr(MSR_IA32_VMX_CR0_FIXED0);
    cr0 &= __readmsr(MSR_IA32_VMX_CR0_FIXED1);
    __writecr0(cr0);
    
    *(U32*)cpu->VmxonRegion = (U32)__readmsr(MSR_IA32_VMX_BASIC);
    if (__vmx_on(&cpu->VmxonPhysical)) return OMBRA_ERROR_VMXON_FAILED;
    cpu->VmxEnabled = true;
    return OMBRA_SUCCESS;
}

OMBRA_STATUS VmxDisable(VMX_CPU* cpu) {
    if (!cpu->VmxEnabled) return OMBRA_SUCCESS;
    __vmx_off();
    __writecr4(__readcr4() & ~CR4_VMXE);
    cpu->VmxEnabled = false;
    return OMBRA_SUCCESS;
}
EOF

# vmcs.h
cat > "$PROJECT_ROOT/hypervisor/hypervisor/vmcs.h" << 'EOF'
#ifndef OMBRA_VMCS_H
#define OMBRA_VMCS_H
#include "vmx.h"
OMBRA_STATUS VmcsInitialize(VMX_CPU* cpu);
void VmcsSetupControls(VMX_CPU* cpu);
void VmcsSetupGuestState(void);
void VmcsSetupHostState(VMX_CPU* cpu);
#endif
EOF

# vmcs.c
cat > "$PROJECT_ROOT/hypervisor/hypervisor/vmcs.c" << 'EOF'
#include "vmcs.h"
#include "../shared/vmcs_fields.h"
#include "../shared/msr_defs.h"
#include <intrin.h>

U64 VmcsRead(U32 field) { U64 v; __vmx_vmread(field, &v); return v; }
void VmcsWrite(U32 field, U64 val) { __vmx_vmwrite(field, val); }

OMBRA_STATUS VmcsInitialize(VMX_CPU* cpu) {
    *(U32*)cpu->VmcsRegion = (U32)__readmsr(MSR_IA32_VMX_BASIC);
    if (__vmx_vmclear(&cpu->VmcsPhysical)) return OMBRA_ERROR_VMCS_FAILED;
    if (__vmx_vmptrld(&cpu->VmcsPhysical)) return OMBRA_ERROR_VMCS_FAILED;
    VmcsSetupControls(cpu);
    VmcsSetupGuestState();
    VmcsSetupHostState(cpu);
    return OMBRA_SUCCESS;
}

void VmcsSetupControls(VMX_CPU* cpu) {
    // TODO: Setup pin/proc/exit/entry controls
    (void)cpu;
}

void VmcsSetupGuestState(void) {
    // TODO: Copy current state to guest area
}

void VmcsSetupHostState(VMX_CPU* cpu) {
    // TODO: Setup host state for VM-exit
    (void)cpu;
}
EOF

# entry.c
cat > "$PROJECT_ROOT/hypervisor/hypervisor/entry.c" << 'EOF'
#include "../shared/types.h"
#include "vmx.h"

OMBRA_STATUS OmbraInitialize(void) {
    OMBRA_STATUS s = VmxCheckSupport();
    if (OMBRA_FAILED(s)) return s;
    // TODO: Per-CPU init, EPT setup, launch
    return OMBRA_SUCCESS;
}

void OmbraShutdown(void) {
    // TODO: VMXOFF on all CPUs
}
EOF

# exit_dispatch.h
cat > "$PROJECT_ROOT/hypervisor/hypervisor/exit_dispatch.h" << 'EOF'
#ifndef OMBRA_EXIT_DISPATCH_H
#define OMBRA_EXIT_DISPATCH_H
#include "../shared/types.h"

typedef struct {
    U64 Rax, Rcx, Rdx, Rbx, Rsp, Rbp, Rsi, Rdi;
    U64 R8, R9, R10, R11, R12, R13, R14, R15;
} GUEST_REGS;

typedef enum { VMEXIT_CONTINUE, VMEXIT_ADVANCE_RIP, VMEXIT_SHUTDOWN } VMEXIT_ACTION;

VMEXIT_ACTION VmexitDispatch(GUEST_REGS* regs);
#endif
EOF

# exit_dispatch.c
cat > "$PROJECT_ROOT/hypervisor/hypervisor/exit_dispatch.c" << 'EOF'
#include "exit_dispatch.h"
#include "handlers/handlers.h"
#include "../shared/vmcs_fields.h"
#include "../shared/exit_reasons.h"

U64 VmcsRead(U32 field);

VMEXIT_ACTION VmexitDispatch(GUEST_REGS* regs) {
    U32 reason = (U32)VmcsRead(VMCS_EXIT_REASON) & 0xFFFF;
    U64 qual = VmcsRead(VMCS_EXIT_QUALIFICATION);
    
    switch (reason) {
    case EXIT_REASON_CPUID:        return HandleCpuid(regs);
    case EXIT_REASON_RDTSC:        return HandleRdtsc(regs);
    case EXIT_REASON_RDTSCP:       return HandleRdtscp(regs);
    case EXIT_REASON_RDMSR:        return HandleRdmsr(regs);
    case EXIT_REASON_WRMSR:        return HandleWrmsr(regs);
    case EXIT_REASON_CR_ACCESS:    return HandleCrAccess(regs, qual);
    case EXIT_REASON_EPT_VIOLATION:return HandleEptViolation(regs, qual);
    case EXIT_REASON_VMCALL:       return HandleVmcall(regs);
    case EXIT_REASON_TRIPLE_FAULT: return VMEXIT_SHUTDOWN;
    default: return VMEXIT_ADVANCE_RIP;
    }
}
EOF

# ept.h / ept.c
cat > "$PROJECT_ROOT/hypervisor/hypervisor/ept.h" << 'EOF'
#ifndef OMBRA_EPT_H
#define OMBRA_EPT_H
#include "../shared/types.h"
#include "../shared/ept_defs.h"
typedef struct { EPT_PML4E* Pml4; U64 Pml4Phys; U64 Eptp; } EPT_STATE;
OMBRA_STATUS EptInitialize(EPT_STATE* ept);
void EptDestroy(EPT_STATE* ept);
#endif
EOF

cat > "$PROJECT_ROOT/hypervisor/hypervisor/ept.c" << 'EOF'
#include "ept.h"
OMBRA_STATUS EptInitialize(EPT_STATE* e) { (void)e; return OMBRA_SUCCESS; }
void EptDestroy(EPT_STATE* e) { (void)e; }
EOF

# timing.h / timing.c
cat > "$PROJECT_ROOT/hypervisor/hypervisor/timing.h" << 'EOF'
#ifndef OMBRA_TIMING_H
#define OMBRA_TIMING_H
#include "../shared/types.h"
void TimingCalibrate(void);
U64 TimingCompensate(U64 tsc);
#endif
EOF

cat > "$PROJECT_ROOT/hypervisor/hypervisor/timing.c" << 'EOF'
#include "timing.h"
static U64 g_Overhead = 2000;
void TimingCalibrate(void) { /* TODO */ }
U64 TimingCompensate(U64 tsc) { return tsc > g_Overhead ? tsc - g_Overhead : tsc; }
EOF

# =============================================================================
echo -e "${GREEN}[5/6]${NC} Creating handlers..."
# =============================================================================

# handlers.h
cat > "$PROJECT_ROOT/hypervisor/hypervisor/handlers/handlers.h" << 'EOF'
#ifndef OMBRA_HANDLERS_H
#define OMBRA_HANDLERS_H
#include "../exit_dispatch.h"
VMEXIT_ACTION HandleCpuid(GUEST_REGS* r);
VMEXIT_ACTION HandleRdtsc(GUEST_REGS* r);
VMEXIT_ACTION HandleRdtscp(GUEST_REGS* r);
VMEXIT_ACTION HandleRdmsr(GUEST_REGS* r);
VMEXIT_ACTION HandleWrmsr(GUEST_REGS* r);
VMEXIT_ACTION HandleCrAccess(GUEST_REGS* r, U64 q);
VMEXIT_ACTION HandleEptViolation(GUEST_REGS* r, U64 q);
VMEXIT_ACTION HandleVmcall(GUEST_REGS* r);
VMEXIT_ACTION HandleException(GUEST_REGS* r);
VMEXIT_ACTION HandleIo(GUEST_REGS* r, U64 q);
#endif
EOF

# cpuid.c - CRITICAL
cat > "$PROJECT_ROOT/hypervisor/hypervisor/handlers/cpuid.c" << 'EOF'
#include "handlers.h"
#include "../../shared/cpu_defs.h"
#include <intrin.h>

VMEXIT_ACTION HandleCpuid(GUEST_REGS* r) {
    int info[4];
    __cpuidex(info, (int)r->Rax, (int)r->Rcx);
    
    // STEALTH: Hide hypervisor presence
    if (r->Rax == 1) info[2] &= ~CPUID_FEAT_ECX_HV;
    if (r->Rax >= 0x40000000 && r->Rax <= 0x400000FF) {
        info[0] = info[1] = info[2] = info[3] = 0;
    }
    
    r->Rax = info[0]; r->Rbx = info[1];
    r->Rcx = info[2]; r->Rdx = info[3];
    return VMEXIT_ADVANCE_RIP;
}
EOF

# rdtsc.c - CRITICAL
cat > "$PROJECT_ROOT/hypervisor/hypervisor/handlers/rdtsc.c" << 'EOF'
#include "handlers.h"
#include <intrin.h>

#define OVERHEAD 2000
static U64 g_LastTsc = 0;

VMEXIT_ACTION HandleRdtsc(GUEST_REGS* r) {
    U64 tsc = __rdtsc();
    if (tsc > OVERHEAD) tsc -= OVERHEAD;
    if (tsc <= g_LastTsc) tsc = g_LastTsc + 1;
    g_LastTsc = tsc;
    r->Rax = (U32)tsc;
    r->Rdx = (U32)(tsc >> 32);
    return VMEXIT_ADVANCE_RIP;
}

VMEXIT_ACTION HandleRdtscp(GUEST_REGS* r) {
    U32 aux;
    U64 tsc = __rdtscp(&aux);
    if (tsc > OVERHEAD) tsc -= OVERHEAD;
    if (tsc <= g_LastTsc) tsc = g_LastTsc + 1;
    g_LastTsc = tsc;
    r->Rax = (U32)tsc;
    r->Rdx = (U32)(tsc >> 32);
    r->Rcx = aux;
    return VMEXIT_ADVANCE_RIP;
}
EOF

# msr.c
cat > "$PROJECT_ROOT/hypervisor/hypervisor/handlers/msr.c" << 'EOF'
#include "handlers.h"
#include <intrin.h>

VMEXIT_ACTION HandleRdmsr(GUEST_REGS* r) {
    U64 v = __readmsr((U32)r->Rcx);
    r->Rax = (U32)v;
    r->Rdx = (U32)(v >> 32);
    return VMEXIT_ADVANCE_RIP;
}

VMEXIT_ACTION HandleWrmsr(GUEST_REGS* r) {
    __writemsr((U32)r->Rcx, (r->Rdx << 32) | (r->Rax & 0xFFFFFFFF));
    return VMEXIT_ADVANCE_RIP;
}
EOF

# Other handlers (stubs)
cat > "$PROJECT_ROOT/hypervisor/hypervisor/handlers/cr_access.c" << 'EOF'
#include "handlers.h"
VMEXIT_ACTION HandleCrAccess(GUEST_REGS* r, U64 q) {
    (void)r; (void)q;
    // TODO: Handle CR0/CR3/CR4 access
    return VMEXIT_ADVANCE_RIP;
}
EOF

cat > "$PROJECT_ROOT/hypervisor/hypervisor/handlers/ept_violation.c" << 'EOF'
#include "handlers.h"
VMEXIT_ACTION HandleEptViolation(GUEST_REGS* r, U64 q) {
    (void)r; (void)q;
    // TODO: Handle EPT violation
    return VMEXIT_ADVANCE_RIP;
}
EOF

cat > "$PROJECT_ROOT/hypervisor/hypervisor/handlers/vmcall.c" << 'EOF'
#include "handlers.h"
#define VMCALL_UNLOAD 0xDEAD
VMEXIT_ACTION HandleVmcall(GUEST_REGS* r) {
    if (r->Rcx == VMCALL_UNLOAD) return VMEXIT_SHUTDOWN;
    r->Rax = 0x4F4D4252; // "OMBR"
    return VMEXIT_ADVANCE_RIP;
}
EOF

cat > "$PROJECT_ROOT/hypervisor/hypervisor/handlers/exception.c" << 'EOF'
#include "handlers.h"
VMEXIT_ACTION HandleException(GUEST_REGS* r) {
    (void)r;
    return VMEXIT_CONTINUE;
}
EOF

cat > "$PROJECT_ROOT/hypervisor/hypervisor/handlers/io.c" << 'EOF'
#include "handlers.h"
VMEXIT_ACTION HandleIo(GUEST_REGS* r, U64 q) {
    (void)r; (void)q;
    return VMEXIT_ADVANCE_RIP;
}
EOF
#!/bin/bash
# generate_ombra_project_part3.sh - ASM, usermode, docs

# =============================================================================
echo -e "${GREEN}[6/6]${NC} Creating ASM, usermode, docs..."
# =============================================================================

# ASM files
cat > "$PROJECT_ROOT/hypervisor/hypervisor/asm/vmexit.asm" << 'EOF'
.code
EXTERN VmexitDispatch:PROC

VmexitHandler PROC
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
    push rbp
    push rbx
    push rdx
    push rcx
    push rax
    
    mov rcx, rsp
    sub rsp, 28h
    call VmexitDispatch
    add rsp, 28h
    
    cmp eax, 2
    je Shutdown
    
    pop rax
    pop rcx
    pop rdx
    pop rbx
    pop rbp
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
    
    vmresume
    jmp $

Shutdown:
    cli
    hlt
    jmp Shutdown
VmexitHandler ENDP

AsmVmxLaunch PROC
    vmlaunch
    mov rax, 1
    ret
AsmVmxLaunch ENDP

END
EOF

cat > "$PROJECT_ROOT/hypervisor/hypervisor/asm/intrinsics.asm" << 'EOF'
.code

AsmReadMsr PROC
    rdmsr
    shl rdx, 32
    or rax, rdx
    ret
AsmReadMsr ENDP

AsmWriteMsr PROC
    mov eax, edx
    shr rdx, 32
    wrmsr
    ret
AsmWriteMsr ENDP

AsmReadCr0 PROC
    mov rax, cr0
    ret
AsmReadCr0 ENDP

AsmWriteCr0 PROC
    mov cr0, rcx
    ret
AsmWriteCr0 ENDP

AsmReadCr4 PROC
    mov rax, cr4
    ret
AsmReadCr4 ENDP

AsmWriteCr4 PROC
    mov cr4, rcx
    ret
AsmWriteCr4 ENDP

AsmInvept PROC
    invept rcx, OWORD PTR [rdx]
    ret
AsmInvept ENDP

END
EOF

cat > "$PROJECT_ROOT/hypervisor/hypervisor/asm/segment.asm" << 'EOF'
.code

AsmReadCs PROC
    mov ax, cs
    ret
AsmReadCs ENDP

AsmReadSs PROC
    mov ax, ss
    ret
AsmReadSs ENDP

AsmReadDs PROC
    mov ax, ds
    ret
AsmReadDs ENDP

AsmReadEs PROC
    mov ax, es
    ret
AsmReadEs ENDP

AsmReadFs PROC
    mov ax, fs
    ret
AsmReadFs ENDP

AsmReadGs PROC
    mov ax, gs
    ret
AsmReadGs ENDP

AsmReadTr PROC
    str ax
    ret
AsmReadTr ENDP

AsmReadLdtr PROC
    sldt ax
    ret
AsmReadLdtr ENDP

AsmReadGdtr PROC
    sgdt [rcx]
    ret
AsmReadGdtr ENDP

AsmReadIdtr PROC
    sidt [rcx]
    ret
AsmReadIdtr ENDP

END
EOF

# Usermode files
cat > "$PROJECT_ROOT/hypervisor/usermode/main.c" << 'EOF'
#include <stdio.h>
#include <Windows.h>
#include "driver_interface.h"

int main(int argc, char* argv[]) {
    printf("[*] OmbraHypervisor Loader\n");
    printf("[!] TODO: Load vulnerable driver, map hypervisor\n");
    (void)argc; (void)argv;
    return 1;
}
EOF

cat > "$PROJECT_ROOT/hypervisor/usermode/driver_interface.h" << 'EOF'
#ifndef DRIVER_INTERFACE_H
#define DRIVER_INTERFACE_H
#include <Windows.h>
BOOL DriverLoad(LPCWSTR path);
BOOL DriverUnload(void);
BOOL DriverMapMemory(PVOID src, PVOID dst, SIZE_T size);
BOOL DriverExecuteRing0(PVOID func);
#endif
EOF

cat > "$PROJECT_ROOT/hypervisor/usermode/driver_interface.c" << 'EOF'
#include "driver_interface.h"
// TODO: BYOVD implementation
BOOL DriverLoad(LPCWSTR p) { (void)p; return FALSE; }
BOOL DriverUnload(void) { return FALSE; }
BOOL DriverMapMemory(PVOID s, PVOID d, SIZE_T z) { (void)s;(void)d;(void)z; return FALSE; }
BOOL DriverExecuteRing0(PVOID f) { (void)f; return FALSE; }
EOF

cat > "$PROJECT_ROOT/hypervisor/usermode/payload_loader.c" << 'EOF'
#include <Windows.h>
// TODO: Map hypervisor payload into kernel
EOF

# Build files
cat > "$PROJECT_ROOT/hypervisor/build/build.bat" << 'EOF'
@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

echo Building usermode...
cd ..\usermode
cl.exe /O2 /W4 /Fe:loader.exe main.c driver_interface.c payload_loader.c /link

echo Building hypervisor...
cd ..\hypervisor
ml64.exe /c /Fo:asm\vmexit.obj asm\vmexit.asm
ml64.exe /c /Fo:asm\intrinsics.obj asm\intrinsics.asm
ml64.exe /c /Fo:asm\segment.obj asm\segment.asm
cl.exe /c /O2 /W4 /I..\shared *.c handlers\*.c
lib.exe /OUT:hypervisor.lib *.obj asm\*.obj

echo Done!
cd ..\build
EOF

cat > "$PROJECT_ROOT/hypervisor/build/Makefile" << 'EOF'
# Use build.bat on Windows or GitHub Actions
all:
	@echo "Run build.bat on Windows"
EOF

# Docs
cat > "$PROJECT_ROOT/hypervisor/docs/vmcs_reference.md" << 'EOF'
# VMCS Reference
Use MCP `vmcs_field_lookup` for encodings.
Key fields: GUEST_RIP (0x681E), GUEST_RSP (0x681C), EXIT_REASON (0x4402)
EOF

cat > "$PROJECT_ROOT/hypervisor/docs/exit_handling.md" << 'EOF'
# Exit Handling
Critical for stealth:
- CPUID: Hide HV bit, zero 0x40000000+
- RDTSC: Subtract ~2000 cycles overhead
- MSR: Virtualize sensitive MSRs
EOF

cat > "$PROJECT_ROOT/hypervisor/docs/detection_vectors.md" << 'EOF'
# Detection Vectors
- Timing: RDTSC around CPUID
- CPUID: Bit 31 ECX, HV leaves
- MSR: VMX capability MSRs
- Memory: Signature scanning
EOF

# VS Code
cat > "$PROJECT_ROOT/hypervisor/.vscode/settings.json" << 'EOF'
{
    "files.associations": {"*.h": "c", "*.c": "c", "*.asm": "masm"},
    "C_Cpp.default.includePath": ["${workspaceFolder}/shared"],
    "editor.tabSize": 4
}
EOF

# .gitignore
cat > "$PROJECT_ROOT/hypervisor/.gitignore" << 'EOF'
*.obj
*.lib
*.exe
*.pdb
.vs/
*.sys
.DS_Store
EOF

# README
cat > "$PROJECT_ROOT/hypervisor/README.md" << 'EOF'
# OmbraHypervisor

Hyper-V hijacking framework. Build via GitHub Actions or Windows.

## Structure
- `usermode/` - Ring-3 loader
- `hypervisor/` - Ring -1 core
- `shared/` - Common headers

## Build
Push to main → GitHub Actions builds on Windows.
Or run `build\build.bat` on Windows with VS2022.
EOF

# =============================================================================
# Done!
# =============================================================================

FILE_COUNT=$(find "$PROJECT_ROOT/hypervisor" -type f | wc -l | tr -d ' ')

echo ""
echo -e "${CYAN}════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}  Project generated!${NC}"
echo -e "${CYAN}════════════════════════════════════════════════════════════${NC}"
echo ""
echo -e "  📁 ${YELLOW}$PROJECT_ROOT/hypervisor${NC}"
echo -e "  📄 ${YELLOW}$FILE_COUNT files${NC}"
echo ""
echo -e "  ${GREEN}Next:${NC}"
echo "  1. Add MCP server alongside hypervisor/"
echo "  2. git init && git push to trigger CI build"
echo "  3. Or build locally on Windows with VS2022"
echo ""