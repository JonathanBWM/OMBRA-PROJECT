"""
Address conversion utilities.

Handles conversions between:
- RVA (Relative Virtual Address)
- VA (Virtual Address = Image Base + RVA)
- File Offset (raw offset in PE file)
"""

from typing import List, Dict, Optional
import pefile


def rva_to_va(rva: int, image_base: int) -> int:
    """
    Convert RVA to VA.

    Args:
        rva: Relative virtual address
        image_base: Image base address

    Returns:
        Virtual address
    """
    return image_base + rva


def va_to_rva(va: int, image_base: int) -> int:
    """
    Convert VA to RVA.

    Args:
        va: Virtual address
        image_base: Image base address

    Returns:
        Relative virtual address
    """
    return va - image_base


def rva_to_file_offset(rva: int, sections: List[Dict]) -> int:
    """
    Convert RVA to file offset using section information.

    Args:
        rva: Relative virtual address
        sections: List of section dicts with virtual_address, virtual_size, raw_address

    Returns:
        File offset, or -1 if RVA not in any section
    """
    for section in sections:
        section_start = section['virtual_address']
        section_end = section_start + section['virtual_size']

        if section_start <= rva < section_end:
            # RVA is in this section
            offset_in_section = rva - section_start
            file_offset = section['raw_address'] + offset_in_section
            return file_offset

    return -1


def file_offset_to_rva(offset: int, sections: List[Dict]) -> int:
    """
    Convert file offset to RVA using section information.

    Args:
        offset: File offset
        sections: List of section dicts with virtual_address, raw_address, raw_size

    Returns:
        RVA, or -1 if offset not in any section
    """
    for section in sections:
        raw_start = section['raw_address']
        raw_end = raw_start + section['raw_size']

        if raw_start <= offset < raw_end:
            # Offset is in this section
            offset_in_section = offset - raw_start
            rva = section['virtual_address'] + offset_in_section
            return rva

    return -1


def va_to_file_offset(va: int, image_base: int, sections: List[Dict]) -> int:
    """
    Convert VA to file offset.

    Args:
        va: Virtual address
        image_base: Image base
        sections: List of section dicts

    Returns:
        File offset
    """
    rva = va_to_rva(va, image_base)
    return rva_to_file_offset(rva, sections)


def file_offset_to_va(offset: int, image_base: int, sections: List[Dict]) -> int:
    """
    Convert file offset to VA.

    Args:
        offset: File offset
        image_base: Image base
        sections: List of section dicts

    Returns:
        Virtual address
    """
    rva = file_offset_to_rva(offset, sections)
    if rva == -1:
        return -1
    return rva_to_va(rva, image_base)


def normalize_address(
    address: int,
    image_base: int,
    sections: Optional[List[Dict]] = None,
    source_type: str = "auto"
) -> Dict[str, int]:
    """
    Normalize an address to all formats.

    Args:
        address: Input address
        image_base: Image base
        sections: Section info (required for file_offset conversion)
        source_type: 'auto', 'rva', 'va', or 'file_offset'

    Returns:
        Dict with rva, va, and file_offset (if sections provided)
    """
    result = {}

    if source_type == "auto":
        # Heuristic: if address > image_base, assume VA; else RVA
        if address >= image_base:
            source_type = "va"
        else:
            source_type = "rva"

    if source_type == "va":
        result['va'] = address
        result['rva'] = va_to_rva(address, image_base)
    elif source_type == "rva":
        result['rva'] = address
        result['va'] = rva_to_va(address, image_base)
    elif source_type == "file_offset":
        if not sections:
            raise ValueError("Sections required for file_offset conversion")
        result['file_offset'] = address
        result['rva'] = file_offset_to_rva(address, sections)
        result['va'] = rva_to_va(result['rva'], image_base)

    # Add file_offset if sections provided and not already set
    if sections and 'file_offset' not in result:
        result['file_offset'] = rva_to_file_offset(result['rva'], sections)

    return result


def get_section_for_rva(rva: int, sections: List[Dict]) -> Optional[Dict]:
    """
    Get section containing an RVA.

    Args:
        rva: Relative virtual address
        sections: List of section dicts

    Returns:
        Section dict or None
    """
    for section in sections:
        section_start = section['virtual_address']
        section_end = section_start + section['virtual_size']

        if section_start <= rva < section_end:
            return section

    return None


def get_section_for_va(va: int, image_base: int, sections: List[Dict]) -> Optional[Dict]:
    """
    Get section containing a VA.

    Args:
        va: Virtual address
        image_base: Image base
        sections: List of section dicts

    Returns:
        Section dict or None
    """
    rva = va_to_rva(va, image_base)
    return get_section_for_rva(rva, sections)


def is_executable_address(rva: int, sections: List[Dict]) -> bool:
    """
    Check if an RVA is in an executable section.

    Args:
        rva: Relative virtual address
        sections: List of section dicts

    Returns:
        True if in executable section
    """
    section = get_section_for_rva(rva, sections)
    return section and section.get('is_executable', False)


def is_writable_address(rva: int, sections: List[Dict]) -> bool:
    """
    Check if an RVA is in a writable section.

    Args:
        rva: Relative virtual address
        sections: List of section dicts

    Returns:
        True if in writable section
    """
    section = get_section_for_rva(rva, sections)
    return section and section.get('is_writable', False)


def align_address(address: int, alignment: int) -> int:
    """
    Align an address to specified boundary.

    Args:
        address: Address to align
        alignment: Alignment boundary (e.g., 0x1000 for page alignment)

    Returns:
        Aligned address
    """
    return (address + alignment - 1) & ~(alignment - 1)


def is_aligned(address: int, alignment: int) -> bool:
    """
    Check if address is aligned to boundary.

    Args:
        address: Address to check
        alignment: Alignment boundary

    Returns:
        True if aligned
    """
    return (address % alignment) == 0
