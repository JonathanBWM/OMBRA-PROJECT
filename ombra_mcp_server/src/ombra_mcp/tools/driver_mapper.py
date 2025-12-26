"""
Driver Mapper Tools — Manual PE mapping guidance for kernel driver injection

This module provides guidance on:
- PE header parsing and validation
- Import resolution strategies
- Relocation handling
- Memory allocation approaches
- Anti-forensic cleanup
"""

import json
from pathlib import Path

DATA_DIR = Path(__file__).parent.parent / "data"


# =============================================================================
# PE STRUCTURE REFERENCE
# =============================================================================

PE_STRUCTURES = {
    "IMAGE_DOS_HEADER": {
        "size": 64,
        "fields": {
            "e_magic": {"offset": 0, "size": 2, "expected": "0x5A4D", "description": "MZ signature"},
            "e_lfanew": {"offset": 60, "size": 4, "description": "Offset to NT headers"}
        }
    },
    "IMAGE_NT_HEADERS64": {
        "size": 264,
        "fields": {
            "Signature": {"offset": 0, "size": 4, "expected": "0x4550", "description": "PE signature"},
            "FileHeader": {"offset": 4, "size": 20, "description": "IMAGE_FILE_HEADER"},
            "OptionalHeader": {"offset": 24, "size": 240, "description": "IMAGE_OPTIONAL_HEADER64"}
        }
    },
    "IMAGE_FILE_HEADER": {
        "size": 20,
        "fields": {
            "Machine": {"offset": 0, "size": 2, "x64_value": "0x8664", "description": "Machine type"},
            "NumberOfSections": {"offset": 2, "size": 2, "description": "Section count"},
            "TimeDateStamp": {"offset": 4, "size": 4, "description": "Build timestamp"},
            "SizeOfOptionalHeader": {"offset": 16, "size": 2, "description": "Optional header size"},
            "Characteristics": {"offset": 18, "size": 2, "description": "File characteristics"}
        }
    },
    "IMAGE_OPTIONAL_HEADER64": {
        "size": 240,
        "fields": {
            "Magic": {"offset": 0, "size": 2, "expected": "0x020B", "description": "PE32+ magic"},
            "AddressOfEntryPoint": {"offset": 16, "size": 4, "description": "Entry point RVA"},
            "ImageBase": {"offset": 24, "size": 8, "description": "Preferred base address"},
            "SectionAlignment": {"offset": 32, "size": 4, "description": "Section alignment in memory"},
            "FileAlignment": {"offset": 36, "size": 4, "description": "Section alignment on disk"},
            "SizeOfImage": {"offset": 56, "size": 4, "description": "Total image size when loaded"},
            "SizeOfHeaders": {"offset": 60, "size": 4, "description": "Header size"},
            "NumberOfRvaAndSizes": {"offset": 108, "size": 4, "description": "Data directory count"}
        }
    },
    "IMAGE_SECTION_HEADER": {
        "size": 40,
        "fields": {
            "Name": {"offset": 0, "size": 8, "description": "Section name"},
            "VirtualSize": {"offset": 8, "size": 4, "description": "Size in memory"},
            "VirtualAddress": {"offset": 12, "size": 4, "description": "RVA of section"},
            "SizeOfRawData": {"offset": 16, "size": 4, "description": "Size on disk"},
            "PointerToRawData": {"offset": 20, "size": 4, "description": "File offset of section"},
            "Characteristics": {"offset": 36, "size": 4, "description": "Section flags"}
        }
    }
}

RELOCATION_TYPES = {
    0: {"name": "ABSOLUTE", "action": "Skip - no fixup needed"},
    1: {"name": "HIGH", "action": "Add delta to high 16 bits of 32-bit address"},
    2: {"name": "LOW", "action": "Add delta to low 16 bits of 32-bit address"},
    3: {"name": "HIGHLOW", "action": "Add delta to entire 32-bit address"},
    10: {"name": "DIR64", "action": "Add delta to entire 64-bit address (most common for x64)"}
}

DATA_DIRECTORIES = {
    0: "EXPORT",
    1: "IMPORT",
    2: "RESOURCE",
    3: "EXCEPTION",
    4: "SECURITY",
    5: "BASERELOC",
    6: "DEBUG",
    7: "ARCHITECTURE",
    8: "GLOBALPTR",
    9: "TLS",
    10: "LOAD_CONFIG",
    11: "BOUND_IMPORT",
    12: "IAT",
    13: "DELAY_IMPORT",
    14: "CLR_RUNTIME",
    15: "RESERVED"
}


def get_pe_parsing_guide() -> dict:
    """
    Get comprehensive PE parsing guidance for driver mapping.

    Returns:
        PE structure reference and validation checklist
    """
    return {
        "structures": PE_STRUCTURES,
        "data_directories": DATA_DIRECTORIES,
        "validation_checklist": [
            {"check": "DOS signature", "value": "0x5A4D (MZ)", "field": "e_magic"},
            {"check": "NT signature", "value": "0x4550 (PE)", "field": "Signature"},
            {"check": "Machine type", "value": "0x8664 (x64)", "field": "Machine"},
            {"check": "Optional header magic", "value": "0x020B (PE32+)", "field": "Magic"},
            {"check": "Section count", "max": 64, "field": "NumberOfSections"},
            {"check": "Relocations present", "flag": "IMAGE_FILE_RELOCS_STRIPPED should NOT be set"}
        ],
        "notes": [
            "Always validate before mapping",
            "Check for stripped relocations - driver must have them",
            "Verify x64 architecture for kernel drivers",
            "SizeOfImage determines allocation size"
        ]
    }


def get_import_resolution_guide() -> dict:
    """
    Get import resolution strategy for manual driver mapping.

    Returns:
        Import resolution approach and common pitfalls
    """
    return {
        "two_stage_resolution": {
            "stage1_module_enumeration": {
                "description": "Find kernel module base addresses",
                "approach": "NtQuerySystemInformation(SystemModuleInformation)",
                "structure": "RTL_PROCESS_MODULES",
                "notes": [
                    "Returns list of all loaded kernel modules",
                    "Extract ImageBase for each module"
                ]
            },
            "stage2_export_walking": {
                "description": "Resolve imports by walking export tables",
                "approach": "Parse target module's export directory",
                "steps": [
                    "Load target module into usermode with DONT_RESOLVE_DLL_REFERENCES",
                    "Parse IMAGE_EXPORT_DIRECTORY",
                    "Walk AddressOfNames to find function by name",
                    "Get ordinal from AddressOfNameOrdinals",
                    "Get RVA from AddressOfFunctions[ordinal]",
                    "Calculate kernel address: module_base + RVA"
                ]
            }
        },
        "common_imports": {
            "ntoskrnl.exe": [
                "ExAllocatePool",
                "ExAllocatePoolWithTag",
                "ExFreePoolWithTag",
                "MmMapIoSpace",
                "MmUnmapIoSpace",
                "IoCreateDevice",
                "IoDeleteDevice",
                "PsCreateSystemThread",
                "KeInitializeEvent",
                "KeWaitForSingleObject",
                "RtlCopyMemory",
                "RtlZeroMemory"
            ],
            "hal.dll": [
                "HalGetBusData",
                "HalSetBusData"
            ]
        },
        "pitfalls": [
            "Forwarded exports redirect to another DLL - may need recursive resolution",
            "Ordinal-only imports are rare in kernel mode",
            "Some exports may be inlined or removed in different Windows versions"
        ]
    }


def get_relocation_guide() -> dict:
    """
    Get relocation processing guidance for manual mapping.

    Returns:
        Relocation handling approach
    """
    return {
        "relocation_types": RELOCATION_TYPES,
        "algorithm": {
            "delta_calculation": "delta = (new_base_address - original_preferred_base)",
            "optimization": "If delta == 0 (loaded at preferred address), skip relocations entirely",
            "block_structure": {
                "VirtualAddress": "4 bytes - RVA of page needing fixups",
                "SizeOfBlock": "4 bytes - Total block size including header",
                "entries": "Variable - array of 16-bit entries"
            },
            "entry_format": {
                "type": "Upper 4 bits - relocation type (see relocation_types)",
                "offset": "Lower 12 bits - offset within page"
            }
        },
        "processing_steps": [
            "Read base relocation directory from data directories",
            "For each relocation block:",
            "  - Extract page RVA and block size",
            "  - Calculate number of entries: (SizeOfBlock - 8) / 2",
            "  - For each entry:",
            "    - Extract type (entry >> 12) and offset (entry & 0xFFF)",
            "    - Apply type-specific fixup at (page_rva + offset)"
        ],
        "error_handling": [
            "Skip ABSOLUTE (type 0) entries",
            "Error on unknown relocation types",
            "Validate addresses are within image bounds"
        ]
    }


def get_memory_allocation_guide() -> dict:
    """
    Get memory allocation strategies for kernel driver mapping.

    Returns:
        Allocation approaches and considerations
    """
    return {
        "allocation_methods": {
            "method_a_exallocatepool": {
                "description": "Direct kernel pool allocation",
                "function": "ExAllocatePool(NonPagedPool, size)",
                "notes": [
                    "Returns kernel virtual address",
                    "NonPagedPool: not pageable, accessible from any IRQL",
                    "Size from PE SizeOfImage"
                ],
                "acquisition": "Via syscall hook or vulnerable driver"
            },
            "method_b_tagged_pool": {
                "description": "Tagged pool allocation (more visible)",
                "function": "ExAllocatePoolWithTag(NonPagedPool, size, 'Tag!')",
                "notes": [
                    "Pool tag visible in kernel debugger",
                    "Should be wiped after loading"
                ]
            },
            "method_c_vmxroot_allocator": {
                "description": "Pre-allocated bump allocator for VMX root",
                "notes": [
                    "8MB+ pre-allocated during payload init",
                    "Used within VMX root where kernel APIs unavailable",
                    "Allocate once, never free (no fragmentation)"
                ]
            }
        },
        "usermode_preparation": {
            "virtual_lock": {
                "function": "VirtualLock(addr, size)",
                "purpose": "Prevent page file involvement",
                "notes": [
                    "Critical for structures that must stay in RAM",
                    "Hypervisor state must not be paged",
                    "May need to expand working set first"
                ]
            }
        },
        "recommendations": [
            "Use NonPagedPool for driver code",
            "Wipe pool tags after successful load",
            "Pre-allocate for VMX root operations",
            "Lock critical usermode buffers"
        ]
    }


def get_cleanup_guide() -> dict:
    """
    Get anti-forensic cleanup guidance after driver mapping.

    Returns:
        Cleanup procedures for stealth
    """
    return {
        "pe_header_elimination": {
            "purpose": "Prevent signature scanning finding MZ/PE headers",
            "approach": "Overwrite headers with random garbage",
            "algorithm": {
                "name": "LCG-based garbage generation",
                "formula": "X = (X * 6364136223846793005 + 1442695040888963407) ^ (X >> 33)",
                "seed": "RDTSC (high-resolution timer)",
                "validation": "Regenerate if garbage contains 0x5A4D or 0x4550"
            },
            "chunk_size": "0x1000 bytes (4KB pages)",
            "notes": [
                "Critical for evading memory forensics",
                "Must not accidentally contain PE signatures"
            ]
        },
        "piddb_cache_clearing": {
            "purpose": "Remove driver load history from Windows cache",
            "target": "PiDDBCacheTable in ntoskrnl",
            "steps": [
                "Acquire PiDDBLock (ERESOURCE synchronization)",
                "Locate entry matching driver name + timestamp",
                "Unlink from RTL_AVL_TREE",
                "Delete from LIST_ENTRY chains",
                "Release lock"
            ],
            "notes": [
                "Windows caches all driver loads since boot",
                "Entries persist even after driver unload",
                "Version-specific offset discovery required"
            ]
        },
        "mmunloaded_drivers": {
            "purpose": "Clear secondary driver unload cache",
            "target": "MmUnloadedDrivers circular buffer",
            "notes": [
                "Windows-version specific signatures",
                "Clears forensic trace of unloaded drivers"
            ]
        },
        "etw_blinding": {
            "purpose": "Disable ETW Threat Intelligence during operations",
            "target": "EtwThreatIntProvRegHandle in ntoskrnl",
            "approach": {
                "pre_operation": "Write 0 to handle (disable provider)",
                "post_operation": "Restore original value",
                "timing": "Blind before mapping, restore immediately after"
            },
            "build_offsets": {
                "note": "Offset varies by Windows build",
                "discovery": "Signature scan or hardcoded offset table"
            }
        },
        "prefetch_cleanup": {
            "purpose": "Remove execution traces from Windows Prefetch",
            "location": "C:\\Windows\\Prefetch\\*.pf",
            "approach": {
                "pattern": "Match files containing loader name",
                "deletion": "Secure 3-pass overwrite (0x00, 0xFF, random)"
            },
            "artifacts": [
                "OMBRALOADER-*.pf",
                "LD9BOXSUP-*.pf",
                "VBOXSUP-*.pf"
            ]
        }
    }


def get_syscall_hook_guide() -> dict:
    """
    Get syscall hooking approach for kernel context acquisition.

    Returns:
        Syscall hook strategy for kernel execution
    """
    return {
        "target_selection": {
            "recommended": "NtShutdownSystem",
            "reason": "Rarely called, low impact if hook fails",
            "alternatives": ["NtQuerySystemTime", "NtQueryPerformanceCounter"]
        },
        "discovery_process": [
            "Load ntoskrnl.exe into usermode with DONT_RESOLVE_DLL_REFERENCES",
            "Find target function RVA within usermode image",
            "Calculate byte offset within 4KB page boundary",
            "Search physical memory for matching byte pattern",
            "Verify correctness by testing hook behavior"
        ],
        "hook_mechanics": {
            "x64_absolute_jump": {
                "size": 12,
                "bytes": "48 B8 [8-byte address] FF E0",
                "description": "mov rax, addr; jmp rax"
            },
            "x86_absolute_jump": {
                "size": 6,
                "bytes": "B8 [4-byte address] FF E0",
                "description": "mov eax, addr; jmp eax"
            }
        },
        "execution_flow": [
            "Install hook (12-byte overwrite)",
            "Call syscall stub from ntdll.dll",
            "CPU transitions to kernel with hook installed",
            "Syscall redirects to our kernel function",
            "Remove hook upon completion"
        ],
        "requirements": [
            "Identity mapping required for physical memory access",
            "Hypervisor must be active for identity mapping VMCALLs",
            "Hook address found via pattern matching in physical memory"
        ]
    }


def validate_driver_binary(pe_info: dict) -> dict:
    """
    Validate driver binary for mapping suitability.

    Args:
        pe_info: Dictionary with PE header information
            Required: dos_magic, nt_signature, machine, optional_magic,
                     num_sections, has_relocations

    Returns:
        Validation result with errors/warnings
    """
    errors = []
    warnings = []

    # Check DOS signature
    if pe_info.get("dos_magic") != 0x5A4D:
        errors.append("Invalid DOS signature (expected 0x5A4D 'MZ')")

    # Check NT signature
    if pe_info.get("nt_signature") != 0x4550:
        errors.append("Invalid NT signature (expected 0x4550 'PE')")

    # Check machine type
    if pe_info.get("machine") != 0x8664:
        errors.append(f"Invalid machine type 0x{pe_info.get('machine', 0):X} (expected 0x8664 for x64)")

    # Check optional header magic
    if pe_info.get("optional_magic") != 0x020B:
        errors.append("Invalid optional header magic (expected 0x020B for PE32+)")

    # Check section count
    num_sections = pe_info.get("num_sections", 0)
    if num_sections > 64:
        warnings.append(f"High section count ({num_sections}) - unusual for drivers")
    if num_sections == 0:
        errors.append("No sections found")

    # Check relocations
    if not pe_info.get("has_relocations", True):
        errors.append("Relocations stripped - driver cannot be loaded at arbitrary address")

    return {
        "valid": len(errors) == 0,
        "errors": errors,
        "warnings": warnings,
        "ready_for_mapping": len(errors) == 0
    }


def generate_mapping_checklist(driver_name: str = "driver.sys") -> dict:
    """
    Generate complete driver mapping checklist.

    Args:
        driver_name: Name of driver being mapped

    Returns:
        Step-by-step checklist for mapping
    """
    return {
        "driver": driver_name,
        "phases": [
            {
                "phase": 1,
                "name": "PE Parsing",
                "steps": [
                    "Read DOS header, validate MZ signature",
                    "Read NT headers at e_lfanew offset",
                    "Validate PE signature and machine type",
                    "Read section headers",
                    "Validate SizeOfImage"
                ]
            },
            {
                "phase": 2,
                "name": "Image Preparation",
                "steps": [
                    "Allocate buffer of SizeOfImage bytes",
                    "Copy headers (0 to SizeOfHeaders)",
                    "For each section: copy from PointerToRawData to VirtualAddress",
                    "Verify section mapping"
                ]
            },
            {
                "phase": 3,
                "name": "Import Resolution",
                "steps": [
                    "Enumerate required modules via NtQuerySystemInformation",
                    "For each import descriptor:",
                    "  - Get module name",
                    "  - Load module into usermode for export parsing",
                    "  - Resolve each import by walking export table",
                    "  - Write resolved address to IAT"
                ]
            },
            {
                "phase": 4,
                "name": "Relocation Processing",
                "steps": [
                    "Calculate delta (new_base - preferred_base)",
                    "If delta != 0, process relocations:",
                    "  - For each relocation block:",
                    "    - Apply type-specific fixup"
                ]
            },
            {
                "phase": 5,
                "name": "Kernel Memory Allocation",
                "steps": [
                    "Allocate kernel pool (NonPagedPool) via syscall hook",
                    "Or use vulnerable driver primitives",
                    "Verify allocation succeeded"
                ]
            },
            {
                "phase": 6,
                "name": "Kernel Memory Write",
                "steps": [
                    "Copy prepared image to kernel allocation",
                    "Use identity mapping or vulnerable driver R/W"
                ]
            },
            {
                "phase": 7,
                "name": "Driver Initialization",
                "steps": [
                    "Calculate entry point VA (allocation + AddressOfEntryPoint)",
                    "Call DriverEntry via syscall hook",
                    "Check NTSTATUS return value"
                ]
            },
            {
                "phase": 8,
                "name": "Anti-Forensic Cleanup",
                "steps": [
                    "Wipe PE headers with random garbage",
                    "Clear PiDDBCacheTable entry",
                    "Clear MmUnloadedDrivers if needed",
                    "Restore ETW-TI if it was disabled"
                ]
            }
        ]
    }


# Test if run directly
if __name__ == "__main__":
    import json

    print("=== PE Parsing Guide ===")
    print(json.dumps(get_pe_parsing_guide(), indent=2))

    print("\n=== Cleanup Guide ===")
    print(json.dumps(get_cleanup_guide(), indent=2))
