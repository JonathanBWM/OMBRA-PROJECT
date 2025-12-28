"""
Evasion Techniques Database Tools

Query and manage bypass techniques, chains, and cleanup procedures.
"""

import sqlite3
from pathlib import Path
from typing import List, Dict, Any, Optional
from datetime import datetime
import json

DATA_DIR = Path(__file__).parent.parent / "data"
DB_PATH = DATA_DIR / "evasion_techniques.db"


def _get_conn():
    """Get database connection."""
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn


def get_categories() -> List[Dict[str, Any]]:
    """Get all technique categories."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT c.*, COUNT(t.id) as technique_count
        FROM categories c
        LEFT JOIN techniques t ON c.id = t.category_id
        GROUP BY c.id
        ORDER BY c.name
    """)

    results = [dict(row) for row in c.fetchall()]
    conn.close()
    return results


def get_techniques(
    category: Optional[str] = None,
    search: Optional[str] = None,
) -> List[Dict[str, Any]]:
    """
    Get evasion techniques with optional filters.

    Args:
        category: Filter by category name
        search: Search in name, description, use_case
    """
    conn = _get_conn()
    c = conn.cursor()

    if search:
        # Use FTS
        c.execute("""
            SELECT t.*, c.name as category_name
            FROM techniques t
            JOIN categories c ON t.category_id = c.id
            JOIN techniques_fts fts ON t.id = fts.rowid
            WHERE techniques_fts MATCH ?
            ORDER BY rank
            LIMIT 20
        """, (search,))
    elif category:
        c.execute("""
            SELECT t.*, c.name as category_name
            FROM techniques t
            JOIN categories c ON t.category_id = c.id
            WHERE LOWER(c.name) = LOWER(?)
            ORDER BY t.name
        """, (category,))
    else:
        c.execute("""
            SELECT t.*, c.name as category_name
            FROM techniques t
            JOIN categories c ON t.category_id = c.id
            ORDER BY c.name, t.name
        """)

    results = [dict(row) for row in c.fetchall()]
    conn.close()
    return results


def get_technique_detail(name_or_short: str) -> Optional[Dict[str, Any]]:
    """Get full details for a technique including procedure steps and code examples."""
    conn = _get_conn()
    c = conn.cursor()

    # Try by short_name first, then name
    c.execute("""
        SELECT t.*, c.name as category_name
        FROM techniques t
        JOIN categories c ON t.category_id = c.id
        WHERE LOWER(t.short_name) = LOWER(?) OR LOWER(t.name) = LOWER(?)
    """, (name_or_short, name_or_short))

    row = c.fetchone()
    if not row:
        conn.close()
        return None

    result = dict(row)

    # Get procedure steps
    c.execute("""
        SELECT * FROM procedure_steps
        WHERE technique_id = ?
        ORDER BY step_number
    """, (result['id'],))
    result['steps'] = [dict(r) for r in c.fetchall()]

    # Get code examples
    c.execute("""
        SELECT * FROM code_examples
        WHERE technique_id = ?
        ORDER BY language
    """, (result['id'],))
    result['code_examples'] = [dict(r) for r in c.fetchall()]

    conn.close()
    return result


def get_bypass_chains() -> List[Dict[str, Any]]:
    """Get all bypass chains with their techniques."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("SELECT * FROM bypass_chains ORDER BY name")
    chains = []

    for chain_row in c.fetchall():
        chain = dict(chain_row)

        # Get techniques in this chain
        c.execute("""
            SELECT ct.step_order, ct.notes, t.name, t.short_name, t.description
            FROM chain_techniques ct
            JOIN techniques t ON ct.technique_id = t.id
            WHERE ct.chain_id = ?
            ORDER BY ct.step_order
        """, (chain['id'],))

        chain['techniques'] = [dict(r) for r in c.fetchall()]
        chains.append(chain)

    conn.close()
    return chains


def get_bypass_chain(name: str) -> Optional[Dict[str, Any]]:
    """Get a specific bypass chain by name."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("SELECT * FROM bypass_chains WHERE LOWER(name) = LOWER(?)", (name,))
    row = c.fetchone()
    if not row:
        conn.close()
        return None

    chain = dict(row)

    # Get techniques with full details
    c.execute("""
        SELECT ct.step_order, ct.notes, t.*
        FROM chain_techniques ct
        JOIN techniques t ON ct.technique_id = t.id
        WHERE ct.chain_id = ?
        ORDER BY ct.step_order
    """, (chain['id'],))

    chain['techniques'] = [dict(r) for r in c.fetchall()]
    conn.close()
    return chain


def get_cleanup_procedures() -> List[Dict[str, Any]]:
    """Get all cleanup procedures."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("SELECT * FROM cleanup_procedures ORDER BY name")
    procedures = []

    for proc_row in c.fetchall():
        proc = dict(proc_row)

        # Parse JSON artifacts list
        if proc['artifacts_cleared']:
            try:
                proc['artifacts_cleared'] = json.loads(proc['artifacts_cleared'])
            except:
                pass

        # Get steps
        c.execute("""
            SELECT * FROM cleanup_steps
            WHERE procedure_id = ?
            ORDER BY step_number
        """, (proc['id'],))
        proc['steps'] = [dict(r) for r in c.fetchall()]

        procedures.append(proc)

    conn.close()
    return procedures


def get_cleanup_procedure(name: str) -> Optional[Dict[str, Any]]:
    """Get a specific cleanup procedure by name."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("SELECT * FROM cleanup_procedures WHERE LOWER(name) = LOWER(?)", (name,))
    row = c.fetchone()
    if not row:
        conn.close()
        return None

    proc = dict(row)

    if proc['artifacts_cleared']:
        try:
            proc['artifacts_cleared'] = json.loads(proc['artifacts_cleared'])
        except:
            pass

    c.execute("""
        SELECT * FROM cleanup_steps
        WHERE procedure_id = ?
        ORDER BY step_number
    """, (proc['id'],))
    proc['steps'] = [dict(r) for r in c.fetchall()]

    conn.close()
    return proc


# Add/Update functions

def add_technique(
    category: str,
    name: str,
    description: str,
    short_name: Optional[str] = None,
    use_case: Optional[str] = None,
    requirements: Optional[str] = None,
    limitations: Optional[str] = None,
    detection_surface: Optional[str] = None,
    source: Optional[str] = None,
    source_url: Optional[str] = None,
) -> Dict[str, Any]:
    """Add a new evasion technique."""
    conn = _get_conn()
    c = conn.cursor()

    # Get category ID
    c.execute("SELECT id FROM categories WHERE LOWER(name) = LOWER(?)", (category,))
    row = c.fetchone()
    if not row:
        # Create category if it doesn't exist
        c.execute("INSERT INTO categories (name) VALUES (?)", (category,))
        category_id = c.lastrowid
    else:
        category_id = row['id']

    c.execute("""
        INSERT INTO techniques
        (category_id, name, short_name, description, use_case, requirements,
         limitations, detection_surface, source, source_url)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    """, (
        category_id, name, short_name, description, use_case, requirements,
        limitations, detection_surface, source, source_url
    ))
    conn.commit()
    new_id = c.lastrowid
    conn.close()
    return {"success": True, "id": new_id, "name": name}


def add_procedure_step(
    technique: str,
    step_number: int,
    action: str,
    details: Optional[str] = None,
    code_example: Optional[str] = None,
) -> Dict[str, Any]:
    """Add a step to a technique's procedure."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT id FROM techniques
        WHERE LOWER(short_name) = LOWER(?) OR LOWER(name) = LOWER(?)
    """, (technique, technique))

    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Unknown technique: {technique}"}

    technique_id = row['id']

    c.execute("""
        INSERT OR REPLACE INTO procedure_steps
        (technique_id, step_number, action, details, code_example)
        VALUES (?, ?, ?, ?, ?)
    """, (technique_id, step_number, action, details, code_example))
    conn.commit()
    new_id = c.lastrowid
    conn.close()
    return {"success": True, "id": new_id}


def add_code_example(
    technique: str,
    code: str,
    language: str = "c",
    title: Optional[str] = None,
    explanation: Optional[str] = None,
    tested: bool = False,
    works_on: Optional[str] = None,
) -> Dict[str, Any]:
    """Add a code example to a technique."""
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT id FROM techniques
        WHERE LOWER(short_name) = LOWER(?) OR LOWER(name) = LOWER(?)
    """, (technique, technique))

    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Unknown technique: {technique}"}

    technique_id = row['id']

    c.execute("""
        INSERT INTO code_examples
        (technique_id, language, title, code, explanation, tested, works_on)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    """, (technique_id, language, title, code, explanation, tested, works_on))
    conn.commit()
    new_id = c.lastrowid
    conn.close()
    return {"success": True, "id": new_id}


def add_bypass_chain(
    name: str,
    description: str,
    goal: str,
    technique_names: List[str],
    prerequisites: Optional[str] = None,
) -> Dict[str, Any]:
    """Add a new bypass chain linking multiple techniques."""
    conn = _get_conn()
    c = conn.cursor()

    # Create the chain
    c.execute("""
        INSERT INTO bypass_chains (name, description, goal, prerequisites, total_steps)
        VALUES (?, ?, ?, ?, ?)
    """, (name, description, goal, prerequisites, len(technique_names)))

    chain_id = c.lastrowid

    # Link techniques
    for i, tech_name in enumerate(technique_names, 1):
        c.execute("""
            SELECT id FROM techniques
            WHERE LOWER(short_name) = LOWER(?) OR LOWER(name) = LOWER(?)
        """, (tech_name, tech_name))

        row = c.fetchone()
        if row:
            c.execute("""
                INSERT INTO chain_techniques (chain_id, technique_id, step_order)
                VALUES (?, ?, ?)
            """, (chain_id, row['id'], i))

    conn.commit()
    conn.close()
    return {"success": True, "id": chain_id, "name": name}


def add_cleanup_procedure(
    name: str,
    description: str,
    steps: List[Dict[str, Any]],
    requires_ring0: bool = True,
    timing: str = "post_activation",
    artifacts_cleared: Optional[List[str]] = None,
) -> Dict[str, Any]:
    """Add a cleanup procedure with steps."""
    conn = _get_conn()
    c = conn.cursor()

    artifacts_json = json.dumps(artifacts_cleared) if artifacts_cleared else None

    c.execute("""
        INSERT INTO cleanup_procedures
        (name, description, requires_ring0, timing, artifacts_cleared)
        VALUES (?, ?, ?, ?, ?)
    """, (name, description, requires_ring0, timing, artifacts_json))

    proc_id = c.lastrowid

    # Add steps
    for step in steps:
        c.execute("""
            INSERT INTO cleanup_steps
            (procedure_id, step_number, target, action, code, risk_level)
            VALUES (?, ?, ?, ?, ?, ?)
        """, (
            proc_id,
            step.get('step_number', 1),
            step.get('target', 'unknown'),
            step.get('action', ''),
            step.get('code'),
            step.get('risk_level', 'medium'),
        ))

    conn.commit()
    conn.close()
    return {"success": True, "id": proc_id, "name": name}
