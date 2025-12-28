"""Driver management tools"""

import uuid
import json
from pathlib import Path
from datetime import datetime
from typing import Optional
from ..database import get_db

async def add_driver(
    original_name: str,
    md5: str,
    sha1: str,
    sha256: str,
    file_size: int,
    image_base: int,
    entry_point_rva: int,
    size_of_image: int = 0,
    machine: int = 0x8664,  # AMD64
    subsystem: int = 1,  # Native
    characteristics: int = 0,
    analyzed_name: Optional[str] = None,
    description: Optional[str] = None,
    **kwargs
) -> dict:
    """Add a new driver to the database"""
    db = get_db()

    driver_id = str(uuid.uuid4())

    data = {
        'id': driver_id,
        'original_name': original_name,
        'analyzed_name': analyzed_name or original_name,
        'md5': md5,
        'sha1': sha1,
        'sha256': sha256,
        'file_size': file_size,
        'image_base': image_base,
        'entry_point_rva': entry_point_rva,
        'size_of_image': size_of_image,
        'machine': machine,
        'subsystem': subsystem,
        'characteristics': characteristics,
        'analysis_status': 'pending',
        'notes': description or ''
    }

    await db.insert('drivers', data)

    return {
        'success': True,
        'driver_id': driver_id,
        'message': f'Driver {original_name} added successfully'
    }

async def get_driver(
    driver_id: Optional[str] = None,
    sha256: Optional[str] = None
) -> dict:
    """Get driver by ID or SHA256"""
    if not driver_id and not sha256:
        return {'success': False, 'error': 'Must provide driver_id or sha256'}

    db = get_db()

    if driver_id:
        driver = await db.fetch_one("SELECT * FROM drivers WHERE id = ?", (driver_id,))
    else:
        driver = await db.fetch_one("SELECT * FROM drivers WHERE sha256 = ?", (sha256,))

    if not driver:
        return {'success': False, 'error': 'Driver not found'}

    return {
        'success': True,
        'driver': driver
    }

async def list_drivers(
    status: Optional[str] = None,
    is_vulnerable: Optional[bool] = None,
    limit: int = 100
) -> dict:
    """List drivers with optional filters"""
    db = get_db()

    query = "SELECT * FROM drivers WHERE 1=1"
    params = []

    if status:
        query += " AND analysis_status = ?"
        params.append(status)

    if is_vulnerable is not None:
        query += " AND is_vulnerable = ?"
        params.append(1 if is_vulnerable else 0)

    query += " ORDER BY created_at DESC LIMIT ?"
    params.append(limit)

    drivers = await db.fetch_all(query, tuple(params))

    return {
        'success': True,
        'count': len(drivers),
        'drivers': drivers
    }

async def update_driver_status(
    driver_id: str,
    status: str
) -> dict:
    """Update driver analysis status"""
    db = get_db()

    # Verify driver exists
    driver = await db.fetch_one("SELECT id FROM drivers WHERE id = ?", (driver_id,))
    if not driver:
        return {'success': False, 'error': 'Driver not found'}

    await db.update('drivers', driver_id, {'analysis_status': status})

    return {
        'success': True,
        'message': f'Driver status updated to {status}'
    }

async def delete_driver(driver_id: str) -> dict:
    """Delete driver from database"""
    db = get_db()

    # Verify driver exists
    driver = await db.fetch_one("SELECT id FROM drivers WHERE id = ?", (driver_id,))
    if not driver:
        return {'success': False, 'error': 'Driver not found'}

    await db.delete('drivers', driver_id)

    return {
        'success': True,
        'message': 'Driver deleted successfully'
    }

async def import_scanner_results(json_path: str) -> dict:
    """
    Import scanner JSON batch output, creating driver entries with all metadata.

    Expects MCPBatchExport format with scan_metadata and analysis_queue containing
    MCPAnalysisRequest objects with tier, score, capabilities, and age info.
    """
    db = get_db()
    json_file = Path(json_path)

    if not json_file.exists():
        return {'success': False, 'error': f'File not found: {json_path}'}

    try:
        with open(json_file, 'r') as f:
            batch = json.load(f)
    except json.JSONDecodeError as e:
        return {'success': False, 'error': f'Invalid JSON: {str(e)}'}

    # Validate batch structure
    if 'analysis_queue' not in batch:
        return {'success': False, 'error': 'Missing analysis_queue in batch JSON'}

    imported_count = 0
    skipped_count = 0
    errors = []

    for req in batch['analysis_queue']:
        try:
            # Check if driver already exists by SHA256
            existing = await db.fetch_one(
                "SELECT id FROM drivers WHERE sha256 = ?",
                (req.get('hash', ''),)
            )

            if existing:
                skipped_count += 1
                continue

            # Create driver entry from scanner results
            driver_id = str(uuid.uuid4())

            # Build tags array with tier and capabilities
            tags = [f"tier_{req.get('tier', 'unknown')}"]
            if req.get('family'):
                tags.append(f"family_{req['family']}")
            if req.get('era'):
                tags.append(f"era_{req['era']}")

            data = {
                'id': driver_id,
                'original_name': req.get('name', 'unknown.sys'),
                'analyzed_name': req.get('name', 'unknown.sys'),
                'md5': req.get('hash_md5', ''),
                'sha1': req.get('hash_sha1', ''),
                'sha256': req.get('hash', ''),
                'file_size': 0,  # Scanner doesn't provide this
                'image_base': 0,
                'entry_point_rva': 0,
                'size_of_image': 0,
                'machine': 0x8664,
                'subsystem': 1,
                'characteristics': 0,
                'analysis_status': 'scanner_imported',
                'notes': f"Imported from scanner. Tier: {req.get('tier', 'unknown')}, Score: {req.get('score', 0.0)}, Priority: {req.get('priority', 0)}",
                'tags': json.dumps(tags)
            }

            await db.insert('drivers', data)
            imported_count += 1

        except Exception as e:
            errors.append(f"{req.get('name', 'unknown')}: {str(e)}")

    return {
        'success': True,
        'imported': imported_count,
        'skipped': skipped_count,
        'errors': errors,
        'message': f'Imported {imported_count} drivers, skipped {skipped_count} duplicates'
    }

async def get_analysis_queue(
    min_priority: int = 1,
    max_priority: int = 5
) -> dict:
    """
    Get priority-sorted queue for MCP analysis.

    Returns drivers imported from scanner sorted by priority (1=highest, 5=lowest).
    Filters for drivers with scanner_imported status.
    """
    db = get_db()

    # Extract priority from notes field (stored as "Priority: N")
    query = """
        SELECT
            id,
            original_name,
            sha256,
            tags,
            notes,
            analysis_status,
            created_at
        FROM drivers
        WHERE analysis_status = 'scanner_imported'
        ORDER BY created_at DESC
    """

    drivers = await db.fetch_all(query)

    # Parse priority from notes and filter
    queue = []
    for driver in drivers:
        notes = driver.get('notes', '')
        priority = 5  # Default to lowest priority

        # Extract priority from notes
        if 'Priority:' in notes:
            try:
                priority_str = notes.split('Priority:')[1].split(',')[0].strip()
                priority = int(priority_str)
            except (IndexError, ValueError):
                pass

        # Filter by priority range
        if min_priority <= priority <= max_priority:
            # Parse tier and score from notes
            tier = 'unknown'
            score = 0.0

            if 'Tier:' in notes:
                tier = notes.split('Tier:')[1].split(',')[0].strip()
            if 'Score:' in notes:
                try:
                    score_str = notes.split('Score:')[1].split(',')[0].strip()
                    score = float(score_str)
                except (IndexError, ValueError):
                    pass

            queue.append({
                'driver_id': driver['id'],
                'name': driver['original_name'],
                'sha256': driver['sha256'],
                'tier': tier,
                'score': score,
                'priority': priority,
                'tags': json.loads(driver.get('tags', '[]')),
                'imported_at': driver['created_at']
            })

    # Sort by priority (lower number = higher priority)
    queue.sort(key=lambda x: (x['priority'], -x['score']))

    return {
        'success': True,
        'count': len(queue),
        'queue': queue
    }

async def import_single_driver_analysis(json_data: str) -> dict:
    """
    Import a single MCPAnalysisRequest JSON object.

    Accepts JSON string containing driver metadata, scanner results, capabilities,
    dangerous APIs, detected IOCTLs, and age analysis.
    """
    db = get_db()

    try:
        req = json.loads(json_data)
    except json.JSONDecodeError as e:
        return {'success': False, 'error': f'Invalid JSON: {str(e)}'}

    # Validate required fields
    if 'driver' not in req or 'scanner_results' not in req:
        return {'success': False, 'error': 'Missing required fields: driver, scanner_results'}

    driver_info = req['driver']
    scanner = req['scanner_results']
    capabilities = req.get('capabilities', {})
    dangerous_apis = req.get('dangerous_apis', [])
    detected_ioctls = req.get('detected_ioctls', [])
    age_analysis = req.get('age_analysis', {})

    # Check if driver already exists
    existing = await db.fetch_one(
        "SELECT id FROM drivers WHERE sha256 = ?",
        (driver_info.get('hash_sha256', ''),)
    )

    if existing:
        return {
            'success': False,
            'error': 'Driver already exists',
            'driver_id': existing['id']
        }

    # Create driver entry
    driver_id = str(uuid.uuid4())

    # Build tags
    tags = [f"tier_{scanner.get('capability_tier', 'unknown')}"]
    if scanner.get('driver_family'):
        tags.append(f"family_{scanner['driver_family']}")
    if age_analysis.get('era'):
        tags.append(f"era_{age_analysis['era']}")

    # Build capabilities tags
    for cap_name, has_cap in capabilities.items():
        if has_cap:
            tags.append(f"cap_{cap_name}")

    # Build detailed notes
    notes_parts = [
        f"Tier: {scanner.get('capability_tier', 'unknown')}",
        f"Score: {scanner.get('final_score', 0.0)}",
        f"Priority: {scanner.get('priority', 5)}",
        f"Classification: {scanner.get('classification', 'unknown')}",
        f"Family: {scanner.get('driver_family', 'unknown')}"
    ]

    if age_analysis.get('era'):
        notes_parts.append(f"Era: {age_analysis['era']}")
    if age_analysis.get('age_warning'):
        notes_parts.append(f"Warning: {age_analysis['age_warning']}")

    if dangerous_apis:
        notes_parts.append(f"Dangerous APIs: {', '.join(dangerous_apis[:5])}")

    if detected_ioctls:
        notes_parts.append(f"IOCTLs detected: {len(detected_ioctls)}")

    data = {
        'id': driver_id,
        'original_name': driver_info.get('name', 'unknown.sys'),
        'analyzed_name': driver_info.get('name', 'unknown.sys'),
        'md5': driver_info.get('hash_md5', ''),
        'sha1': driver_info.get('hash_sha1', ''),
        'sha256': driver_info.get('hash_sha256', ''),
        'file_size': 0,
        'image_base': 0,
        'entry_point_rva': 0,
        'size_of_image': 0,
        'machine': 0x8664,
        'subsystem': 1,
        'characteristics': 0,
        'analysis_status': 'scanner_imported',
        'notes': ' | '.join(notes_parts),
        'tags': json.dumps(tags)
    }

    await db.insert('drivers', data)

    return {
        'success': True,
        'driver_id': driver_id,
        'message': f'Driver {driver_info.get("name")} imported successfully',
        'tier': scanner.get('capability_tier'),
        'score': scanner.get('final_score'),
        'priority': scanner.get('priority')
    }

async def set_driver_tier_info(
    driver_id: str,
    tier: str,
    score: float,
    classification: str,
    synergies: Optional[str] = None
) -> dict:
    """
    Update driver with scanner scoring info.

    Sets tier (S/A/B/C/D), final score, classification, and optional synergies.
    Updates tags and notes to reflect tier and scoring information.
    """
    db = get_db()

    # Verify driver exists
    driver = await db.fetch_one("SELECT id, tags, notes FROM drivers WHERE id = ?", (driver_id,))
    if not driver:
        return {'success': False, 'error': 'Driver not found'}

    # Update tags
    existing_tags = json.loads(driver.get('tags', '[]'))

    # Remove old tier tags
    existing_tags = [t for t in existing_tags if not t.startswith('tier_')]

    # Add new tier tag
    existing_tags.append(f'tier_{tier}')

    # Build updated notes
    notes_parts = [
        f"Tier: {tier}",
        f"Score: {score}",
        f"Classification: {classification}"
    ]

    if synergies:
        notes_parts.append(f"Synergies: {synergies}")

    # Preserve existing notes that aren't tier/score/classification
    existing_notes = driver.get('notes', '')
    preserved_parts = []
    for part in existing_notes.split(' | '):
        if not any(part.startswith(prefix) for prefix in ['Tier:', 'Score:', 'Classification:', 'Synergies:']):
            preserved_parts.append(part)

    notes = ' | '.join(notes_parts + preserved_parts)

    # Update driver
    await db.update('drivers', driver_id, {
        'tags': json.dumps(existing_tags),
        'notes': notes
    })

    return {
        'success': True,
        'message': f'Driver tier info updated: {tier} ({score})'
    }

async def get_drivers_by_capability(capability: str) -> dict:
    """
    List drivers with specific capability.

    Capabilities: module_loading, physical_memory, msr_access, process_control
    Searches tags for cap_{capability} markers added during scanner import.
    """
    db = get_db()

    # Search for capability tag
    cap_tag = f'cap_{capability}'

    query = """
        SELECT
            id,
            original_name,
            sha256,
            tags,
            notes,
            analysis_status,
            created_at
        FROM drivers
        WHERE tags LIKE ?
        ORDER BY created_at DESC
    """

    drivers = await db.fetch_all(query, (f'%{cap_tag}%',))

    # Parse tier and score from each driver
    results = []
    for driver in drivers:
        notes = driver.get('notes', '')
        tags = json.loads(driver.get('tags', '[]'))

        # Extract tier and score
        tier = 'unknown'
        score = 0.0

        if 'Tier:' in notes:
            tier = notes.split('Tier:')[1].split(',')[0].strip()
        if 'Score:' in notes:
            try:
                score_str = notes.split('Score:')[1].split(',')[0].strip()
                score = float(score_str)
            except (IndexError, ValueError):
                pass

        results.append({
            'driver_id': driver['id'],
            'name': driver['original_name'],
            'sha256': driver['sha256'],
            'tier': tier,
            'score': score,
            'tags': tags,
            'status': driver['analysis_status'],
            'imported_at': driver['created_at']
        })

    # Sort by score descending
    results.sort(key=lambda x: -x['score'])

    return {
        'success': True,
        'capability': capability,
        'count': len(results),
        'drivers': results
    }
