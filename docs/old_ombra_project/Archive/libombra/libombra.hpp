#pragma once
#include <intrin.h>
#include <communication.hpp>

#ifndef _KERNEL_MODE
#include <Windows.h>
#include <iostream>
#endif

#define PAGE_4KB 0x1000
#define PAGE_2MB PAGE_4KB * 512
#define PAGE_1GB PAGE_2MB * 512

#define STORAGE_MAX_INDEX 127

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

namespace ombra
{
	extern UINT64 VMEXIT_KEY;

	using guest_virt_t = u64;
	using guest_phys_t = u64;
	using host_virt_t = u64;
	using host_phys_t = u64;

	void set_vmcall_key(u64 key);

	/// <summary>
	/// this function is used to cause a vmexit as though its calling a function...
	/// </summary>
	extern "C" auto hypercall(u64 code, PCOMMAND_DATA param1, u64 param2, u64 key) -> VMX_ROOT_ERROR;

	template<typename T>
	auto hypercall(u64 code, T param1, u64 param2, u64 key) -> VMX_ROOT_ERROR
	{
		return hypercall(code, (PCOMMAND_DATA)param1, param2, key);
	}

	auto current_dirbase() -> guest_phys_t;

	auto root_dirbase() -> guest_phys_t;

	auto current_ept_base() -> guest_phys_t;

	auto vmcb() -> host_phys_t;

	VMX_ROOT_ERROR set_ept_base(guest_phys_t nCr3);

	void set_ept_handler(guest_virt_t handler);

	VMX_ROOT_ERROR disable_ept();

	VMX_ROOT_ERROR enable_ept();

	auto read_phys(guest_phys_t phys_addr, guest_virt_t buffer, u64 size) -> VMX_ROOT_ERROR;

	auto write_phys(guest_phys_t phys_addr, guest_virt_t buffer, u64 size) -> VMX_ROOT_ERROR;

	auto read_virt(guest_virt_t virt_addr, guest_virt_t buffer, u64 size, u64 target_cr3) -> VMX_ROOT_ERROR;

	auto write_virt(guest_virt_t virt_addr, guest_virt_t buffer, u64 size, u64 target_cr3) -> VMX_ROOT_ERROR;

	__forceinline auto malloc_locked(u64 size) -> guest_virt_t
	{
#ifndef _KERNEL_MODE
		auto p = malloc(size);
		if (!VirtualLock(p, size)) {
			if (!SetProcessWorkingSetSize(OpenProcess(PROCESS_ALL_ACCESS | PROCESS_SET_QUOTA, FALSE, GetCurrentProcessId()), 0x200 * 0x200 * 0x1000, 0x200 * 0x200 * 0x1000)) {
				if (p)
					free(p);
				return 0;
			}
			VirtualLock(p, size);
		}
		return (guest_virt_t)p;
#else
		return 0;
#endif
	}

	__forceinline auto malloc_locked_aligned(u64 size, u64 alignment) -> guest_virt_t
	{
#ifndef _KERNEL_MODE
		auto p = _aligned_malloc(size, alignment);
		if (!VirtualLock(p, size)) {
			if (!SetProcessWorkingSetSize(OpenProcess(PROCESS_ALL_ACCESS | PROCESS_SET_QUOTA, FALSE, GetCurrentProcessId()), 0x200 * 0x200 * 0x1000, 0x200 * 0x200 * 0x1000)) {
				if (p)
					free(p);
				return 0;
			}
			VirtualLock(p, size);
		}
		return (guest_virt_t)p;
#else
		return 0;
#endif
	}

	__forceinline auto free_locked(guest_virt_t p)
	{
#ifndef _KERNEL_MODE
		free((void*)p);
#endif
	}

	auto virt_to_phy(guest_virt_t p, u64 dirbase = 0) -> guest_phys_t;

	template<typename T>
	T storage_get(u64 id) {
		COMMAND_DATA data = { 0 };
		data.storage.bWrite = false;
		data.storage.id = id;
		auto status = hypercall(VMCALL_STORAGE_QUERY, &data, 0, VMEXIT_KEY);

		if (status != VMX_ROOT_ERROR::SUCCESS) {
			return T();
		}
		return (T)data.storage.uint64;
	}

	template<typename T>
	void storage_set(u64 id, T value) {
		COMMAND_DATA data = { 0 };
		data.storage.bWrite = true;
		data.storage.id = id;
		data.storage.uint64 = (UINT64)value;
		hypercall(VMCALL_STORAGE_QUERY, &data, 0, VMEXIT_KEY);
	}

	//===----------------------------------------------------------------------===//
	// ETW Threat Intelligence Provider Control (Phase 3)
	//===----------------------------------------------------------------------===//

	struct EtwState {
		u64 ntoskrnl_base;
		u64 offset;
		u64 saved_value;
		bool disabled;
	};

	// Disable ETW-TI provider to suppress telemetry during sensitive operations
	// Returns saved_value that must be passed to enable_etw_ti() for restoration
	inline VMX_ROOT_ERROR disable_etw_ti(u64 ntoskrnl_base, u64 offset, u64& out_saved_value) {
		COMMAND_DATA data = { 0 };
		data.etw.ntoskrnl_base = ntoskrnl_base;
		data.etw.offset = offset;
		data.etw.saved_value = 0;

		auto result = hypercall(VMCALL_DISABLE_ETW_TI, &data, 0, VMEXIT_KEY);

		if (result == VMX_ROOT_ERROR::SUCCESS) {
			out_saved_value = data.etw.saved_value;
		}

		return result;
	}

	// Restore ETW-TI provider using saved value from disable_etw_ti()
	inline VMX_ROOT_ERROR enable_etw_ti(u64 ntoskrnl_base, u64 offset, u64 saved_value) {
		COMMAND_DATA data = { 0 };
		data.etw.ntoskrnl_base = ntoskrnl_base;
		data.etw.offset = offset;
		data.etw.saved_value = saved_value;

		return hypercall(VMCALL_ENABLE_ETW_TI, &data, 0, VMEXIT_KEY);
	}

	// RAII wrapper for scoped ETW-TI blinding
	class ScopedEtwBlind {
	public:
		ScopedEtwBlind(u64 ntoskrnl_base, u64 offset)
			: m_ntoskrnl_base(ntoskrnl_base)
			, m_offset(offset)
			, m_saved_value(0)
			, m_disabled(false)
		{
			if (ntoskrnl_base && offset) {
				auto result = disable_etw_ti(ntoskrnl_base, offset, m_saved_value);
				m_disabled = (result == VMX_ROOT_ERROR::SUCCESS);
			}
		}

		~ScopedEtwBlind() {
			if (m_disabled) {
				enable_etw_ti(m_ntoskrnl_base, m_offset, m_saved_value);
			}
		}

		bool is_disabled() const { return m_disabled; }
		u64 saved_value() const { return m_saved_value; }

		// Non-copyable
		ScopedEtwBlind(const ScopedEtwBlind&) = delete;
		ScopedEtwBlind& operator=(const ScopedEtwBlind&) = delete;

	private:
		u64 m_ntoskrnl_base;
		u64 m_offset;
		u64 m_saved_value;
		bool m_disabled;
	};

	//===----------------------------------------------------------------------===//
	// ETW Buffer Wiping (Phase 3 - Post-Hypervisor Cleanup)
	//===----------------------------------------------------------------------===//

	struct EtwWipeResult {
		u32 events_wiped;
		u32 buffers_scanned;
		bool success;
	};

	// Wipe ETW events matching timestamp range from circular buffers
	// Call this AFTER hypervisor is active to retroactively clean up events
	// that were generated before ETW-TI was disabled
	//
	// Parameters:
	//   ntoskrnl_base: Base address of ntoskrnl.exe
	//   etwp_logger_list_offset: Offset to EtwpLoggerList from ntoskrnl base
	//   timestamp_start: Start of time range (100ns intervals since boot)
	//   timestamp_end: End of time range (0 = no limit)
	//   target_driver: Optional driver name to match (currently unused)
	inline EtwWipeResult wipe_etw_buffers(
		u64 ntoskrnl_base,
		u64 etwp_logger_list_offset,
		u64 timestamp_start,
		u64 timestamp_end = 0,
		const char* target_driver = nullptr
	) {
		COMMAND_DATA data = { 0 };
		data.etw_wipe.ntoskrnl_base = ntoskrnl_base;
		data.etw_wipe.etwp_logger_list_offset = etwp_logger_list_offset;
		data.etw_wipe.timestamp_start = timestamp_start;
		data.etw_wipe.timestamp_end = timestamp_end;
		data.etw_wipe.events_wiped = 0;
		data.etw_wipe.buffers_scanned = 0;

		if (target_driver) {
			strncpy(data.etw_wipe.target_driver_name, target_driver,
			        sizeof(data.etw_wipe.target_driver_name) - 1);
		}

		auto result = hypercall(VMCALL_WIPE_ETW_BUFFERS, &data, 0, VMEXIT_KEY);

		EtwWipeResult ret;
		ret.success = (result == VMX_ROOT_ERROR::SUCCESS);
		ret.events_wiped = data.etw_wipe.events_wiped;
		ret.buffers_scanned = data.etw_wipe.buffers_scanned;

		return ret;
	}

	// Get current timestamp in 100ns intervals (for use with wipe_etw_buffers)
	// This returns a value compatible with ETW event timestamps
	inline u64 get_etw_timestamp() {
#ifndef _KERNEL_MODE
		LARGE_INTEGER perfCounter, perfFreq;
		QueryPerformanceCounter(&perfCounter);
		QueryPerformanceFrequency(&perfFreq);
		// Convert to 100ns intervals
		return (perfCounter.QuadPart * 10000000ULL) / perfFreq.QuadPart;
#else
		return 0;
#endif
	}

	//===----------------------------------------------------------------------===//
	// Event Log Clearing (Phase 3 - Service Install Event Cleanup)
	//===----------------------------------------------------------------------===//

	struct EventLogClearResult {
		u32 events_cleared;
		u32 logs_processed;
		bool success;
	};

	// Clear Windows Event Log entries for specific service names
	// Targets Event ID 7045 (Service Install) which records driver loading
	//
	// Parameters:
	//   ntoskrnl_base: Base address of ntoskrnl.exe
	//   timestamp_start: Start of time range to clear
	//   timestamp_end: End of time range (0 = no limit)
	//   sources: Service names to match (e.g., "Ld9BoxSup", "ThrottleStop")
	//   event_ids: Event IDs to clear (default: 7045 = service install)
	inline EventLogClearResult clear_event_logs(
		u64 ntoskrnl_base,
		u64 timestamp_start,
		u64 timestamp_end = 0,
		const char* sources[] = nullptr,
		u32 source_count = 0,
		const u32* event_ids = nullptr,
		u32 event_id_count = 0
	) {
		COMMAND_DATA data = { 0 };
		data.event_log_clear.ntoskrnl_base = ntoskrnl_base;
		data.event_log_clear.timestamp_start = timestamp_start;
		data.event_log_clear.timestamp_end = timestamp_end;
		data.event_log_clear.events_cleared = 0;
		data.event_log_clear.logs_processed = 0;

		// Copy sources (up to 4)
		if (sources) {
			for (u32 i = 0; i < source_count && i < 4; i++) {
				if (sources[i]) {
					strncpy(data.event_log_clear.target_sources[i], sources[i], 31);
				}
			}
		} else {
			// Default sources
			strncpy(data.event_log_clear.target_sources[0], "Ld9BoxSup", 31);
			strncpy(data.event_log_clear.target_sources[1], "ThrottleStop", 31);
			strncpy(data.event_log_clear.target_sources[2], "Ld9BoxDrv", 31);
		}

		// Copy event IDs (up to 8)
		if (event_ids) {
			for (u32 i = 0; i < event_id_count && i < 8; i++) {
				data.event_log_clear.target_event_ids[i] = event_ids[i];
			}
		} else {
			// Default event IDs
			data.event_log_clear.target_event_ids[0] = 7045;  // Service install
			data.event_log_clear.target_event_ids[1] = 7040;  // Service start type changed
			data.event_log_clear.target_event_ids[2] = 7036;  // Service entered state
		}

		auto result = hypercall(VMCALL_CLEAR_EVENT_LOGS, &data, 0, VMEXIT_KEY);

		EventLogClearResult ret;
		ret.success = (result == VMX_ROOT_ERROR::SUCCESS);
		ret.events_cleared = data.event_log_clear.events_cleared;
		ret.logs_processed = data.event_log_clear.logs_processed;

		return ret;
	}
}