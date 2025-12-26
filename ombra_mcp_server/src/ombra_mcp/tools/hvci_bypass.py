"""
HVCI/Hyper-V Bypass Tools — Runtime hypervisor hijacking guidance

This module provides guidance on:
- HVCI bypass architecture
- Runtime Hyper-V hijacking concepts
- Phase-based implementation approach
- Hypercall verification methods
- Build integration patterns
"""


# =============================================================================
# ZEROHVCI ARCHITECTURE CONCEPTS
# =============================================================================

ZEROHVCI_PHASES = {
    "phase_1_exploitation": {
        "name": "Kernel R/W Exploitation",
        "description": "Obtain kernel read/write primitives",
        "approaches": [
            {
                "name": "CVE-based exploitation",
                "notes": [
                    "Modify KTHREAD->PreviousMode from UserMode to KernelMode",
                    "Enables direct kernel memory R/W via NtRead/WriteVirtualMemory",
                    "Requires admin privileges"
                ],
                "constraints": [
                    "HVCI must be disabled",
                    "Unpatched Windows required for CVE approach"
                ]
            },
            {
                "name": "Vulnerable driver exploitation",
                "notes": [
                    "Use signed vulnerable driver for R/W primitives",
                    "Ld9BoxSup.sys (VirtualBox SUPDrv fork)",
                    "ThrottleStop.sys for physical memory access"
                ],
                "advantages": [
                    "Works on patched systems",
                    "Driver is legitimately signed"
                ]
            }
        ],
        "output": "Kernel R/W primitives, KernelForge initialized"
    },

    "phase_2_hijacking": {
        "name": "Runtime Hyper-V Hijacking",
        "description": "Inject custom payload into Hyper-V VMExit handler",
        "steps": [
            "Detect CPU vendor (Intel/AMD)",
            "Locate hvix64.exe/hvax64.exe module in memory",
            "Scan for VMExit handler entry point",
            "Allocate kernel memory for payload",
            "Map payload PE (fix relocations, imports)",
            "Patch VMExit handler CALL instruction"
        ],
        "intel_target": "hvix64.exe",
        "amd_target": "hvax64.exe",
        "output": "Hypervisor payload running in Ring -1"
    },

    "phase_3_verification": {
        "name": "Hypercall Verification",
        "description": "Verify hypervisor payload is active and responding",
        "steps": [
            "Generate random 64-bit session key",
            "Set key with hypervisor via VMCALL_SET_COMM_KEY",
            "Test communication with VMCALL_GET_CR3",
            "Validate CR3 response (non-zero, page-aligned)"
        ],
        "output": "Verified session key for future hypercalls"
    },

    "phase_4_driver_mapping": {
        "name": "Driver Integration",
        "description": "Map and initialize kernel driver via hypervisor",
        "paths": {
            "path_a_kernel_rw": "Use kernel R/W directly from Phase 1",
            "path_b_hypercalls": "Use verified hypercalls for VMCALL_WRITE_VIRT"
        },
        "steps": [
            "Load driver file from disk or embedded resource",
            "Parse PE sections to virtual layout",
            "Process base relocations",
            "Resolve ntoskrnl imports",
            "Copy to kernel memory via chosen path",
            "Call DriverEntry via KernelForge or hypercall"
        ]
    }
}


HYPERCALL_CONCEPTS = {
    "vehicle": {
        "instruction": "CPUID",
        "reason": [
            "Always causes VMExit (hardware enforced)",
            "Anti-cheat software ignores it (benign)",
            "Identical behavior on Intel VMX and AMD SVM",
            "Unhookable from Ring 0 (exits before hook execution)"
        ],
        "magic_leaf": "0x13371337"
    },

    "register_convention": {
        "RCX": "VMCALL_TYPE command code",
        "RDX": "Pointer to COMMAND_DATA structure",
        "R8": "Optional parameter (target CR3, etc)",
        "R9": "Session key (XOR'd with 0xBABAB00E)",
        "RAX": "Return value (VMX_ROOT_ERROR status)"
    },

    "authentication_flow": {
        "bootstrap": {
            "condition": "key == 0",
            "action": "VMCALL_SET_COMM_KEY allowed",
            "result": "Stores new key in VMXRoot storage"
        },
        "subsequent": {
            "obfuscation": "R9 = SessionKey XOR 0xBABAB00E",
            "validation": "Deobfuscate and compare to stored key",
            "if_valid": "Execute command, return result in RAX",
            "if_invalid": "Ignore command, return error"
        }
    },

    "session_key_generation": {
        "entropy_sources": [
            "RDTSC (CPU timestamp)",
            "(PID << 32) XOR (PID >> 32)",
            "QueryPerformanceCounter"
        ],
        "characteristics": [
            "Per-process (each process generates own)",
            "Temporary (lost on process exit)",
            "Obfuscated during transmission",
            "Sufficient entropy for session isolation"
        ]
    }
}


VMCALL_COMMANDS = {
    "memory_operations": {
        "VMCALL_READ_VIRT": "Cross-process virtual memory read",
        "VMCALL_WRITE_VIRT": "Cross-process virtual memory write",
        "VMCALL_READ_PHY": "Physical memory read",
        "VMCALL_WRITE_PHY": "Physical memory write"
    },
    "translation": {
        "VMCALL_VIRT_TO_PHY": "Virtual to physical address translation",
        "VMCALL_GET_CR3": "Get guest CR3 (page table base)",
        "VMCALL_GET_CR3_ROOT": "Get system CR3"
    },
    "ept_control": {
        "VMCALL_GET_EPT_BASE": "Get current EPT base",
        "VMCALL_SET_EPT_BASE": "Switch EPT base",
        "VMCALL_ENABLE_EPT": "Enable EPT for guest",
        "VMCALL_DISABLE_EPT": "Disable EPT"
    },
    "storage": {
        "VMCALL_STORAGE_QUERY": "Read/write VMX root storage (0-127 slots)"
    },
    "anti_forensics": {
        "VMCALL_SET_COMM_KEY": "Set communication key",
        "VMCALL_DISABLE_ETW_TI": "Blind ETW Threat Intelligence",
        "VMCALL_ENABLE_ETW_TI": "Restore ETW-TI",
        "VMCALL_WIPE_ETW_BUFFERS": "Clear ETW circular buffers",
        "VMCALL_CLEAR_EVENT_LOGS": "Clear Windows Event Logs"
    }
}


# =============================================================================
# TOOL FUNCTIONS
# =============================================================================

def get_zerohvci_architecture() -> dict:
    """
    Get comprehensive ZeroHVCI architecture overview.

    Returns:
        Phase-based architecture and concepts
    """
    return {
        "overview": "Runtime hypervisor hijacking on Windows without bootkit",
        "phases": ZEROHVCI_PHASES,
        "advantages": [
            "No bootkit required - works on running systems",
            "No reboot needed - runtime injection",
            "Unified approach for Intel/AMD via payload abstraction",
            "Session verification prevents accidental triggering"
        ],
        "requirements": [
            "Administrator privileges",
            "HVCI/VBS disabled",
            "Kernel R/W primitives (CVE or vulnerable driver)"
        ]
    }


def get_hypercall_protocol() -> dict:
    """
    Get hypercall protocol specification.

    Returns:
        Complete hypercall communication protocol
    """
    return {
        "concepts": HYPERCALL_CONCEPTS,
        "commands": VMCALL_COMMANDS,
        "implementation_notes": [
            "Use CPUID instead of VMCALL for stealth",
            "XOR authentication key before transmission",
            "Validate response before trusting data",
            "CR3 response must be page-aligned (lower 12 bits = 0)"
        ]
    }


def get_hyperv_hijack_concepts() -> dict:
    """
    Get Hyper-V hijacking concepts.

    Returns:
        VMExit handler patching approach
    """
    return {
        "target_modules": {
            "intel": {
                "module": "hvix64.exe",
                "description": "Intel VT-x Hyper-V hypervisor"
            },
            "amd": {
                "module": "hvax64.exe",
                "description": "AMD SVM Hyper-V hypervisor"
            }
        },
        "hijack_approach": {
            "discovery": "Scan for VMExit handler entry point via signatures",
            "allocation": "Allocate kernel memory for payload",
            "mapping": "Map payload PE with proper relocations",
            "patching": "Patch VMExit handler CALL instruction to redirect",
            "notes": [
                "Payload must be position-independent or properly relocated",
                "Must match Hyper-V calling convention",
                "Intel and AMD have different entry point signatures"
            ]
        },
        "payload_requirements": [
            "DLL format for easy PE manipulation",
            "Extern 'C' entry point for calling convention",
            "Architecture-specific (separate Intel/AMD builds)",
            "Must handle VMExit and VMRESUME correctly"
        ]
    }


def get_phase_implementation_guide(phase: int) -> dict:
    """
    Get detailed implementation guidance for specific phase.

    Args:
        phase: Phase number (1-4)

    Returns:
        Implementation details for requested phase
    """
    phase_key = f"phase_{phase}_{'exploitation' if phase == 1 else 'hijacking' if phase == 2 else 'verification' if phase == 3 else 'driver_mapping'}"

    if phase_key not in ZEROHVCI_PHASES:
        return {"error": f"Unknown phase: {phase}", "available": [1, 2, 3, 4]}

    phase_data = ZEROHVCI_PHASES[phase_key]

    if phase == 1:
        return {
            **phase_data,
            "kernelforge_concepts": {
                "purpose": "Bypass normal syscall mechanisms for kernel function execution",
                "gadgets_needed": [
                    "mov rax/rcx/rdx/r8/r9 from stack; jmp rax (argument loading)",
                    "add rsp, 68h; ret (stack reservation)",
                    "pop rcx; ret (RCX control)",
                    "mov [rcx], rax; ret (store return value)"
                ],
                "discovery": "Pattern match gadgets in ntoskrnl.exe userspace image",
                "usage": "Build ROP chain to call arbitrary kernel functions"
            }
        }

    elif phase == 2:
        return {
            **phase_data,
            "patching_details": {
                "patch_size": 12,
                "patch_type": "Absolute 64-bit JMP",
                "format": "48 B8 [8-byte address] FF E0",
                "description": "movabs rax, payload_entry; jmp rax",
                "notes": [
                    "Patch VMExit handler dispatch CALL",
                    "Payload receives guest context",
                    "Payload must eventually VMRESUME"
                ]
            }
        }

    elif phase == 3:
        return {
            **phase_data,
            "verification_steps": [
                {
                    "step": 1,
                    "action": "Generate entropy-based session key",
                    "code_hint": "key = RDTSC ^ (PID << 32) ^ QPC"
                },
                {
                    "step": 2,
                    "action": "Bootstrap authentication with hypervisor",
                    "command": "VMCALL_SET_COMM_KEY",
                    "key_value": 0,
                    "note": "Key=0 is only allowed for this specific command"
                },
                {
                    "step": 3,
                    "action": "Verify hypercall actually works",
                    "command": "VMCALL_GET_CR3",
                    "validation": "CR3 must be non-zero and page-aligned"
                }
            ]
        }

    elif phase == 4:
        return {
            **phase_data,
            "mapping_considerations": [
                "Use Phase 1 kernel R/W OR Phase 3 hypercalls (not both)",
                "Phase 3 hypercalls safer but require verification",
                "Driver imports resolved against kernel modules",
                "DriverEntry called with custom params or allocation info"
            ]
        }


def get_detection_mitigation() -> dict:
    """
    Get detection vectors and mitigations for ZeroHVCI approach.

    Returns:
        Security considerations and mitigations
    """
    return {
        "detection_vectors": {
            "memory_signature": {
                "risk": "MZ/PE headers visible in pool allocations",
                "mitigation": "EPT hiding or header wiping"
            },
            "timing_anomaly": {
                "risk": "Hypercall latency detectable",
                "mitigation": "Timing compensation in payload"
            },
            "cpuid_hooking": {
                "risk": "Anti-cheat could intercept CPUID",
                "mitigation": "CPUID unhookable from Ring 0 (exits first)"
            },
            "pool_tags": {
                "risk": "ExAllocatePool uses detectable tags",
                "mitigation": "Use generic tags or wipe after allocation"
            },
            "behavioral": {
                "risk": "VMExit frequency anomalies",
                "mitigation": "CPUID cloaking, minimize exit overhead"
            }
        },
        "architectural_advantages": [
            "No boot chain modification - TPM clean",
            "No driver loading - PsLoadedModuleList clean",
            "Hyper-V already trusted by OS",
            "Runtime injection undetectable at boot"
        ]
    }


def get_build_integration_guide() -> dict:
    """
    Get build integration patterns for ZeroHVCI framework.

    Returns:
        Build configuration and integration guidance
    """
    return {
        "file_organization": {
            "zerohvci/": {
                "ntdefs.h": "Kernel structure definitions",
                "utils.h": "PE utilities, address leak functions",
                "exploit.h": "Vulnerability implementations",
                "kforge.h": "ROP gadget framework",
                "zerohvci.h": "Main API header",
                "zerohvci.cpp": "API implementation",
                "hyperv_hijack.h": "Runtime hijacking logic",
                "hypercall_verify.h": "Phase 3 verification",
                "hypercall.asm": "MASM x64 CPUID assembly",
                "driver_mapper.h": "Phase 4 PE mapping",
                "driver_mapper.cpp": "Phase 4 implementation"
            }
        },
        "compiler_config": {
            "platform": "x64 only",
            "cpp_standard": "C++17 or later",
            "exception_handling": "/EHsc",
            "masm": "Required for hypercall.asm"
        },
        "library_dependencies": [
            "ntdll.lib",
            "Psapi.lib"
        ],
        "usage_pattern": [
            "zerohvci::Initialize() - Phase 1",
            "hyperv::RuntimeHijacker::HijackHyperV() - Phase 2",
            "hypercall::VerifyHypervisorActive() - Phase 3",
            "mapper::MapDriver() - Phase 4"
        ]
    }


def get_amd_vs_intel_considerations() -> dict:
    """
    Get AMD vs Intel architecture considerations for payload.

    Returns:
        Architecture-specific guidance
    """
    return {
        "separation_approach": {
            "pattern": "Two completely independent backend implementations",
            "shared_core": "Unified dispatch layer (architecture-agnostic)",
            "abstraction": "VmExitContext struct bridges backends and core"
        },
        "intel_specifics": {
            "hypervisor_module": "hvix64.exe",
            "instruction_set": "VMX (Virtual Machine Extensions)",
            "control_structure": "VMCS (Virtual Machine Control Structure)",
            "exit_handler_return": "void"
        },
        "amd_specifics": {
            "hypervisor_module": "hvax64.exe",
            "instruction_set": "SVM (Secure Virtual Machine)",
            "control_structure": "VMCB (Virtual Machine Control Block)",
            "exit_handler_return": "pgs_base_struct*",
            "extra_requirements": [
                "MASM exception handlers (IDT.asm)",
                "Explicit #PF/#NP handling",
                "Hyper-V expects specific return structure"
            ]
        },
        "dual_build_strategy": [
            "Two separate DLLs: PayLoad-Intel.dll, PayLoad-AMD.dll",
            "Shared core compiled into both",
            "Preprocessor defines: INTEL_BUILD vs AMD_BUILD",
            "CPU detection at runtime selects correct DLL"
        ]
    }


# Test if run directly
if __name__ == "__main__":
    import json

    print("=== ZeroHVCI Architecture ===")
    print(json.dumps(get_zerohvci_architecture(), indent=2))

    print("\n=== Hypercall Protocol ===")
    print(json.dumps(get_hypercall_protocol(), indent=2))
