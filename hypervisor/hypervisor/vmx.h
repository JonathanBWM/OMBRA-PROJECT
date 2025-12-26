// vmx.h — VMX Operations and Per-CPU Structures
// OmbraHypervisor

#ifndef OMBRA_VMX_H
#define OMBRA_VMX_H

#include "../shared/types.h"

// =============================================================================
// Forward Declarations
// =============================================================================

struct _EPT_STATE;

// =============================================================================
// Descriptor Table Registers
// =============================================================================

#pragma pack(push, 1)
typedef struct _DESCRIPTOR_TABLE_REG {
    U16 Limit;
    U64 Base;
} DESCRIPTOR_TABLE_REG;
#pragma pack(pop)

// =============================================================================
// Segment Descriptor (from GDT)
// =============================================================================

#pragma pack(push, 1)
typedef struct _SEGMENT_DESCRIPTOR {
    U16 LimitLow;
    U16 BaseLow;
    U8  BaseMiddle;
    U8  Access;
    U8  LimitHighAndFlags;
    U8  BaseHigh;
} SEGMENT_DESCRIPTOR;

typedef struct _SEGMENT_DESCRIPTOR_64 {
    SEGMENT_DESCRIPTOR Desc;
    U32 BaseUpper;
    U32 Reserved;
} SEGMENT_DESCRIPTOR_64;
#pragma pack(pop)

// =============================================================================
// Segment State (for VMCS)
// =============================================================================

typedef struct _SEGMENT_STATE {
    U16 Selector;
    U64 Base;
    U32 Limit;
    U32 AccessRights;
} SEGMENT_STATE;

// =============================================================================
// Per-CPU VMX State
// =============================================================================

typedef struct _VMX_CPU {
    // CPU identification
    U32 CpuId;
    bool Virtualized;

    // VMXON Region (4KB, physically contiguous, 4KB aligned)
    void*   VmxonRegion;        // Virtual address
    U64     VmxonPhysical;      // Physical address

    // VMCS Region (4KB, physically contiguous, 4KB aligned)
    void*   VmcsRegion;         // Virtual address
    U64     VmcsPhysical;       // Physical address

    // Host Stack (grows down, need top address)
    void*   HostStackBase;      // Bottom of stack (allocation address)
    void*   HostStackTop;       // Top of stack (RSP value for VM-exit)
    U64     HostStackSize;      // Size in bytes

    // MSR Bitmap (4KB, 4KB aligned)
    void*   MsrBitmap;          // Virtual address
    U64     MsrBitmapPhysical;  // Physical address

    // EPT State (shared, pointer to global)
    struct _EPT_STATE* Ept;

    // VMX state
    bool    VmxEnabled;         // VMXON executed
    bool    VmcsLoaded;         // VMPTRLD executed
    bool    VmxLaunched;        // VMLAUNCH executed successfully

    // Cached guest state (for quick access during VM-exit)
    U64     GuestRip;
    U64     GuestRsp;
    U64     GuestRflags;

    // Statistics
    U64     VmexitCount;
    U64     LastExitReason;

    // Timing compensation
    U64     TscOffset;          // Accumulated TSC offset for stealth
    U64     LastTsc;            // Last TSC value returned to guest

    // Self-reference for assembly access
    struct _VMX_CPU* Self;
} VMX_CPU;

// =============================================================================
// Initialization Data (passed from usermode loader)
// =============================================================================

typedef struct _HV_INIT_PARAMS {
    U32     CpuId;
    U32     TotalCpus;

    // Per-CPU physical addresses (from usermode allocations)
    U64     VmxonPhysical;
    U64     VmcsPhysical;
    U64     HostStackTop;       // Virtual address of stack top
    U64     MsrBitmapPhysical;

    // Virtual addresses for R0 access
    void*   VmxonVirtual;
    void*   VmcsVirtual;
    void*   MsrBitmapVirtual;

    // Shared EPT
    U64     EptPml4Physical;
    void*   EptPml4Virtual;

    // VMX MSRs (pre-fetched by usermode)
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
    U64     VmexitHandler;

    // Debug infrastructure
    U64     DebugBufferPhysical;
    void*   DebugBufferVirtual;
    U64     DebugBufferSize;
} HV_INIT_PARAMS;

// =============================================================================
// Global State
// =============================================================================

#define MAX_CPUS 256

typedef struct _OMBRA_STATE {
    VMX_CPU*    Cpus[MAX_CPUS];
    U32         NumCpus;
    bool        Initialized;

    // Shared EPT
    struct _EPT_STATE* Ept;
} OMBRA_STATE;

extern OMBRA_STATE g_Ombra;

// =============================================================================
// Function Declarations
// =============================================================================

// VMX Support
OMBRA_STATUS VmxCheckSupport(void);
bool VmxIsSupported(void);

// VMX Enable/Disable
OMBRA_STATUS VmxEnable(VMX_CPU* cpu);
OMBRA_STATUS VmxDisable(VMX_CPU* cpu);

// VMCS Operations
U64  VmcsRead(U32 field);
void VmcsWrite(U32 field, U64 value);
OMBRA_STATUS VmcsInitialize(VMX_CPU* cpu, HV_INIT_PARAMS* params);

// Control Adjustment
U32 AdjustControls(U32 requested, U32 msr);
U64 AdjustControls64(U64 requested, U32 msr);

// Segment Helpers
void GetSegmentState(U16 selector, SEGMENT_STATE* state);
U32 GetSegmentAccessRights(U16 selector);
U64 GetSegmentBase(U16 selector);
U32 GetSegmentLimit(U16 selector);

// Per-CPU Initialization
OMBRA_STATUS VmxInitializeCpu(HV_INIT_PARAMS* params);
OMBRA_STATUS VmxLaunchCpu(VMX_CPU* cpu);

// Utility
VMX_CPU* VmxGetCurrentCpu(void);

// =============================================================================
// Assembly Functions (defined in asm/*.asm)
// =============================================================================

// From vmexit.asm
extern void VmexitHandler(void);
extern U64 AsmVmxLaunch(void);
extern U64 AsmVmxResume(void);

// From intrinsics.asm
extern U64 AsmReadMsr(U32 msr);
extern void AsmWriteMsr(U32 msr, U64 value);
extern U64 AsmReadCr0(void);
extern void AsmWriteCr0(U64 value);
extern U64 AsmReadCr3(void);
extern void AsmWriteCr3(U64 value);
extern U64 AsmReadCr4(void);
extern void AsmWriteCr4(U64 value);
extern void AsmInvept(U64 type, void* descriptor);

// From segment.asm
extern U16 AsmReadCs(void);
extern U16 AsmReadSs(void);
extern U16 AsmReadDs(void);
extern U16 AsmReadEs(void);
extern U16 AsmReadFs(void);
extern U16 AsmReadGs(void);
extern U16 AsmReadTr(void);
extern U16 AsmReadLdtr(void);
extern void AsmReadGdtr(DESCRIPTOR_TABLE_REG* gdtr);
extern void AsmReadIdtr(DESCRIPTOR_TABLE_REG* idtr);
extern U64 AsmReadRflags(void);
extern U64 AsmReadDr7(void);

#endif // OMBRA_VMX_H
