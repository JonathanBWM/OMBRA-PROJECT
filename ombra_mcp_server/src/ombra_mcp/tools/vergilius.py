"""
Vergilius Project Integration - Windows Kernel Structure Reference

Provides version-specific Windows kernel structure offsets for hypervisor development.
Data sourced from https://www.vergiliusproject.com/
"""

import sqlite3
from pathlib import Path
from typing import Any

DATA_DIR = Path(__file__).parent.parent / "data"
VERGILIUS_DB = DATA_DIR / "vergilius.db"


def get_db() -> sqlite3.Connection:
    """Get database connection."""
    conn = sqlite3.connect(VERGILIUS_DB)
    conn.row_factory = sqlite3.Row
    return conn


def resolve_version(version: str) -> dict | None:
    """Resolve version string to database record."""
    conn = get_db()
    cur = conn.cursor()

    # Try exact match first
    result = cur.execute(
        "SELECT * FROM os_versions WHERE short_name = ?", (version,)
    ).fetchone()

    if not result:
        # Try partial match
        result = cur.execute(
            "SELECT * FROM os_versions WHERE short_name LIKE ?",
            (f"%{version}%",)
        ).fetchone()

    conn.close()
    return dict(result) if result else None


async def get_structure(name: str, version: str = "win10-22h2") -> dict[str, Any]:
    """
    Get complete structure definition with all fields.

    Args:
        name: Structure name (e.g., "_EPROCESS", "_MDL")
        version: OS version (e.g., "win10-22h2", "win11-21h2")

    Returns:
        Complete structure info with fields and references
    """
    conn = get_db()
    cur = conn.cursor()

    # Resolve version
    ver = resolve_version(version)
    if not ver:
        conn.close()
        return {"error": f"Unknown version: {version}", "available_versions": _list_versions()}

    # Get structure
    struct = cur.execute("""
        SELECT * FROM type_definitions
        WHERE version_id = ? AND name = ?
    """, (ver["id"], name)).fetchone()

    if not struct:
        conn.close()
        return {"error": f"Structure {name} not found for {version}"}

    # Get fields
    fields = cur.execute("""
        SELECT offset_dec, name, field_type, bit_field, array_size
        FROM type_fields
        WHERE type_id = ?
        ORDER BY offset_dec
    """, (struct["id"],)).fetchall()

    # Get references
    refs = cur.execute("""
        SELECT used_by_name FROM type_references
        WHERE type_id = ?
    """, (struct["id"],)).fetchall()

    conn.close()

    return {
        "name": struct["name"],
        "type_kind": struct["type_kind"],
        "size": struct["size_bytes"],
        "size_hex": f"0x{struct['size_bytes']:x}" if struct["size_bytes"] else None,
        "version": f"{ver['os_family']} {ver['version']} ({ver['codename']})",
        "version_short": ver["short_name"],
        "fields": [
            {
                "offset": f["offset_dec"],
                "offset_hex": f"0x{f['offset_dec']:x}",
                "name": f["name"],
                "type": f["field_type"],
                "bit_field": f["bit_field"],
                "array_size": f["array_size"],
            }
            for f in fields
        ],
        "used_by": [r["used_by_name"] for r in refs],
        "field_count": len(fields),
    }


async def get_field_offset(struct: str, field: str, version: str = "win10-22h2") -> dict[str, Any]:
    """
    Get offset of a specific field.

    Args:
        struct: Structure name (e.g., "_EPROCESS")
        field: Field name (e.g., "UniqueProcessId")
        version: OS version

    Returns:
        Field offset details
    """
    conn = get_db()
    cur = conn.cursor()

    ver = resolve_version(version)
    if not ver:
        conn.close()
        return {"error": f"Unknown version: {version}"}

    result = cur.execute("""
        SELECT tf.offset_dec, tf.field_type, tf.bit_field, tf.array_size
        FROM type_fields tf
        JOIN type_definitions td ON tf.type_id = td.id
        WHERE td.version_id = ? AND td.name = ? AND tf.name = ?
    """, (ver["id"], struct, field)).fetchone()

    conn.close()

    if not result:
        return {"error": f"Field {struct}.{field} not found for {version}"}

    return {
        "struct": struct,
        "field": field,
        "offset": result["offset_dec"],
        "offset_hex": f"0x{result['offset_dec']:x}",
        "type": result["field_type"],
        "bit_field": result["bit_field"],
        "array_size": result["array_size"],
        "version": ver["short_name"],
        "c_access": f"*(({result['field_type']}*)((ULONG_PTR){struct.lower()} + 0x{result['offset_dec']:x}))",
    }


async def compare_versions(struct: str, field: str = None) -> dict[str, Any]:
    """
    Compare structure/field across all tracked versions.

    Args:
        struct: Structure name
        field: Optional field name (if None, compares struct sizes)

    Returns:
        Comparison across versions
    """
    conn = get_db()
    cur = conn.cursor()

    versions = cur.execute("SELECT * FROM os_versions ORDER BY id").fetchall()

    if field:
        # Compare specific field
        comparison = {}
        for ver in versions:
            result = cur.execute("""
                SELECT tf.offset_dec, tf.field_type
                FROM type_fields tf
                JOIN type_definitions td ON tf.type_id = td.id
                WHERE td.version_id = ? AND td.name = ? AND tf.name = ?
            """, (ver["id"], struct, field)).fetchone()

            if result:
                comparison[ver["short_name"]] = {
                    "offset": result["offset_dec"],
                    "offset_hex": f"0x{result['offset_dec']:x}",
                    "type": result["field_type"],
                }

        conn.close()

        # Calculate drift
        offsets = [v["offset"] for v in comparison.values()]
        drift = max(offsets) - min(offsets) if offsets else 0

        return {
            "struct": struct,
            "field": field,
            "versions": comparison,
            "drift": drift,
            "drift_hex": f"0x{drift:x}",
            "note": "Offsets differ!" if drift > 0 else "Consistent across versions",
        }
    else:
        # Compare struct sizes
        comparison = {}
        for ver in versions:
            result = cur.execute("""
                SELECT size_bytes FROM type_definitions
                WHERE version_id = ? AND name = ?
            """, (ver["id"], struct)).fetchone()

            if result and result["size_bytes"]:
                comparison[ver["short_name"]] = {
                    "size": result["size_bytes"],
                    "size_hex": f"0x{result['size_bytes']:x}",
                }

        conn.close()

        sizes = [v["size"] for v in comparison.values()]
        drift = max(sizes) - min(sizes) if sizes else 0

        return {
            "struct": struct,
            "versions": comparison,
            "size_drift": drift,
            "size_drift_hex": f"0x{drift:x}",
            "note": "Size changed across versions!" if drift > 0 else "Consistent size",
        }


async def get_hypervisor_offsets(version: str = "win10-22h2") -> dict[str, Any]:
    """
    Get all critical offsets for hypervisor development.

    Args:
        version: OS version

    Returns:
        Dictionary of all hypervisor-relevant offsets
    """
    conn = get_db()
    cur = conn.cursor()

    ver = resolve_version(version)
    if not ver:
        conn.close()
        return {"error": f"Unknown version: {version}"}

    offsets = cur.execute("""
        SELECT struct_name, field_name, offset_dec, offset_hex, use_case
        FROM critical_offsets
        WHERE version_id = ?
        ORDER BY struct_name, offset_dec
    """, (ver["id"],)).fetchall()

    conn.close()

    # Group by structure
    by_struct = {}
    flat = {}

    for o in offsets:
        key = f"{o['struct_name']}_{o['field_name']}"
        flat[key] = o["offset_dec"]

        if o["struct_name"] not in by_struct:
            by_struct[o["struct_name"]] = {}
        by_struct[o["struct_name"]][o["field_name"]] = {
            "offset": o["offset_dec"],
            "offset_hex": o["offset_hex"],
            "use_case": o["use_case"],
        }

    return {
        "version": f"{ver['os_family']} {ver['version']}",
        "version_short": ver["short_name"],
        "by_structure": by_struct,
        "flat": flat,
        "count": len(offsets),
    }


async def find_field_usage(field_type: str, version: str = "win10-22h2") -> dict[str, Any]:
    """
    Find all structures containing a specific type.

    Args:
        field_type: Type to search for (e.g., "_MDL*", "_EPROCESS*")
        version: OS version

    Returns:
        List of structures using the type
    """
    conn = get_db()
    cur = conn.cursor()

    ver = resolve_version(version)
    if not ver:
        conn.close()
        return {"error": f"Unknown version: {version}"}

    # Use LIKE for partial matching
    results = cur.execute("""
        SELECT DISTINCT td.name as struct_name, tf.name as field_name,
               tf.offset_dec, tf.field_type
        FROM type_fields tf
        JOIN type_definitions td ON tf.type_id = td.id
        WHERE td.version_id = ? AND tf.field_type LIKE ?
        ORDER BY td.name
    """, (ver["id"], f"%{field_type}%")).fetchall()

    conn.close()

    return {
        "search_type": field_type,
        "version": ver["short_name"],
        "matches": [
            {
                "struct": r["struct_name"],
                "field": r["field_name"],
                "offset": r["offset_dec"],
                "offset_hex": f"0x{r['offset_dec']:x}",
                "full_type": r["field_type"],
            }
            for r in results
        ],
        "count": len(results),
    }


async def list_structures(version: str = "win10-22h2", category: str = None) -> dict[str, Any]:
    """
    List all available structures for a version.

    Args:
        version: OS version
        category: Optional filter (not yet implemented)

    Returns:
        List of available structures
    """
    conn = get_db()
    cur = conn.cursor()

    ver = resolve_version(version)
    if not ver:
        conn.close()
        return {"error": f"Unknown version: {version}"}

    results = cur.execute("""
        SELECT name, type_kind, size_bytes
        FROM type_definitions
        WHERE version_id = ?
        ORDER BY name
    """, (ver["id"],)).fetchall()

    conn.close()

    return {
        "version": ver["short_name"],
        "structures": [
            {
                "name": r["name"],
                "type": r["type_kind"],
                "size": r["size_bytes"],
                "size_hex": f"0x{r['size_bytes']:x}" if r["size_bytes"] else None,
            }
            for r in results
        ],
        "count": len(results),
    }


async def list_versions() -> dict[str, Any]:
    """List all available Windows versions."""
    return {"versions": _list_versions()}


def _list_versions() -> list[dict]:
    """Internal version list helper."""
    conn = get_db()
    cur = conn.cursor()

    versions = cur.execute("""
        SELECT short_name, os_family, version, codename, build
        FROM os_versions ORDER BY id
    """).fetchall()

    conn.close()

    return [
        {
            "short_name": v["short_name"],
            "os": f"{v['os_family']} {v['version']}",
            "codename": v["codename"],
            "build": v["build"],
        }
        for v in versions
    ]


async def generate_offsets_header(version: str = "win10-22h2") -> str:
    """
    Generate C header with version-specific offsets.

    Args:
        version: OS version

    Returns:
        C header file content
    """
    offsets = await get_hypervisor_offsets(version)

    if "error" in offsets:
        return f"// Error: {offsets['error']}"

    lines = [
        f"// Windows Kernel Offsets - {offsets['version']}",
        f"// Generated from Vergilius Project data",
        f"// DO NOT EDIT - Regenerate with OmbraMCP",
        "",
        "#pragma once",
        "",
    ]

    for struct_name, fields in offsets["by_structure"].items():
        # Clean struct name for C define
        prefix = struct_name.lstrip("_").upper()
        lines.append(f"// {struct_name}")
        for field_name, info in fields.items():
            define_name = f"OFFSET_{prefix}_{field_name.upper()}"
            lines.append(f"#define {define_name} {info['offset_hex']}")
        lines.append("")

    return "\n".join(lines)
