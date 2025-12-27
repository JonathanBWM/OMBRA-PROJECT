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
// Forward declaration for nested state
// =============================================================================
struct _NESTED_STATE;

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

    // Nested virtualization state (shared, pointer to global)
    struct _NESTED_STATE* Nested;

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
    U64     TscResetCounter;    // VM-exits since last TSC reset
    U64     TscCumulativeOffset;// Total offset before last reset (for continuity)

    // APERF/MPERF compensation (CRITICAL: ESEA monitors ratio)
    // MCP verified: IA32_APERF=0xE8, IA32_MPERF=0xE7
    U64     AperfOffset;        // Accumulated APERF offset for anti-detection
    U64     MperfOffset;        // Accumulated MPERF offset for anti-detection

    // Calibrated timing values
    bool    TimingCalibrated;
    U64     CalibratedCpuidOverhead;
    U64     CalibratedRdtscBaseline;
    U64     CalibratedMsrOverhead;

    // MSR virtualization
    U32     GuestTscAux;        // Virtualized IA32_TSC_AUX (processor ID)

    // CR8 (TPR) virtualization
    U64     GuestCr8;           // Shadow CR8 value (Task Priority Register)

    // NMI blocking for EAC evasion
    // EAC sends NMIs to detect hypervisors - we queue them for specific CR3s
    #define MAX_NMI_BLOCKED_CR3 16
    U64     NmiBlockedCr3[MAX_NMI_BLOCKED_CR3];  // CR3s that should have NMIs blocked
    U32     NmiBlockedCount;                     // Active blocked CR3 entries
    U32     NmiQueuedCount;                      // NMIs waiting to be delivered
    bool    NmiBlockingEnabled;                  // Master switch for NMI blocking

    // Shutdown coordination
    bool    ShutdownPending;    // VMXOFF requested for this CPU

    // Self-reference for assembly access
    struct _VMX_CPU* Self;
} VMX_CPU;

// =============================================================================
// Per-CPU Initialization Data (used by VmxInitializeCpu)
// Note: This extends the base HV_INIT_PARAMS from types.h for per-CPU init
// =============================================================================

#ifndef HV_PER_CPU_PARAMS_DEFINED
#define HV_PER_CPU_PARAMS_DEFINED
typedef struct _HV_PER_CPU_PARAMS {
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

    // Shadow VMCS resources (shared across CPUs, for nested virtualization)
    U64     VmreadBitmapPhysical;   // 4KB, controls VMREAD shadowing
    U64     VmwriteBitmapPhysical;  // 4KB, controls VMWRITE shadowing
    U64     ShadowVmcsPhysical;     // 4KB, shadow VMCS for L2 guests
    void*   VmreadBitmapVirtual;
    void*   VmwriteBitmapVirtual;
    void*   ShadowVmcsVirtual;

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
} HV_PER_CPU_PARAMS;
#endif // HV_PER_CPU_PARAMS_DEFINED

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

    // Driver mapping state (Phase 2)
    U64         DriverCr3;      // CR3 of the driver mapping process
    bool        DriverLoaded;   // Whether a mapped driver is active
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
OMBRA_STATUS VmcsInitialize(VMX_CPU* cpu, HV_PER_CPU_PARAMS* params);

// Control Adjustment
U32 AdjustControls(U32 requested, U32 msr);
U64 AdjustControls64(U64 requested, U32 msr);

// Segment Helpers
void GetSegmentState(U16 selector, SEGMENT_STATE* state);
U32 GetSegmentAccessRights(U16 selector);
U64 GetSegmentBase(U16 selector);
U32 GetSegmentLimit(U16 selector);

// Per-CPU Initialization
OMBRA_STATUS VmxInitializeCpu(HV_PER_CPU_PARAMS* params);
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
