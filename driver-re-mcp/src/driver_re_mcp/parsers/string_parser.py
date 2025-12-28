"""
String extraction from PE files.

Extracts ASCII and Unicode strings with:
- RVA/VA addresses
- Section location
- Categorization (error message, debug, path, registry key, device name, etc.)
"""

import re
from typing import Dict, List
import pefile


async def extract_strings(file_path: str, min_length: int = 4) -> List[Dict]:
    """
    Extract all strings from PE file.

    Args:
        file_path: Path to PE file
        min_length: Minimum string length

    Returns:
        List of strings with metadata: [{value, rva, va, length, encoding, category}, ...]
    """
    pe = pefile.PE(file_path)
    image_base = pe.OPTIONAL_HEADER.ImageBase

    # Read file data
    with open(file_path, 'rb') as f:
        data = f.read()

    strings = []

    # Extract ASCII strings
    ascii_strings = extract_ascii_strings(data, min_length)
    for s in ascii_strings:
        # Try to convert file offset to RVA
        try:
            rva = pe.get_rva_from_offset(s["offset"])
            section = pe.get_section_by_rva(rva)

            strings.append({
                "value": s["value"],
                "rva": rva,
                "va": image_base + rva,
                "length": len(s["value"]),
                "encoding": "ascii",
                "section_name": section.Name.decode().rstrip('\x00') if section else None,
                "category": categorize_string(s["value"])
            })
        except:
            # String might be in headers or not mapped, skip
            pass

    # Extract Unicode strings
    unicode_strings = extract_unicode_strings(data, min_length)
    for s in unicode_strings:
        try:
            rva = pe.get_rva_from_offset(s["offset"])
            section = pe.get_section_by_rva(rva)

            strings.append({
                "value": s["value"],
                "rva": rva,
                "va": image_base + rva,
                "length": len(s["value"]),
                "encoding": "unicode",
                "section_name": section.Name.decode().rstrip('\x00') if section else None,
                "category": categorize_string(s["value"])
            })
        except:
            pass

    return strings


def extract_ascii_strings(data: bytes, min_length: int = 4) -> List[Dict]:
    """
    Extract printable ASCII strings.

    Args:
        data: Binary data
        min_length: Minimum string length

    Returns:
        List of strings with offsets
    """
    # Regex for printable ASCII (0x20-0x7E)
    pattern = b'[\x20-\x7E]{' + str(min_length).encode() + b',}'

    strings = []
    for match in re.finditer(pattern, data):
        strings.append({
            "value": match.group().decode('ascii'),
            "offset": match.start()
        })

    return strings


def extract_unicode_strings(data: bytes, min_length: int = 4) -> List[Dict]:
    """
    Extract UTF-16LE (Windows Unicode) strings.

    Args:
        data: Binary data
        min_length: Minimum string length

    Returns:
        List of strings with offsets
    """
    # Regex for printable ASCII in UTF-16LE (each char is 2 bytes: 0x00 0xXX)
    # Match sequences of: printable_byte null_byte
    pattern = b'(?:[\x20-\x7E]\x00){' + str(min_length).encode() + b',}'

    strings = []
    for match in re.finditer(pattern, data):
        try:
            decoded = match.group().decode('utf-16le')
            strings.append({
                "value": decoded,
                "offset": match.start()
            })
        except:
            pass

    return strings


def categorize_string(s: str) -> str:
    """
    Categorize a string based on content patterns.

    Returns:
        Category: error_message, debug, path, registry, device_name, url, etc.
    """
    s_lower = s.lower()

    # Error messages
    if any(keyword in s_lower for keyword in ['error', 'failed', 'invalid', 'cannot', 'unable']):
        return "error_message"

    # Debug/logging
    if any(keyword in s_lower for keyword in ['debug', 'log', 'trace', 'verbose', '[%d]', '%s', '%x']):
        return "debug"

    # File paths
    if '\\' in s and any(s_lower.startswith(p) for p in ['c:\\', 'd:\\', '\\device\\', '\\systemroot\\']):
        return "path"

    # Registry keys
    if s_lower.startswith(('hkey_', 'hklm\\', 'hkcu\\', 'software\\', 'system\\currentcontrolset\\')):
        return "registry"

    # Device names
    if s_lower.startswith(('\\device\\', '\\dosdevices\\', '\\\\.\\', '\\??\\')) or s_lower.endswith('device'):
        return "device_name"

    # URLs
    if s_lower.startswith(('http://', 'https://', 'ftp://')):
        return "url"

    # GUIDs
    if re.match(r'^[{]?[0-9a-fA-F]{8}-([0-9a-fA-F]{4}-){3}[0-9a-fA-F]{12}[}]?$', s):
        return "guid"

    # Version strings
    if re.match(r'^\d+\.\d+\.\d+', s):
        return "version"

    # Function/export names (CamelCase or snake_case with uppercase)
    if re.match(r'^[A-Z][a-zA-Z0-9_]+$', s) and len(s) > 8:
        return "function_name"

    # Generic message
    if len(s) > 20 and ' ' in s:
        return "message"

    return "unknown"


async def search_strings(
    file_path: str,
    pattern: str,
    min_length: int = 4,
    regex: bool = False
) -> List[Dict]:
    """
    Search for strings matching a pattern.

    Args:
        file_path: Path to PE file
        pattern: Search pattern (literal or regex)
        min_length: Minimum string length
        regex: If True, treat pattern as regex

    Returns:
        Matching strings
    """
    all_strings = await extract_strings(file_path, min_length)

    if regex:
        compiled = re.compile(pattern, re.IGNORECASE)
        return [s for s in all_strings if compiled.search(s["value"])]
    else:
        pattern_lower = pattern.lower()
        return [s for s in all_strings if pattern_lower in s["value"].lower()]


async def get_strings_by_category(
    file_path: str,
    category: str,
    min_length: int = 4
) -> List[Dict]:
    """
    Get all strings of a specific category.

    Args:
        file_path: Path to PE file
        category: String category
        min_length: Minimum string length

    Returns:
        Strings matching category
    """
    all_strings = await extract_strings(file_path, min_length)
    return [s for s in all_strings if s["category"] == category]


async def get_error_messages(file_path: str) -> List[str]:
    """
    Extract all error message strings.

    Args:
        file_path: Path to PE file

    Returns:
        List of error message strings
    """
    strings = await get_strings_by_category(file_path, "error_message")
    return [s["value"] for s in strings]


async def get_device_names(file_path: str) -> List[str]:
    """
    Extract all device name strings.

    Args:
        file_path: Path to PE file

    Returns:
        List of device names
    """
    strings = await get_strings_by_category(file_path, "device_name")
    return [s["value"] for s in strings]


async def get_registry_keys(file_path: str) -> List[str]:
    """
    Extract all registry key strings.

    Args:
        file_path: Path to PE file

    Returns:
        List of registry keys
    """
    strings = await get_strings_by_category(file_path, "registry")
    return [s["value"] for s in strings]
