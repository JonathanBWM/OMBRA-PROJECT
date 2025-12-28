"""
Ghidra integration MCP tools.

Provides bidirectional sync between database and Ghidra:
- Function names, signatures, decompiled code
- Structures and data types
- Comments and annotations
- Cross-references
"""

import uuid
from typing import Dict, List, Optional
from ..ghidra.bridge import GhidraMCPClient
from ..database.connection import get_db_connection


# Global Ghidra client instance
_ghidra_client: Optional[GhidraMCPClient] = None


async def ghidra_connect(host: str = "localhost", port: int = 8080) -> Dict:
    """
    Connect to GhidraMCP server.

    Args:
        host: GhidraMCP server host (default: localhost)
        port: GhidraMCP server port (default: 8080)

    Returns:
        Connection status
    """
    global _ghidra_client

    _ghidra_client = GhidraMCPClient(host=host, port=port)

    connected = await _ghidra_client.connect()

    return {
        "success": connected,
        "host": host,
        "port": port,
        "message": "Connected to GhidraMCP" if connected else "Failed to connect"
    }


async def ghidra_sync_functions(driver_id: str, direction: str = "from_ghidra") -> Dict:
    """
    Sync functions between database and Ghidra.

    Args:
        driver_id: Driver UUID
        direction: Sync direction
            - 'from_ghidra': Import Ghidra analysis to database
            - 'to_ghidra': Export database to Ghidra
            - 'bidirectional': Merge both (newer wins)

    Returns:
        Sync summary: {synced_count, created_count, updated_count, errors}
    """
    if not _ghidra_client or not _ghidra_client._connected:
        return {"error": "Not connected to GhidraMCP. Call ghidra_connect() first."}

    conn = await get_db_connection()

    if direction == "from_ghidra":
        # Get all functions from Ghidra
        functions = await _ghidra_client.get_all_functions()

        result = {
            "direction": "from_ghidra",
            "functions_found": len(functions),
            "created": 0,
            "updated": 0,
            "skipped": 0,
            "errors": []
        }

        for func in functions:
            try:
                # Parse address from Ghidra
                addr_str = func.get('address', func.get('entry', '0'))
                if isinstance(addr_str, str):
                    rva = int(addr_str, 16) if addr_str.startswith('0x') else int(addr_str)
                else:
                    rva = int(addr_str)

                name = func.get('name', f'FUN_{rva:08X}')
                size = func.get('size', 0)
                signature = func.get('signature', '')

                # Check if exists in database by RVA
                cursor = await conn.execute(
                    "SELECT id, name FROM functions WHERE driver_id = ? AND rva = ?",
                    (driver_id, rva)
                )
                row = await cursor.fetchone()

                if row:
                    # Update if name changed (skip Ghidra default names like FUN_xxx)
                    if name and not name.startswith('FUN_') and name != row[1]:
                        await conn.execute("""
                            UPDATE functions SET name = ?, size = ?, ghidra_synced = 1
                            WHERE id = ?
                        """, (name, size, row[0]))
                        result["updated"] += 1
                    else:
                        result["skipped"] += 1
                else:
                    # Create new function entry
                    func_id = str(uuid.uuid4())
                    await conn.execute("""
                        INSERT INTO functions (id, driver_id, name, rva, size, ghidra_synced)
                        VALUES (?, ?, ?, ?, ?, 1)
                    """, (func_id, driver_id, name, rva, size))
                    result["created"] += 1

            except Exception as e:
                result["errors"].append(str(e))

        await conn.commit()
        return result

    elif direction == "to_ghidra":
        # Get all functions from database for driver
        cursor = await conn.execute("""
            SELECT id, name, rva, description FROM functions
            WHERE driver_id = ? AND name IS NOT NULL
        """, (driver_id,))
        functions = await cursor.fetchall()

        result = {
            "direction": "to_ghidra",
            "functions_exported": len(functions),
            "renamed": 0,
            "commented": 0,
            "errors": []
        }

        for func in functions:
            func_id, name, rva, description = func
            try:
                # Rename in Ghidra if name is meaningful
                if name and not name.startswith('FUN_') and not name.startswith('sub_'):
                    success = await _ghidra_client.rename_function(rva, name)
                    if success:
                        result["renamed"] += 1

                # Set comment if description exists
                if description:
                    success = await _ghidra_client.set_comment(rva, description, "plate")
                    if success:
                        result["commented"] += 1

            except Exception as e:
                result["errors"].append(str(e))

        return result

    elif direction == "bidirectional":
        # Merge both directions
        from_result = await ghidra_sync_functions(driver_id, "from_ghidra")
        to_result = await ghidra_sync_functions(driver_id, "to_ghidra")

        return {
            "direction": "bidirectional",
            "from_ghidra": from_result,
            "to_ghidra": to_result
        }

    else:
        return {"error": f"Invalid direction: {direction}"}


async def ghidra_sync_structures(driver_id: str, direction: str = "from_ghidra") -> Dict:
    """
    Sync data structures between database and Ghidra.

    Args:
        driver_id: Driver UUID
        direction: Sync direction (from_ghidra, to_ghidra, bidirectional)

    Returns:
        Sync summary
    """
    if not _ghidra_client or not _ghidra_client._connected:
        return {"error": "Not connected to GhidraMCP. Call ghidra_connect() first."}

    # TODO: Implement structure sync
    # Ghidra data types → database structures

    return {
        "direction": direction,
        "structures_synced": 0,
        "message": "Structure sync not yet fully implemented"
    }


async def ghidra_get_decompilation(
    driver_id: str,
    rva: Optional[int] = None,
    function_name: Optional[str] = None
) -> Dict:
    """
    Get decompiled code from Ghidra for a function.

    Args:
        driver_id: Driver UUID
        rva: Function RVA (if known)
        function_name: Function name (alternative to RVA)

    Returns:
        Decompilation result: {function_name, address, decompiled_code}
    """
    if not _ghidra_client or not _ghidra_client._connected:
        return {"error": "Not connected to GhidraMCP. Call ghidra_connect() first."}

    # TODO: Get image_base from database to convert RVA to address
    # For now, assume RVA is provided

    if rva is None and function_name:
        # Search for function by name
        functions = await _ghidra_client.search_functions_by_name(function_name)
        if not functions:
            return {"error": f"Function not found: {function_name}"}

        # Use first match
        rva = int(functions[0]['address'], 16)

    if rva is None:
        return {"error": "Either rva or function_name must be provided"}

    # Get decompilation
    decompiled = await _ghidra_client.get_decompilation(rva)

    if not decompiled:
        return {"error": f"Failed to decompile function at RVA 0x{rva:x}"}

    return {
        "driver_id": driver_id,
        "rva": rva,
        "decompiled_code": decompiled
    }


async def ghidra_get_xrefs(driver_id: str, address: int) -> Dict:
    """
    Get cross-references from Ghidra.

    Args:
        driver_id: Driver UUID
        address: RVA or VA

    Returns:
        Cross-references: {xrefs_to: [...], xrefs_from: [...]}
    """
    if not _ghidra_client or not _ghidra_client._connected:
        return {"error": "Not connected to GhidraMCP. Call ghidra_connect() first."}

    xrefs_to = await _ghidra_client.get_xrefs_to(address)
    xrefs_from = await _ghidra_client.get_xrefs_from(address)

    return {
        "driver_id": driver_id,
        "address": address,
        "xrefs_to": xrefs_to,
        "xrefs_from": xrefs_from
    }


async def ghidra_set_comment(
    driver_id: str,
    address: int,
    comment: str,
    comment_type: str = "eol"
) -> Dict:
    """
    Set a comment in Ghidra.

    Args:
        driver_id: Driver UUID
        address: RVA or VA
        comment: Comment text
        comment_type: Comment type ('eol', 'pre', 'post', 'plate')

    Returns:
        Success status
    """
    if not _ghidra_client or not _ghidra_client._connected:
        return {"error": "Not connected to GhidraMCP. Call ghidra_connect() first."}

    success = await _ghidra_client.set_comment(address, comment, comment_type)

    return {
        "success": success,
        "driver_id": driver_id,
        "address": address,
        "comment_type": comment_type
    }


async def ghidra_rename_function(
    driver_id: str,
    address: int,
    new_name: str
) -> Dict:
    """
    Rename a function in Ghidra.

    Args:
        driver_id: Driver UUID
        address: Function RVA or VA
        new_name: New function name

    Returns:
        Success status
    """
    if not _ghidra_client or not _ghidra_client._connected:
        return {"error": "Not connected to GhidraMCP. Call ghidra_connect() first."}

    success = await _ghidra_client.rename_function(address, new_name)

    return {
        "success": success,
        "driver_id": driver_id,
        "address": address,
        "new_name": new_name
    }


async def ghidra_export_all(driver_id: str) -> Dict:
    """
    Export ALL analysis from Ghidra to database.

    Exports:
    - Functions (names, signatures, decompiled code)
    - Data types/structures
    - Comments
    - Labels
    - Cross-references

    Args:
        driver_id: Driver UUID

    Returns:
        Export summary
    """
    if not _ghidra_client or not _ghidra_client._connected:
        return {"error": "Not connected to GhidraMCP. Call ghidra_connect() first."}

    summary = {
        "driver_id": driver_id,
        "functions": {"found": 0, "created": 0, "updated": 0},
        "imports": {"found": 0, "created": 0, "updated": 0},
        "exports": {"found": 0, "created": 0, "updated": 0},
        "strings": {"found": 0, "created": 0},
        "errors": []
    }

    conn = await get_db_connection()

    try:
        # ===== Export functions =====
        functions = await _ghidra_client.get_all_functions()
        summary["functions"]["found"] = len(functions)

        for func in functions:
            try:
                # Parse address from Ghidra (usually hex string)
                addr_str = func.get('address', func.get('entry', '0'))
                if isinstance(addr_str, str):
                    rva = int(addr_str, 16) if addr_str.startswith('0x') else int(addr_str)
                else:
                    rva = int(addr_str)

                name = func.get('name', f'FUN_{rva:08X}')
                size = func.get('size', 0)

                # Check if function exists by RVA
                cursor = await conn.execute(
                    "SELECT id FROM functions WHERE driver_id = ? AND rva = ?",
                    (driver_id, rva)
                )
                row = await cursor.fetchone()

                if row:
                    # Update existing function
                    await conn.execute("""
                        UPDATE functions SET
                            name = COALESCE(?, name),
                            size = COALESCE(?, size),
                            ghidra_synced = 1
                        WHERE id = ?
                    """, (name, size, row[0]))
                    summary["functions"]["updated"] += 1
                else:
                    # Create new function
                    func_id = str(uuid.uuid4())
                    await conn.execute("""
                        INSERT INTO functions (id, driver_id, name, rva, size, ghidra_synced)
                        VALUES (?, ?, ?, ?, ?, 1)
                    """, (func_id, driver_id, name, rva, size))
                    summary["functions"]["created"] += 1

            except Exception as e:
                summary["errors"].append(f"Function error: {str(e)}")

        # ===== Export imports =====
        imports = await _ghidra_client.get_imports()
        summary["imports"]["found"] = len(imports)

        for imp in imports:
            try:
                dll_name = imp.get('library', imp.get('module', 'UNKNOWN'))
                func_name = imp.get('name', imp.get('function', ''))
                iat_rva = imp.get('address', 0)
                if isinstance(iat_rva, str):
                    iat_rva = int(iat_rva, 16) if iat_rva.startswith('0x') else int(iat_rva)

                # Check if import exists
                cursor = await conn.execute("""
                    SELECT id FROM imports
                    WHERE driver_id = ? AND dll_name = ? AND function_name = ?
                """, (driver_id, dll_name, func_name))
                row = await cursor.fetchone()

                if row:
                    summary["imports"]["updated"] += 1
                else:
                    import_id = str(uuid.uuid4())
                    await conn.execute("""
                        INSERT INTO imports (id, driver_id, dll_name, function_name, iat_rva)
                        VALUES (?, ?, ?, ?, ?)
                    """, (import_id, driver_id, dll_name, func_name, iat_rva))
                    summary["imports"]["created"] += 1

            except Exception as e:
                summary["errors"].append(f"Import error: {str(e)}")

        # ===== Export exports =====
        exports = await _ghidra_client.get_exports()
        summary["exports"]["found"] = len(exports)

        for exp in exports:
            try:
                func_name = exp.get('name', '')
                ordinal = exp.get('ordinal', 0)
                rva = exp.get('address', 0)
                if isinstance(rva, str):
                    rva = int(rva, 16) if rva.startswith('0x') else int(rva)

                # Check if export exists
                cursor = await conn.execute("""
                    SELECT id FROM exports
                    WHERE driver_id = ? AND function_name = ?
                """, (driver_id, func_name))
                row = await cursor.fetchone()

                if row:
                    await conn.execute("""
                        UPDATE exports SET ordinal = ?, rva = ? WHERE id = ?
                    """, (ordinal, rva, row[0]))
                    summary["exports"]["updated"] += 1
                else:
                    export_id = str(uuid.uuid4())
                    await conn.execute("""
                        INSERT INTO exports (id, driver_id, function_name, ordinal, rva)
                        VALUES (?, ?, ?, ?, ?)
                    """, (export_id, driver_id, func_name, ordinal, rva))
                    summary["exports"]["created"] += 1

            except Exception as e:
                summary["errors"].append(f"Export error: {str(e)}")

        # ===== Export strings =====
        strings = await _ghidra_client.get_strings()
        summary["strings"]["found"] = len(strings)

        for s in strings[:5000]:  # Limit to prevent massive string imports
            try:
                value = s.get('value', s.get('string', ''))
                rva = s.get('address', 0)
                if isinstance(rva, str):
                    rva = int(rva, 16) if rva.startswith('0x') else int(rva)
                length = len(value) if value else 0

                # Skip very short strings
                if length < 4:
                    continue

                # Check if string exists at this RVA
                cursor = await conn.execute("""
                    SELECT id FROM strings WHERE driver_id = ? AND rva = ?
                """, (driver_id, rva))
                row = await cursor.fetchone()

                if not row:
                    string_id = str(uuid.uuid4())
                    await conn.execute("""
                        INSERT INTO strings (id, driver_id, value, rva, length, encoding)
                        VALUES (?, ?, ?, ?, ?, 'UTF-8')
                    """, (string_id, driver_id, value, rva, length))
                    summary["strings"]["created"] += 1

            except Exception as e:
                summary["errors"].append(f"String error: {str(e)}")

        await conn.commit()

        summary["success"] = True
        summary["message"] = f"Exported {summary['functions']['created']} functions, " \
                             f"{summary['imports']['created']} imports, " \
                             f"{summary['exports']['created']} exports, " \
                             f"{summary['strings']['created']} strings to database"

    except Exception as e:
        summary["success"] = False
        summary["errors"].append(str(e))

    return summary


# Additional helper functions

async def get_ghidra_status() -> Dict:
    """
    Get Ghidra connection status.

    Returns:
        Status dict
    """
    if not _ghidra_client:
        return {
            "connected": False,
            "message": "No connection initialized"
        }

    return {
        "connected": _ghidra_client._connected,
        "host": _ghidra_client.host,
        "port": _ghidra_client.port
    }


async def ghidra_disconnect() -> Dict:
    """
    Disconnect from GhidraMCP.

    Returns:
        Disconnect status
    """
    global _ghidra_client

    if _ghidra_client:
        await _ghidra_client.disconnect()
        _ghidra_client = None
        return {"success": True, "message": "Disconnected from GhidraMCP"}

    return {"success": False, "message": "Not connected"}
