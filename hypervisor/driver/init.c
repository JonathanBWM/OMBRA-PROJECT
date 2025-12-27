#include "context.h"
#include "command_ring.h"
#include "vmcall.h"

OMBRA_DRIVER_CTX g_DriverCtx = {0};

// Forward declaration of worker thread
void OmbraWorkerThread(void* ctx);

// Memory copy without CRT dependency
static void ombra_memcpy(void* dst, const void* src, U64 size) {
    volatile U8* d = (volatile U8*)dst;
    const volatile U8* s = (const volatile U8*)src;
    for (U64 i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

// Memory zero without CRT dependency
static void ombra_memzero(void* dst, U64 size) {
    volatile U8* d = (volatile U8*)dst;
    for (U64 i = 0; i < size; i++) {
        d[i] = 0;
    }
}

// DriverEntry - called by hypervisor after manual mapping
// initData: Pointer to OMBRA_DRIVER_INIT filled by loader
// Returns: 0 on success, negative on error
long OmbraDriverEntry(void* initData, void* reserved) {
    (void)reserved;

    if (!initData) return -1;

    POMBRA_DRIVER_INIT init = (POMBRA_DRIVER_INIT)initData;

    // Validate magic
    if (init->Magic != OMBRA_DRIVER_INIT_MAGIC) return -2;

    // Zero out context first
    ombra_memzero(&g_DriverCtx, sizeof(g_DriverCtx));

    // Store VMCALL credentials
    g_DriverCtx.VmcallMagic = init->VmcallMagic;
    g_DriverCtx.VmcallKey = init->VmcallKey;
    VmCallInit(init->VmcallMagic, init->VmcallKey);

    // Store command ring pointer
    g_DriverCtx.Ring = (POMBRA_COMMAND_RING)init->CommandRing;

    // Calculate scratch buffer location
    // Layout: [header][commands][responses][scratch]
    U64 dataSize = OMBRA_RING_HEADER_SIZE +
                   (OMBRA_RING_SIZE * OMBRA_COMMAND_SIZE) +
                   (OMBRA_RING_SIZE * OMBRA_RESPONSE_SIZE);
    g_DriverCtx.ScratchBuffer = (U8*)init->CommandRing + dataSize;
    g_DriverCtx.ScratchSize = init->ScratchSize;

    // Copy offsets and imports
    ombra_memcpy(&g_DriverCtx.Offsets, &init->Offsets, sizeof(EPROCESS_OFFSETS_EX));
    ombra_memcpy(&g_DriverCtx.Imports, &init->Imports, sizeof(OMBRA_KERNEL_IMPORTS));

    // Get owner CR3 (loader's process)
    // Use intrinsics for MSVC, inline asm for GCC/Clang
    U64 ownerCr3;
#ifdef _MSC_VER
    ownerCr3 = __readcr3();
#else
    __asm__ __volatile__("mov %%cr3, %0" : "=r"(ownerCr3));
#endif
    g_DriverCtx.OwnerCr3 = ownerCr3;

    // Record start time via TSC
    U64 tsc_low, tsc_high;
#ifdef _MSC_VER
    __asm {
        rdtsc
        mov tsc_low, eax
        mov tsc_high, edx
    }
    g_DriverCtx.StartTime = ((U64)tsc_high << 32) | tsc_low;
#else
    __asm__ __volatile__("rdtsc" : "=a"(tsc_low), "=d"(tsc_high));
    g_DriverCtx.StartTime = ((U64)tsc_high << 32) | tsc_low;
#endif

    // Initialize counters
    g_DriverCtx.SubscriptionCount = 0;
    g_DriverCtx.CommandsProcessed = 0;
    g_DriverCtx.VmexitCount = 0;

    // Initialize subscription array
    for (U32 i = 0; i < OMBRA_MAX_SUBSCRIPTIONS; i++) {
        g_DriverCtx.Subscriptions[i] = 0;
    }

    // Notify hypervisor driver is ready
    I64 status = VmDriverReady(g_DriverCtx.OwnerCr3);
    if (status < 0) return -3;

    // Create worker thread via kernel import
    // PsCreateSystemThread(ThreadHandle, DesiredAccess, ObjectAttribs, ProcessHandle, ClientId, StartRoutine, StartContext)
    typedef long (*PsCreateSystemThreadFn)(void**, U32, void*, void*, void*, void*, void*);
    PsCreateSystemThreadFn createThread = (PsCreateSystemThreadFn)g_DriverCtx.Imports.PsCreateSystemThread;

    if (createThread) {
        void* threadHandle = 0;
        long ntStatus = createThread(
            &threadHandle,
            0x1F03FF,           // THREAD_ALL_ACCESS
            0,                  // ObjectAttributes
            0,                  // ProcessHandle (system process)
            0,                  // ClientId
            OmbraWorkerThread,
            &g_DriverCtx
        );

        if (ntStatus >= 0) {
            g_DriverCtx.WorkerThread = threadHandle;
        } else {
            // Thread creation failed - driver will still be usable but won't process commands
            g_DriverCtx.WorkerThread = 0;
        }
    }

    // Mark as initialized
    g_DriverCtx.Initialized = true;
    g_DriverCtx.ShutdownRequested = false;

    return 0;
}

// Shutdown driver
void OmbraDriverShutdown(void) {
    if (!g_DriverCtx.Initialized) return;

    // Signal shutdown
    g_DriverCtx.ShutdownRequested = true;

    // Wait for worker thread to exit
    // In a real implementation, we would:
    // 1. Set ShutdownRequested = true
    // 2. Wait on thread handle with KeWaitForSingleObject
    // 3. Close thread handle with ZwClose
    // For now, we just give it a moment via busy wait

    U64 wait_start, wait_now;
#ifdef _MSC_VER
    U32 tsc_low, tsc_high;
    __asm {
        rdtsc
        mov tsc_low, eax
        mov tsc_high, edx
    }
    wait_start = ((U64)tsc_high << 32) | tsc_low;
#else
    U32 tsc_low, tsc_high;
    __asm__ __volatile__("rdtsc" : "=a"(tsc_low), "=d"(tsc_high));
    wait_start = ((U64)tsc_high << 32) | tsc_low;
#endif

    // Busy wait for ~100ms (assuming 3GHz CPU = 300M cycles)
    do {
#ifdef _MSC_VER
        __asm {
            rdtsc
            mov tsc_low, eax
            mov tsc_high, edx
        }
        wait_now = ((U64)tsc_high << 32) | tsc_low;
#else
        __asm__ __volatile__("rdtsc" : "=a"(tsc_low), "=d"(tsc_high));
        wait_now = ((U64)tsc_high << 32) | tsc_low;
#endif

        // Yield CPU
#ifdef _MSC_VER
        __asm { pause }
#else
        __asm__ __volatile__("pause");
#endif
    } while ((wait_now - wait_start) < 300000000ULL);

    // Clean up subscriptions
    for (U32 i = 0; i < OMBRA_MAX_SUBSCRIPTIONS; i++) {
        if (g_DriverCtx.Subscriptions[i]) {
            // In a real implementation, we would free the subscription structure
            // For now, just null it out
            g_DriverCtx.Subscriptions[i] = 0;
        }
    }
    g_DriverCtx.SubscriptionCount = 0;

    // Notify hypervisor we're shutting down
    VmDriverShutdown();

    // Clear initialized flag
    g_DriverCtx.Initialized = false;

    // Zero out sensitive data
    ombra_memzero(&g_DriverCtx, sizeof(g_DriverCtx));
}
