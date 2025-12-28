#!/usr/bin/env python3
"""
Scrape priority structures from Vergilius Project.
Rate-limited to be respectful to the server.

Usage:
    python scrape_vergilius.py                    # Scrape all priority structures
    python scrape_vergilius.py --struct _EPROCESS # Scrape single structure
    python scrape_vergilius.py --version win10-22h2  # Scrape single version
"""

import argparse
import re
import sqlite3
import time
from datetime import datetime
from pathlib import Path
from urllib.parse import urljoin

import requests
from bs4 import BeautifulSoup

DB_PATH = Path(__file__).parent.parent / "src" / "ombra_mcp" / "data" / "vergilius.db"
BASE_URL = "https://www.vergiliusproject.com"
RATE_LIMIT = 1.5  # seconds between requests

# Priority structures for hypervisor development
PRIORITY_STRUCTURES = [
    # Process/Thread
    "_EPROCESS", "_KPROCESS", "_ETHREAD", "_KTHREAD",
    "_PEB", "_TEB", "_PEB32", "_TEB32",
    "_KAPC_STATE", "_CLIENT_ID",
    # Memory
    "_MDL", "_POOL_HEADER", "_POOL_TRACKER_BIG_PAGES",
    "_MMPTE", "_MMPTE_HARDWARE", "_MMPFN", "_MMPFNENTRY",
    "_MMVAD", "_MMVAD_SHORT", "_MMVAD_FLAGS",
    "_MI_SYSTEM_PTE_TYPE", "_MMSUPPORT", "_MMSUPPORT_FULL",
    # Driver/IO
    "_DRIVER_OBJECT", "_DEVICE_OBJECT", "_FILE_OBJECT",
    "_IRP", "_IO_STACK_LOCATION", "_LDR_DATA_TABLE_ENTRY",
    "_KLDR_DATA_TABLE_ENTRY",
    # Security
    "_TOKEN", "_OBJECT_HEADER", "_OBJECT_TYPE",
    "_HANDLE_TABLE", "_HANDLE_TABLE_ENTRY",
    "_SE_AUDIT_PROCESS_CREATION_INFO", "_SECURITY_DESCRIPTOR",
    # System/CPU
    "_KPCR", "_KPRCB", "_CONTEXT", "_KTRAP_FRAME",
    "_KIDTENTRY64", "_KGDTENTRY64", "_KSPECIAL_REGISTERS",
    "_DISPATCHER_HEADER", "_KWAIT_BLOCK",
    # Misc useful
    "_LIST_ENTRY", "_UNICODE_STRING", "_RTL_BALANCED_NODE",
    "_EX_PUSH_LOCK", "_EX_RUNDOWN_REF", "_LARGE_INTEGER",
]


def get_db_connection():
    """Get database connection."""
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn


def fetch_page(url: str) -> str:
    """Fetch a page with rate limiting."""
    headers = {
        "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) OmbraMCP/1.0"
    }
    time.sleep(RATE_LIMIT)
    resp = requests.get(url, headers=headers, timeout=30)
    resp.raise_for_status()
    return resp.text


def parse_size(definition: str) -> int | None:
    """Extract structure size from definition comment."""
    # Pattern: //0x30 bytes (sizeof)
    match = re.search(r'//\s*(0x[0-9a-fA-F]+)\s*bytes\s*\(sizeof\)', definition)
    if match:
        return int(match.group(1), 16)
    return None


def parse_fields(definition: str) -> list[dict]:
    """Parse field definitions from C struct."""
    fields = []

    # Match lines like: TYPE name; //0xOFFSET
    # Also handles: TYPE name[N]; //0xOFFSET
    # Also handles: TYPE name:N; //0xOFFSET (bitfields)
    pattern = re.compile(
        r'^\s*'
        r'(?:(?:struct|union|enum)\s+)?'  # Optional struct/union/enum keyword
        r'([\w\s\*]+?)'  # Type (capture group 1)
        r'\s+'
        r'(\w+)'  # Field name (capture group 2)
        r'(?:\[(\d+)\])?'  # Optional array size (capture group 3)
        r'(?::(\d+))?'  # Optional bit field (capture group 4)
        r'\s*;'
        r'\s*//\s*(0x[0-9a-fA-F]+)',  # Offset comment (capture group 5)
        re.MULTILINE
    )

    for match in pattern.finditer(definition):
        field_type = match.group(1).strip()
        field_name = match.group(2)
        array_size = int(match.group(3)) if match.group(3) else None
        bit_field = f":{match.group(4)}" if match.group(4) else None
        offset_hex = match.group(5)
        offset_dec = int(offset_hex, 16)

        fields.append({
            "offset_dec": offset_dec,
            "name": field_name,
            "field_type": field_type,
            "bit_field": bit_field,
            "array_size": array_size,
        })

    return fields


def parse_used_in(soup: BeautifulSoup) -> list[str]:
    """Extract 'Used in' structure references."""
    used_in = []

    # Look for "Used in" section - it's typically a heading followed by links
    for elem in soup.find_all(string=re.compile(r'Used in', re.I)):
        parent = elem.find_parent()
        if parent:
            # Find all links after this element
            container = parent.find_next_sibling()
            if container:
                for link in container.find_all('a'):
                    href = link.get('href', '')
                    if '/kernels/' in href:
                        # Extract structure name from URL
                        name = href.split('/')[-1]
                        if name.startswith('_'):
                            used_in.append(name)

    return used_in


def scrape_structure(version_path: str, struct_name: str) -> dict | None:
    """Scrape a single structure definition."""
    url = f"{BASE_URL}{version_path}/{struct_name}"
    print(f"    Fetching {struct_name}...")

    try:
        html = fetch_page(url)
    except requests.exceptions.HTTPError as e:
        if e.response.status_code == 404:
            print(f"    [!] {struct_name} not found for this version")
            return None
        raise

    soup = BeautifulSoup(html, 'html.parser')

    # Find the code block with structure definition
    code_block = soup.find('code') or soup.find('pre')
    if not code_block:
        print(f"    [!] No code block found for {struct_name}")
        return None

    definition = code_block.get_text()

    # Determine type kind
    if 'enum ' in definition[:50]:
        type_kind = "enum"
    elif 'union ' in definition[:50]:
        type_kind = "union"
    else:
        type_kind = "struct"

    size_bytes = parse_size(definition)
    fields = parse_fields(definition)
    used_in = parse_used_in(soup)

    return {
        "name": struct_name,
        "type_kind": type_kind,
        "size_bytes": size_bytes,
        "definition": definition,
        "fields": fields,
        "used_in": used_in,
        "url": url,
    }


def save_structure(conn: sqlite3.Connection, version_id: int, data: dict):
    """Save structure data to database."""
    cur = conn.cursor()

    # Insert type definition
    cur.execute("""
        INSERT OR REPLACE INTO type_definitions
        (version_id, type_kind, name, size_bytes, definition, vergilius_url, scraped_at)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    """, (
        version_id,
        data["type_kind"],
        data["name"],
        data["size_bytes"],
        data["definition"],
        data["url"],
        datetime.now().isoformat(),
    ))
    type_id = cur.lastrowid

    # Delete old fields and refs
    cur.execute("DELETE FROM type_fields WHERE type_id = ?", (type_id,))
    cur.execute("DELETE FROM type_references WHERE type_id = ?", (type_id,))

    # Insert fields
    for field in data["fields"]:
        cur.execute("""
            INSERT INTO type_fields
            (type_id, offset_dec, name, field_type, bit_field, array_size)
            VALUES (?, ?, ?, ?, ?, ?)
        """, (
            type_id,
            field["offset_dec"],
            field["name"],
            field["field_type"],
            field["bit_field"],
            field["array_size"],
        ))

    # Insert references
    for ref_name in data["used_in"]:
        cur.execute("""
            INSERT INTO type_references (type_id, used_by_name)
            VALUES (?, ?)
        """, (type_id, ref_name))

    conn.commit()


def populate_critical_offsets(conn: sqlite3.Connection):
    """Populate critical_offsets table with hypervisor-relevant offsets."""
    cur = conn.cursor()

    # Define critical offset mappings
    CRITICAL_FIELDS = [
        ("_EPROCESS", "UniqueProcessId", "Process ID for filtering"),
        ("_EPROCESS", "ActiveProcessLinks", "Process list walking"),
        ("_EPROCESS", "DirectoryTableBase", "CR3 for EPT switching"),
        ("_EPROCESS", "ImageFileName", "Process name matching"),
        ("_EPROCESS", "Peb", "PEB access"),
        ("_EPROCESS", "Token", "Token manipulation"),
        ("_EPROCESS", "VadRoot", "VAD tree walking"),
        ("_EPROCESS", "ObjectTable", "Handle table access"),
        ("_KPROCESS", "DirectoryTableBase", "CR3 value"),
        ("_KPROCESS", "ThreadListHead", "Thread enumeration"),
        ("_ETHREAD", "Cid", "Thread ID"),
        ("_ETHREAD", "ThreadListEntry", "Thread list walking"),
        ("_PEB", "Ldr", "Module list"),
        ("_PEB", "ImageBaseAddress", "Base address"),
        ("_LDR_DATA_TABLE_ENTRY", "DllBase", "Module base"),
        ("_LDR_DATA_TABLE_ENTRY", "InLoadOrderLinks", "Module list"),
        ("_LDR_DATA_TABLE_ENTRY", "BaseDllName", "Module name"),
        ("_MDL", "MappedSystemVa", "Kernel VA mapping"),
        ("_MDL", "StartVa", "Original VA"),
        ("_MDL", "ByteCount", "Size"),
        ("_POOL_HEADER", "PoolTag", "Tag for detection"),
        ("_POOL_HEADER", "BlockSize", "Allocation size"),
        ("_DRIVER_OBJECT", "DriverStart", "Driver base address"),
        ("_DRIVER_OBJECT", "DriverSize", "Driver size"),
        ("_DRIVER_OBJECT", "DriverSection", "LDR entry"),
        ("_DRIVER_OBJECT", "MajorFunction", "IRP handlers"),
        ("_OBJECT_HEADER", "Body", "Object body offset"),
        ("_OBJECT_HEADER", "TypeIndex", "Object type"),
        ("_HANDLE_TABLE_ENTRY", "ObjectPointerBits", "Object pointer"),
        ("_KPCR", "Prcb", "PRCB access"),
        ("_KPCR", "CurrentThread", "Current ETHREAD"),
        ("_KPRCB", "CurrentThread", "Current thread"),
        ("_CONTEXT", "Rip", "Instruction pointer"),
        ("_CONTEXT", "Rsp", "Stack pointer"),
    ]

    for version in cur.execute("SELECT id, short_name FROM os_versions").fetchall():
        version_id = version["id"]

        for struct_name, field_name, use_case in CRITICAL_FIELDS:
            # Find the field offset
            result = cur.execute("""
                SELECT tf.offset_dec
                FROM type_fields tf
                JOIN type_definitions td ON tf.type_id = td.id
                WHERE td.version_id = ? AND td.name = ? AND tf.name = ?
            """, (version_id, struct_name, field_name)).fetchone()

            if result:
                offset_dec = result["offset_dec"]
                offset_hex = f"0x{offset_dec:x}"

                cur.execute("""
                    INSERT OR REPLACE INTO critical_offsets
                    (version_id, struct_name, field_name, offset_dec, offset_hex, use_case)
                    VALUES (?, ?, ?, ?, ?, ?)
                """, (version_id, struct_name, field_name, offset_dec, offset_hex, use_case))

    conn.commit()
    print("[+] Populated critical offsets")


def main():
    parser = argparse.ArgumentParser(description="Scrape Vergilius Project structures")
    parser.add_argument("--struct", help="Scrape single structure")
    parser.add_argument("--version", help="Scrape single version (e.g., win10-22h2)")
    parser.add_argument("--list", action="store_true", help="List priority structures")
    args = parser.parse_args()

    if args.list:
        print("Priority structures:")
        for s in PRIORITY_STRUCTURES:
            print(f"  {s}")
        return

    conn = get_db_connection()
    cur = conn.cursor()

    # Get versions to scrape
    if args.version:
        versions = cur.execute(
            "SELECT id, short_name, vergilius_path FROM os_versions WHERE short_name = ?",
            (args.version,)
        ).fetchall()
    else:
        versions = cur.execute(
            "SELECT id, short_name, vergilius_path FROM os_versions"
        ).fetchall()

    if not versions:
        print("[!] No versions found")
        return

    # Get structures to scrape
    structures = [args.struct] if args.struct else PRIORITY_STRUCTURES

    total = len(versions) * len(structures)
    scraped = 0
    failed = 0

    print(f"[*] Scraping {len(structures)} structures across {len(versions)} versions")
    print(f"[*] Rate limit: {RATE_LIMIT}s between requests")
    print(f"[*] Estimated time: ~{(total * RATE_LIMIT) / 60:.1f} minutes")
    print()

    for version in versions:
        print(f"[*] {version['short_name']} ({version['vergilius_path']})")

        for struct_name in structures:
            try:
                data = scrape_structure(version["vergilius_path"], struct_name)
                if data:
                    save_structure(conn, version["id"], data)
                    scraped += 1
                    print(f"        Size: {data['size_bytes']} bytes, {len(data['fields'])} fields")
                else:
                    failed += 1
            except Exception as e:
                print(f"    [!] Error scraping {struct_name}: {e}")
                failed += 1

        print()

    # Populate critical offsets after all scraping
    populate_critical_offsets(conn)

    conn.close()

    print(f"[+] Done! Scraped: {scraped}, Failed/Missing: {failed}")


if __name__ == "__main__":
    main()
