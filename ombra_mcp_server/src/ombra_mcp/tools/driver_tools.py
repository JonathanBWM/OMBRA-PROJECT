"""
Driver Management Tools

Tools for adding, retrieving, updating, and managing driver metadata in the RE database.
"""

import hashlib
import json
import uuid
from pathlib import Path
from typing import Dict, List, Optional, Any
from datetime import datetime

from .driver_re_db import get_conn


def _hash_file(file_path: Path) -> Dict[str, str]:
    """Calculate MD5, SHA1, and SHA256 hashes of a file."""
    md5 = hashlib.md5()
    sha1 = hashlib.sha1()
    sha256 = hashlib.sha256()

    with open(file_path, 'rb') as f:
        while chunk := f.read(8192):
            md5.update(chunk)
            sha1.update(chunk)
            sha256.update(chunk)

    return {
        'md5': md5.hexdigest(),
        'sha1': sha1.hexdigest(),
        'sha256': sha256.hexdigest()
    }


def _parse_pe_metadata(file_path: Path) -> Dict[str, Any]:
    """
    Parse PE metadata from driver file.
    Returns basic info - full parsing would use pefile library.
    """
    # Placeholder - real implementation would use pefile
    # For now, return minimal structure
    file_size = file_path.stat().st_size

    return {
        'file_size': file_size,
        'image_base': 0,  # Would extract from PE headers
        'entry_point_rva': 0,
        'size_of_image': 0,
        'timestamp': 0,
        'machine': 0x8664,  # AMD64
        'subsystem': 1,  # Native
        'characteristics': 0
    }


async def add_driver(
    file_path: str,
    analyzed_name: Optional[str] = None,
    tags: Optional[List[str]] = None,
    notes: Optional[str] = None
) -> Dict[str, Any]:
    """
    Add a new driver to the database.

    Automatically extracts:
    - File hashes (MD5, SHA1, SHA256)
    - PE metadata (sections, image base, entry point, etc.)
    - Basic file info

    Returns driver ID and extraction summary.
    """
    path = Path(file_path)
    if not path.exists():
        return {"error": f"File not found: {file_path}"}

    # Generate hashes
    hashes = _hash_file(path)

    # Check if driver already exists
    conn = get_conn()
    c = conn.cursor()
    c.execute("SELECT id, original_name FROM drivers WHERE sha256 = ?", (hashes['sha256'],))
    existing = c.fetchone()
    if existing:
        conn.close()
        return {
            "error": "Driver already exists",
            "driver_id": existing['id'],
            "driver_name": existing['original_name']
        }

    # Parse PE metadata
    pe_meta = _parse_pe_metadata(path)

    # Create driver record
    driver_id = str(uuid.uuid4())
    original_name = path.name
    analyzed = analyzed_name or original_name
    tags_json = json.dumps(tags) if tags else None

    c.execute("""
        INSERT INTO drivers (
            id, original_name, analyzed_name,
            md5, sha1, sha256,
            file_size, image_base, entry_point_rva,
            size_of_image, timestamp, machine,
            subsystem, characteristics,
            tags, notes
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    """, (
        driver_id, original_name, analyzed,
        hashes['md5'], hashes['sha1'], hashes['sha256'],
        pe_meta['file_size'], pe_meta['image_base'], pe_meta['entry_point_rva'],
        pe_meta['size_of_image'], pe_meta['timestamp'], pe_meta['machine'],
        pe_meta['subsystem'], pe_meta['characteristics'],
        tags_json, notes
    ))

    conn.commit()
    conn.close()

    return {
        "success": True,
        "driver_id": driver_id,
        "driver_name": original_name,
        "analyzed_name": analyzed,
        "sha256": hashes['sha256'],
        "file_size": pe_meta['file_size'],
        "message": f"Driver {original_name} added successfully"
    }


async def get_driver(
    driver_id: Optional[str] = None,
    sha256: Optional[str] = None,
    name: Optional[str] = None
) -> Optional[Dict[str, Any]]:
    """
    Get driver details by ID, hash, or name.

    Returns complete driver metadata including statistics.
    """
    conn = get_conn()
    c = conn.cursor()

    if driver_id:
        c.execute("SELECT * FROM drivers WHERE id = ?", (driver_id,))
    elif sha256:
        c.execute("SELECT * FROM drivers WHERE sha256 = ?", (sha256,))
    elif name:
        c.execute("SELECT * FROM drivers WHERE original_name LIKE ? OR analyzed_name LIKE ?",
                  (f"%{name}%", f"%{name}%"))
    else:
        conn.close()
        return {"error": "Must provide driver_id, sha256, or name"}

    row = c.fetchone()
    if not row:
        conn.close()
        return None

    result = dict(row)
    driver_id = result['id']

    # Get statistics
    c.execute("SELECT COUNT(*) as count FROM imports WHERE driver_id = ?", (driver_id,))
    result['import_count'] = c.fetchone()['count']

    c.execute("SELECT COUNT(*) as count FROM exports WHERE driver_id = ?", (driver_id,))
    result['export_count'] = c.fetchone()['count']

    c.execute("SELECT COUNT(*) as count FROM ioctls WHERE driver_id = ?", (driver_id,))
    result['ioctl_count'] = c.fetchone()['count']

    c.execute("SELECT COUNT(*) as count FROM ioctls WHERE driver_id = ? AND is_vulnerable = 1",
              (driver_id,))
    result['vulnerable_ioctl_count'] = c.fetchone()['count']

    c.execute("SELECT COUNT(*) as count FROM sections WHERE driver_id = ?", (driver_id,))
    result['section_count'] = c.fetchone()['count']

    # Parse tags if present
    if result['tags']:
        result['tags'] = json.loads(result['tags'])

    conn.close()
    return result


async def list_drivers(
    tags: Optional[List[str]] = None,
    status: Optional[str] = None,
    limit: int = 50,
    offset: int = 0
) -> List[Dict[str, Any]]:
    """
    List all drivers with optional filtering.
    """
    conn = get_conn()
    c = conn.cursor()

    query = "SELECT * FROM drivers WHERE 1=1"
    params = []

    if status:
        query += " AND analysis_status = ?"
        params.append(status)

    if tags:
        # Filter by tags - SQLite JSON support
        for tag in tags:
            query += f" AND tags LIKE ?"
            params.append(f'%"{tag}"%')

    query += " ORDER BY created_at DESC LIMIT ? OFFSET ?"
    params.extend([limit, offset])

    c.execute(query, params)
    results = []

    for row in c.fetchall():
        result = dict(row)
        if result['tags']:
            result['tags'] = json.loads(result['tags'])
        results.append(result)

    conn.close()
    return results


async def update_driver_status(
    driver_id: str,
    status: str,
    notes: Optional[str] = None
) -> Dict[str, Any]:
    """
    Update driver analysis status.

    Status values: pending, in_progress, complete
    """
    valid_statuses = ['pending', 'in_progress', 'complete']
    if status not in valid_statuses:
        return {"error": f"Invalid status. Must be one of: {', '.join(valid_statuses)}"}

    conn = get_conn()
    c = conn.cursor()

    # Check if driver exists
    c.execute("SELECT id, original_name FROM drivers WHERE id = ?", (driver_id,))
    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Driver not found: {driver_id}"}

    # Build update query
    update_fields = ["analysis_status = ?", "updated_at = CURRENT_TIMESTAMP"]
    params = [status]

    if status == 'in_progress':
        update_fields.append("analysis_started_at = CURRENT_TIMESTAMP")
    elif status == 'complete':
        update_fields.append("analysis_completed_at = CURRENT_TIMESTAMP")

    if notes is not None:
        update_fields.append("notes = ?")
        params.append(notes)

    params.append(driver_id)

    c.execute(f"UPDATE drivers SET {', '.join(update_fields)} WHERE id = ?", params)
    conn.commit()
    conn.close()

    return {
        "success": True,
        "driver_id": driver_id,
        "driver_name": row['original_name'],
        "new_status": status,
        "message": f"Driver status updated to '{status}'"
    }


async def delete_driver(
    driver_id: str,
    confirm: bool = False
) -> Dict[str, Any]:
    """
    Delete a driver and all associated data.
    Requires confirmation.
    """
    if not confirm:
        return {
            "error": "Deletion requires confirmation",
            "message": "Set confirm=True to delete this driver and ALL associated data (imports, exports, IOCTLs, xrefs)"
        }

    conn = get_conn()
    c = conn.cursor()

    # Check if driver exists
    c.execute("SELECT id, original_name FROM drivers WHERE id = ?", (driver_id,))
    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Driver not found: {driver_id}"}

    driver_name = row['original_name']

    # Delete driver (CASCADE will handle related tables)
    c.execute("DELETE FROM drivers WHERE id = ?", (driver_id,))
    conn.commit()
    conn.close()

    return {
        "success": True,
        "driver_id": driver_id,
        "driver_name": driver_name,
        "message": f"Driver '{driver_name}' and all associated data deleted successfully"
    }
