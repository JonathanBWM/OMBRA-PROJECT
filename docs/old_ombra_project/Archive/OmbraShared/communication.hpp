#pragma once

// For vmxroot payloads, we don't need kernel headers - just basic Windows types
#if defined(_KERNEL_MODE) && !defined(_VMXROOT_MODE)
#include <ntddk.h>
#elif !defined(_VMXROOT_MODE)
#include <Windows.h>
#endif
// In _VMXROOT_MODE, types come from types.h/Windows.h included by payload

typedef enum : ULONG
{
	INST_MIN = 0,
	INST_GET_INFO = 1,
	INST_RESET_INFO,
	INST_MAP,
	INST_HIDE,
	INST_UNHIDE,
	INST_GET_OVERLAY_HANDLE,
	INST_SET_OVERLAY_HANDLE,
	INST_SET_DEFAULT_OVERLAY_HANDLE,
	INST_UNLOCK,
	INST_UNLOCK_ALL,
	INST_SUBSCRIBE_GAME,
	INST_UNSUBSCRIBE_GAME,
	INST_CRASH_SETUP,
	INST_IDENTITY_MAP,
	INST_IDENTITY_UNMAP,
	INST_LOCK_MODULE,
	INST_SHADOW,
	INST_BLOCK_IMAGE,
	INST_UNBLOCK_IMAGE,
	INST_PROTECT,
	INST_UNPROTECT,
	INST_REGISTER_SCORE_NOTIFY,
	INST_GET_SCORE,
	INST_GET_MOD_TRACKING,
	INST_SET_MOD_TRACKING,
	INST_SET_MOD_BACKUP,
	INST_ADD_SHADOW,
	INST_SPOOF,
	INST_MAX
} COMM_CODE, * PCOMM_CODE;

#pragma pack(push, 1)
typedef struct _MAP_INFO {
	PVOID pBuffer;
	SIZE_T sz;
	PVOID pOutBuffer;
	SIZE_T szOut;
	BOOLEAN bMapped;
	BOOLEAN bEPTHide;
} MAP_INFO, * PMAP_INFO;

typedef struct _DLL_TRACK_INFO {
	char* pDllName;
	unsigned long long dllBase;

	__forceinline bool operator==(_DLL_TRACK_INFO& rhs) {
		return !memcmp(&rhs, this, sizeof(rhs));
	}
	__forceinline bool operator!=(_DLL_TRACK_INFO& rhs) {
		return !(*this == rhs);
	}
} DLL_TRACK_INFO, * PDLL_TRACK_INFO;

__declspec(align(256)) typedef struct _PROC_INFO {
	unsigned long long* cr3;
	unsigned long long imageBase;
	unsigned long long pEprocess;
	unsigned long long pPeb;
	union {
		unsigned long long pTeb;
		unsigned long long lastTrackedCr3;
	};
	unsigned long long threadsStarted;
	unsigned long long dllsQueueShadow;
	unsigned long long lastDllBase;
	char* pImageName;

	MAP_INFO mapInfo;

	union {
		HANDLE hProc;
		unsigned long long pRequestor;
	};
	HANDLE dwPid;
	//Window to hide from this process
	HANDLE hWnd;

	bool bDead;
	bool bSetCr3;
	bool bMainThreadHidden;
	bool bDllInjected;
	bool bMapping;

	union {
		char* dllsToShadow;
		PDLL_TRACK_INFO dllToTrack;
	};

	//Used for EAC protection bypass
	bool lock;

	_PROC_INFO() {
		memset(this, 0, sizeof(*this));
#if defined(_KERNEL_MODE) && !defined(_VMXROOT_MODE)
		dllsToShadow = (char*)cpp::kMalloc(MAX_PATH);
#endif
	}

	__forceinline bool operator==(_PROC_INFO& rhs) {
		return !memcmp(&rhs, this, sizeof(rhs));
	}
	__forceinline bool operator!=(_PROC_INFO& rhs) {
		return !(*this == rhs);
	}
} PROC_INFO, * PPROC_INFO;

typedef struct _MEMORY_INFO {
	unsigned long long opSrcAddr;
	unsigned long long opDstAddr;
	unsigned long long opSize;
} MEMORY_INFO, * PMEMORY_INFO;

typedef struct _SCORE_INFO {
	DWORD score;
	DWORD halfLife;
	unsigned long long warningScore;
	unsigned long long untrustedScore;
	PBOOLEAN pDetected;
	PBOOLEAN pWarned;
} SCORE_INFO, * PSCORE_INFO;

typedef struct _BLOCK_INFO {
	char* pName;
	unsigned long long score;
} BLOCK_INFO, * PBLOCK_INFO;

typedef struct _BUGCHECK_INFO_EX {
	unsigned int ulBugCheckCode;
	wchar_t* pCaption;
	wchar_t* pMessage;
	wchar_t* pLink;
	unsigned int bgColor;

	__int64 pBugCheckTrampoline;
} BUGCHECK_INFO_EX, * PBUGCHECK_INFO_EX;

typedef struct _MOD_BACKUP_INFO {
	PVOID pBase;
	unsigned long long szMod;
	unsigned long long pEprocess;
} MOD_BACKUP_INFO, *PMOD_BACKUP_INFO;

typedef struct _KERNEL_REQUEST
{
	DWORD64 identifier;
	MEMORY_INFO memoryInfo;
	PROC_INFO procInfo;
	COMM_CODE instructionID;
	BUGCHECK_INFO_EX bugCheckInfo;
	SCORE_INFO scoreInfo;
	BLOCK_INFO blockInfo;
	union {
		unsigned long long seed;
		unsigned long long pIdentityMapping;
	};

	_KERNEL_REQUEST() {
		memset(this, 0, sizeof(*this));
		Init();
	}

	void Init() {
		identifier = 0xdeaddeadbeefbeef;
	}

	bool IsValid() {
		return identifier == 0xdeaddeadbeefbeef;
	}
} KERNEL_REQUEST, * PKERNEL_REQUEST;

typedef enum _CUSTOM_VMCALL_CODE : unsigned long long {
	VMCALL_GET_CALLBACK = 0x9898,
	VMCALL_ENABLE_CR3_EXIT,
	VMCALL_GET_HV_BUILD_FLAGS,
	VMCALL_GET_INFO
} CUSTOM_VMCALL_CODE, * PCUSTOM_VMCALL_CODE;

// VMCALL_TYPE - Hypercall command codes
// These commands are implemented in PayLoad/core/dispatch.cpp and
// PayLoad/{amd,intel}/*_handler.cpp
enum VMCALL_TYPE {
	// === IMPLEMENTED COMMANDS ===
	// Memory operations
	VMCALL_READ_VIRT = 0x100B,   // Read virtual memory (cross-process capable)
	VMCALL_WRITE_VIRT,          // Write virtual memory (cross-process capable)
	VMCALL_READ_PHY,            // Read physical memory
	VMCALL_WRITE_PHY,           // Write physical memory
	VMCALL_VIRT_TO_PHY = 0x1013,// Translate virtual to physical address

	// CR3 operations
	VMCALL_GET_CR3 = 0x1011,    // Get guest CR3
	VMCALL_GET_CR3_ROOT = 0x1016,// Get host/vmxroot CR3

	// EPT/NPT operations
	VMCALL_GET_EPT_BASE = 0x1012,// Get EPT/NPT base pointer
	VMCALL_SET_EPT_BASE = 0x1015,// Set EPT/NPT base pointer
	VMCALL_DISABLE_EPT = 0x100F,// Disable EPT/NPT
	VMCALL_ENABLE_EPT = 0x1017, // Enable EPT/NPT

	// Storage and communication
	VMCALL_STORAGE_QUERY = 0x1014,// Read/write storage slots
	VMCALL_SET_COMM_KEY = 0x1010,// Set authentication key

	// AMD-specific
	VMCALL_GET_VMCB = 0x1018,   // Get VMCB physical address (AMD only)

	// ETW Telemetry Control (Phase 3 Artifact Elimination)
	VMCALL_DISABLE_ETW_TI = 0x1020,  // Disable ETW Threat Intelligence provider
	VMCALL_ENABLE_ETW_TI = 0x1021,   // Re-enable ETW Threat Intelligence provider
	VMCALL_WIPE_ETW_BUFFERS = 0x1022,// Wipe specific events from ETW circular buffers
	VMCALL_CLEAR_EVENT_LOGS = 0x1023,// Clear specific Windows Event Log entries

	// === RESERVED FOR FUTURE IMPLEMENTATION ===
	// EPT page table manipulation (planned for Phase 4)
	// VMCALL_GET_EPT_PML3 = 0x1019,
	// VMCALL_GET_EPT_PML2 = 0x101A,
	// VMCALL_GET_EPT_PML1 = 0x101B,
	// VMCALL_SET_EPT_PML3 = 0x101C,
	// VMCALL_SET_EPT_PML2 = 0x101D,
	// VMCALL_SET_EPT_PML1 = 0x101E,

	// === DEPRECATED/UNUSED (kept for enum value stability) ===
	// VMCALL_TEST = 0x1001,     // Legacy test command
	// VMCALL_VMXOFF,            // Not implemented
	// VMCALL_INVEPT_CONTEXT,    // Not implemented
	// VMCALL_HOOK_PAGE,         // Not implemented
	// VMCALL_UNHOOK_PAGE,       // Not implemented
	// VMCALL_HOOK_PAGE_RANGE,   // Not implemented
	// VMCALL_HOOK_PAGE_INDEX,   // Not implemented
	// VMCALL_SUBSTITUTE_PAGE,   // Not implemented
	// VMCALL_CRASH,             // Not implemented
	// VMCALL_PROBE,             // Not implemented
};

typedef struct _READ_DATA {
	PVOID pOutBuf;
	PVOID pTarget;
	UINT64 length;
} READ_DATA, * PREAD_DATA;

typedef struct _WRITE_DATA {
	PVOID pInBuf;
	PVOID pTarget;
	UINT64 length;
} WRITE_DATA, * PWRITE_DATA;

typedef struct _CR3_DATA {
	UINT64 value;
} CR3_DATA, *PCR3_DATA;

typedef struct _TRANSLATION_DATA {
	PVOID va;
	UINT64 pa;
} TRANSLATION_DATA, *PTRANSLATION_DATA;

typedef struct _STORAGE_DATA {
	UINT64 id;
	union {
		PVOID pvoid;
		UINT64 uint64;
	};
	BOOLEAN bWrite;
} STORAGE_DATA, *PSTORAGE_DATA;

typedef struct _EPT_TRANSLATION_DATA {
	UINT64 pa;
	UINT64 pteIn;
	UINT64 pteOut;
} EPT_TRANSLATION_DATA, *PEPT_TRANSLATION_DATA;

typedef struct _ETW_DATA {
	UINT64 ntoskrnl_base;    // Base address of ntoskrnl.exe
	UINT64 offset;           // Offset to EtwThreatIntProvRegHandle
	UINT64 saved_value;      // Saved ProviderEnableInfo for restoration
} ETW_DATA, *PETW_DATA;

// ETW buffer wipe request - wipes events matching criteria from circular buffers
typedef struct _ETW_WIPE_DATA {
	UINT64 ntoskrnl_base;         // Base of ntoskrnl.exe for EtwpLoggerList
	UINT64 etwp_logger_list_offset; // Offset to EtwpLoggerList from ntoskrnl base
	UINT64 timestamp_start;       // Wipe events after this timestamp (100ns intervals)
	UINT64 timestamp_end;         // Wipe events before this timestamp
	UINT32 events_wiped;          // [OUT] Number of events wiped
	UINT32 buffers_scanned;       // [OUT] Number of buffers scanned
	char   target_driver_name[64]; // Driver name to match (e.g., "ThrottleStop")
} ETW_WIPE_DATA, *PETW_WIPE_DATA;

// Event log clear request - clears Windows Event Log entries (Service Control Manager)
typedef struct _EVENT_LOG_CLEAR_DATA {
	UINT64 ntoskrnl_base;         // Base of ntoskrnl.exe
	UINT64 timestamp_start;       // Clear events after this timestamp
	UINT64 timestamp_end;         // Clear events before this timestamp
	UINT32 events_cleared;        // [OUT] Number of events cleared
	UINT32 logs_processed;        // [OUT] Number of logs processed
	char   target_sources[4][32]; // Sources to clear (e.g., "Ld9BoxSup", "ThrottleStop")
	UINT32 target_event_ids[8];   // Event IDs to clear (e.g., 7045 = service install)
} EVENT_LOG_CLEAR_DATA, *PEVENT_LOG_CLEAR_DATA;

typedef union _COMMAND_DATA {
	READ_DATA read;
	WRITE_DATA write;
	CR3_DATA cr3;
	TRANSLATION_DATA translation;
	EPT_TRANSLATION_DATA ept;
	STORAGE_DATA storage;
	ETW_DATA etw;
	ETW_WIPE_DATA etw_wipe;
	EVENT_LOG_CLEAR_DATA event_log_clear;
	PVOID handler;
	UINT64 pa;
} COMMAND_DATA, * PCOMMAND_DATA;

enum VMX_ROOT_ERROR : unsigned long long
{
	SUCCESS,
	PML4E_NOT_PRESENT,
	PDPTE_NOT_PRESENT,
	PDE_NOT_PRESENT,
	PTE_NOT_PRESENT,
	VMXROOT_TRANSLATE_FAILURE,
	INVALID_SELF_REF_PML4E,
	INVALID_MAPPING_PML4E,
	INVALID_HOST_VIRTUAL,
	INVALID_GUEST_PHYSICAL,
	INVALID_GUEST_VIRTUAL,
	PAGE_TABLE_INIT_FAILED,
	PAGE_FAULT,
	INVALID_GUEST_PARAM
};

enum VMX_ROOT_STORAGE : unsigned long long
{
	CALLBACK_ADDRESS = 0,
	EPT_HANDLER_ADDRESS,
	EPT_OS_INIT_BITMAP,
	EPT_OS_INIT_BITMAP_END = EPT_OS_INIT_BITMAP + 8,
	DRIVER_BASE_PA,
	NTOSKRNL_CR3,
	CURRENT_CONTROLLER_PROCESS,
	PAYLOAD_BASE,             // SUPDrv-loaded payload base address
	MAX_STORAGE = 127
};

#pragma pack(pop)
