"""
File indexing and management tools.

Provides capabilities for:
- Indexing individual files and directories
- Searching files by content and metadata
- Retrieving file analysis results
- Tracking implementation status at file level
"""

import os
from pathlib import Path
from typing import Optional
from datetime import datetime

from ..parser import analyze_file, scan_directory
from ..database import fetch_one, fetch_all, insert, update, delete, count, search_fts


async def index_file(
    file_path: str,
    component_id: Optional[int] = None,
    force_reindex: bool = False
) -> dict:
    """
    Index a single source file.

    Parses the file for functions, structs, and stubs, then stores
    all analysis results in the database.

    Args:
        file_path: Absolute path to the file
        component_id: Optional component ID to associate with
        force_reindex: Force re-indexing even if file hasn't changed

    Returns:
        Summary of indexing results including counts of functions,
        structs, and stubs found.
    """
    # Resolve to absolute path
    path = Path(file_path).resolve()

    if not path.exists():
        return {
            "success": False,
            "error": f"File not found: {file_path}"
        }

    # Get file modification time
    mtime = path.stat().st_mtime

    # Check if file already indexed
    existing = await fetch_one(
        "SELECT id, mtime, content_hash FROM files WHERE absolute_path = ?",
        (str(path),)
    )

    if existing and not force_reindex:
        # Check if file has changed
        if existing['mtime'] == mtime:
            return {
                "success": True,
                "skipped": True,
                "reason": "File unchanged since last index",
                "file_id": existing['id']
            }

    # Parse the file
    analysis = analyze_file(str(path))

    if analysis is None:
        return {
            "success": False,
            "error": f"Unable to analyze file (unsupported type or parse error): {file_path}"
        }

    # Prepare file data
    file_data = {
        "absolute_path": analysis.path,
        "relative_path": None,  # Will be computed later if needed
        "filename": path.name,
        "extension": path.suffix,
        "component_id": component_id,
        "language": analysis.language,
        "line_count": analysis.line_count,
        "function_count": len(analysis.functions),
        "struct_count": len(analysis.structs),
        "stub_line_count": len([s for s in analysis.stubs]),
        "todo_count": analysis.todo_count,
        "fixme_count": analysis.fixme_count,
        "content_hash": analysis.content_hash,
        "last_indexed": datetime.now().isoformat(),
        "mtime": mtime,
    }

    # Insert or update file record
    if existing:
        file_id = existing['id']
        await update("files", file_data, "id = ?", (file_id,))

        # Delete old functions and structs (will re-insert)
        await delete("functions", "file_id = ?", (file_id,))
        await delete("structs", "file_id = ?", (file_id,))
        await delete("stubs", "file_id = ?", (file_id,))
    else:
        file_id = await insert("files", file_data)

    # Insert functions
    function_ids = []
    stub_count = 0

    for func in analysis.functions:
        func_data = {
            "file_id": file_id,
            "name": func.name,
            "signature": func.signature,
            "return_type": func.return_type,
            "line_start": func.line_start,
            "line_end": func.line_end,
            "is_stub": 1 if func.is_stub else 0,
            "stub_reason": func.stub_reason,
            "has_doc_comment": 1 if func.doc_comment else 0,
            "doc_comment": func.doc_comment,
        }

        func_id = await insert("functions", func_data)
        function_ids.append(func_id)

        if func.is_stub:
            stub_count += 1

    # Insert structs
    struct_ids = []

    for struct in analysis.structs:
        struct_data = {
            "file_id": file_id,
            "name": struct.name,
            "definition": struct.definition[:1000],  # Limit size
            "member_count": struct.member_count,
            "line_start": struct.line_start,
            "line_end": struct.line_end,
        }

        struct_id = await insert("structs", struct_data)
        struct_ids.append(struct_id)

    # Insert stubs
    stub_ids = []

    for stub in analysis.stubs:
        # Find associated function_id if any
        function_id = None
        if stub.function_name:
            for fid, func in zip(function_ids, analysis.functions):
                if func.name == stub.function_name:
                    function_id = fid
                    break

        stub_data = {
            "file_id": file_id,
            "function_id": function_id,
            "line_number": stub.line_number,
            "stub_type": stub.stub_type,
            "stub_text": stub.stub_text[:500],  # Limit size
            "surrounding_context": stub.context[:1000],
        }

        stub_id = await insert("stubs", stub_data)
        stub_ids.append(stub_id)

    return {
        "success": True,
        "file_id": file_id,
        "path": str(path),
        "language": analysis.language,
        "line_count": analysis.line_count,
        "functions_indexed": len(function_ids),
        "functions_stubbed": stub_count,
        "structs_indexed": len(struct_ids),
        "stubs_detected": len(stub_ids),
        "todo_count": analysis.todo_count,
        "fixme_count": analysis.fixme_count,
    }


async def index_directory(
    directory_path: str,
    component_id: Optional[int] = None,
    recursive: bool = True,
    file_extensions: Optional[list[str]] = None
) -> dict:
    """
    Index all source files in a directory.

    Scans the directory for source files and indexes each one.

    Args:
        directory_path: Path to directory
        component_id: Optional component ID to associate all files with
        recursive: Whether to scan subdirectories
        file_extensions: List of extensions to include (e.g., ['.c', '.h'])

    Returns:
        Summary statistics of the indexing operation.
    """
    dir_path = Path(directory_path).resolve()

    if not dir_path.exists():
        return {
            "success": False,
            "error": f"Directory not found: {directory_path}"
        }

    if not dir_path.is_dir():
        return {
            "success": False,
            "error": f"Path is not a directory: {directory_path}"
        }

    # Scan directory for files
    analyses = scan_directory(
        str(dir_path),
        extensions=file_extensions,
        recursive=recursive
    )

    # Index each file
    results = {
        "success": True,
        "directory": str(dir_path),
        "total_files": len(analyses),
        "indexed": 0,
        "skipped": 0,
        "errors": 0,
        "total_functions": 0,
        "total_structs": 0,
        "total_stubs": 0,
        "files": []
    }

    for analysis in analyses:
        result = await index_file(
            analysis.path,
            component_id=component_id,
            force_reindex=False
        )

        if result.get("success"):
            if result.get("skipped"):
                results["skipped"] += 1
            else:
                results["indexed"] += 1
                results["total_functions"] += result.get("functions_indexed", 0)
                results["total_structs"] += result.get("structs_indexed", 0)
                results["total_stubs"] += result.get("stubs_detected", 0)
        else:
            results["errors"] += 1

        results["files"].append(result)

    return results


async def search_files(
    query: str,
    component_id: Optional[int] = None,
    extension: Optional[str] = None,
    contains_stubs: Optional[bool] = None
) -> dict:
    """
    Search for files by name or path.

    Uses full-text search on filename and relative_path fields.

    Args:
        query: Search query (matches filename and path)
        component_id: Filter by component
        extension: Filter by file extension (e.g., '.c')
        contains_stubs: Filter by stub presence

    Returns:
        List of matching files with metadata.
    """
    # Start with FTS search if query provided
    if query:
        fts_results = await search_fts("files_fts", query, limit=100)
        file_ids = [r['rowid'] for r in fts_results]

        if not file_ids:
            return {
                "success": True,
                "count": 0,
                "files": []
            }

        # Build additional filters
        where_parts = [f"id IN ({','.join('?' * len(file_ids))})"]
        params = file_ids
    else:
        where_parts = ["1=1"]
        params = []

    # Add filters
    if component_id is not None:
        where_parts.append("component_id = ?")
        params.append(component_id)

    if extension is not None:
        where_parts.append("extension = ?")
        params.append(extension)

    if contains_stubs is not None:
        if contains_stubs:
            where_parts.append("stub_line_count > 0")
        else:
            where_parts.append("stub_line_count = 0")

    # Execute query
    where_clause = " AND ".join(where_parts)
    files = await fetch_all(
        f"""
        SELECT
            id, absolute_path, filename, extension, language,
            component_id, line_count, function_count, struct_count,
            stub_line_count, todo_count, fixme_count, last_indexed
        FROM files
        WHERE {where_clause}
        ORDER BY filename
        LIMIT 100
        """,
        tuple(params)
    )

    return {
        "success": True,
        "count": len(files),
        "files": files
    }


async def get_file_info(file_path: str) -> dict:
    """
    Get complete information about an indexed file.

    Returns file metadata, all functions, structs, stubs, and
    feature linkages.

    Args:
        file_path: Absolute path to the file

    Returns:
        Complete file information including all contained entities.
    """
    path = Path(file_path).resolve()

    # Get file record
    file_record = await fetch_one(
        """
        SELECT
            f.*,
            c.name as component_name,
            c.component_type
        FROM files f
        LEFT JOIN components c ON f.component_id = c.id
        WHERE f.absolute_path = ?
        """,
        (str(path),)
    )

    if not file_record:
        return {
            "success": False,
            "error": f"File not indexed: {file_path}"
        }

    file_id = file_record['id']

    # Get all functions
    functions = await fetch_all(
        """
        SELECT
            id, name, signature, return_type,
            line_start, line_end, is_stub, stub_reason,
            has_doc_comment
        FROM functions
        WHERE file_id = ?
        ORDER BY line_start
        """,
        (file_id,)
    )

    # Get all structs
    structs = await fetch_all(
        """
        SELECT
            id, name, member_count, line_start, line_end
        FROM structs
        WHERE file_id = ?
        ORDER BY line_start
        """,
        (file_id,)
    )

    # Get all stubs
    stubs = await fetch_all(
        """
        SELECT
            id, line_number, stub_type, stub_text,
            function_id, feature_id, severity,
            resolved_at
        FROM stubs
        WHERE file_id = ?
        ORDER BY line_number
        """,
        (file_id,)
    )

    # Get linked features
    features = await fetch_all(
        """
        SELECT
            f.id, f.name, f.status, ff.role, ff.confidence
        FROM features f
        JOIN feature_files ff ON f.id = ff.feature_id
        WHERE ff.file_id = ?
        """,
        (file_id,)
    )

    return {
        "success": True,
        "file": dict(file_record),
        "functions": functions,
        "functions_count": len(functions),
        "functions_stubbed": sum(1 for f in functions if f['is_stub']),
        "structs": structs,
        "structs_count": len(structs),
        "stubs": stubs,
        "stubs_count": len(stubs),
        "stubs_unresolved": sum(1 for s in stubs if not s['resolved_at']),
        "features": features,
        "features_count": len(features),
    }


async def list_files(
    component_id: Optional[int] = None,
    extension: Optional[str] = None,
    limit: int = 100
) -> dict:
    """
    List indexed files with optional filters.

    Args:
        component_id: Filter by component
        extension: Filter by file extension
        limit: Maximum number of results

    Returns:
        List of files matching the filters.
    """
    where_parts = ["1=1"]
    params = []

    if component_id is not None:
        where_parts.append("component_id = ?")
        params.append(component_id)

    if extension is not None:
        where_parts.append("extension = ?")
        params.append(extension)

    where_clause = " AND ".join(where_parts)

    files = await fetch_all(
        f"""
        SELECT
            id, filename, absolute_path, extension, language,
            component_id, line_count, function_count, struct_count,
            stub_line_count, todo_count, fixme_count, last_indexed
        FROM files
        WHERE {where_clause}
        ORDER BY filename
        LIMIT ?
        """,
        tuple(params + [limit])
    )

    total = await count("files", where_clause, tuple(params))

    return {
        "success": True,
        "total": total,
        "returned": len(files),
        "files": files
    }


# Export tools
FILE_TOOLS = {
    "index_file": index_file,
    "index_directory": index_directory,
    "search_files": search_files,
    "get_file_info": get_file_info,
    "list_files": list_files,
}
