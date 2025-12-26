"""
SDM Query Tools - Intel SDM Natural Language Search and Reference
"""

import sqlite3
import json
from pathlib import Path
from typing import Optional

DATA_DIR = Path(__file__).parent.parent / "data"
DB_PATH = DATA_DIR / "intel_sdm.db"


def get_db():
    """Get database connection."""
    return sqlite3.connect(DB_PATH)


def ask_sdm(question: str) -> dict:
    """
    Natural language search of Intel SDM content.
    Returns relevant sections with SDM references.
    """
    conn = get_db()
    results = {"question": question, "answers": [], "references": []}

    # Extract keywords
    keywords = question.lower().split()

    # Search VMCS fields
    for kw in keywords:
        cursor = conn.execute(
            "SELECT name, encoding, category, description FROM vmcs_fields WHERE name LIKE ? OR description LIKE ?",
            (f"%{kw}%", f"%{kw}%")
        )
        for row in cursor:
            results["answers"].append({
                "type": "vmcs_field",
                "name": row[0],
                "encoding": hex(row[1]),
                "category": row[2],
                "description": row[3]
            })

    # Search exit reasons
    for kw in keywords:
        cursor = conn.execute(
            "SELECT reason_number, name, description, handling_notes FROM exit_reasons WHERE name LIKE ? OR description LIKE ?",
            (f"%{kw}%", f"%{kw}%")
        )
        for row in cursor:
            results["answers"].append({
                "type": "exit_reason",
                "reason": row[0],
                "name": row[1],
                "description": row[2],
                "handling": row[3]
            })

    # Search MSRs
    for kw in keywords:
        cursor = conn.execute(
            "SELECT name, address, category, description FROM msrs WHERE name LIKE ? OR description LIKE ?",
            (f"%{kw}%", f"%{kw}%")
        )
        for row in cursor:
            results["answers"].append({
                "type": "msr",
                "name": row[0],
                "address": hex(row[1]),
                "category": row[2],
                "description": row[3]
            })

    # Search VMX controls
    for kw in keywords:
        cursor = conn.execute(
            "SELECT control_type, bit_position, name, description FROM vmx_controls WHERE name LIKE ? OR description LIKE ?",
            (f"%{kw}%", f"%{kw}%")
        )
        for row in cursor:
            results["answers"].append({
                "type": "vmx_control",
                "control_type": row[0],
                "bit": row[1],
                "name": row[2],
                "description": row[3]
            })

    conn.close()
    return results


def vmcs_field_complete(field_name: str) -> dict:
    """
    Get complete information about a VMCS field.
    """
    conn = get_db()

    cursor = conn.execute(
        "SELECT name, encoding, width, category, subcategory, description, sdm_table FROM vmcs_fields WHERE name LIKE ?",
        (f"%{field_name}%",)
    )

    results = []
    for row in cursor:
        field = {
            "name": row[0],
            "encoding": hex(row[1]),
            "encoding_decimal": row[1],
            "width": row[2],
            "category": row[3],
            "subcategory": row[4],
            "description": row[5],
            "sdm_table": row[6],
            "code_example": f"__vmx_vmread({hex(row[1])}, &value);  // {row[0]}",
            "write_example": f"__vmx_vmwrite({hex(row[1])}, value);  // {row[0]}"
        }
        results.append(field)

    conn.close()
    return {"fields": results, "count": len(results)}


def exit_reason_complete(reason: int) -> dict:
    """
    Get complete information about a VM-exit reason.
    """
    conn = get_db()

    cursor = conn.execute(
        "SELECT reason_number, name, has_qualification, qualification_format, description, handling_notes, sdm_section FROM exit_reasons WHERE reason_number = ?",
        (reason,)
    )

    row = cursor.fetchone()
    if not row:
        conn.close()
        return {"error": f"Exit reason {reason} not found"}

    result = {
        "reason_number": row[0],
        "name": row[1],
        "has_qualification": bool(row[2]),
        "qualification_format": row[3],
        "description": row[4],
        "handling_notes": row[5],
        "sdm_section": row[6]
    }

    # Get qualification bit fields if applicable
    if row[2]:
        qual_cursor = conn.execute(
            "SELECT bit_start, bit_end, field_name, description, field_values FROM exit_qualifications WHERE exit_reason = ?",
            (reason,)
        )
        result["qualification_fields"] = []
        for qrow in qual_cursor:
            result["qualification_fields"].append({
                "bits": f"{qrow[0]}:{qrow[1]}",
                "name": qrow[2],
                "description": qrow[3],
                "values": qrow[4]
            })

    conn.close()
    return result


def get_msr_info(msr_name_or_addr) -> dict:
    """
    Get MSR information by name or address.
    """
    conn = get_db()

    if isinstance(msr_name_or_addr, int) or msr_name_or_addr.startswith("0x"):
        addr = int(msr_name_or_addr, 16) if isinstance(msr_name_or_addr, str) else msr_name_or_addr
        cursor = conn.execute(
            "SELECT name, address, category, description, bit_fields FROM msrs WHERE address = ?",
            (addr,)
        )
    else:
        cursor = conn.execute(
            "SELECT name, address, category, description, bit_fields FROM msrs WHERE name LIKE ?",
            (f"%{msr_name_or_addr}%",)
        )

    results = []
    for row in cursor:
        results.append({
            "name": row[0],
            "address": hex(row[1]),
            "category": row[2],
            "description": row[3],
            "bit_fields": json.loads(row[4]) if row[4] else None
        })

    conn.close()
    return {"msrs": results}


def get_exception_info(vector: int) -> dict:
    """
    Get exception vector information.
    """
    conn = get_db()

    cursor = conn.execute(
        "SELECT vector, mnemonic, name, type, error_code, description, source FROM exceptions WHERE vector = ?",
        (vector,)
    )

    row = cursor.fetchone()
    conn.close()

    if not row:
        return {"error": f"Exception vector {vector} not found"}

    return {
        "vector": row[0],
        "mnemonic": row[1],
        "name": row[2],
        "type": row[3],
        "has_error_code": bool(row[4]),
        "description": row[5],
        "source": row[6]
    }


def list_vmcs_by_category(category: str) -> dict:
    """
    List all VMCS fields in a category.
    Categories: control, guest_state, host_state, exit_info
    """
    conn = get_db()

    cursor = conn.execute(
        "SELECT name, encoding, width, subcategory, description FROM vmcs_fields WHERE category = ? ORDER BY encoding",
        (category,)
    )

    fields = []
    for row in cursor:
        fields.append({
            "name": row[0],
            "encoding": hex(row[1]),
            "width": row[2],
            "subcategory": row[3],
            "description": row[4]
        })

    conn.close()
    return {"category": category, "fields": fields, "count": len(fields)}


def get_vmx_control_bits(control_type: str) -> dict:
    """
    Get all bits for a VMX control type.
    Types: pin_based, proc_based, proc_based2, exit, entry
    """
    conn = get_db()

    cursor = conn.execute(
        "SELECT bit_position, name, description, default_setting, capability_msr FROM vmx_controls WHERE control_type = ? ORDER BY bit_position",
        (control_type,)
    )

    bits = []
    for row in cursor:
        bits.append({
            "bit": row[0],
            "name": row[1],
            "description": row[2],
            "default": row[3],
            "capability_msr": row[4]
        })

    conn.close()
    return {"control_type": control_type, "bits": bits}
