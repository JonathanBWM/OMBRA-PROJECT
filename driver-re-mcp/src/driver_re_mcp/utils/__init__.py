"""
Utility functions for driver analysis.
"""

from .hashing import (
    compute_hashes,
    compute_md5,
    compute_sha1,
    compute_sha256,
    compute_imphash,
    compute_ssdeep,
    compare_ssdeep,
    hash_data,
    verify_hash
)

from .address import (
    rva_to_va,
    va_to_rva,
    rva_to_file_offset,
    file_offset_to_rva,
    va_to_file_offset,
    file_offset_to_va,
    normalize_address,
    get_section_for_rva,
    get_section_for_va,
    is_executable_address,
    is_writable_address,
    align_address,
    is_aligned
)

__all__ = [
    # Hashing
    "compute_hashes",
    "compute_md5",
    "compute_sha1",
    "compute_sha256",
    "compute_imphash",
    "compute_ssdeep",
    "compare_ssdeep",
    "hash_data",
    "verify_hash",

    # Address conversion
    "rva_to_va",
    "va_to_rva",
    "rva_to_file_offset",
    "file_offset_to_rva",
    "va_to_file_offset",
    "file_offset_to_va",
    "normalize_address",
    "get_section_for_rva",
    "get_section_for_va",
    "is_executable_address",
    "is_writable_address",
    "align_address",
    "is_aligned"
]
