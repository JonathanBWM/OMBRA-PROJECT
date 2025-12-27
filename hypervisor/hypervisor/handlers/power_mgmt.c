// power_mgmt.c — Power Management Instruction Handlers (MONITOR/MWAIT/PAUSE)
// OmbraHypervisor

#include "handlers.h"
#include "../vmx.h"
#include "../debug.h"
#include "../timing.h"
#include "../../shared/vmcs_fields.h"

// =============================================================================
// MONITOR Handler (Exit Reason 39)
// =============================================================================
//
// MONITOR sets up a linear address range to monitor for memory writes.
// It's paired with MWAIT for power-efficient waiting.
//
// Intel SDM Vol 2B: MONITOR instruction
// - Loads ECX into hint register (implementation-specific)
// - Uses RAX for linear address to monitor
// - Uses ECX and EDX as hints
//
// Handling strategy:
// - Allow the instruction to execute (pass through)
// - The CPU will handle the monitoring setup
// - Just advance RIP since we're not intercepting the actual monitoring
//
VMEXIT_ACTION HandleMonitor(GUEST_REGS* regs) {
    U64 entryTsc = TimingStart();
    VMX_CPU* cpu = VmxGetCurrentCpu();

    // MONITOR uses:
    // RAX = linear address to monitor
    // ECX = extensions (reserved, must be 0)
    // EDX = hints (implementation-specific)

    (void)regs;  // Parameters are in guest registers, already set up

    // For stability, we simply pass through MONITOR behavior
    // The CPU will track the address in hardware
    // No need to emulate - just acknowledge the instruction completed

    TRACE("MONITOR executed at RIP=0x%llx (RAX=0x%llx)",
          VmcsRead(VMCS_GUEST_RIP), regs->Rax);

    if (cpu) TimingEnd(cpu, entryTsc, TIMING_POWER_OVERHEAD);
    return VMEXIT_ADVANCE_RIP;
}

// =============================================================================
// MWAIT Handler (Exit Reason 36)
// =============================================================================
//
// MWAIT causes the processor to enter an optimized state while waiting
// for a write to the monitored address range set up by MONITOR.
//
// Intel SDM Vol 2B: MWAIT instruction
// - Requires prior MONITOR instruction
// - Uses EAX for hints (C-states)
// - Uses ECX for extensions (break on interrupt even if disabled)
//
// Handling strategy:
// - We can't actually let the guest enter MWAIT state (would halt our VMX)
// - Instead, just advance RIP to simulate completion
// - The guest will loop back and check its condition
// - Performance is acceptable since MWAIT is for idle loops
//
VMEXIT_ACTION HandleMwait(GUEST_REGS* regs) {
    U64 entryTsc = TimingStart();
    VMX_CPU* cpu = VmxGetCurrentCpu();

    // MWAIT uses:
    // EAX = hints (C-state sub-states)
    //       Bits 3:0 - C-state sub-state (implementation specific)
    //       Bits 7:4 - C-state (0=C0, 1=C1, etc)
    //       Bits 31:8 - Reserved
    // ECX = extensions
    //       Bit 0 - Interrupt as break event (1 = break even if interrupts masked)
    //       Bits 31:1 - Reserved

    U32 hints = (U32)regs->Rax;
    U32 extensions = (U32)regs->Rcx;

    U32 cState = (hints >> 4) & 0x0F;
    U32 cSubState = hints & 0x0F;
    bool breakOnInterrupt = (extensions & 0x01) != 0;

    // We cannot let the guest actually enter MWAIT state because:
    // 1. It would halt the processor
    // 2. We'd lose control of the VM
    //
    // Instead, we simulate immediate completion.
    //
    // Proper handling would be:
    // - If ECX[0]=1 and interrupts pending, break immediately
    // - Otherwise, wait for monitored address write or interrupt
    // - Could use single-stepping to simulate timing accurately
    //
    // For now, immediate return forces guest to re-check its condition.

    (void)cState;
    (void)cSubState;
    (void)breakOnInterrupt;

    TRACE("MWAIT executed at RIP=0x%llx (C%u.%u, break_on_int=%u)",
          VmcsRead(VMCS_GUEST_RIP), cState, cSubState, breakOnInterrupt);

    if (cpu) TimingEnd(cpu, entryTsc, TIMING_POWER_OVERHEAD);
    return VMEXIT_ADVANCE_RIP;
}

// =============================================================================
// PAUSE Handler (Exit Reason 40)
// =============================================================================
//
// PAUSE is a hint to the processor that this is a spin-wait loop.
// It's used extensively in spinlocks and busy-wait scenarios.
//
// Intel SDM Vol 2B: PAUSE instruction
// - Improves performance of spin-wait loops
// - Reduces power consumption
// - Avoids memory order violations on speculative execution
//
// Performance considerations:
// - PAUSE can execute THOUSANDS of times per second in spinlocks
// - Each VM-exit adds ~1000 cycles of overhead
// - This can destroy performance if enabled unconditionally
//
// Handling strategy:
// - Just advance RIP (fastest possible handling)
// - Consider DISABLING pause exiting in proc-based controls if not needed
// - Only enable if you need to:
//   a) Detect spin-loop behavior
//   b) Implement pause-loop exiting (PLE) tuning
//   c) Track performance characteristics
//
VMEXIT_ACTION HandlePause(GUEST_REGS* regs) {
    U64 entryTsc = TimingStart();
    VMX_CPU* cpu = VmxGetCurrentCpu();

    (void)regs;

    // PAUSE has no operands and no side effects except the hint to CPU
    // We just acknowledge it completed

    // NOTE: If you're seeing this exit frequently, consider disabling
    // pause exiting in VM-execution controls to improve performance:
    //
    // In vmcs.c setup:
    // proc_based &= ~CPU_BASED_PAUSE_EXITING;
    //
    // Or use pause-loop exiting (PLE) with appropriate window/gap values
    // to only exit on excessive spinning.

    // No trace - would spam logs due to high frequency

    if (cpu) TimingEnd(cpu, entryTsc, TIMING_POWER_OVERHEAD);
    return VMEXIT_ADVANCE_RIP;
}
