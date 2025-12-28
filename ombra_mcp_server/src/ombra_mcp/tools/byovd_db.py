"""
BYOVD (Bring Your Own Vulnerable Driver) Database Tools

Query and manage vulnerable driver information, IOCTLs, magic values, and blocklist status.
"""

import sqlite3
from pathlib import Path
from typing import List, Dict, Any, Optional
from datetime import datetime

DATA_DIR = Path(__file__).parent.parent / "data"
DB_PATH = DATA_DIR / "byovd_drivers.db"


def _get_conn():
    """Get database connection."""
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn


def get_drivers() -> List[Dict[str, Any]]:
    """Get list of all vulnerable drivers."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT d.*,
               COUNT(DISTINCT p.id) as primitive_count,
               COUNT(DISTINCT i.id) as ioctl_count
        FROM drivers d
        LEFT JOIN primitives p ON d.id = p.driver_id
        LEFT JOIN ioctls i ON d.id = i.driver_id
        GROUP BY d.id
        ORDER BY d.name
    """)

    results = [dict(row) for row in c.fetchall()]
    conn.close()
    return results


def get_driver(name: str) -> Optional[Dict[str, Any]]:
    """Get full details for a driver including primitives, IOCTLs, magic values."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("SELECT * FROM drivers WHERE LOWER(name) = LOWER(?)", (name,))
    row = c.fetchone()
    if not row:
        conn.close()
        return None

    result = dict(row)
    driver_id = result['id']

    # Get primitives
    c.execute("SELECT * FROM primitives WHERE driver_id = ?", (driver_id,))
    result['primitives'] = [dict(r) for r in c.fetchall()]

    # Get IOCTLs
    c.execute("SELECT * FROM ioctls WHERE driver_id = ? ORDER BY function_number", (driver_id,))
    result['ioctls'] = [dict(r) for r in c.fetchall()]

    # Get magic values
    c.execute("SELECT * FROM magic_values WHERE driver_id = ?", (driver_id,))
    result['magic_values'] = {r['value_name']: dict(r) for r in c.fetchall()}

    # Get blocklist status
    c.execute("SELECT * FROM blocklist_status WHERE driver_id = ?", (driver_id,))
    result['blocklist'] = {r['anticheat']: dict(r) for r in c.fetchall()}

    # Get gotchas
    c.execute("SELECT * FROM driver_gotchas WHERE driver_id = ?", (driver_id,))
    result['gotchas'] = [dict(r) for r in c.fetchall()]

    conn.close()
    return result


def get_driver_primitives(name: str) -> List[Dict[str, Any]]:
    """Get all primitives a driver provides."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT p.* FROM primitives p
        JOIN drivers d ON p.driver_id = d.id
        WHERE LOWER(d.name) = LOWER(?)
        ORDER BY p.primitive_type
    """, (name,))

    results = [dict(row) for row in c.fetchall()]
    conn.close()
    return results


def get_driver_ioctls(name: str) -> List[Dict[str, Any]]:
    """Get all IOCTLs for a driver."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT i.* FROM ioctls i
        JOIN drivers d ON i.driver_id = d.id
        WHERE LOWER(d.name) = LOWER(?)
        ORDER BY i.function_number
    """, (name,))

    results = [dict(row) for row in c.fetchall()]
    conn.close()
    return results


def get_magic_values(name: str) -> Dict[str, Any]:
    """Get all magic values for a driver."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT m.* FROM magic_values m
        JOIN drivers d ON m.driver_id = d.id
        WHERE LOWER(d.name) = LOWER(?)
    """, (name,))

    results = {r['value_name']: dict(r) for r in c.fetchall()}
    conn.close()
    return results


def check_blocklist_status(name: str) -> Dict[str, Any]:
    """Check blocklist status for a driver across all anti-cheats."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT b.* FROM blocklist_status b
        JOIN drivers d ON b.driver_id = d.id
        WHERE LOWER(d.name) = LOWER(?)
    """, (name,))

    results = {}
    for row in c.fetchall():
        r = dict(row)
        results[r['anticheat']] = {
            "blocked": r['blocked'],
            "block_type": r['block_type'],
            "first_blocked": r['first_blocked'],
            "last_checked": r['last_checked'],
            "notes": r['notes'],
        }

    conn.close()
    return results


def get_driver_gotchas(name: str) -> List[Dict[str, Any]]:
    """Get known issues/gotchas for a driver."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT g.* FROM driver_gotchas g
        JOIN drivers d ON g.driver_id = d.id
        WHERE LOWER(d.name) = LOWER(?)
    """, (name,))

    results = [dict(row) for row in c.fetchall()]
    conn.close()
    return results


def search_drivers(query: str) -> List[Dict[str, Any]]:
    """Search drivers by name, vendor, or notes."""
    conn = _get_conn()
    c = conn.cursor()

    query_pattern = f"%{query}%"
    c.execute("""
        SELECT * FROM drivers
        WHERE name LIKE ? OR original_name LIKE ? OR vendor LIKE ?
              OR signer LIKE ? OR notes LIKE ?
        ORDER BY name
    """, (query_pattern, query_pattern, query_pattern, query_pattern, query_pattern))

    results = [dict(row) for row in c.fetchall()]
    conn.close()
    return results


def find_drivers_with_primitive(primitive_type: str) -> List[Dict[str, Any]]:
    """Find all drivers that provide a specific primitive."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT d.*, p.description as primitive_description, p.restrictions
        FROM drivers d
        JOIN primitives p ON d.id = p.driver_id
        WHERE LOWER(p.primitive_type) = LOWER(?)
        ORDER BY d.name
    """, (primitive_type,))

    results = [dict(row) for row in c.fetchall()]
    conn.close()
    return results


# Add/Update functions

def add_driver(
    name: str,
    vendor: str,
    device_path: str,
    original_name: Optional[str] = None,
    signer: Optional[str] = None,
    signer_thumbprint: Optional[str] = None,
    version: Optional[str] = None,
    file_size: Optional[int] = None,
    sha256: Optional[str] = None,
    md5: Optional[str] = None,
    legitimate_use: Optional[str] = None,
    cve: Optional[str] = None,
    public_exploit: bool = False,
    notes: Optional[str] = None,
) -> Dict[str, Any]:
    """Add a new vulnerable driver to the database."""
    conn = _get_conn()
    c = conn.cursor()

    try:
        c.execute("""
            INSERT INTO drivers
            (name, original_name, vendor, signer, signer_thumbprint, version,
             file_size, sha256, md5, device_path, legitimate_use,
             discovery_date, cve, public_exploit, notes)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            name, original_name, vendor, signer, signer_thumbprint, version,
            file_size, sha256, md5, device_path, legitimate_use,
            datetime.now().isoformat(), cve, public_exploit, notes
        ))
        conn.commit()
        new_id = c.lastrowid
        conn.close()
        return {"success": True, "id": new_id, "name": name}
    except sqlite3.IntegrityError as e:
        conn.close()
        return {"error": f"Driver already exists: {name}"}


def add_primitive(
    driver: str,
    primitive_type: str,
    description: str,
    restrictions: Optional[str] = None,
    requires_auth: bool = False,
) -> Dict[str, Any]:
    """Add a primitive to a driver."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("SELECT id FROM drivers WHERE LOWER(name) = LOWER(?)", (driver,))
    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Unknown driver: {driver}"}

    driver_id = row['id']

    try:
        c.execute("""
            INSERT INTO primitives
            (driver_id, primitive_type, description, restrictions, requires_auth)
            VALUES (?, ?, ?, ?, ?)
        """, (driver_id, primitive_type, description, restrictions, requires_auth))
        conn.commit()
        new_id = c.lastrowid
        conn.close()
        return {"success": True, "id": new_id}
    except sqlite3.IntegrityError:
        conn.close()
        return {"error": f"Primitive already exists: {primitive_type}"}


def add_ioctl(
    driver: str,
    name: str,
    code: str,
    description: str,
    function_number: Optional[int] = None,
    input_size: Optional[int] = None,
    output_size: Optional[int] = None,
    input_struct: Optional[str] = None,
    output_struct: Optional[str] = None,
    requires_session: bool = True,
    notes: Optional[str] = None,
) -> Dict[str, Any]:
    """Add an IOCTL to a driver."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("SELECT id FROM drivers WHERE LOWER(name) = LOWER(?)", (driver,))
    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Unknown driver: {driver}"}

    driver_id = row['id']

    # Parse code to decimal
    code_decimal = None
    if code.startswith("0x"):
        code_decimal = int(code, 16)
    elif code.isdigit():
        code_decimal = int(code)

    try:
        c.execute("""
            INSERT INTO ioctls
            (driver_id, name, code, code_decimal, function_number,
             description, input_struct, output_struct, input_size, output_size,
             requires_session, notes)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            driver_id, name, code, code_decimal, function_number,
            description, input_struct, output_struct, input_size, output_size,
            requires_session, notes
        ))
        conn.commit()
        new_id = c.lastrowid
        conn.close()
        return {"success": True, "id": new_id, "name": name}
    except sqlite3.IntegrityError:
        conn.close()
        return {"error": f"IOCTL already exists: {name}"}


def add_magic_value(
    driver: str,
    value_name: str,
    description: str,
    value_hex: Optional[str] = None,
    value_string: Optional[str] = None,
    binary_offset: Optional[str] = None,
) -> Dict[str, Any]:
    """Add a magic value to a driver."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("SELECT id FROM drivers WHERE LOWER(name) = LOWER(?)", (driver,))
    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Unknown driver: {driver}"}

    driver_id = row['id']

    try:
        c.execute("""
            INSERT INTO magic_values
            (driver_id, value_name, value_hex, value_string, binary_offset, description)
            VALUES (?, ?, ?, ?, ?, ?)
        """, (driver_id, value_name, value_hex, value_string, binary_offset, description))
        conn.commit()
        new_id = c.lastrowid
        conn.close()
        return {"success": True, "id": new_id}
    except sqlite3.IntegrityError:
        conn.close()
        return {"error": f"Magic value already exists: {value_name}"}


def update_blocklist_status(
    driver: str,
    anticheat: str,
    blocked: bool,
    block_type: Optional[str] = None,
    notes: Optional[str] = None,
) -> Dict[str, Any]:
    """Update blocklist status for a driver."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("SELECT id FROM drivers WHERE LOWER(name) = LOWER(?)", (driver,))
    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Unknown driver: {driver}"}

    driver_id = row['id']
    now = datetime.now().isoformat()

    first_blocked = now if blocked else None

    c.execute("""
        INSERT OR REPLACE INTO blocklist_status
        (driver_id, anticheat, blocked, block_type, first_blocked, last_checked, notes)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    """, (driver_id, anticheat, blocked, block_type, first_blocked, now, notes))

    conn.commit()
    conn.close()
    return {"success": True, "driver": driver, "anticheat": anticheat, "blocked": blocked}


def add_driver_gotcha(
    driver: str,
    symptom: str,
    cause: str,
    fix: Optional[str] = None,
    affected_environments: Optional[str] = None,
) -> Dict[str, Any]:
    """Add a known issue/gotcha for a driver."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("SELECT id FROM drivers WHERE LOWER(name) = LOWER(?)", (driver,))
    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Unknown driver: {driver}"}

    driver_id = row['id']

    c.execute("""
        INSERT INTO driver_gotchas
        (driver_id, symptom, cause, fix, affected_environments)
        VALUES (?, ?, ?, ?, ?)
    """, (driver_id, symptom, cause, fix, affected_environments))

    conn.commit()
    new_id = c.lastrowid
    conn.close()
    return {"success": True, "id": new_id}
