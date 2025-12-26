#!/usr/bin/env python3
"""
Anti-Cheat Intelligence — Detection methods and bypass techniques for major anti-cheats

This module provides intelligence on:
- Detection methods used by each anti-cheat
- Timing thresholds
- Bypass techniques
- Known signatures they scan for
"""

from datetime import datetime
from typing import Optional

# =============================================================================
# ANTI-CHEAT DATABASE
# =============================================================================

ANTICHEAT_DATABASE = {
    "EAC": {
        "name": "Easy Anti-Cheat",
        "vendor": "Epic Games (formerly Kamu)",
        "games": ["Fortnite", "Apex Legends", "Rust", "Dead by Daylight", "Elden Ring"],
        "kernel_driver": True,
        "ring0_component": "EasyAntiCheat.sys",
        "last_updated": "2025-12",
        
        "detection_methods": [
            {
                "id": "EAC-001",
                "method": "CPUID Timing Check",
                "description": "Measures RDTSC cycles around CPUID instruction",
                "threshold": 100,
                "threshold_unit": "cycles",
                "check_frequency": "periodic",
                "bypass": {
                    "technique": "TSC offset accumulation",
                    "implementation": """
// Track cumulative VM-exit overhead
vcpu->tsc_offset += (exit_end - exit_start);

// In RDTSC handler, subtract offset
uint64_t adjusted = __rdtsc() - vcpu->tsc_offset;
""",
                    "difficulty": "medium"
                }
            },
            {
                "id": "EAC-002", 
                "method": "CPUID Hypervisor Leaf",
                "description": "Checks CPUID.1:ECX[31] for hypervisor present bit",
                "bypass": {
                    "technique": "Clear hypervisor bit in CPUID handler",
                    "implementation": """
if (leaf == 1) {
    regs[ECX] &= ~(1 << 31);  // Clear hypervisor present
}
""",
                    "difficulty": "easy"
                }
            },
            {
                "id": "EAC-003",
                "method": "CPUID Vendor String Check",
                "description": "Queries CPUID.0x40000000 for hypervisor vendor",
                "bypass": {
                    "technique": "Return zeros for hypervisor leaves",
                    "implementation": """
if ((leaf & 0xF0000000) == 0x40000000) {
    regs[EAX] = regs[EBX] = regs[ECX] = regs[EDX] = 0;
}
""",
                    "difficulty": "easy"
                }
            },
            {
                "id": "EAC-004",
                "method": "VMX MSR Access",
                "description": "Attempts to read IA32_VMX_BASIC and related MSRs",
                "bypass": {
                    "technique": "Inject #GP on VMX MSR read attempts",
                    "implementation": """
if (msr >= 0x480 && msr <= 0x491) {
    inject_exception(vcpu, EXCEPTION_GP, 0);
    return;
}
""",
                    "difficulty": "easy"
                }
            },
            {
                "id": "EAC-005",
                "method": "CR4.VMXE Check",
                "description": "Reads CR4 to check if VMXE bit is set",
                "bypass": {
                    "technique": "Shadow CR4, return without VMXE bit",
                    "implementation": """
// Set up CR4 shadow in VMCS
__vmx_vmwrite(CR4_GUEST_HOST_MASK, CR4_VMXE);
__vmx_vmwrite(CR4_READ_SHADOW, real_cr4 & ~CR4_VMXE);
""",
                    "difficulty": "medium"
                }
            },
            {
                "id": "EAC-006",
                "method": "Loaded Driver Enumeration",
                "description": "Walks PsLoadedModuleList for suspicious drivers",
                "bypass": {
                    "technique": "Don't load a driver - use vulnerable driver primitives",
                    "implementation": "Use Ld9BoxSup.sys or similar to inject code without loading custom driver",
                    "difficulty": "N/A (architectural)"
                }
            },
            {
                "id": "EAC-007",
                "method": "Memory Pattern Scanning",
                "description": "Scans kernel memory for known hypervisor signatures",
                "bypass": {
                    "technique": "Write original code, avoid public implementations",
                    "implementation": "Don't copy code from HyperPlatform, SimpleSvm, etc.",
                    "difficulty": "high"
                }
            },
            {
                "id": "EAC-008",
                "method": "Exception Timing",
                "description": "Measures exception delivery timing",
                "check_frequency": "rare",
                "bypass": {
                    "technique": "Ensure exception injection is fast",
                    "difficulty": "medium"
                }
            }
        ],
        
        "known_signatures": [
            "HyperPlatform",
            "SimpleSvm", 
            "Voyager",
            "gbhv",
            "hvpp",
            "Pool tag: HvPl",
            "Pool tag: SSvm",
        ],
        
        "evasion_notes": [
            "EAC v2.4+ added more aggressive timing checks",
            "Kernel-mode component runs at DISPATCH_LEVEL",
            "Periodically checks from game process context",
            "May use hardware breakpoints for monitoring",
        ]
    },
    
    "BattlEye": {
        "name": "BattlEye",
        "vendor": "BattlEye Innovations",
        "games": ["PUBG", "Rainbow Six Siege", "Escape from Tarkov", "DayZ", "Arma 3"],
        "kernel_driver": True,
        "ring0_component": "BEDaisy.sys",
        "last_updated": "2025-12",
        
        "detection_methods": [
            {
                "id": "BE-001",
                "method": "CPUID Timing (Aggressive)",
                "description": "Multiple timing measurements with statistical analysis",
                "threshold": 120,
                "threshold_unit": "cycles",
                "samples": 100,
                "bypass": {
                    "technique": "Consistent TSC offset across all samples",
                    "implementation": """
// Must be consistent - they take multiple samples
// Use per-exit-reason offset tracking for accuracy
vcpu->timing.cpuid_overhead = measured_average;
""",
                    "difficulty": "hard"
                }
            },
            {
                "id": "BE-002",
                "method": "RDTSC Consistency",
                "description": "Compares RDTSC values across multiple calls",
                "bypass": {
                    "technique": "Ensure monotonic increasing adjusted TSC",
                    "implementation": """
// Never return a TSC value lower than previous
if (adjusted_tsc <= vcpu->last_reported_tsc) {
    adjusted_tsc = vcpu->last_reported_tsc + 1;
}
vcpu->last_reported_tsc = adjusted_tsc;
""",
                    "difficulty": "medium"
                }
            },
            {
                "id": "BE-003",
                "method": "Kernel Integrity Checks",
                "description": "Hashes critical kernel functions",
                "bypass": {
                    "technique": "Don't hook kernel functions, use EPT-based interception",
                    "difficulty": "N/A (architectural)"
                }
            },
            {
                "id": "BE-004",
                "method": "Stack Walking",
                "description": "Walks call stack looking for suspicious return addresses",
                "bypass": {
                    "technique": "Ensure VM-exit handler returns to legitimate code",
                    "difficulty": "medium"
                }
            },
            {
                "id": "BE-005",
                "method": "Debug Register Check",
                "description": "Monitors DR7 and DR0-DR3 for hardware breakpoints",
                "bypass": {
                    "technique": "Virtualize debug registers",
                    "implementation": """
// Shadow debug registers
// Return guest's expected values on MOV from DR
""",
                    "difficulty": "medium"
                }
            },
            {
                "id": "BE-006",
                "method": "NMI Timing",
                "description": "Uses NMI to interrupt and check execution state",
                "bypass": {
                    "technique": "Handle NMI exits properly",
                    "implementation": """
// NMI must be reflected to guest properly
// Don't add excessive latency to NMI delivery
""",
                    "difficulty": "hard"
                }
            }
        ],
        
        "known_signatures": [
            "Known hypervisor pool tags",
            "VMXON region patterns",
            "EPT structure signatures",
        ],
        
        "evasion_notes": [
            "BattlEye has kernel and usermode components",
            "Uses multiple threads for concurrent checks",
            "May request ring0 execution from hypervisor",
            "Sophisticated statistical analysis of timing",
        ]
    },
    
    "Vanguard": {
        "name": "Vanguard",
        "vendor": "Riot Games",
        "games": ["Valorant", "League of Legends"],
        "kernel_driver": True,
        "ring0_component": "vgk.sys",
        "boots_with_system": True,
        "last_updated": "2025-12",
        
        "detection_methods": [
            {
                "id": "VG-001",
                "method": "Boot-Time Initialization",
                "description": "Loads at boot, before most rootkits",
                "bypass": {
                    "technique": "Load hypervisor AFTER Vanguard (or use boot-chain injection)",
                    "implementation": "Runtime virtualization after system boot",
                    "difficulty": "medium"
                }
            },
            {
                "id": "VG-002",
                "method": "CPUID Full Analysis",
                "description": "Comprehensive CPUID leaf analysis including timing",
                "threshold": 100,
                "bypass": {
                    "technique": "Spoof all leaves consistently",
                    "difficulty": "medium"
                }
            },
            {
                "id": "VG-003",
                "method": "Hypervisor Feature Detection",
                "description": "Checks for hypervisor-specific CPUID leaves",
                "bypass": {
                    "technique": "Return no hypervisor features",
                    "implementation": """
// Block all 0x4000xxxx leaves
if ((leaf >> 28) == 4) {
    regs[EAX] = regs[EBX] = regs[ECX] = regs[EDX] = 0;
}
""",
                    "difficulty": "easy"
                }
            },
            {
                "id": "VG-004",
                "method": "Secure Boot Verification",
                "description": "Verifies Secure Boot chain integrity",
                "bypass": {
                    "technique": "Don't modify boot chain, use runtime virtualization",
                    "difficulty": "N/A (architectural)"
                }
            },
            {
                "id": "VG-005",
                "method": "TPM Attestation",
                "description": "May use TPM for system integrity",
                "bypass": {
                    "technique": "Runtime virtualization doesn't affect TPM measurements",
                    "difficulty": "N/A"
                }
            },
            {
                "id": "VG-006",
                "method": "Kernel Memory Scanning",
                "description": "Scans for unauthorized kernel modifications",
                "bypass": {
                    "technique": "Don't modify kernel memory, use EPT isolation",
                    "difficulty": "medium"
                }
            },
            {
                "id": "VG-007",
                "method": "HVCI Compatibility Check",
                "description": "Checks if HVCI/VBS is properly enabled",
                "bypass": {
                    "technique": "Only works when Hyper-V/VBS is disabled",
                    "note": "If Hyper-V is enabled, custom hypervisor approach won't work",
                    "difficulty": "N/A (limitation)"
                }
            }
        ],
        
        "evasion_notes": [
            "Vanguard runs at boot - earliest kernel driver",
            "Blocks known vulnerable drivers from loading",
            "Has BYOVD (vulnerable driver) blocklist",
            "May check for Ld9BoxSup.sys specifically",
            "Requires Secure Boot on modern versions",
        ],
        
        "vulnerable_driver_notes": [
            "Vanguard maintains blocklist of known vulnerable drivers",
            "Check if Ld9BoxSup.sys is blocked",
            "May need alternative driver or technique",
        ]
    },
    
    "FACEIT": {
        "name": "FACEIT Anti-Cheat",
        "vendor": "FACEIT",
        "games": ["CS2", "CS:GO (legacy)"],
        "kernel_driver": True,
        "ring0_component": "faceit.sys",
        "last_updated": "2025-12",
        
        "detection_methods": [
            {
                "id": "FC-001",
                "method": "CPUID Timing",
                "description": "Standard RDTSC around CPUID",
                "threshold": 150,
                "bypass": {
                    "technique": "TSC offset compensation",
                    "difficulty": "medium"
                }
            },
            {
                "id": "FC-002",
                "method": "MSR Probing",
                "description": "Attempts to read VMX capability MSRs",
                "bypass": {
                    "technique": "Inject #GP on VMX MSR access",
                    "difficulty": "easy"
                }
            },
            {
                "id": "FC-003",
                "method": "Known Signature Scan",
                "description": "Scans for public hypervisor code patterns",
                "bypass": {
                    "technique": "Original implementation",
                    "difficulty": "high"
                }
            }
        ],
        
        "evasion_notes": [
            "Less aggressive than EAC/BattlEye",
            "Focus on known cheats rather than generic detection",
            "May be easier to bypass with novel hypervisor",
        ]
    },
    
    "ESEA": {
        "name": "ESEA Anti-Cheat",
        "vendor": "ESL/ESEA",
        "games": ["CS2", "CS:GO (legacy)"],
        "kernel_driver": True,
        "last_updated": "2025-12",
        
        "detection_methods": [
            {
                "id": "ESEA-001",
                "method": "Ring-0 Integrity",
                "description": "Monitors kernel for modifications",
                "bypass": {
                    "technique": "EPT-based interception, no kernel mods",
                    "difficulty": "medium"
                }
            },
            {
                "id": "ESEA-002", 
                "method": "Hypervisor Detection",
                "description": "Standard CPUID and timing checks",
                "bypass": {
                    "technique": "Standard hypervisor hiding techniques",
                    "difficulty": "medium"
                }
            }
        ],
        
        "evasion_notes": [
            "Similar detection to FACEIT",
            "Maintains cheat signature database",
        ]
    }
}

# Generic timing thresholds across anti-cheats
TIMING_THRESHOLDS = {
    "cpuid_rdtsc": {
        "safe": 50,
        "warning": 100,
        "detected": 150,
        "unit": "cycles",
        "notes": "Keep under 100 cycles for safety margin"
    },
    "rdtsc_consistency": {
        "max_jitter": 20,
        "unit": "cycles",
        "notes": "Back-to-back RDTSC should be consistent"
    },
    "exception_delivery": {
        "safe": 200,
        "detected": 500,
        "unit": "cycles",
        "notes": "Exception injection should be fast"
    },
    "nmi_delivery": {
        "safe": 100,
        "detected": 300,
        "unit": "cycles",
        "notes": "NMI must be handled quickly"
    }
}

# =============================================================================
# MCP TOOL INTERFACE
# =============================================================================

async def get_anticheat_intel(anticheat: str) -> dict:
    """
    Get intelligence on specific anti-cheat detection methods.
    
    Args:
        anticheat: "EAC", "BattlEye", "Vanguard", "FACEIT", "ESEA", or "all"
    
    Returns:
        Detection methods, thresholds, and bypass techniques
    """
    if anticheat.lower() == "all":
        return {
            "anti_cheats": list(ANTICHEAT_DATABASE.keys()),
            "summary": {
                name: {
                    "detection_count": len(data["detection_methods"]),
                    "games": data["games"][:3],
                    "difficulty": "hard" if name in ["BattlEye", "Vanguard"] else "medium"
                }
                for name, data in ANTICHEAT_DATABASE.items()
            }
        }
    
    # Normalize name
    ac_map = {
        "eac": "EAC",
        "easy anti-cheat": "EAC",
        "easyanticheat": "EAC",
        "battleye": "BattlEye",
        "be": "BattlEye",
        "vanguard": "Vanguard",
        "riot": "Vanguard",
        "faceit": "FACEIT",
        "esea": "ESEA",
    }
    
    normalized = ac_map.get(anticheat.lower(), anticheat.upper())
    
    if normalized in ANTICHEAT_DATABASE:
        return ANTICHEAT_DATABASE[normalized]
    
    return {"error": f"Unknown anti-cheat: {anticheat}", "available": list(ANTICHEAT_DATABASE.keys())}


async def get_timing_requirements() -> dict:
    """
    Get timing requirements to evade all major anti-cheats.
    
    Returns:
        Timing thresholds and recommendations
    """
    return {
        "thresholds": TIMING_THRESHOLDS,
        "recommendations": {
            "target_cpuid_cycles": 50,
            "max_safe_cpuid_cycles": 100,
            "implementation": """
// Per-CPU timing state
typedef struct {
    uint64_t tsc_offset;           // Accumulated exit overhead
    uint64_t last_exit_tsc;        // When last exit occurred
    uint64_t cpuid_avg_overhead;   // Running average for CPUID
} TIMING_STATE;

// In CPUID handler
void handle_cpuid(VCPU* vcpu) {
    uint64_t start = __rdtsc();
    
    // Execute CPUID
    __cpuidex(regs, leaf, subleaf);
    
    // Spoof results...
    
    uint64_t end = __rdtsc();
    uint64_t overhead = end - start;
    
    // Update rolling average
    vcpu->timing.cpuid_avg_overhead = 
        (vcpu->timing.cpuid_avg_overhead * 7 + overhead) / 8;
    
    // Add to cumulative offset
    vcpu->timing.tsc_offset += overhead;
}

// In RDTSC handler
void handle_rdtsc(VCPU* vcpu) {
    uint64_t real = __rdtsc();
    uint64_t adjusted = real - vcpu->timing.tsc_offset;
    
    // Ensure monotonic
    if (adjusted <= vcpu->timing.last_reported_tsc) {
        adjusted = vcpu->timing.last_reported_tsc + 1;
    }
    vcpu->timing.last_reported_tsc = adjusted;
    
    vcpu->regs.rax = (uint32_t)adjusted;
    vcpu->regs.rdx = (uint32_t)(adjusted >> 32);
}
""",
            "critical_notes": [
                "Track offset per-CPU, not globally",
                "Ensure monotonically increasing TSC",
                "Account for ALL exit types that could be timed",
                "RDTSCP includes processor ID - handle separately",
            ]
        }
    }


async def get_bypass_for_detection(detection_id: str) -> dict:
    """
    Get specific bypass technique for a detection method.
    
    Args:
        detection_id: Detection ID like "EAC-001", "BE-002", etc.
    
    Returns:
        Detailed bypass information
    """
    for ac_name, ac_data in ANTICHEAT_DATABASE.items():
        for detection in ac_data["detection_methods"]:
            if detection.get("id") == detection_id:
                return {
                    "anti_cheat": ac_name,
                    "detection": detection,
                    "implementation_notes": ac_data.get("evasion_notes", [])
                }
    
    return {"error": f"Detection ID {detection_id} not found"}


async def check_evasion_coverage(implemented_bypasses: list) -> dict:
    """
    Check which detection methods are covered by implemented bypasses.
    
    Args:
        implemented_bypasses: List of bypass techniques implemented
            e.g., ["tsc_offset", "cpuid_spoofing", "msr_gp_injection"]
    
    Returns:
        Coverage analysis
    """
    bypass_mapping = {
        "tsc_offset": ["EAC-001", "BE-001", "BE-002", "VG-002", "FC-001"],
        "cpuid_spoofing": ["EAC-002", "EAC-003", "VG-002", "VG-003"],
        "msr_gp_injection": ["EAC-004", "FC-002"],
        "cr4_shadow": ["EAC-005"],
        "ept_isolation": ["BE-003", "VG-006"],
        "dr_virtualization": ["BE-005"],
        "nmi_handling": ["BE-006"],
    }
    
    covered = set()
    for bypass in implemented_bypasses:
        if bypass in bypass_mapping:
            covered.update(bypass_mapping[bypass])
    
    all_detections = set()
    for ac_data in ANTICHEAT_DATABASE.values():
        for detection in ac_data["detection_methods"]:
            if "id" in detection:
                all_detections.add(detection["id"])
    
    uncovered = all_detections - covered
    
    return {
        "implemented": implemented_bypasses,
        "covered_detections": list(covered),
        "uncovered_detections": list(uncovered),
        "coverage_percentage": len(covered) / len(all_detections) * 100 if all_detections else 0,
        "recommendations": [
            f"Implement bypass for: {det}" for det in list(uncovered)[:5]
        ]
    }


# Test if run directly
if __name__ == "__main__":
    import json
    import asyncio
    
    async def test():
        result = await get_anticheat_intel("EAC")
        print(json.dumps(result, indent=2, default=str))
    
    asyncio.run(test())
