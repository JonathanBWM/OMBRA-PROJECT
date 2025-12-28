"""
PE file parser for Windows kernel drivers.

Extracts:
- PE headers (file header, optional header)
- Sections (name, RVA, size, characteristics)
- Imports (DLL, function name/ordinal, IAT)
- Exports (name, ordinal, RVA)
- Version info (FileVersion, CompanyName, etc.)
- Rich header (compiler info)
"""

import pefile
from typing import Dict, List, Optional
import os


async def parse_pe(file_path: str) -> Dict:
    """
    Parse PE file and extract all metadata.

    Args:
        file_path: Path to .sys file

    Returns:
        Dict with all extracted metadata ready for database insertion
    """
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"File not found: {file_path}")

    pe = pefile.PE(file_path)

    return {
        "headers": extract_headers(pe),
        "sections": await extract_sections(pe),
        "imports": await extract_imports(pe),
        "exports": await extract_exports(pe),
        "version_info": await extract_version_info(pe),
        "rich_header": extract_rich_header(pe),
        "certificates": extract_certificates(pe),
        "debug_info": extract_debug_info(pe)
    }


def extract_headers(pe: pefile.PE) -> Dict:
    """
    Extract PE headers.

    Returns:
        Dict with file header, optional header, and DOS header info
    """
    file_header = pe.FILE_HEADER
    optional_header = pe.OPTIONAL_HEADER

    return {
        # File header
        "machine": file_header.Machine,
        "machine_name": pefile.MACHINE_TYPE.get(file_header.Machine, "UNKNOWN"),
        "number_of_sections": file_header.NumberOfSections,
        "timestamp": file_header.TimeDateStamp,
        "pointer_to_symbol_table": file_header.PointerToSymbolTable,
        "number_of_symbols": file_header.NumberOfSymbols,
        "size_of_optional_header": file_header.SizeOfOptionalHeader,
        "characteristics": file_header.Characteristics,

        # Optional header
        "magic": optional_header.Magic,
        "linker_version": f"{optional_header.MajorLinkerVersion}.{optional_header.MinorLinkerVersion}",
        "size_of_code": optional_header.SizeOfCode,
        "size_of_initialized_data": optional_header.SizeOfInitializedData,
        "size_of_uninitialized_data": optional_header.SizeOfUninitializedData,
        "entry_point_rva": optional_header.AddressOfEntryPoint,
        "base_of_code": optional_header.BaseOfCode,
        "image_base": optional_header.ImageBase,
        "section_alignment": optional_header.SectionAlignment,
        "file_alignment": optional_header.FileAlignment,
        "os_version": f"{optional_header.MajorOperatingSystemVersion}.{optional_header.MinorOperatingSystemVersion}",
        "image_version": f"{optional_header.MajorImageVersion}.{optional_header.MinorImageVersion}",
        "subsystem_version": f"{optional_header.MajorSubsystemVersion}.{optional_header.MinorSubsystemVersion}",
        "size_of_image": optional_header.SizeOfImage,
        "size_of_headers": optional_header.SizeOfHeaders,
        "checksum": optional_header.CheckSum,
        "subsystem": optional_header.Subsystem,
        "subsystem_name": pefile.SUBSYSTEM_TYPE.get(optional_header.Subsystem, "UNKNOWN"),
        "dll_characteristics": optional_header.DllCharacteristics,
        "size_of_stack_reserve": optional_header.SizeOfStackReserve,
        "size_of_stack_commit": optional_header.SizeOfStackCommit,
        "size_of_heap_reserve": optional_header.SizeOfHeapReserve,
        "size_of_heap_commit": optional_header.SizeOfHeapCommit,
        "number_of_rva_and_sizes": optional_header.NumberOfRvaAndSizes
    }


async def extract_sections(pe: pefile.PE) -> List[Dict]:
    """
    Extract PE sections.

    Returns:
        List of section dicts
    """
    sections = []

    for section in pe.sections:
        # Calculate entropy (0-8, higher = more packed/encrypted)
        entropy = section.get_entropy()

        sections.append({
            "name": section.Name.decode().rstrip('\x00'),
            "virtual_address": section.VirtualAddress,
            "virtual_size": section.Misc_VirtualSize,
            "raw_address": section.PointerToRawData,
            "raw_size": section.SizeOfRawData,
            "characteristics": section.Characteristics,
            "entropy": entropy,
            # Derived flags
            "is_executable": bool(section.Characteristics & 0x20000000),
            "is_writable": bool(section.Characteristics & 0x80000000),
            "is_readable": bool(section.Characteristics & 0x40000000)
        })

    return sections


async def extract_imports(pe: pefile.PE) -> List[Dict]:
    """
    Extract imports (imported functions from DLLs).

    Returns:
        List of import dicts
    """
    imports = []

    if not hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
        return imports

    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        dll_name = entry.dll.decode()

        for imp in entry.imports:
            import_entry = {
                "dll_name": dll_name,
                "function_name": imp.name.decode() if imp.name else None,
                "ordinal": imp.ordinal,
                "hint": imp.hint,
                "iat_rva": imp.address,
                "iat_va": imp.address + pe.OPTIONAL_HEADER.ImageBase if imp.address else None
            }

            imports.append(import_entry)

    return imports


async def extract_exports(pe: pefile.PE) -> List[Dict]:
    """
    Extract exports (functions exported by this driver).

    Returns:
        List of export dicts
    """
    exports = []

    if not hasattr(pe, 'DIRECTORY_ENTRY_EXPORT'):
        return exports

    for exp in pe.DIRECTORY_ENTRY_EXPORT.symbols:
        export_entry = {
            "function_name": exp.name.decode() if exp.name else None,
            "ordinal": exp.ordinal,
            "rva": exp.address,
            "va": exp.address + pe.OPTIONAL_HEADER.ImageBase if exp.address else None
        }

        # Extract prefix for categorization (ASM*, RT*, SUPR0*, etc.)
        if exp.name:
            name = exp.name.decode()
            if '_' in name or name.isupper():
                prefix = name.split('_')[0] if '_' in name else name[:4]
                export_entry["prefix"] = prefix

        exports.append(export_entry)

    return exports


async def extract_version_info(pe: pefile.PE) -> Dict:
    """
    Extract version resource information.

    Returns:
        Dict with version fields
    """
    version_info = {}

    if not hasattr(pe, 'VS_VERSIONINFO'):
        return version_info

    if hasattr(pe, 'FileInfo'):
        for file_info in pe.FileInfo:
            if hasattr(file_info, 'StringTable'):
                for st in file_info.StringTable:
                    for entry in st.entries.items():
                        key = entry[0].decode() if isinstance(entry[0], bytes) else entry[0]
                        value = entry[1].decode() if isinstance(entry[1], bytes) else entry[1]
                        version_info[key] = value

    return version_info


def extract_rich_header(pe: pefile.PE) -> Optional[Dict]:
    """
    Extract Rich header (compiler/linker info).

    Returns:
        Dict with Rich header data if present
    """
    try:
        if hasattr(pe, 'RICH_HEADER'):
            rich = pe.RICH_HEADER
            return {
                "checksum": rich.checksum,
                "values": [
                    {
                        "product_id": entry[0],
                        "build_id": entry[1],
                        "count": entry[2]
                    }
                    for entry in rich.values
                ]
            }
    except:
        pass

    return None


def extract_certificates(pe: pefile.PE) -> Optional[List[Dict]]:
    """
    Extract code signing certificates.

    Returns:
        List of certificate info if signed
    """
    certs = []

    try:
        if hasattr(pe, 'DIRECTORY_ENTRY_SECURITY'):
            for cert in pe.DIRECTORY_ENTRY_SECURITY:
                certs.append({
                    "type": cert.wCertificateType,
                    "length": cert.dwLength,
                    "revision": cert.wRevision
                })
    except:
        pass

    return certs if certs else None


def extract_debug_info(pe: pefile.PE) -> Optional[Dict]:
    """
    Extract debug directory info (PDB path, etc.).

    Returns:
        Debug info dict
    """
    debug_info = {}

    try:
        if hasattr(pe, 'DIRECTORY_ENTRY_DEBUG'):
            for entry in pe.DIRECTORY_ENTRY_DEBUG:
                if hasattr(entry.entry, 'PdbFileName'):
                    pdb_path = entry.entry.PdbFileName.decode()
                    debug_info["pdb_path"] = pdb_path

                debug_info["type"] = entry.struct.Type
                debug_info["size_of_data"] = entry.struct.SizeOfData
                debug_info["address_of_raw_data"] = entry.struct.AddressOfRawData
    except:
        pass

    return debug_info if debug_info else None


async def get_pe_basic_info(file_path: str) -> Dict:
    """
    Quick PE info extraction (no full parse).

    Args:
        file_path: Path to PE file

    Returns:
        Basic info: {size, image_base, entry_point, sections_count}
    """
    pe = pefile.PE(file_path)

    return {
        "file_size": os.path.getsize(file_path),
        "image_base": pe.OPTIONAL_HEADER.ImageBase,
        "entry_point_rva": pe.OPTIONAL_HEADER.AddressOfEntryPoint,
        "size_of_image": pe.OPTIONAL_HEADER.SizeOfImage,
        "number_of_sections": pe.FILE_HEADER.NumberOfSections,
        "timestamp": pe.FILE_HEADER.TimeDateStamp
    }


def get_section_by_rva(pe: pefile.PE, rva: int) -> Optional[pefile.SectionStructure]:
    """
    Get section containing an RVA.

    Args:
        pe: pefile.PE instance
        rva: Relative virtual address

    Returns:
        Section structure or None
    """
    return pe.get_section_by_rva(rva)


def rva_to_file_offset(pe: pefile.PE, rva: int) -> int:
    """
    Convert RVA to file offset.

    Args:
        pe: pefile.PE instance
        rva: Relative virtual address

    Returns:
        File offset
    """
    return pe.get_offset_from_rva(rva)
