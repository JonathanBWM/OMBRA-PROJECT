#pragma once

#include "VMMDef.h"
#include "Vmcall.h"
#include "VectorEx.h"
#include "event.h"

typedef enum _OMBRA_EVENT_TYPE : ULONG64 {
	EVT_DETECTION
} OMBRA_EVENT_TYPE, * POMBRA_EVENT_TYPE;

struct EVENT_INFO {
	OMBRA_EVENT_TYPE type;
	PVOID guestAddr;
	ULONG64 guestCr3;

	__forceinline bool operator==(EVENT_INFO& rhs) {
		return !memcmp(this, &rhs, sizeof(rhs));
	}
	__forceinline bool operator!=(EVENT_INFO& rhs) {
		return !(*this == rhs);
	}
};

#ifdef _KERNEL_MODE

namespace eventhandler {
	void Init();

	void OnDetection();
}

#endif