"""
PE file parsers for driver analysis.
"""

from .pe_parser import (
    parse_pe,
    extract_headers,
    extract_sections,
    extract_imports,
    extract_exports,
    extract_version_info,
    get_pe_basic_info,
    rva_to_file_offset
)
from .string_parser import (
    extract_strings,
    search_strings,
    get_strings_by_category,
    get_error_messages,
    get_device_names,
    get_registry_keys,
    categorize_string
)

__all__ = [
    # PE parser
    "parse_pe",
    "extract_headers",
    "extract_sections",
    "extract_imports",
    "extract_exports",
    "extract_version_info",
    "get_pe_basic_info",
    "rva_to_file_offset",

    # String parser
    "extract_strings",
    "search_strings",
    "get_strings_by_category",
    "get_error_messages",
    "get_device_names",
    "get_registry_keys",
    "categorize_string"
]
