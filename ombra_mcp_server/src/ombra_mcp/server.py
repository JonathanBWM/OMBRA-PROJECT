#!/usr/bin/env python3
"""
OmbraMCP — Hypervisor Development MCP Server

Provides Claude Code with instant access to:
- Intel SDM specifications (VMCS, MSRs, Exit Reasons)
- Code generation (headers, skeletons, templates)
- Build system integration
- Detection testing
- Project memory
- Binary signature scanning
- VMCS validation
- Anti-cheat intelligence
- Timing simulation
"""

import asyncio
import json
import sqlite3
from pathlib import Path
from typing import Any, Dict

from mcp.server import Server, InitializationOptions
from mcp.server.stdio import stdio_server
from mcp.types import Tool, TextContent, ServerCapabilities

# Import advanced tools
from .tools import (
    scan_binary_signatures,
    scan_source_for_signatures,
    validate_vmcs_setup,
    get_vmcs_checklist,
    get_anticheat_intel,
    get_timing_requirements,
    get_bypass_for_detection,
    check_evasion_coverage,
    simulate_handler_timing,
    get_timing_best_practices,
    compare_handler_implementations,
    # SDM Query tools
    ask_sdm,
    vmcs_field_complete,
    exit_reason_complete,
    get_msr_info,
    get_exception_info,
    list_vmcs_by_category,
    get_vmx_control_bits,
    # Code Generator tools
    generate_vmcs_setup,
    generate_exit_handler,
    generate_ept_setup,
    generate_msr_bitmap_setup,
    # Stealth tools
    get_detection_vectors,
    audit_stealth,
    generate_timing_compensation,
    generate_cpuid_spoofing,
    # BYOVD tools
    ld9boxsup_ioctl_guide,
    generate_driver_wrapper,
    generate_hypervisor_loader,
    # Project Brain tools
    get_project_status,
    get_findings,
    dismiss_finding,
    get_suggestions,
    get_component,
    get_exit_handler_status,
    add_decision,
    get_decision,
    list_decisions,
    add_gotcha,
    search_gotchas,
    get_session_context,
    save_session_context,
    refresh_analysis,
    get_daemon_status,
)

# Server instance
app = Server("ombra-mcp")

# Database path
DATA_DIR = Path(__file__).parent / "data"
SDM_DB = DATA_DIR / "intel_sdm.db"
PROJECT_DB = DATA_DIR / "project_memory.db"

# ============================================================================
# DATABASE HELPERS
# ============================================================================

def get_sdm_db():
    """Get connection to Intel SDM database."""
    return sqlite3.connect(SDM_DB)

def get_project_db():
    """Get connection to project memory database."""
    conn = sqlite3.connect(PROJECT_DB)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS implementation_status (
            file TEXT PRIMARY KEY,
            status TEXT,
            notes TEXT,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    """)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS implementation_notes (
            id INTEGER PRIMARY KEY,
            component TEXT,
            note TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    """)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS known_issues (
            id INTEGER PRIMARY KEY,
            component TEXT,
            issue TEXT,
            status TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    """)
    conn.commit()
    return conn

# ============================================================================
# INTEL SDM TOOLS
# ============================================================================

async def vmcs_field(field_name: str) -> dict:
    """Get complete VMCS field specification."""
    conn = get_sdm_db()
    cursor = conn.execute(
        "SELECT * FROM vmcs_fields WHERE name LIKE ? OR name = ?",
        (f"%{field_name}%", field_name)
    )
    row = cursor.fetchone()
    conn.close()
    
    if not row:
        return {"error": f"Field '{field_name}' not found"}
    
    return {
        "name": row[1],
        "encoding": f"0x{row[2]:04X}",
        "width": row[3],
        "category": row[4],
        "description": row[5],
        "c_define": f"#define VMCS_{row[1]} 0x{row[2]:04X}"
    }

async def vmcs_fields_by_category(category: str) -> list:
    """Get all VMCS fields in a category."""
    conn = get_sdm_db()
    
    if category == "all":
        cursor = conn.execute("SELECT * FROM vmcs_fields ORDER BY encoding")
    else:
        cursor = conn.execute(
            "SELECT * FROM vmcs_fields WHERE category = ? ORDER BY encoding",
            (category,)
        )
    
    rows = cursor.fetchall()
    conn.close()
    
    return [
        {
            "name": row[1],
            "encoding": f"0x{row[2]:04X}",
            "width": row[3],
            "category": row[4],
            "description": row[5]
        }
        for row in rows
    ]

async def exit_reason(reason: int) -> dict:
    """Get complete exit reason specification."""
    conn = get_sdm_db()
    cursor = conn.execute(
        "SELECT * FROM exit_reasons WHERE reason_number = ?",
        (reason,)
    )
    row = cursor.fetchone()
    conn.close()
    
    if not row:
        return {"error": f"Exit reason {reason} not found"}
    
    return {
        "number": row[1],
        "name": row[2],
        "has_qualification": bool(row[3]),
        "qualification_format": json.loads(row[4]) if row[4] else None,
        "handling_notes": row[5],
        "sdm_section": row[6]
    }

async def exit_qualification_format(reason: int) -> dict:
    """Get exit qualification bit layout for specific exit reason."""
    result = await exit_reason(reason)
    if "error" in result:
        return result
    return {
        "exit_reason": result["number"],
        "name": result["name"],
        "qualification_bits": result["qualification_format"]
    }

async def msr_info(msr: str) -> dict:
    """Get MSR specification by name or number."""
    conn = get_sdm_db()
    
    # Try as number first
    try:
        if msr.startswith("0x"):
            msr_num = int(msr, 16)
        else:
            msr_num = int(msr)
        cursor = conn.execute(
            "SELECT * FROM msrs WHERE msr_number = ?",
            (msr_num,)
        )
    except ValueError:
        # Try as name
        cursor = conn.execute(
            "SELECT * FROM msrs WHERE name LIKE ?",
            (f"%{msr}%",)
        )
    
    row = cursor.fetchone()
    conn.close()
    
    if not row:
        return {"error": f"MSR '{msr}' not found"}
    
    return {
        "number": f"0x{row[1]:X}",
        "name": row[2],
        "bit_fields": json.loads(row[3]) if row[3] else None,
        "read_behavior": row[4],
        "write_behavior": row[5],
        "vmx_related": bool(row[6]),
        "c_define": f"#define {row[2]} 0x{row[1]:X}"
    }

async def search_sdm(query: str, max_results: int = 3) -> list:
    """Semantic search across Intel SDM."""
    conn = get_sdm_db()
    
    # Simple FTS search (semantic search would require embeddings)
    cursor = conn.execute(
        "SELECT chapter, section, page, content FROM sdm_fts WHERE content MATCH ? LIMIT ?",
        (query, max_results)
    )
    
    rows = cursor.fetchall()
    conn.close()
    
    return [
        {
            "chapter": row[0],
            "section": row[1],
            "page": row[2],
            "excerpt": row[3][:500] + "..." if len(row[3]) > 500 else row[3]
        }
        for row in rows
    ]

# ============================================================================
# CODE GENERATION TOOLS
# ============================================================================

async def generate_vmcs_header() -> str:
    """Generate complete vmcs_fields.h with all field encodings."""
    
    fields = await vmcs_fields_by_category("all")
    
    output = """// vmcs_fields.h — Auto-generated VMCS Field Encodings
// Source: Intel SDM Vol 3C, Chapter 24
// DO NOT EDIT MANUALLY — Regenerate with: generate_vmcs_header()

#ifndef VMCS_FIELDS_H
#define VMCS_FIELDS_H

#include <stdint.h>

// =============================================================================
// VMCS Field Encoding Format (Intel SDM Vol 3C, Section 24.11.2)
// =============================================================================
// Bits 0:0   - Access type (0 = full, 1 = high)
// Bits 9:1   - Index
// Bits 11:10 - Type (0 = control, 1 = exit info, 2 = guest state, 3 = host state)
// Bits 14:13 - Width (0 = 16-bit, 1 = 64-bit, 2 = 32-bit, 3 = natural)
// =============================================================================

"""
    
    # Group by category
    categories = {}
    for field in fields:
        cat = field["category"]
        if cat not in categories:
            categories[cat] = []
        categories[cat].append(field)
    
    category_names = {
        "control": "VM-Execution Control Fields",
        "exit_info": "VM-Exit Information Fields", 
        "guest_state": "Guest-State Fields",
        "host_state": "Host-State Fields"
    }
    
    for cat, cat_name in category_names.items():
        if cat in categories:
            output += f"// -----------------------------------------------------------------------------\n"
            output += f"// {cat_name}\n"
            output += f"// -----------------------------------------------------------------------------\n\n"
            
            for field in categories[cat]:
                output += f"#define VMCS_{field['name']:<40} {field['encoding']}\n"
            
            output += "\n"
    
    output += "#endif // VMCS_FIELDS_H\n"
    return output

async def generate_exit_handler_skeleton() -> str:
    """Generate exit_dispatch.c skeleton with all exit reasons."""
    
    conn = get_sdm_db()
    cursor = conn.execute("SELECT reason_number, name FROM exit_reasons ORDER BY reason_number")
    exits = cursor.fetchall()
    conn.close()
    
    output = """// exit_dispatch.c — VM-Exit Handler Dispatch
// Auto-generated — DO NOT EDIT MANUALLY

#include "exit_dispatch.h"
#include "vmcs_fields.h"
#include "handlers/handlers.h"

void exit_dispatch(VCPU* vcpu) {
    uint64_t exit_reason_full = __vmx_vmread(VMCS_VM_EXIT_REASON);
    uint32_t exit_reason = (uint32_t)(exit_reason_full & 0xFFFF);
    uint32_t exit_failed = (uint32_t)((exit_reason_full >> 31) & 1);
    
    // Check for VM-entry failure
    if (exit_failed) {
        handle_vmentry_failure(vcpu, exit_reason);
        return;
    }
    
    // Record exit for timing compensation
    vcpu->timing.last_exit_tsc = __rdtsc();
    
    switch (exit_reason) {
"""
    
    for reason_num, name in exits:
        handler_name = f"handle_{name.lower()}"
        output += f"        case {reason_num}: // {name}\n"
        output += f"            {handler_name}(vcpu);\n"
        output += f"            break;\n\n"
    
    output += """        default:
            // Unknown exit reason — inject #UD
            handle_unknown_exit(vcpu, exit_reason);
            break;
    }
    
    // Update timing compensation
    uint64_t exit_end_tsc = __rdtsc();
    vcpu->timing.tsc_offset += (exit_end_tsc - vcpu->timing.last_exit_tsc);
}
"""
    
    return output

async def generate_msr_header() -> str:
    """Generate msr_defs.h with all relevant MSR numbers."""
    
    conn = get_sdm_db()
    cursor = conn.execute("SELECT msr_number, name, vmx_related FROM msrs ORDER BY msr_number")
    msrs = cursor.fetchall()
    conn.close()
    
    output = """// msr_defs.h — MSR Definitions
// Source: Intel SDM Vol 4
// DO NOT EDIT MANUALLY

#ifndef MSR_DEFS_H
#define MSR_DEFS_H

// =============================================================================
// VMX-Related MSRs (Intel SDM Vol 3C, Appendix A)
// =============================================================================

"""
    
    vmx_msrs = [m for m in msrs if m[2]]
    other_msrs = [m for m in msrs if not m[2]]
    
    for msr_num, name, _ in vmx_msrs:
        output += f"#define {name:<45} 0x{msr_num:X}\n"
    
    output += """
// =============================================================================
// Other Relevant MSRs
// =============================================================================

"""
    
    for msr_num, name, _ in other_msrs:
        output += f"#define {name:<45} 0x{msr_num:X}\n"
    
    output += "\n#endif // MSR_DEFS_H\n"
    return output

async def generate_handler_template(exit_reason_num: int) -> str:
    """Generate handler function template for specific exit reason."""
    
    exit_info = await exit_reason(exit_reason_num)
    if "error" in exit_info:
        return f"// Error: {exit_info['error']}"
    
    name = exit_info["name"]
    handler_name = f"handle_{name.lower()}"
    
    qual_comment = ""
    if exit_info["has_qualification"] and exit_info["qualification_format"]:
        qual_comment = "    // Exit Qualification Bits:\n"
        for bit_name, bit_info in exit_info["qualification_format"].items():
            qual_comment += f"    // - {bit_name}: {bit_info}\n"
    
    template = f"""// {handler_name}.c — Handler for {name} (Exit Reason {exit_reason_num})
// SDM Reference: {exit_info.get('sdm_section', 'Vol 3C, Chapter 25')}

#include "handlers.h"
#include "../vmcs_fields.h"
#include "../timing.h"

void {handler_name}(VCPU* vcpu) {{
{qual_comment}
    uint64_t qualification = __vmx_vmread(VMCS_EXIT_QUALIFICATION);
    uint64_t guest_rip = __vmx_vmread(VMCS_GUEST_RIP);
    uint32_t instruction_length = (uint32_t)__vmx_vmread(VMCS_VM_EXIT_INSTRUCTION_LENGTH);
    
    // {exit_info.get('handling_notes', 'TODO: Implement handling logic')}
    
    // TODO: Implement handler logic here
    
    // Advance guest RIP past the instruction (if applicable)
    __vmx_vmwrite(VMCS_GUEST_RIP, guest_rip + instruction_length);
}}
"""
    
    return template

async def generate_ept_structures() -> str:
    """Generate EPT structure definitions header."""
    
    return """// ept_defs.h — EPT Structure Definitions
// Source: Intel SDM Vol 3C, Section 28.2

#ifndef EPT_DEFS_H
#define EPT_DEFS_H

#include <stdint.h>

// =============================================================================
// EPT Pointer (EPTP) — Intel SDM Vol 3C, Section 24.6.11
// =============================================================================

typedef union _EPTP {
    uint64_t value;
    struct {
        uint64_t memory_type : 3;        // 0 = UC, 6 = WB
        uint64_t page_walk_length : 3;   // Must be 3 (4-level paging - 1)
        uint64_t enable_dirty_flag : 1;  // Enable accessed/dirty flags
        uint64_t enable_sss : 1;         // Enable supervisor shadow stack
        uint64_t reserved1 : 4;
        uint64_t pml4_pfn : 40;          // Physical frame number of PML4
        uint64_t reserved2 : 12;
    };
} EPTP, *PEPTP;

// =============================================================================
// EPT PML4 Entry (PML4E) — Intel SDM Vol 3C, Table 28-1
// =============================================================================

typedef union _EPT_PML4E {
    uint64_t value;
    struct {
        uint64_t read : 1;               // Read access
        uint64_t write : 1;              // Write access
        uint64_t execute : 1;            // Execute access (supervisor)
        uint64_t reserved1 : 5;
        uint64_t accessed : 1;           // Accessed flag
        uint64_t ignored1 : 1;
        uint64_t user_execute : 1;       // Execute access (user)
        uint64_t ignored2 : 1;
        uint64_t pdpt_pfn : 40;          // Physical frame number of PDPT
        uint64_t ignored3 : 12;
    };
} EPT_PML4E, *PEPT_PML4E;

// =============================================================================
// EPT Page Directory Pointer Table Entry (PDPTE)
// =============================================================================

typedef union _EPT_PDPTE {
    uint64_t value;
    struct {
        uint64_t read : 1;
        uint64_t write : 1;
        uint64_t execute : 1;
        uint64_t reserved1 : 4;
        uint64_t large_page : 1;         // 1 = 1GB page
        uint64_t accessed : 1;
        uint64_t ignored1 : 1;
        uint64_t user_execute : 1;
        uint64_t ignored2 : 1;
        uint64_t pd_pfn : 40;            // PFN of PD (or 1GB page if large)
        uint64_t ignored3 : 12;
    };
} EPT_PDPTE, *PEPT_PDPTE;

// =============================================================================
// EPT Page Directory Entry (PDE)
// =============================================================================

typedef union _EPT_PDE {
    uint64_t value;
    struct {
        uint64_t read : 1;
        uint64_t write : 1;
        uint64_t execute : 1;
        uint64_t memory_type : 3;        // Only valid for large pages
        uint64_t ignore_pat : 1;         // Only valid for large pages
        uint64_t large_page : 1;         // 1 = 2MB page
        uint64_t accessed : 1;
        uint64_t dirty : 1;              // Only valid for large pages
        uint64_t user_execute : 1;
        uint64_t ignored1 : 1;
        uint64_t pt_pfn : 40;            // PFN of PT (or 2MB page if large)
        uint64_t ignored2 : 11;
        uint64_t suppress_ve : 1;        // Suppress #VE
    };
} EPT_PDE, *PEPT_PDE;

// =============================================================================
// EPT Page Table Entry (PTE)
// =============================================================================

typedef union _EPT_PTE {
    uint64_t value;
    struct {
        uint64_t read : 1;
        uint64_t write : 1;
        uint64_t execute : 1;
        uint64_t memory_type : 3;
        uint64_t ignore_pat : 1;
        uint64_t ignored1 : 1;
        uint64_t accessed : 1;
        uint64_t dirty : 1;
        uint64_t user_execute : 1;
        uint64_t ignored2 : 1;
        uint64_t page_pfn : 40;          // Physical frame number
        uint64_t ignored3 : 11;
        uint64_t suppress_ve : 1;
    };
} EPT_PTE, *PEPT_PTE;

// =============================================================================
// EPT Violation Exit Qualification — Intel SDM Vol 3C, Table 27-7
// =============================================================================

typedef union _EPT_VIOLATION_QUALIFICATION {
    uint64_t value;
    struct {
        uint64_t read_access : 1;        // Caused by read
        uint64_t write_access : 1;       // Caused by write
        uint64_t execute_access : 1;     // Caused by instruction fetch
        uint64_t ept_readable : 1;       // EPT entry allows read
        uint64_t ept_writable : 1;       // EPT entry allows write
        uint64_t ept_executable : 1;     // EPT entry allows execute
        uint64_t ept_user_exec : 1;      // EPT entry allows user execute
        uint64_t guest_linear_valid : 1; // Guest linear address valid
        uint64_t caused_by_translation : 1; // Caused by page walk
        uint64_t user_mode : 1;          // User mode access
        uint64_t read_write_page : 1;    // R/W page
        uint64_t execute_disable : 1;    // XD/NX bit
        uint64_t nmi_unblocking : 1;     // NMI unblocking due to IRET
        uint64_t shadow_stack : 1;       // Shadow stack access
        uint64_t supervisor_shadow : 1;  // Supervisor shadow stack
        uint64_t reserved : 49;
    };
} EPT_VIOLATION_QUALIFICATION, *PEPT_VIOLATION_QUALIFICATION;

// =============================================================================
// Memory Types
// =============================================================================

#define EPT_MEMORY_TYPE_UC  0  // Uncacheable
#define EPT_MEMORY_TYPE_WC  1  // Write Combining
#define EPT_MEMORY_TYPE_WT  4  // Write Through
#define EPT_MEMORY_TYPE_WP  5  // Write Protected
#define EPT_MEMORY_TYPE_WB  6  // Write Back

// =============================================================================
// Helper Macros
// =============================================================================

#define EPT_PML4_INDEX(addr)  (((addr) >> 39) & 0x1FF)
#define EPT_PDPT_INDEX(addr)  (((addr) >> 30) & 0x1FF)
#define EPT_PD_INDEX(addr)    (((addr) >> 21) & 0x1FF)
#define EPT_PT_INDEX(addr)    (((addr) >> 12) & 0x1FF)

#define PAGE_SIZE_4KB   0x1000
#define PAGE_SIZE_2MB   0x200000
#define PAGE_SIZE_1GB   0x40000000

#endif // EPT_DEFS_H
"""

async def generate_asm_stub(stub_type: str) -> str:
    """Generate assembly stub."""
    
    stubs = {
        "vmexit": """; vmexit.asm — VM-Exit Handler Entry Point
; MASM x64 syntax

.code

EXTERN exit_dispatch:PROC

; =============================================================================
; Guest Register Context Structure (must match C definition)
; =============================================================================
; Offset  Register
; 0x00    RAX
; 0x08    RCX
; 0x10    RDX
; 0x18    RBX
; 0x20    (RSP placeholder)
; 0x28    RBP
; 0x30    RSI
; 0x38    RDI
; 0x40    R8
; 0x48    R9
; 0x50    R10
; 0x58    R11
; 0x60    R12
; 0x68    R13
; 0x70    R14
; 0x78    R15
; =============================================================================

VmExitHandler PROC
    ; Save all general-purpose registers
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rdi
    push rsi
    push rbp
    sub rsp, 8          ; Placeholder for RSP
    push rbx
    push rdx
    push rcx
    push rax
    
    ; Save XMM registers (optional, but safer)
    sub rsp, 100h
    movaps [rsp + 00h], xmm0
    movaps [rsp + 10h], xmm1
    movaps [rsp + 20h], xmm2
    movaps [rsp + 30h], xmm3
    movaps [rsp + 40h], xmm4
    movaps [rsp + 50h], xmm5
    movaps [rsp + 60h], xmm6
    movaps [rsp + 70h], xmm7
    movaps [rsp + 80h], xmm8
    movaps [rsp + 90h], xmm9
    movaps [rsp + 0A0h], xmm10
    movaps [rsp + 0B0h], xmm11
    movaps [rsp + 0C0h], xmm12
    movaps [rsp + 0D0h], xmm13
    movaps [rsp + 0E0h], xmm14
    movaps [rsp + 0F0h], xmm15
    
    ; First parameter (RCX) = pointer to VCPU context
    mov rcx, rsp
    
    ; Allocate shadow space for x64 calling convention
    sub rsp, 28h
    
    ; Call C handler
    call exit_dispatch
    
    ; Remove shadow space
    add rsp, 28h
    
    ; Restore XMM registers
    movaps xmm0, [rsp + 00h]
    movaps xmm1, [rsp + 10h]
    movaps xmm2, [rsp + 20h]
    movaps xmm3, [rsp + 30h]
    movaps xmm4, [rsp + 40h]
    movaps xmm5, [rsp + 50h]
    movaps xmm6, [rsp + 60h]
    movaps xmm7, [rsp + 70h]
    movaps xmm8, [rsp + 80h]
    movaps xmm9, [rsp + 90h]
    movaps xmm10, [rsp + 0A0h]
    movaps xmm11, [rsp + 0B0h]
    movaps xmm12, [rsp + 0C0h]
    movaps xmm13, [rsp + 0D0h]
    movaps xmm14, [rsp + 0E0h]
    movaps xmm15, [rsp + 0F0h]
    add rsp, 100h
    
    ; Restore GPRs
    pop rax
    pop rcx
    pop rdx
    pop rbx
    add rsp, 8          ; Skip RSP placeholder
    pop rbp
    pop rsi
    pop rdi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    
    ; Resume guest execution
    vmresume
    
    ; If we reach here, VMRESUME failed
    jmp VmResumeFailed
    
VmExitHandler ENDP

VmResumeFailed PROC
    ; VMRESUME failed — this is fatal
    ; Read error info from VMCS for debugging
    int 3
    hlt
    jmp VmResumeFailed
VmResumeFailed ENDP

END
""",
        
        "intrinsics": """; intrinsics.asm — VMX Intrinsics
; MASM x64 syntax

.code

; =============================================================================
; uint64_t __vmx_vmread(uint64_t field)
; =============================================================================
__vmx_vmread PROC
    vmread rax, rcx
    ret
__vmx_vmread ENDP

; =============================================================================
; void __vmx_vmwrite(uint64_t field, uint64_t value)
; =============================================================================
__vmx_vmwrite PROC
    vmwrite rcx, rdx
    ret
__vmx_vmwrite ENDP

; =============================================================================
; uint8_t __vmx_vmxon(uint64_t* vmxon_region_pa)
; Returns: 0 = success, 1 = failure
; =============================================================================
__vmx_vmxon PROC
    vmxon qword ptr [rcx]
    jc vmxon_fail
    jz vmxon_fail
    xor eax, eax
    ret
vmxon_fail:
    mov eax, 1
    ret
__vmx_vmxon ENDP

; =============================================================================
; void __vmx_vmxoff(void)
; =============================================================================
__vmx_vmxoff PROC
    vmxoff
    ret
__vmx_vmxoff ENDP

; =============================================================================
; uint8_t __vmx_vmclear(uint64_t* vmcs_pa)
; =============================================================================
__vmx_vmclear PROC
    vmclear qword ptr [rcx]
    jc vmclear_fail
    jz vmclear_fail
    xor eax, eax
    ret
vmclear_fail:
    mov eax, 1
    ret
__vmx_vmclear ENDP

; =============================================================================
; uint8_t __vmx_vmptrld(uint64_t* vmcs_pa)
; =============================================================================
__vmx_vmptrld PROC
    vmptrld qword ptr [rcx]
    jc vmptrld_fail
    jz vmptrld_fail
    xor eax, eax
    ret
vmptrld_fail:
    mov eax, 1
    ret
__vmx_vmptrld ENDP

; =============================================================================
; uint8_t __vmx_vmlaunch(void)
; =============================================================================
__vmx_vmlaunch PROC
    vmlaunch
    ; If we reach here, vmlaunch failed
    mov eax, 1
    ret
__vmx_vmlaunch ENDP

; =============================================================================
; uint8_t __vmx_vmresume(void)
; =============================================================================
__vmx_vmresume PROC
    vmresume
    ; If we reach here, vmresume failed
    mov eax, 1
    ret
__vmx_vmresume ENDP

; =============================================================================
; void __invept(uint64_t type, void* descriptor)
; =============================================================================
__invept PROC
    invept rcx, oword ptr [rdx]
    ret
__invept ENDP

; =============================================================================
; void __invvpid(uint64_t type, void* descriptor)
; =============================================================================
__invvpid PROC
    invvpid rcx, oword ptr [rdx]
    ret
__invvpid ENDP

; =============================================================================
; uint64_t __readcr0(void)
; =============================================================================
__readcr0 PROC
    mov rax, cr0
    ret
__readcr0 ENDP

; =============================================================================
; uint64_t __readcr3(void)
; =============================================================================
__readcr3 PROC
    mov rax, cr3
    ret
__readcr3 ENDP

; =============================================================================
; uint64_t __readcr4(void)
; =============================================================================
__readcr4 PROC
    mov rax, cr4
    ret
__readcr4 ENDP

; =============================================================================
; void __writecr0(uint64_t value)
; =============================================================================
__writecr0 PROC
    mov cr0, rcx
    ret
__writecr0 ENDP

; =============================================================================
; void __writecr4(uint64_t value)
; =============================================================================
__writecr4 PROC
    mov cr4, rcx
    ret
__writecr4 ENDP

; =============================================================================
; uint64_t __rdmsr(uint32_t msr)
; =============================================================================
__rdmsr PROC
    rdmsr
    shl rdx, 32
    or rax, rdx
    ret
__rdmsr ENDP

; =============================================================================
; void __wrmsr(uint32_t msr, uint64_t value)
; =============================================================================
__wrmsr PROC
    mov eax, edx
    shr rdx, 32
    wrmsr
    ret
__wrmsr ENDP

; =============================================================================
; uint64_t __rdtsc(void)
; =============================================================================
__rdtsc PROC
    rdtsc
    shl rdx, 32
    or rax, rdx
    ret
__rdtsc ENDP

; =============================================================================
; uint16_t __read_cs(void), etc.
; =============================================================================
__read_cs PROC
    mov ax, cs
    ret
__read_cs ENDP

__read_ds PROC
    mov ax, ds
    ret
__read_ds ENDP

__read_es PROC
    mov ax, es
    ret
__read_es ENDP

__read_fs PROC
    mov ax, fs
    ret
__read_fs ENDP

__read_gs PROC
    mov ax, gs
    ret
__read_gs ENDP

__read_ss PROC
    mov ax, ss
    ret
__read_ss ENDP

__read_tr PROC
    str ax
    ret
__read_tr ENDP

__read_ldtr PROC
    sldt ax
    ret
__read_ldtr ENDP

; =============================================================================
; void __sgdt(void* gdtr)
; =============================================================================
__sgdt PROC
    sgdt [rcx]
    ret
__sgdt ENDP

; =============================================================================
; void __sidt(void* idtr)
; =============================================================================
__sidt PROC
    sidt [rcx]
    ret
__sidt ENDP

END
"""
    }
    
    if stub_type not in stubs:
        return f"; Error: Unknown stub type '{stub_type}'. Valid types: {list(stubs.keys())}"
    
    return stubs[stub_type]

# ============================================================================
# PROJECT MEMORY TOOLS  
# ============================================================================

async def get_implementation_status() -> dict:
    """Get current implementation status of all components."""
    conn = get_project_db()
    
    cursor = conn.execute("SELECT file, status, notes FROM implementation_status")
    files = {row[0]: {"status": row[1], "notes": row[2]} for row in cursor.fetchall()}
    
    cursor = conn.execute("SELECT component, issue, status FROM known_issues WHERE status != 'resolved'")
    issues = [{"component": row[0], "issue": row[1], "status": row[2]} for row in cursor.fetchall()]
    
    conn.close()
    
    completed = [f for f, s in files.items() if s["status"] == "completed"]
    in_progress = [f for f, s in files.items() if s["status"] == "in_progress"]
    blocked = [f for f, s in files.items() if s["status"] == "blocked"]
    
    return {
        "completed": completed,
        "in_progress": in_progress,
        "blocked": blocked,
        "issues": issues,
        "total_files": len(files),
        "completion_percentage": len(completed) / max(len(files), 1) * 100
    }

async def update_status(file: str, status: str, notes: str = None) -> dict:
    """Update implementation status for a file."""
    conn = get_project_db()
    conn.execute(
        "INSERT OR REPLACE INTO implementation_status (file, status, notes) VALUES (?, ?, ?)",
        (file, status, notes)
    )
    conn.commit()
    conn.close()
    return {"success": True, "file": file, "status": status}

async def add_implementation_note(component: str, note: str) -> dict:
    """Add a note about implementation decisions or issues."""
    conn = get_project_db()
    conn.execute(
        "INSERT INTO implementation_notes (component, note) VALUES (?, ?)",
        (component, note)
    )
    conn.commit()
    conn.close()
    return {"success": True, "component": component}

async def get_known_issues() -> list:
    """Get list of known issues and blockers."""
    conn = get_project_db()
    cursor = conn.execute("SELECT component, issue, status FROM known_issues")
    issues = [{"component": row[0], "issue": row[1], "status": row[2]} for row in cursor.fetchall()]
    conn.close()
    return issues

# ============================================================================
# TOOL REGISTRATION
# ============================================================================

TOOLS = [
    # Intel SDM
    Tool(name="vmcs_field", description="Get VMCS field specification by name", inputSchema={
        "type": "object",
        "properties": {"field_name": {"type": "string", "description": "Field name (e.g., GUEST_RIP)"}},
        "required": ["field_name"]
    }),
    Tool(name="vmcs_fields_by_category", description="Get all VMCS fields in a category", inputSchema={
        "type": "object",
        "properties": {"category": {"type": "string", "enum": ["guest_state", "host_state", "control", "exit_info", "all"]}},
        "required": ["category"]
    }),
    Tool(name="exit_reason", description="Get exit reason specification", inputSchema={
        "type": "object",
        "properties": {"reason": {"type": "integer", "description": "Exit reason number (0-65)"}},
        "required": ["reason"]
    }),
    Tool(name="exit_qualification_format", description="Get exit qualification bit layout", inputSchema={
        "type": "object",
        "properties": {"reason": {"type": "integer"}},
        "required": ["reason"]
    }),
    Tool(name="msr_info", description="Get MSR specification", inputSchema={
        "type": "object",
        "properties": {"msr": {"type": "string", "description": "MSR name or hex number"}},
        "required": ["msr"]
    }),
    Tool(name="search_sdm", description="Search Intel SDM", inputSchema={
        "type": "object",
        "properties": {
            "query": {"type": "string"},
            "max_results": {"type": "integer", "default": 3}
        },
        "required": ["query"]
    }),
    
    # Code Generation
    Tool(name="generate_vmcs_header", description="Generate vmcs_fields.h", inputSchema={"type": "object", "properties": {}}),
    Tool(name="generate_exit_handler_skeleton", description="Generate exit handler skeleton", inputSchema={"type": "object", "properties": {}}),
    Tool(name="generate_msr_header", description="Generate msr_defs.h", inputSchema={"type": "object", "properties": {}}),
    Tool(name="generate_handler_template", description="Generate handler for exit reason", inputSchema={
        "type": "object",
        "properties": {"exit_reason_num": {"type": "integer"}},
        "required": ["exit_reason_num"]
    }),
    Tool(name="generate_ept_structures", description="Generate EPT structure definitions", inputSchema={"type": "object", "properties": {}}),
    Tool(name="generate_asm_stub", description="Generate assembly stub", inputSchema={
        "type": "object",
        "properties": {"stub_type": {"type": "string", "enum": ["vmexit", "intrinsics"]}},
        "required": ["stub_type"]
    }),
    
    # Project Memory
    Tool(name="get_implementation_status", description="Get project status", inputSchema={"type": "object", "properties": {}}),
    Tool(name="update_status", description="Update file status", inputSchema={
        "type": "object",
        "properties": {
            "file": {"type": "string"},
            "status": {"type": "string", "enum": ["completed", "in_progress", "blocked", "not_started"]},
            "notes": {"type": "string"}
        },
        "required": ["file", "status"]
    }),
    Tool(name="add_implementation_note", description="Add implementation note", inputSchema={
        "type": "object",
        "properties": {"component": {"type": "string"}, "note": {"type": "string"}},
        "required": ["component", "note"]
    }),
    Tool(name="get_known_issues", description="Get known issues", inputSchema={"type": "object", "properties": {}}),
    
    # Binary Signature Scanner
    Tool(name="scan_binary_signatures", description="Scan binary for hypervisor signatures", inputSchema={
        "type": "object",
        "properties": {"binary_path": {"type": "string", "description": "Path to .sys/.exe/.dll file"}},
        "required": ["binary_path"]
    }),
    Tool(name="scan_source_for_signatures", description="Scan source code for signature patterns", inputSchema={
        "type": "object",
        "properties": {"source_code": {"type": "string", "description": "C source code to scan"}},
        "required": ["source_code"]
    }),
    
    # VMCS Validator
    Tool(name="validate_vmcs_setup", description="Validate VMCS configuration code", inputSchema={
        "type": "object",
        "properties": {"vmcs_code": {"type": "string", "description": "C code with VMCS setup (vmwrite calls)"}},
        "required": ["vmcs_code"]
    }),
    Tool(name="get_vmcs_checklist", description="Get VMCS setup requirements checklist", inputSchema={
        "type": "object", "properties": {}
    }),
    
    # Anti-Cheat Intelligence
    Tool(name="get_anticheat_intel", description="Get anti-cheat detection methods and bypasses", inputSchema={
        "type": "object",
        "properties": {"anticheat": {"type": "string", "description": "EAC, BattlEye, Vanguard, FACEIT, ESEA, or 'all'"}},
        "required": ["anticheat"]
    }),
    Tool(name="get_timing_requirements", description="Get timing requirements for anti-cheat evasion", inputSchema={
        "type": "object", "properties": {}
    }),
    Tool(name="get_bypass_for_detection", description="Get specific bypass for detection method", inputSchema={
        "type": "object",
        "properties": {"detection_id": {"type": "string", "description": "Detection ID like EAC-001, BE-002"}},
        "required": ["detection_id"]
    }),
    Tool(name="check_evasion_coverage", description="Check which detections your bypasses cover", inputSchema={
        "type": "object",
        "properties": {"implemented_bypasses": {"type": "array", "items": {"type": "string"}, "description": "List of implemented bypass techniques"}},
        "required": ["implemented_bypasses"]
    }),
    
    # Timing Simulator
    Tool(name="simulate_handler_timing", description="Simulate VM-exit handler timing", inputSchema={
        "type": "object",
        "properties": {
            "handler_code": {"type": "string", "description": "C code for the handler"},
            "handler_type": {"type": "string", "enum": ["cpuid", "rdtsc", "rdtscp", "msr_read", "msr_write", "cr_access", "ept_violation", "exception", "generic"]}
        },
        "required": ["handler_code"]
    }),
    Tool(name="get_timing_best_practices", description="Get timing optimization best practices", inputSchema={
        "type": "object", "properties": {}
    }),
    Tool(name="compare_handler_implementations", description="Compare multiple handler implementations", inputSchema={
        "type": "object",
        "properties": {"implementations": {"type": "object", "description": "Dict of {name: code}"}},
        "required": ["implementations"]
    }),

    # SDM Query Tools
    Tool(name="ask_sdm", description="Natural language search of Intel SDM", inputSchema={
        "type": "object",
        "properties": {"question": {"type": "string", "description": "Question about VMX/hypervisor development"}},
        "required": ["question"]
    }),
    Tool(name="vmcs_field_complete", description="Get complete VMCS field info", inputSchema={
        "type": "object",
        "properties": {"field_name": {"type": "string", "description": "VMCS field name or partial match"}},
        "required": ["field_name"]
    }),
    Tool(name="exit_reason_complete", description="Get complete VM-exit reason info", inputSchema={
        "type": "object",
        "properties": {"reason": {"type": "integer", "description": "Exit reason number 0-77"}},
        "required": ["reason"]
    }),
    Tool(name="get_msr_info_new", description="Get MSR info by name or address", inputSchema={
        "type": "object",
        "properties": {"msr_name_or_addr": {"type": "string", "description": "MSR name or hex address"}},
        "required": ["msr_name_or_addr"]
    }),
    Tool(name="get_exception_info", description="Get exception vector info", inputSchema={
        "type": "object",
        "properties": {"vector": {"type": "integer", "description": "Exception vector 0-21"}},
        "required": ["vector"]
    }),
    Tool(name="list_vmcs_by_category_new", description="List all VMCS fields in category", inputSchema={
        "type": "object",
        "properties": {"category": {"type": "string", "enum": ["control", "guest_state", "host_state", "exit_info"]}},
        "required": ["category"]
    }),
    Tool(name="get_vmx_control_bits", description="Get VMX control bits", inputSchema={
        "type": "object",
        "properties": {"control_type": {"type": "string", "enum": ["pin_based", "proc_based", "proc_based2", "exit", "entry"]}},
        "required": ["control_type"]
    }),

    # Code Generator Tools
    Tool(name="generate_vmcs_setup_complete", description="Generate complete VMCS initialization code", inputSchema={
        "type": "object", "properties": {}
    }),
    Tool(name="generate_exit_handler_new", description="Generate exit handler for specific reason", inputSchema={
        "type": "object",
        "properties": {
            "reason": {"type": "integer", "description": "Exit reason number"},
            "stealth": {"type": "boolean", "description": "Include stealth measures", "default": True}
        },
        "required": ["reason"]
    }),
    Tool(name="generate_ept_setup_new", description="Generate EPT identity mapping code", inputSchema={
        "type": "object",
        "properties": {"memory_gb": {"type": "integer", "description": "Physical memory in GB", "default": 512}},
    }),
    Tool(name="generate_msr_bitmap_setup_new", description="Generate MSR bitmap setup code", inputSchema={
        "type": "object",
        "properties": {"intercept_msrs": {"type": "array", "items": {"type": "integer"}, "description": "MSRs to intercept"}},
    }),

    # Stealth Tools
    Tool(name="get_detection_vectors", description="Get all hypervisor detection techniques", inputSchema={
        "type": "object", "properties": {}
    }),
    Tool(name="audit_stealth", description="Audit code for detection risks", inputSchema={
        "type": "object",
        "properties": {"code": {"type": "string", "description": "C code to analyze"}},
        "required": ["code"]
    }),
    Tool(name="generate_timing_compensation", description="Generate timing compensation code", inputSchema={
        "type": "object",
        "properties": {"exit_cycles": {"type": "integer", "description": "Expected exit overhead in cycles", "default": 1000}},
    }),
    Tool(name="generate_cpuid_spoofing", description="Generate CPUID spoofing code", inputSchema={
        "type": "object", "properties": {}
    }),

    # BYOVD Tools
    Tool(name="ld9boxsup_ioctl_guide", description="Get Ld9BoxSup.sys IOCTL guide", inputSchema={
        "type": "object",
        "properties": {"operation": {"type": "string", "description": "Operation: cookie, alloc, load, call_ring0, msr, etc."}},
        "required": ["operation"]
    }),
    Tool(name="generate_driver_wrapper", description="Generate driver wrapper code", inputSchema={
        "type": "object", "properties": {}
    }),
    Tool(name="generate_hypervisor_loader", description="Generate hypervisor loader code", inputSchema={
        "type": "object", "properties": {}
    }),

    # Project Brain Tools
    Tool(name="get_project_status", description="Get overall project health dashboard with findings count, critical issues, and component status", inputSchema={
        "type": "object",
        "properties": {"verbose": {"type": "boolean", "description": "Include detailed findings and suggestions", "default": False}},
    }),
    Tool(name="get_findings", description="Query active code analysis findings", inputSchema={
        "type": "object",
        "properties": {
            "severity": {"type": "string", "enum": ["critical", "warning", "info"], "description": "Filter by severity"},
            "file": {"type": "string", "description": "Filter by file path (partial match)"},
            "type_": {"type": "string", "description": "Filter by analyzer type (hypervisor, consistency, security)"},
            "limit": {"type": "integer", "description": "Maximum findings to return", "default": 50}
        },
    }),
    Tool(name="dismiss_finding", description="Dismiss a finding as false positive", inputSchema={
        "type": "object",
        "properties": {
            "finding_id": {"type": "integer", "description": "ID of the finding to dismiss"},
            "reason": {"type": "string", "description": "Optional reason for dismissal"}
        },
        "required": ["finding_id"]
    }),
    Tool(name="get_suggestions", description="Get pending suggestions for what to work on next", inputSchema={
        "type": "object",
        "properties": {
            "priority": {"type": "string", "enum": ["high", "medium", "low"], "description": "Filter by priority"},
            "limit": {"type": "integer", "description": "Maximum suggestions to return", "default": 10}
        },
    }),
    Tool(name="get_component", description="Get details on a specific hypervisor component", inputSchema={
        "type": "object",
        "properties": {"component_id": {"type": "string", "description": "Component identifier (e.g., vmcs_setup, ept)"}},
        "required": ["component_id"]
    }),
    Tool(name="get_exit_handler_status", description="Get status of all exit handlers", inputSchema={
        "type": "object",
        "properties": {"status": {"type": "string", "enum": ["implemented", "partial", "stub", "missing"], "description": "Filter by status"}},
    }),
    Tool(name="add_decision", description="Record a design decision for future reference", inputSchema={
        "type": "object",
        "properties": {
            "topic": {"type": "string", "description": "What the decision is about"},
            "choice": {"type": "string", "description": "What was decided"},
            "rationale": {"type": "string", "description": "Why this choice was made"},
            "alternatives": {"type": "array", "items": {"type": "string"}, "description": "Other options considered"},
            "affects": {"type": "array", "items": {"type": "string"}, "description": "Files or components affected"}
        },
        "required": ["topic", "choice"]
    }),
    Tool(name="get_decision", description="Look up a design decision by ID", inputSchema={
        "type": "object",
        "properties": {"decision_id": {"type": "string", "description": "Decision ID (e.g., D001)"}},
        "required": ["decision_id"]
    }),
    Tool(name="list_decisions", description="List all design decisions", inputSchema={
        "type": "object",
        "properties": {
            "topic": {"type": "string", "description": "Filter by topic (partial match)"},
            "affects": {"type": "string", "description": "Filter by affected file/component (partial match)"}
        },
    }),
    Tool(name="add_gotcha", description="Record a solved bug for future reference", inputSchema={
        "type": "object",
        "properties": {
            "symptom": {"type": "string", "description": "What the bug looked like"},
            "cause": {"type": "string", "description": "What actually caused it"},
            "fix": {"type": "string", "description": "How it was fixed"},
            "files": {"type": "array", "items": {"type": "string"}, "description": "Affected files (e.g., vmcs.c:45)"}
        },
        "required": ["symptom", "cause", "fix"]
    }),
    Tool(name="search_gotchas", description="Search gotchas by keyword", inputSchema={
        "type": "object",
        "properties": {"keyword": {"type": "string", "description": "Search term (matches symptom, cause, or fix)"}},
        "required": ["keyword"]
    }),
    Tool(name="get_session_context", description="Get context from the last session for resuming work", inputSchema={
        "type": "object", "properties": {}
    }),
    Tool(name="save_session_context", description="Save current session context for later resumption", inputSchema={
        "type": "object",
        "properties": {
            "working_on": {"type": "string", "description": "Brief description of current work"},
            "context": {"type": "string", "description": "Detailed context/notes"},
            "files_touched": {"type": "array", "items": {"type": "string"}, "description": "List of files modified this session"}
        },
        "required": ["working_on"]
    }),
    Tool(name="refresh_analysis", description="Force a full rescan of the codebase", inputSchema={
        "type": "object",
        "properties": {"path": {"type": "string", "description": "Optional specific path to scan (default: entire project)"}},
    }),
    Tool(name="get_daemon_status", description="Get status of the watcher daemon (running state, last scan time, findings)", inputSchema={
        "type": "object", "properties": {}
    }),
]

TOOL_HANDLERS = {
    "vmcs_field": vmcs_field,
    "vmcs_fields_by_category": vmcs_fields_by_category,
    "exit_reason": exit_reason,
    "exit_qualification_format": exit_qualification_format,
    "msr_info": msr_info,
    "search_sdm": search_sdm,
    "generate_vmcs_header": generate_vmcs_header,
    "generate_exit_handler_skeleton": generate_exit_handler_skeleton,
    "generate_msr_header": generate_msr_header,
    "generate_handler_template": generate_handler_template,
    "generate_ept_structures": generate_ept_structures,
    "generate_asm_stub": generate_asm_stub,
    "get_implementation_status": get_implementation_status,
    "update_status": update_status,
    "add_implementation_note": add_implementation_note,
    "get_known_issues": get_known_issues,
    # Binary Signature Scanner
    "scan_binary_signatures": scan_binary_signatures,
    "scan_source_for_signatures": scan_source_for_signatures,
    # VMCS Validator
    "validate_vmcs_setup": validate_vmcs_setup,
    "get_vmcs_checklist": get_vmcs_checklist,
    # Anti-Cheat Intelligence
    "get_anticheat_intel": get_anticheat_intel,
    "get_timing_requirements": get_timing_requirements,
    "get_bypass_for_detection": get_bypass_for_detection,
    "check_evasion_coverage": check_evasion_coverage,
    # Timing Simulator
    "simulate_handler_timing": simulate_handler_timing,
    "get_timing_best_practices": get_timing_best_practices,
    "compare_handler_implementations": compare_handler_implementations,
    # SDM Query
    "ask_sdm": ask_sdm,
    "vmcs_field_complete": vmcs_field_complete,
    "exit_reason_complete": exit_reason_complete,
    "get_msr_info_new": get_msr_info,
    "get_exception_info": get_exception_info,
    "list_vmcs_by_category_new": list_vmcs_by_category,
    "get_vmx_control_bits": get_vmx_control_bits,
    # Code Generator
    "generate_vmcs_setup_complete": generate_vmcs_setup,
    "generate_exit_handler_new": generate_exit_handler,
    "generate_ept_setup_new": generate_ept_setup,
    "generate_msr_bitmap_setup_new": generate_msr_bitmap_setup,
    # Stealth
    "get_detection_vectors": get_detection_vectors,
    "audit_stealth": audit_stealth,
    "generate_timing_compensation": generate_timing_compensation,
    "generate_cpuid_spoofing": generate_cpuid_spoofing,
    # BYOVD
    "ld9boxsup_ioctl_guide": ld9boxsup_ioctl_guide,
    "generate_driver_wrapper": generate_driver_wrapper,
    "generate_hypervisor_loader": generate_hypervisor_loader,
    # Project Brain
    "get_project_status": get_project_status,
    "get_findings": get_findings,
    "dismiss_finding": dismiss_finding,
    "get_suggestions": get_suggestions,
    "get_component": get_component,
    "get_exit_handler_status": get_exit_handler_status,
    "add_decision": add_decision,
    "get_decision": get_decision,
    "list_decisions": list_decisions,
    "add_gotcha": add_gotcha,
    "search_gotchas": search_gotchas,
    "get_session_context": get_session_context,
    "save_session_context": save_session_context,
    "refresh_analysis": refresh_analysis,
    "get_daemon_status": get_daemon_status,
}

@app.list_tools()
async def list_tools() -> list[Tool]:
    return TOOLS

@app.call_tool()
async def call_tool(name: str, arguments: dict) -> list[TextContent]:
    if name not in TOOL_HANDLERS:
        raise ValueError(f"Unknown tool: {name}")
    
    handler = TOOL_HANDLERS[name]
    result = await handler(**arguments)
    
    if isinstance(result, str):
        return [TextContent(type="text", text=result)]
    else:
        return [TextContent(type="text", text=json.dumps(result, indent=2))]

# ============================================================================
# MAIN
# ============================================================================

async def main():
    init_options = InitializationOptions(
        server_name="ombra-mcp",
        server_version="2.1.0",
        capabilities=ServerCapabilities(tools=True),
    )
    async with stdio_server() as (read_stream, write_stream):
        await app.run(read_stream, write_stream, init_options)

if __name__ == "__main__":
    asyncio.run(main())
