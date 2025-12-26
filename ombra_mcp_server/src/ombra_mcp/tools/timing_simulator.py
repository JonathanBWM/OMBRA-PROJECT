#!/usr/bin/env python3
"""
Timing Simulator — Estimate VM-exit handler execution time

This module:
- Analyzes handler code to estimate cycle count
- Identifies timing bottlenecks
- Suggests optimizations
- Validates against anti-cheat thresholds
"""

import re
from typing import List, Dict, Tuple
from dataclasses import dataclass

# =============================================================================
# INSTRUCTION TIMING DATABASE
# =============================================================================

# Approximate cycle counts for x86-64 instructions
# Based on Intel optimization manual / Agner Fog's tables
# These are rough estimates for Skylake/newer

INSTRUCTION_CYCLES = {
    # Basic ALU operations
    "mov": 1,
    "add": 1,
    "sub": 1,
    "and": 1,
    "or": 1,
    "xor": 1,
    "inc": 1,
    "dec": 1,
    "neg": 1,
    "not": 1,
    "lea": 1,
    "nop": 1,
    
    # Shifts and rotates
    "shl": 1,
    "shr": 1,
    "sar": 1,
    "rol": 1,
    "ror": 1,
    
    # Multiplication/Division
    "mul": 3,
    "imul": 3,
    "div": 20,
    "idiv": 20,
    
    # Comparisons and tests
    "cmp": 1,
    "test": 1,
    
    # Jumps and branches
    "jmp": 1,
    "je": 1,
    "jne": 1,
    "jz": 1,
    "jnz": 1,
    "jl": 1,
    "jg": 1,
    "jle": 1,
    "jge": 1,
    "call": 2,
    "ret": 2,
    
    # Stack operations
    "push": 1,
    "pop": 1,
    "pushfq": 3,
    "popfq": 5,
    
    # Memory operations (cache hit)
    "load": 4,
    "store": 3,
    
    # VMX instructions (EXPENSIVE)
    "vmread": 10,
    "vmwrite": 10,
    "vmcall": 50,
    "vmresume": 50,
    
    # Control register access
    "mov_cr": 20,
    "mov_dr": 20,
    
    # MSR access
    "rdmsr": 15,
    "wrmsr": 20,
    
    # CPUID
    "cpuid": 30,
    
    # RDTSC
    "rdtsc": 10,
    "rdtscp": 15,
    
    # Serializing instructions
    "mfence": 30,
    "lfence": 5,
    "sfence": 5,
    
    # Function call overhead
    "function_call": 5,
    "function_return": 3,
}

# VM-exit specific overhead
VMEXIT_OVERHEAD = {
    "hardware_exit": 500,
    "context_save": 50,
    "context_restore": 50,
    "vmresume": 400,
    "minimum_roundtrip": 1000,
}

# Handler-specific baseline costs
HANDLER_BASELINES = {
    "cpuid": {
        "base_cost": 50,
        "spoofing_cost": 10,
        "total_typical": 60
    },
    "rdtsc": {
        "base_cost": 15,
        "offset_calc": 5,
        "total_typical": 20
    },
    "rdtscp": {
        "base_cost": 20,
        "offset_calc": 5,
        "processor_id": 5,
        "total_typical": 30
    },
    "msr_read": {
        "base_cost": 20,
        "filtering_cost": 10,
        "total_typical": 30
    },
    "msr_write": {
        "base_cost": 25,
        "filtering_cost": 10,
        "total_typical": 35
    },
    "cr_access": {
        "base_cost": 25,
        "shadow_update": 10,
        "total_typical": 35
    },
    "ept_violation": {
        "base_cost": 30,
        "page_walk": 50,
        "total_typical": 80
    },
    "exception": {
        "base_cost": 20,
        "injection": 30,
        "total_typical": 50
    },
}

# =============================================================================
# CODE ANALYZER
# =============================================================================

@dataclass
class TimingAnalysis:
    estimated_cycles: int
    breakdown: Dict[str, int]
    bottlenecks: List[Dict]
    optimizations: List[Dict]
    safe_for_anticheat: bool
    warnings: List[str]


class TimingSimulator:
    def __init__(self):
        self.anti_cheat_threshold = 100  # cycles
        
    def count_instructions(self, code: str) -> Dict[str, int]:
        """Count instruction types in code."""
        counts = {}
        
        patterns = {
            "vmread": r'__vmx_vmread|vmread|VMREAD|VmxRead',
            "vmwrite": r'__vmx_vmwrite|vmwrite|VMWRITE|VmxWrite',
            "rdtsc": r'__rdtsc\s*\(|rdtsc\b',
            "cpuid": r'__cpuid|__cpuidex|cpuid\s*\(',
            "rdmsr": r'__readmsr|rdmsr\b|ReadMsr',
            "wrmsr": r'__writemsr|wrmsr\b|WriteMsr',
            "function_call": r'\b\w+\s*\([^)]*\)\s*;',
            "conditional": r'\bif\s*\(|\bswitch\s*\(|\bwhile\s*\(|\bfor\s*\(',
            "mov_cr": r'__readcr\d|__writecr\d|mov\s+cr',
            "mov_dr": r'__readdr|__writedr',
        }
        
        for name, pattern in patterns.items():
            matches = re.findall(pattern, code, re.IGNORECASE)
            counts[name] = len(matches)
        
        # Count memory accesses (rough estimate)
        counts["memory_access"] = code.count("->") + code.count("[")
        
        # Count assignments
        counts["assignments"] = len(re.findall(r'[^=!<>]=[^=]', code))
        
        return counts
    
    def estimate_cycles(self, code: str, handler_type: str = "generic") -> int:
        """Estimate total cycle count for code."""
        counts = self.count_instructions(code)
        
        total = 0
        
        # VMX instructions
        total += counts.get("vmread", 0) * INSTRUCTION_CYCLES["vmread"]
        total += counts.get("vmwrite", 0) * INSTRUCTION_CYCLES["vmwrite"]
        
        # MSR access
        total += counts.get("rdmsr", 0) * INSTRUCTION_CYCLES["rdmsr"]
        total += counts.get("wrmsr", 0) * INSTRUCTION_CYCLES["wrmsr"]
        
        # CPUID/RDTSC
        total += counts.get("cpuid", 0) * INSTRUCTION_CYCLES["cpuid"]
        total += counts.get("rdtsc", 0) * INSTRUCTION_CYCLES["rdtsc"]
        
        # CR/DR access
        total += counts.get("mov_cr", 0) * INSTRUCTION_CYCLES["mov_cr"]
        total += counts.get("mov_dr", 0) * INSTRUCTION_CYCLES["mov_dr"]
        
        # Function calls (internal)
        # Subtract 1 to not count the handler function itself
        func_calls = max(0, counts.get("function_call", 0) - 1)
        total += func_calls * INSTRUCTION_CYCLES["function_call"]
        
        # Memory accesses
        total += counts.get("memory_access", 0) * INSTRUCTION_CYCLES["load"]
        
        # Simple operations (assignments, conditionals)
        total += counts.get("assignments", 0) * 2
        total += counts.get("conditional", 0) * 3
        
        # Add handler baseline if known
        if handler_type in HANDLER_BASELINES:
            baseline = HANDLER_BASELINES[handler_type]["total_typical"]
            total = max(total, baseline)
        
        return total
    
    def identify_bottlenecks(self, code: str) -> List[Dict]:
        """Identify timing bottlenecks in code."""
        bottlenecks = []
        counts = self.count_instructions(code)
        
        # Check for multiple VMREADs/VMWRITEs
        if counts.get("vmread", 0) > 5:
            bottlenecks.append({
                "type": "excessive_vmread",
                "count": counts["vmread"],
                "impact_cycles": counts["vmread"] * 10,
                "severity": "high" if counts["vmread"] > 10 else "medium",
                "suggestion": "Cache VMREAD results in local variables or vcpu structure"
            })
        
        if counts.get("vmwrite", 0) > 5:
            bottlenecks.append({
                "type": "excessive_vmwrite",
                "count": counts["vmwrite"],
                "impact_cycles": counts["vmwrite"] * 10,
                "severity": "medium",
                "suggestion": "Batch VMWRITE operations, avoid redundant writes"
            })
        
        # Check for MSR access in hot path
        msr_count = counts.get("rdmsr", 0) + counts.get("wrmsr", 0)
        if msr_count > 0:
            bottlenecks.append({
                "type": "msr_access_in_handler",
                "count": msr_count,
                "impact_cycles": 20 * msr_count,
                "severity": "medium",
                "suggestion": "Cache MSR values at virtualization start, avoid runtime reads"
            })
        
        # Check for multiple CPUID calls
        if counts.get("cpuid", 0) > 1:
            bottlenecks.append({
                "type": "multiple_cpuid",
                "count": counts["cpuid"],
                "impact_cycles": counts["cpuid"] * 30,
                "severity": "high",
                "suggestion": "Single __cpuidex() call should suffice for any CPUID exit"
            })
        
        # Check for many function calls
        if counts.get("function_call", 0) > 5:
            bottlenecks.append({
                "type": "function_call_overhead",
                "count": counts["function_call"],
                "impact_cycles": counts["function_call"] * 5,
                "severity": "low",
                "suggestion": "Consider inlining hot helper functions with __forceinline"
            })
        
        # Check for serializing patterns
        if re.search(r'mfence|lfence|sfence', code, re.IGNORECASE):
            fence_count = len(re.findall(r'mfence|lfence|sfence', code, re.IGNORECASE))
            bottlenecks.append({
                "type": "serializing_instruction",
                "count": fence_count,
                "impact_cycles": fence_count * 20,
                "severity": "medium",
                "suggestion": "Remove unnecessary memory fences from hot path"
            })
        
        # Check for loops
        if re.search(r'\bfor\s*\(|\bwhile\s*\(', code):
            bottlenecks.append({
                "type": "loop_in_handler",
                "severity": "high",
                "suggestion": "Loops in exit handlers are dangerous for timing. Consider unrolling or restructuring."
            })
        
        # Check for debug output
        if re.search(r'DbgPrint|KdPrint|printf|LOG|TRACE', code, re.IGNORECASE):
            bottlenecks.append({
                "type": "debug_output",
                "severity": "critical",
                "suggestion": "REMOVE all debug output from release builds - adds 1000+ cycles"
            })
        
        return bottlenecks
    
    def suggest_optimizations(self, code: str, handler_type: str) -> List[Dict]:
        """Suggest optimizations for the handler."""
        optimizations = []
        counts = self.count_instructions(code)
        
        # VMREAD caching
        if counts.get("vmread", 0) > 3:
            optimizations.append({
                "type": "vmread_caching",
                "priority": "high",
                "description": "Cache frequently read VMCS fields at exit entry",
                "example": """
// At start of vmexit handler (once):
vcpu->cached.exit_reason = __vmx_vmread(VM_EXIT_REASON);
vcpu->cached.exit_qual = __vmx_vmread(EXIT_QUALIFICATION);
vcpu->cached.guest_rip = __vmx_vmread(GUEST_RIP);
vcpu->cached.guest_rsp = __vmx_vmread(GUEST_RSP);

// Then use vcpu->cached.* throughout handlers
""",
                "estimated_savings_cycles": (counts["vmread"] - 4) * 10
            })
        
        # Inline RDTSC handler
        if handler_type in ["rdtsc", "rdtscp"]:
            optimizations.append({
                "type": "inline_rdtsc_handler",
                "priority": "high",
                "description": "Inline RDTSC/RDTSCP handling in dispatch for minimal latency",
                "example": """
// In exit dispatch, avoid function call:
case EXIT_REASON_RDTSC: {
    uint64_t tsc = __rdtsc() - vcpu->tsc_offset;
    vcpu->gpr.rax = (uint32_t)tsc;
    vcpu->gpr.rdx = (uint32_t)(tsc >> 32);
    vcpu->rip += 2;
    return;
}
""",
                "estimated_savings_cycles": 15
            })
        
        # Inline CPUID handler
        if handler_type == "cpuid":
            optimizations.append({
                "type": "inline_cpuid_spoofing",
                "priority": "high",
                "description": "Inline common CPUID leaf handling",
                "example": """
// Fast path for leaf 1 (most common):
case EXIT_REASON_CPUID: {
    int regs[4];
    __cpuidex(regs, vcpu->gpr.rax, vcpu->gpr.rcx);
    
    if (vcpu->gpr.rax == 1) {
        regs[2] &= ~(1 << 31);  // Clear hypervisor bit
        regs[2] &= ~(1 << 5);   // Clear VMX bit
    } else if ((vcpu->gpr.rax >> 28) == 4) {
        regs[0] = regs[1] = regs[2] = regs[3] = 0;  // Hide HV leaves
    }
    
    vcpu->gpr.rax = regs[0];
    vcpu->gpr.rbx = regs[1];
    vcpu->gpr.rcx = regs[2];
    vcpu->gpr.rdx = regs[3];
    vcpu->rip += 2;
    return;
}
""",
                "estimated_savings_cycles": 10
            })
        
        # Use lookup table for exit dispatch
        if counts.get("conditional", 0) > 5:
            optimizations.append({
                "type": "dispatch_table",
                "priority": "medium",
                "description": "Use function pointer table instead of switch/case",
                "example": """
typedef void (*EXIT_HANDLER)(VCPU*);

static EXIT_HANDLER g_handlers[65] = {
    [EXIT_REASON_CPUID] = handle_cpuid,
    [EXIT_REASON_RDTSC] = handle_rdtsc,
    // ...
};

// Dispatch:
if (exit_reason < 65 && g_handlers[exit_reason]) {
    g_handlers[exit_reason](vcpu);
} else {
    handle_unexpected(vcpu, exit_reason);
}
""",
                "estimated_savings_cycles": 5
            })
        
        # Branch prediction hints
        optimizations.append({
            "type": "branch_hints",
            "priority": "low",
            "description": "Use likely/unlikely for predictable branches",
            "example": """
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

// MSVC equivalent:
#define likely(x)   (x)
#define unlikely(x) (x)
// Use PGO for MSVC instead

if (unlikely(exit_reason == EXIT_REASON_TRIPLE_FAULT)) {
    // Rare error path
}
""",
            "estimated_savings_cycles": 3
        })
        
        # Avoid redundant RIP advancement
        if code.count("vcpu->rip") > 1 or code.count("GUEST_RIP") > 1:
            optimizations.append({
                "type": "single_rip_update",
                "priority": "medium",
                "description": "Update RIP once at handler exit, not multiple times",
                "example": """
// Don't do:
vcpu->rip = __vmx_vmread(GUEST_RIP);
vcpu->rip += instr_len;
__vmx_vmwrite(GUEST_RIP, vcpu->rip);

// Do:
uint64_t new_rip = __vmx_vmread(GUEST_RIP) + instr_len;
__vmx_vmwrite(GUEST_RIP, new_rip);
""",
                "estimated_savings_cycles": 10
            })
        
        return optimizations
    
    def analyze_handler(self, code: str, handler_type: str = "generic") -> dict:
        """Complete timing analysis for a handler."""
        
        counts = self.count_instructions(code)
        estimated = self.estimate_cycles(code, handler_type)
        bottlenecks = self.identify_bottlenecks(code)
        optimizations = self.suggest_optimizations(code, handler_type)
        
        # Build breakdown
        breakdown = {
            "vmread": counts.get("vmread", 0) * 10,
            "vmwrite": counts.get("vmwrite", 0) * 10,
            "msr_access": (counts.get("rdmsr", 0) + counts.get("wrmsr", 0)) * 20,
            "cpuid": counts.get("cpuid", 0) * 30,
            "rdtsc": counts.get("rdtsc", 0) * 10,
            "cr_dr_access": (counts.get("mov_cr", 0) + counts.get("mov_dr", 0)) * 20,
            "memory_access": counts.get("memory_access", 0) * 4,
            "function_calls": max(0, counts.get("function_call", 0) - 1) * 5,
            "conditionals": counts.get("conditional", 0) * 3,
            "other_alu": counts.get("assignments", 0) * 2,
        }
        
        # Calculate totals
        breakdown_total = sum(breakdown.values())
        if breakdown_total < estimated:
            breakdown["baseline_overhead"] = estimated - breakdown_total
        
        # Determine warnings
        warnings = []
        
        if estimated > 150:
            warnings.append(f"CRITICAL: Estimated {estimated} cycles exceeds all anti-cheat thresholds (100-150)")
        elif estimated > 100:
            warnings.append(f"WARNING: Estimated {estimated} cycles may trigger some anti-cheats (threshold: 100)")
        elif estimated > 80:
            warnings.append(f"CAUTION: Estimated {estimated} cycles is close to detection threshold")
        
        for bn in bottlenecks:
            if bn.get("severity") == "critical":
                warnings.append(f"CRITICAL: {bn['type']} - {bn.get('suggestion', '')}")
        
        # Safe determination
        safe = estimated <= 80 and not any(b.get("severity") == "critical" for b in bottlenecks)
        
        return {
            "handler_type": handler_type,
            "estimated_cycles": estimated,
            "detection_threshold": 100,
            "safety_margin": 100 - estimated,
            "safe_for_anticheat": safe,
            "verdict": "SAFE" if safe else ("WARNING" if estimated <= 100 else "UNSAFE"),
            "breakdown": breakdown,
            "instruction_counts": counts,
            "bottlenecks": bottlenecks,
            "optimizations": optimizations,
            "warnings": warnings,
            "full_roundtrip_estimate": {
                "handler_only": estimated,
                "with_context_switch": estimated + VMEXIT_OVERHEAD["context_save"] + VMEXIT_OVERHEAD["context_restore"],
                "note": "Anti-cheats measure instruction-level timing, not full VM-exit roundtrip"
            }
        }
    
    def compare_handlers(self, handlers: Dict[str, str]) -> dict:
        """Compare multiple handler implementations."""
        results = {}
        
        for name, code in handlers.items():
            handler_type = "generic"
            if "cpuid" in name.lower():
                handler_type = "cpuid"
            elif "rdtsc" in name.lower():
                handler_type = "rdtsc"
            elif "msr" in name.lower():
                handler_type = "msr_read"
            
            results[name] = self.analyze_handler(code, handler_type)
        
        # Find fastest/slowest
        sorted_by_cycles = sorted(results.items(), key=lambda x: x[1]["estimated_cycles"])
        
        return {
            "handlers": results,
            "ranking": [name for name, _ in sorted_by_cycles],
            "fastest": sorted_by_cycles[0][0] if sorted_by_cycles else None,
            "slowest": sorted_by_cycles[-1][0] if sorted_by_cycles else None,
            "all_safe": all(r["safe_for_anticheat"] for r in results.values())
        }


# =============================================================================
# MCP TOOL INTERFACE
# =============================================================================

async def simulate_handler_timing(handler_code: str, handler_type: str = "generic") -> dict:
    """
    Simulate and estimate VM-exit handler timing.
    
    Args:
        handler_code: C code for the handler
        handler_type: "cpuid", "rdtsc", "rdtscp", "msr_read", "msr_write", 
                      "cr_access", "ept_violation", "exception", or "generic"
    
    Returns:
        Timing analysis with cycle estimates, bottlenecks, and optimizations
    """
    simulator = TimingSimulator()
    return simulator.analyze_handler(handler_code, handler_type)


async def get_timing_best_practices() -> dict:
    """
    Get best practices for timing-safe handler implementation.
    
    Returns:
        Comprehensive timing guidelines
    """
    return {
        "target_cycles": {
            "cpuid_handler": 50,
            "rdtsc_handler": 20,
            "msr_handler": 30,
            "cr_handler": 35,
            "generic_handler": 60,
            "maximum_safe": 80,
            "detection_threshold": 100
        },
        
        "critical_rules": [
            "NO debug output (DbgPrint, etc.) in release builds",
            "NO loops in hot path handlers",
            "Cache VMCS fields - don't read same field twice",
            "Inline simple handlers (RDTSC, RDTSCP)",
            "Use function pointer dispatch, not giant switch",
            "Track TSC offset for ALL exits, not just timed ones",
        ],
        
        "timing_accumulation": {
            "description": "Track cumulative VM-exit overhead to subtract from RDTSC",
            "implementation": """
typedef struct _TIMING_STATE {
    uint64_t tsc_offset;          // Total accumulated overhead
    uint64_t last_exit_start;     // TSC at exit entry
    uint64_t last_guest_tsc;      // Last TSC returned to guest
} TIMING_STATE;

// At EVERY VM-exit entry:
void on_vmexit_entry(VCPU* vcpu) {
    vcpu->timing.last_exit_start = __rdtsc();
}

// At EVERY VM-exit return (before VMRESUME):
void on_vmexit_return(VCPU* vcpu) {
    uint64_t exit_cost = __rdtsc() - vcpu->timing.last_exit_start;
    vcpu->timing.tsc_offset += exit_cost;
}

// In RDTSC handler:
void handle_rdtsc(VCPU* vcpu) {
    uint64_t real_tsc = __rdtsc();
    uint64_t adjusted = real_tsc - vcpu->timing.tsc_offset;
    
    // Ensure monotonic
    if (adjusted <= vcpu->timing.last_guest_tsc) {
        adjusted = vcpu->timing.last_guest_tsc + 1;
    }
    vcpu->timing.last_guest_tsc = adjusted;
    
    vcpu->gpr.rax = (uint32_t)adjusted;
    vcpu->gpr.rdx = (uint32_t)(adjusted >> 32);
}
"""
        },
        
        "instruction_costs": INSTRUCTION_CYCLES,
        "handler_baselines": HANDLER_BASELINES,
        
        "measurement_methodology": {
            "description": "How to measure your actual handler timing",
            "code": """
// Measure handler timing:
uint64_t measure_handler(HANDLER_FUNC handler, VCPU* test_vcpu) {
    uint64_t total = 0;
    const int iterations = 1000;
    
    for (int i = 0; i < iterations; i++) {
        uint64_t start = __rdtsc();
        _mm_lfence();  // Serialize
        
        handler(test_vcpu);
        
        _mm_lfence();
        uint64_t end = __rdtsc();
        
        total += (end - start);
    }
    
    return total / iterations;
}
"""
        }
    }


async def compare_handler_implementations(implementations: Dict[str, str]) -> dict:
    """
    Compare multiple handler implementations for timing.
    
    Args:
        implementations: Dict of {"name": "code", ...}
    
    Returns:
        Comparative analysis
    """
    simulator = TimingSimulator()
    return simulator.compare_handlers(implementations)


# Test if run directly
if __name__ == "__main__":
    import json
    
    test_code = """
void handle_cpuid(VCPU* vcpu) {
    int regs[4];
    uint32_t leaf = (uint32_t)vcpu->gpr.rax;
    uint32_t subleaf = (uint32_t)vcpu->gpr.rcx;
    
    __cpuidex(regs, leaf, subleaf);
    
    if (leaf == 1) {
        regs[2] &= ~(1 << 31);  // Clear hypervisor present
        regs[2] &= ~(1 << 5);   // Clear VMX
    }
    
    if ((leaf >> 28) == 4) {
        regs[0] = regs[1] = regs[2] = regs[3] = 0;
    }
    
    vcpu->gpr.rax = regs[0];
    vcpu->gpr.rbx = regs[1];
    vcpu->gpr.rcx = regs[2];
    vcpu->gpr.rdx = regs[3];
    
    vcpu->rip += __vmx_vmread(VM_EXIT_INSTRUCTION_LEN);
}
"""
    
    simulator = TimingSimulator()
    result = simulator.analyze_handler(test_code, "cpuid")
    print(json.dumps(result, indent=2))
