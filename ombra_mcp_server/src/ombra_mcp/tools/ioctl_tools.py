"""
IOCTL Management Tools

Tools for adding, querying, and managing driver IOCTL handlers and vulnerabilities.
"""

import json
import uuid
from typing import Dict, List, Optional, Any

from .driver_re_db import get_conn


def _parse_ioctl_code(code: int) -> Dict[str, int]:
    """
    Parse IOCTL code into CTL_CODE components.

    CTL_CODE macro format:
    ((device_type) << 16) | ((access) << 14) | ((function) << 2) | (method)
    """
    device_type = (code >> 16) & 0xFFFF
    access = (code >> 14) & 0x3
    function_code = (code >> 2) & 0xFFF
    method = code & 0x3

    return {
        'device_type': device_type,
        'function_code': function_code,
        'method': method,
        'access': access
    }


async def add_ioctl(
    driver_id: str,
    name: str,
    code: Optional[int] = None,
    description: Optional[str] = None,
    handler_rva: Optional[int] = None,
    input_struct: Optional[str] = None,
    output_struct: Optional[str] = None,
    min_input_size: Optional[int] = None,
    max_input_size: Optional[int] = None,
    min_output_size: Optional[int] = None,
    max_output_size: Optional[int] = None,
    requires_admin: bool = False,
    is_vulnerable: bool = False,
    vulnerability_type: Optional[str] = None,
    vulnerability_severity: Optional[str] = None,
    vulnerability_description: Optional[str] = None,
    exploitation_notes: Optional[str] = None
) -> Dict[str, Any]:
    """
    Add an IOCTL to a driver.

    Automatically parses IOCTL code into components and generates embedding.
    """
    conn = get_conn()
    c = conn.cursor()

    # Verify driver exists
    c.execute("SELECT id, original_name FROM drivers WHERE id = ?", (driver_id,))
    driver = c.fetchone()
    if not driver:
        conn.close()
        return {"error": f"Driver not found: {driver_id}"}

    # Parse IOCTL code
    code_components = _parse_ioctl_code(code) if code else {}

    ioctl_id = str(uuid.uuid4())
    code_hex = f"0x{code:08X}" if code else None

    c.execute("""
        INSERT INTO ioctls (
            id, driver_id, name, code, code_hex,
            device_type, function_code, method, access,
            handler_rva, description,
            min_input_size, max_input_size,
            min_output_size, max_output_size,
            requires_admin, is_vulnerable,
            vulnerability_type, vulnerability_severity,
            vulnerability_description, exploitation_notes
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    """, (
        ioctl_id, driver_id, name, code, code_hex,
        code_components.get('device_type'),
        code_components.get('function_code'),
        code_components.get('method'),
        code_components.get('access'),
        handler_rva, description,
        min_input_size, max_input_size,
        min_output_size, max_output_size,
        1 if requires_admin else 0,
        1 if is_vulnerable else 0,
        vulnerability_type, vulnerability_severity,
        vulnerability_description, exploitation_notes
    ))

    conn.commit()
    conn.close()

    return {
        "success": True,
        "ioctl_id": ioctl_id,
        "driver_name": driver['original_name'],
        "ioctl_name": name,
        "code": code_hex if code else None,
        "components": code_components if code else None,
        "is_vulnerable": is_vulnerable,
        "message": f"IOCTL '{name}' added successfully"
    }


async def get_ioctl(
    ioctl_id: Optional[str] = None,
    driver_id: Optional[str] = None,
    name: Optional[str] = None,
    code: Optional[int] = None
) -> Optional[Dict[str, Any]]:
    """
    Get IOCTL details including structures, handler info, and vulnerability status.
    """
    conn = get_conn()
    c = conn.cursor()

    if ioctl_id:
        c.execute("SELECT * FROM ioctls WHERE id = ?", (ioctl_id,))
    elif driver_id and name:
        c.execute("SELECT * FROM ioctls WHERE driver_id = ? AND name = ?", (driver_id, name))
    elif driver_id and code:
        c.execute("SELECT * FROM ioctls WHERE driver_id = ? AND code = ?", (driver_id, code))
    else:
        conn.close()
        return {"error": "Must provide ioctl_id, or (driver_id + name/code)"}

    row = c.fetchone()
    if not row:
        conn.close()
        return None

    result = dict(row)
    ioctl_id = result['id']

    # Get xrefs to handler
    if result['handler_rva']:
        c.execute("""
            SELECT * FROM xrefs
            WHERE driver_id = ? AND to_rva = ?
            ORDER BY from_rva
        """, (result['driver_id'], result['handler_rva']))
        result['handler_xrefs'] = [dict(r) for r in c.fetchall()]

    # Get driver name
    c.execute("SELECT original_name FROM drivers WHERE id = ?", (result['driver_id'],))
    result['driver_name'] = c.fetchone()['original_name']

    # Parse CVE IDs if present
    if result.get('cve_ids'):
        result['cve_ids'] = json.loads(result['cve_ids'])

    conn.close()
    return result


async def list_ioctls(
    driver_id: str,
    vulnerable_only: bool = False,
    category: Optional[str] = None
) -> List[Dict[str, Any]]:
    """
    List all IOCTLs for a driver.
    """
    conn = get_conn()
    c = conn.cursor()

    query = "SELECT * FROM ioctls WHERE driver_id = ?"
    params = [driver_id]

    if vulnerable_only:
        query += " AND is_vulnerable = 1"

    if category:
        query += " AND vulnerability_type = ?"
        params.append(category)

    query += " ORDER BY function_code"

    c.execute(query, params)
    results = [dict(row) for row in c.fetchall()]

    # Parse CVE IDs for each IOCTL
    for result in results:
        if result.get('cve_ids'):
            result['cve_ids'] = json.loads(result['cve_ids'])

    conn.close()
    return results


async def get_vulnerable_ioctls(
    driver_id: Optional[str] = None,
    severity: Optional[str] = None,
    vulnerability_type: Optional[str] = None
) -> List[Dict[str, Any]]:
    """
    Get all vulnerable IOCTLs, optionally filtered by driver/severity/type.
    """
    conn = get_conn()
    c = conn.cursor()

    query = """
        SELECT i.*, d.original_name as driver_name
        FROM ioctls i
        JOIN drivers d ON i.driver_id = d.id
        WHERE i.is_vulnerable = 1
    """
    params = []

    if driver_id:
        query += " AND i.driver_id = ?"
        params.append(driver_id)

    if severity:
        query += " AND i.vulnerability_severity = ?"
        params.append(severity)

    if vulnerability_type:
        query += " AND i.vulnerability_type = ?"
        params.append(vulnerability_type)

    query += " ORDER BY CASE i.vulnerability_severity "
    query += "WHEN 'critical' THEN 1 WHEN 'high' THEN 2 WHEN 'medium' THEN 3 WHEN 'low' THEN 4 ELSE 5 END, "
    query += "d.original_name, i.function_code"

    c.execute(query, params)
    results = [dict(row) for row in c.fetchall()]

    # Parse CVE IDs
    for result in results:
        if result.get('cve_ids'):
            result['cve_ids'] = json.loads(result['cve_ids'])

    conn.close()
    return results


async def update_ioctl_vulnerability(
    ioctl_id: str,
    is_vulnerable: bool,
    vulnerability_type: Optional[str] = None,
    vulnerability_severity: Optional[str] = None,
    vulnerability_description: Optional[str] = None,
    exploitation_notes: Optional[str] = None,
    cve_ids: Optional[List[str]] = None
) -> Dict[str, Any]:
    """
    Update IOCTL vulnerability status and details.
    """
    conn = get_conn()
    c = conn.cursor()

    # Verify IOCTL exists
    c.execute("SELECT i.id, i.name, d.original_name as driver_name FROM ioctls i "
              "JOIN drivers d ON i.driver_id = d.id WHERE i.id = ?", (ioctl_id,))
    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"IOCTL not found: {ioctl_id}"}

    # Build update query
    update_fields = ["is_vulnerable = ?"]
    params = [1 if is_vulnerable else 0]

    if vulnerability_type is not None:
        update_fields.append("vulnerability_type = ?")
        params.append(vulnerability_type)

    if vulnerability_severity is not None:
        valid_severities = ['critical', 'high', 'medium', 'low']
        if vulnerability_severity not in valid_severities:
            conn.close()
            return {"error": f"Invalid severity. Must be one of: {', '.join(valid_severities)}"}
        update_fields.append("vulnerability_severity = ?")
        params.append(vulnerability_severity)

    if vulnerability_description is not None:
        update_fields.append("vulnerability_description = ?")
        params.append(vulnerability_description)

    if exploitation_notes is not None:
        update_fields.append("exploitation_notes = ?")
        params.append(exploitation_notes)

    if cve_ids is not None:
        update_fields.append("cve_ids = ?")
        params.append(json.dumps(cve_ids))

    params.append(ioctl_id)

    c.execute(f"UPDATE ioctls SET {', '.join(update_fields)} WHERE id = ?", params)
    conn.commit()
    conn.close()

    return {
        "success": True,
        "ioctl_id": ioctl_id,
        "ioctl_name": row['name'],
        "driver_name": row['driver_name'],
        "is_vulnerable": is_vulnerable,
        "vulnerability_type": vulnerability_type,
        "vulnerability_severity": vulnerability_severity,
        "message": f"IOCTL vulnerability status updated"
    }
