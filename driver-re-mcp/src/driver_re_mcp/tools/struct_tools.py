"""
Structure management tools for driver reverse engineering.
Handles data structure definitions, member parsing, and IOCTL linking.
"""

import uuid
import re
from typing import Dict, List, Optional
from datetime import datetime
import json


async def add_structure(
    name: str,
    definition_c: str,
    driver_id: Optional[str] = None,
    struct_type: Optional[str] = None,
    size: Optional[int] = None,
    description: Optional[str] = None
) -> Dict:
    """
    Add a structure definition to the database.
    Automatically parses C definition to extract members.

    Args:
        name: Structure name
        definition_c: C struct definition (e.g., "struct FOO { int bar; }")
        driver_id: Driver UUID (None for shared/common structures)
        struct_type: Type category (ioctl_input, ioctl_output, session, internal, windows)
        size: Structure size in bytes (calculated if not provided)
        description: Structure purpose/description

    Returns:
        Structure details including UUID and parsed members
    """
    from ..database.connection import get_db_connection
    from ..embeddings.provider import generate_embedding

    structure_id = str(uuid.uuid4())
    now = datetime.utcnow().isoformat()

    # Parse structure members from C definition
    members = _parse_c_structure(definition_c)

    # Calculate size if not provided
    if size is None and members:
        # Rough estimate: max offset + last member size
        if members:
            last_member = members[-1]
            size = last_member['offset'] + last_member['size']

    # Generate embedding for semantic search
    embedding_text = f"Structure {name}: {description or ''}. {definition_c}"
    embedding = await generate_embedding(embedding_text)

    conn = await get_db_connection()

    # Insert structure
    await conn.execute("""
        INSERT INTO structures (
            id, driver_id, name, size, struct_type,
            definition_c, description, created_at, embedding
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    """, (
        structure_id, driver_id, name, size, struct_type,
        definition_c, description, now, json.dumps(embedding)
    ))

    # Insert members
    for member in members:
        member_id = str(uuid.uuid4())
        await conn.execute("""
            INSERT INTO structure_members (
                id, structure_id, name, offset, size, type_name,
                is_array, array_count, is_pointer, pointer_depth,
                is_bitfield, bit_offset, bit_size, description, created_at
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            member_id, structure_id,
            member['name'], member['offset'], member['size'], member['type_name'],
            member.get('is_array', False), member.get('array_count'),
            member.get('is_pointer', False), member.get('pointer_depth', 0),
            member.get('is_bitfield', False), member.get('bit_offset'),
            member.get('bit_size'), member.get('description'),
            now
        ))

    await conn.commit()

    return {
        'structure_id': structure_id,
        'name': name,
        'size': size,
        'member_count': len(members),
        'members': members
    }


def _parse_c_structure(definition_c: str) -> List[Dict]:
    """
    Parse C structure definition to extract members.
    Handles basic types, pointers, arrays, and bitfields.

    Returns:
        List of member dicts with offset, size, type, etc.
    """
    members = []
    current_offset = 0

    # Remove comments
    definition_c = re.sub(r'/\*.*?\*/', '', definition_c, flags=re.DOTALL)
    definition_c = re.sub(r'//.*', '', definition_c)

    # Extract struct body
    match = re.search(r'struct\s+\w+\s*\{([^}]+)\}', definition_c, re.DOTALL)
    if not match:
        match = re.search(r'typedef\s+struct\s*\{([^}]+)\}\s*\w+', definition_c, re.DOTALL)

    if not match:
        return members

    body = match.group(1)

    # Parse each member line
    for line in body.split(';'):
        line = line.strip()
        if not line:
            continue

        member = _parse_member_line(line, current_offset)
        if member:
            members.append(member)
            current_offset = member['offset'] + member['size']

    return members


def _parse_member_line(line: str, offset: int) -> Optional[Dict]:
    """Parse a single structure member line."""
    # Pattern: type_name member_name [array] [: bitfield]
    # Examples:
    #   ULONG Magic
    #   PVOID Buffer
    #   UCHAR Reserved[8]
    #   ULONG Flags : 4

    # Handle arrays
    array_match = re.search(r'(\w+)\s+(\w+)\s*\[(\d+)\]', line)
    if array_match:
        type_name = array_match.group(1)
        member_name = array_match.group(2)
        array_count = int(array_match.group(3))
        base_size = _get_type_size(type_name)
        return {
            'name': member_name,
            'offset': offset,
            'size': base_size * array_count,
            'type_name': type_name,
            'is_array': True,
            'array_count': array_count,
            'is_pointer': '*' in type_name,
            'pointer_depth': type_name.count('*')
        }

    # Handle bitfields
    bitfield_match = re.search(r'(\w+)\s+(\w+)\s*:\s*(\d+)', line)
    if bitfield_match:
        type_name = bitfield_match.group(1)
        member_name = bitfield_match.group(2)
        bit_size = int(bitfield_match.group(3))
        return {
            'name': member_name,
            'offset': offset,
            'size': _get_type_size(type_name),
            'type_name': type_name,
            'is_bitfield': True,
            'bit_size': bit_size,
            'is_pointer': False,
            'pointer_depth': 0
        }

    # Handle regular members
    regular_match = re.search(r'([\w\s*]+)\s+(\w+)$', line)
    if regular_match:
        type_part = regular_match.group(1).strip()
        member_name = regular_match.group(2)

        pointer_depth = type_part.count('*')
        type_name = type_part.replace('*', '').strip()

        size = _get_type_size(type_name) if pointer_depth == 0 else 8  # Pointers are 8 bytes on x64

        return {
            'name': member_name,
            'offset': offset,
            'size': size,
            'type_name': type_part,
            'is_pointer': pointer_depth > 0,
            'pointer_depth': pointer_depth,
            'is_array': False
        }

    return None


def _get_type_size(type_name: str) -> int:
    """Get size of a C type in bytes (x64 Windows)."""
    type_sizes = {
        'UCHAR': 1, 'CHAR': 1, 'BYTE': 1, 'BOOLEAN': 1, 'INT8': 1, 'UINT8': 1,
        'USHORT': 2, 'SHORT': 2, 'WORD': 2, 'INT16': 2, 'UINT16': 2,
        'ULONG': 4, 'LONG': 4, 'DWORD': 4, 'INT32': 4, 'UINT32': 4,
        'ULONG64': 8, 'LONG64': 8, 'ULONGLONG': 8, 'LONGLONG': 8,
        'QWORD': 8, 'INT64': 8, 'UINT64': 8, 'SIZE_T': 8,
        'PVOID': 8, 'HANDLE': 8, 'PHANDLE': 8,
        'int': 4, 'unsigned int': 4, 'signed int': 4,
        'short': 2, 'unsigned short': 2,
        'long': 4, 'unsigned long': 4,
        'long long': 8, 'unsigned long long': 8,
        'char': 1, 'unsigned char': 1, 'signed char': 1,
        'void*': 8, 'char*': 8, 'void *': 8
    }

    return type_sizes.get(type_name, 4)  # Default to 4 bytes


async def get_structure(
    structure_id: Optional[str] = None,
    name: Optional[str] = None,
    driver_id: Optional[str] = None
) -> Dict:
    """
    Get structure definition with all members.

    Args:
        structure_id: Structure UUID
        name: Structure name (requires driver_id if not unique)
        driver_id: Driver UUID

    Returns:
        Complete structure definition with members
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    # Build query
    if structure_id:
        cursor = await conn.execute(
            "SELECT * FROM structures WHERE id = ?",
            (structure_id,)
        )
    elif name and driver_id:
        cursor = await conn.execute(
            "SELECT * FROM structures WHERE name = ? AND driver_id = ?",
            (name, driver_id)
        )
    elif name:
        cursor = await conn.execute(
            "SELECT * FROM structures WHERE name = ? AND driver_id IS NULL",
            (name,)
        )
    else:
        raise ValueError("Must provide structure_id or name")

    row = await cursor.fetchone()
    if not row:
        raise ValueError("Structure not found")

    struct = dict(row)

    # Get members
    cursor = await conn.execute("""
        SELECT * FROM structure_members
        WHERE structure_id = ?
        ORDER BY offset
    """, (struct['id'],))

    struct['members'] = [dict(r) for r in await cursor.fetchall()]

    return struct


async def list_structures(
    driver_id: Optional[str] = None,
    struct_type: Optional[str] = None
) -> List[Dict]:
    """
    List all structures, optionally filtered by driver or type.

    Args:
        driver_id: Filter by driver UUID (None = only shared structures)
        struct_type: Filter by structure type

    Returns:
        List of structure summaries
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    query = "SELECT id, name, size, struct_type, description FROM structures WHERE 1=1"
    params = []

    if driver_id is not None:
        query += " AND driver_id = ?"
        params.append(driver_id)

    if struct_type:
        query += " AND struct_type = ?"
        params.append(struct_type)

    query += " ORDER BY name"

    cursor = await conn.execute(query, params)
    structures = [dict(row) for row in await cursor.fetchall()]

    # Get member counts
    for struct in structures:
        cursor = await conn.execute(
            "SELECT COUNT(*) FROM structure_members WHERE structure_id = ?",
            (struct['id'],)
        )
        struct['member_count'] = (await cursor.fetchone())[0]

    return structures


async def link_structure_to_ioctl(
    ioctl_id: str,
    input_struct_id: Optional[str] = None,
    output_struct_id: Optional[str] = None
) -> Dict:
    """
    Link structures to an IOCTL as input/output buffers.

    Args:
        ioctl_id: IOCTL UUID
        input_struct_id: Input structure UUID (None to clear)
        output_struct_id: Output structure UUID (None to clear)

    Returns:
        Updated IOCTL info with structure names
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    # Verify IOCTL exists
    cursor = await conn.execute(
        "SELECT id, name FROM ioctls WHERE id = ?",
        (ioctl_id,)
    )
    row = await cursor.fetchone()
    if not row:
        raise ValueError(f"IOCTL {ioctl_id} not found")

    ioctl_name = row[1]

    # Update IOCTL
    await conn.execute("""
        UPDATE ioctls
        SET input_struct_id = ?, output_struct_id = ?
        WHERE id = ?
    """, (input_struct_id, output_struct_id, ioctl_id))

    # If structures provided, get their sizes and update min/max sizes
    if input_struct_id:
        cursor = await conn.execute(
            "SELECT name, size FROM structures WHERE id = ?",
            (input_struct_id,)
        )
        row = await cursor.fetchone()
        if row:
            input_struct_name, input_size = row
            await conn.execute("""
                UPDATE ioctls
                SET min_input_size = ?, max_input_size = ?
                WHERE id = ?
            """, (input_size, input_size, ioctl_id))

    if output_struct_id:
        cursor = await conn.execute(
            "SELECT name, size FROM structures WHERE id = ?",
            (output_struct_id,)
        )
        row = await cursor.fetchone()
        if row:
            output_struct_name, output_size = row
            await conn.execute("""
                UPDATE ioctls
                SET min_output_size = ?, max_output_size = ?
                WHERE id = ?
            """, (output_size, output_size, ioctl_id))

    await conn.commit()

    # Get updated info
    cursor = await conn.execute("""
        SELECT
            i.id, i.name,
            s1.name as input_struct_name, s1.size as input_size,
            s2.name as output_struct_name, s2.size as output_size
        FROM ioctls i
        LEFT JOIN structures s1 ON i.input_struct_id = s1.id
        LEFT JOIN structures s2 ON i.output_struct_id = s2.id
        WHERE i.id = ?
    """, (ioctl_id,))

    result = dict(await cursor.fetchone())

    return {
        'ioctl_id': ioctl_id,
        'ioctl_name': result['name'],
        'input_struct': {
            'id': input_struct_id,
            'name': result.get('input_struct_name'),
            'size': result.get('input_size')
        } if input_struct_id else None,
        'output_struct': {
            'id': output_struct_id,
            'name': result.get('output_struct_name'),
            'size': result.get('output_size')
        } if output_struct_id else None
    }
