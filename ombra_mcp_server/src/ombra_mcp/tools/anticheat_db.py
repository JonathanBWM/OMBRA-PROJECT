"""
Anti-Cheat Intelligence Database Tools

Query and manage anti-cheat detection methods, bypasses, and signatures.
"""

import sqlite3
from pathlib import Path
from typing import List, Dict, Any, Optional
from datetime import datetime

DATA_DIR = Path(__file__).parent.parent / "data"
DB_PATH = DATA_DIR / "anticheat_intel.db"


def _get_conn():
    """Get database connection."""
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn


def get_anticheat_list() -> List[Dict[str, Any]]:
    """Get list of all anti-cheats in the database."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT a.*, COUNT(dm.id) as detection_count
        FROM anticheats a
        LEFT JOIN detection_methods dm ON a.id = dm.anticheat_id
        GROUP BY a.id
        ORDER BY a.name
    """)

    results = [dict(row) for row in c.fetchall()]
    conn.close()
    return results


def get_detection_methods(
    anticheat: Optional[str] = None,
    category: Optional[str] = None,
    severity: Optional[str] = None,
) -> List[Dict[str, Any]]:
    """
    Get detection methods with optional filters.

    Args:
        anticheat: Filter by anti-cheat name (EAC, BattlEye, etc.)
        category: Filter by category (timing, memory, msr, behavioral)
        severity: Filter by severity (low, medium, high, critical)
    """
    conn = _get_conn()
    c = conn.cursor()

    query = """
        SELECT dm.*, a.name as anticheat_name
        FROM detection_methods dm
        JOIN anticheats a ON dm.anticheat_id = a.id
        WHERE 1=1
    """
    params = []

    if anticheat:
        query += " AND LOWER(a.name) = LOWER(?)"
        params.append(anticheat)

    if category:
        query += " AND LOWER(dm.category) = LOWER(?)"
        params.append(category)

    if severity:
        query += " AND LOWER(dm.severity) = LOWER(?)"
        params.append(severity)

    query += " ORDER BY dm.severity DESC, dm.name"

    c.execute(query, params)
    results = [dict(row) for row in c.fetchall()]
    conn.close()
    return results


def get_detection_method_detail(method_id: str) -> Optional[Dict[str, Any]]:
    """Get full details for a specific detection method including bypasses."""
    conn = _get_conn()
    c = conn.cursor()

    # Get method details
    c.execute("""
        SELECT dm.*, a.name as anticheat_name
        FROM detection_methods dm
        JOIN anticheats a ON dm.anticheat_id = a.id
        WHERE dm.method_id = ?
    """, (method_id,))

    row = c.fetchone()
    if not row:
        conn.close()
        return None

    result = dict(row)

    # Get bypasses for this method
    c.execute("""
        SELECT * FROM bypasses
        WHERE detection_method_id = ?
        ORDER BY effectiveness DESC
    """, (result['id'],))

    result['bypasses'] = [dict(r) for r in c.fetchall()]

    conn.close()
    return result


def get_bypasses_for_anticheat(anticheat: str) -> List[Dict[str, Any]]:
    """Get all bypasses for a specific anti-cheat."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT b.*, dm.method_id, dm.name as detection_name, dm.category
        FROM bypasses b
        JOIN detection_methods dm ON b.detection_method_id = dm.id
        JOIN anticheats a ON dm.anticheat_id = a.id
        WHERE LOWER(a.name) = LOWER(?)
        ORDER BY b.effectiveness DESC, b.difficulty ASC
    """, (anticheat,))

    results = [dict(row) for row in c.fetchall()]
    conn.close()
    return results


def get_timing_thresholds(anticheat: Optional[str] = None) -> List[Dict[str, Any]]:
    """Get timing thresholds for detection evasion."""
    conn = _get_conn()
    c = conn.cursor()

    if anticheat:
        c.execute("""
            SELECT t.*, a.name as anticheat_name
            FROM timing_thresholds t
            JOIN anticheats a ON t.anticheat_id = a.id
            WHERE LOWER(a.name) = LOWER(?)
            ORDER BY t.operation
        """, (anticheat,))
    else:
        c.execute("""
            SELECT t.*, a.name as anticheat_name
            FROM timing_thresholds t
            JOIN anticheats a ON t.anticheat_id = a.id
            ORDER BY a.name, t.operation
        """)

    results = [dict(row) for row in c.fetchall()]
    conn.close()
    return results


def get_signatures(anticheat: Optional[str] = None) -> List[Dict[str, Any]]:
    """Get known signatures that anti-cheats scan for."""
    conn = _get_conn()
    c = conn.cursor()

    if anticheat:
        c.execute("""
            SELECT s.*, a.name as anticheat_name
            FROM signatures s
            JOIN anticheats a ON s.anticheat_id = a.id
            WHERE LOWER(a.name) = LOWER(?)
            ORDER BY s.signature_type
        """, (anticheat,))
    else:
        c.execute("""
            SELECT s.*, a.name as anticheat_name
            FROM signatures s
            JOIN anticheats a ON s.anticheat_id = a.id
            ORDER BY a.name, s.signature_type
        """)

    results = [dict(row) for row in c.fetchall()]
    conn.close()
    return results


def search_detections(query: str) -> List[Dict[str, Any]]:
    """Full-text search across detection methods."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT dm.*, a.name as anticheat_name
        FROM detection_methods dm
        JOIN anticheats a ON dm.anticheat_id = a.id
        JOIN detection_methods_fts fts ON dm.id = fts.rowid
        WHERE detection_methods_fts MATCH ?
        ORDER BY rank
        LIMIT 20
    """, (query,))

    results = [dict(row) for row in c.fetchall()]
    conn.close()
    return results


# Add/Update functions

def add_detection_method(
    anticheat: str,
    method_id: str,
    category: str,
    name: str,
    description: str,
    technique: Optional[str] = None,
    threshold_value: Optional[float] = None,
    threshold_unit: Optional[str] = None,
    check_frequency: str = "unknown",
    severity: str = "medium",
    source: Optional[str] = None,
    source_url: Optional[str] = None,
) -> Dict[str, Any]:
    """Add a new detection method."""
    conn = _get_conn()
    c = conn.cursor()

    # Get anticheat ID
    c.execute("SELECT id FROM anticheats WHERE LOWER(name) = LOWER(?)", (anticheat,))
    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Unknown anticheat: {anticheat}"}

    anticheat_id = row['id']

    try:
        c.execute("""
            INSERT INTO detection_methods
            (anticheat_id, method_id, category, name, description, technique,
             threshold_value, threshold_unit, check_frequency, severity,
             first_seen, source, source_url)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            anticheat_id, method_id, category, name, description, technique,
            threshold_value, threshold_unit, check_frequency, severity,
            datetime.now().isoformat(), source, source_url
        ))
        conn.commit()
        new_id = c.lastrowid
        conn.close()
        return {"success": True, "id": new_id, "method_id": method_id}
    except sqlite3.IntegrityError as e:
        conn.close()
        return {"error": f"Method ID already exists: {method_id}"}


def add_bypass(
    method_id: str,
    technique: str,
    description: str,
    implementation: Optional[str] = None,
    difficulty: str = "medium",
    effectiveness: str = "tested",
    side_effects: Optional[str] = None,
    requires: Optional[str] = None,
) -> Dict[str, Any]:
    """Add a bypass technique for a detection method."""
    conn = _get_conn()
    c = conn.cursor()

    # Get detection method ID
    c.execute("SELECT id FROM detection_methods WHERE method_id = ?", (method_id,))
    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Unknown detection method: {method_id}"}

    detection_id = row['id']

    c.execute("""
        INSERT INTO bypasses
        (detection_method_id, technique, description, implementation,
         difficulty, effectiveness, side_effects, requires)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    """, (
        detection_id, technique, description, implementation,
        difficulty, effectiveness, side_effects, requires
    ))
    conn.commit()
    new_id = c.lastrowid
    conn.close()
    return {"success": True, "id": new_id}


def add_timing_threshold(
    anticheat: str,
    operation: str,
    threshold_cycles: Optional[int] = None,
    threshold_ns: Optional[int] = None,
    notes: Optional[str] = None,
    source: Optional[str] = None,
) -> Dict[str, Any]:
    """Add a timing threshold for an anti-cheat."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("SELECT id FROM anticheats WHERE LOWER(name) = LOWER(?)", (anticheat,))
    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Unknown anticheat: {anticheat}"}

    anticheat_id = row['id']

    c.execute("""
        INSERT OR REPLACE INTO timing_thresholds
        (anticheat_id, operation, threshold_cycles, threshold_ns, notes, source)
        VALUES (?, ?, ?, ?, ?, ?)
    """, (anticheat_id, operation, threshold_cycles, threshold_ns, notes, source))
    conn.commit()
    new_id = c.lastrowid
    conn.close()
    return {"success": True, "id": new_id}


def add_signature(
    anticheat: str,
    signature_type: str,
    pattern: str,
    description: Optional[str] = None,
    location: Optional[str] = None,
    avoidance: Optional[str] = None,
) -> Dict[str, Any]:
    """Add a known signature that an anti-cheat scans for."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("SELECT id FROM anticheats WHERE LOWER(name) = LOWER(?)", (anticheat,))
    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Unknown anticheat: {anticheat}"}

    anticheat_id = row['id']

    c.execute("""
        INSERT INTO signatures
        (anticheat_id, signature_type, pattern, description, location, avoidance)
        VALUES (?, ?, ?, ?, ?, ?)
    """, (anticheat_id, signature_type, pattern, description, location, avoidance))
    conn.commit()
    new_id = c.lastrowid
    conn.close()
    return {"success": True, "id": new_id}
