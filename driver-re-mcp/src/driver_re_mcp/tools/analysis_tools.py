"""
Analysis session and utility tools for Driver RE MCP server.

Provides session management, note-taking, report generation, and various
utility functions for driver reverse engineering.
"""

import sqlite3
import json
from typing import Dict, List, Optional, Any
from datetime import datetime
from pathlib import Path


class AnalysisTools:
    """Analysis session and utility tools."""

    def __init__(self, db_path: str):
        """
        Initialize analysis tools.

        Args:
            db_path: Path to SQLite database
        """
        self.db_path = db_path

    def _get_db_connection(self):
        """Get SQLite database connection."""
        conn = sqlite3.connect(self.db_path)
        conn.row_factory = sqlite3.Row
        return conn


async def start_analysis_session(
    driver_id: str,
    name: Optional[str] = None,
    ghidra_project_path: Optional[str] = None,
    db_path: str = "./data/driver_re.db"
) -> Dict:
    """
    Start a new analysis session for a driver.

    Sessions track analysis progress, notes, and Ghidra project association.

    Args:
        driver_id: Driver UUID
        name: Optional session name (defaults to driver name + timestamp)
        ghidra_project_path: Optional path to Ghidra project file
        db_path: Path to SQLite database

    Returns:
        Created session record with ID
    """
    tools = AnalysisTools(db_path)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        # Get driver info for default name
        cursor.execute(
            "SELECT original_name, analyzed_name FROM drivers WHERE id = ?",
            (driver_id,)
        )
        row = cursor.fetchone()
        if not row:
            raise ValueError(f"Driver not found: {driver_id}")

        if not name:
            driver_name = row["analyzed_name"] or row["original_name"]
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            name = f"{driver_name}_{timestamp}"

        # Extract Ghidra project name if path provided
        ghidra_project_name = None
        if ghidra_project_path:
            ghidra_project_name = Path(ghidra_project_path).stem

        # Insert session
        cursor.execute("""
            INSERT INTO analysis_sessions (
                driver_id, name, ghidra_project_path, ghidra_project_name,
                status, started_at
            ) VALUES (?, ?, ?, ?, 'active', ?)
        """, (
            driver_id, name, ghidra_project_path, ghidra_project_name,
            datetime.now().isoformat()
        ))

        session_id = cursor.lastrowid
        conn.commit()

        return {
            "session_id": session_id,
            "driver_id": driver_id,
            "name": name,
            "ghidra_project_path": ghidra_project_path,
            "status": "active",
            "started_at": datetime.now().isoformat()
        }

    finally:
        conn.close()


async def add_analysis_note(
    driver_id: str,
    content: str,
    title: Optional[str] = None,
    note_type: str = "observation",
    priority: str = "medium",
    related_function_id: Optional[str] = None,
    related_ioctl_id: Optional[str] = None,
    rva: Optional[int] = None,
    db_path: str = "./data/driver_re.db"
) -> Dict:
    """
    Add an analysis note.

    Notes can be observations, TODOs, questions, or findings. They can be
    associated with specific functions, IOCTLs, or addresses.

    Args:
        driver_id: Driver UUID
        content: Note content
        title: Optional note title
        note_type: Type (observation, todo, question, finding)
        priority: Priority level (high, medium, low)
        related_function_id: Optional function UUID
        related_ioctl_id: Optional IOCTL UUID
        rva: Optional RVA this note refers to
        db_path: Path to SQLite database

    Returns:
        Created note record with ID

    Note Types:
        - observation: General finding or observation
        - todo: Action item for further investigation
        - question: Question that needs answering
        - finding: Security finding or vulnerability insight
    """
    tools = AnalysisTools(db_path)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        # Calculate VA if RVA provided
        va = None
        if rva is not None:
            cursor.execute("SELECT image_base FROM drivers WHERE id = ?", (driver_id,))
            row = cursor.fetchone()
            if row:
                va = row["image_base"] + rva

        # Get active session for this driver (if any)
        cursor.execute("""
            SELECT id FROM analysis_sessions
            WHERE driver_id = ? AND status = 'active'
            ORDER BY started_at DESC
            LIMIT 1
        """, (driver_id,))
        session_row = cursor.fetchone()
        session_id = session_row["id"] if session_row else None

        # Insert note
        cursor.execute("""
            INSERT INTO analysis_notes (
                driver_id, session_id, related_function_id, related_ioctl_id,
                rva, va, title, content, note_type, priority
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            driver_id, session_id, related_function_id, related_ioctl_id,
            rva, va, title, content, note_type, priority
        ))

        note_id = cursor.lastrowid
        conn.commit()

        return {
            "note_id": note_id,
            "driver_id": driver_id,
            "session_id": session_id,
            "title": title,
            "content": content,
            "note_type": note_type,
            "priority": priority,
            "rva": rva,
            "created_at": datetime.now().isoformat()
        }

    finally:
        conn.close()


async def get_analysis_notes(
    driver_id: str,
    note_type: Optional[str] = None,
    priority: Optional[str] = None,
    db_path: str = "./data/driver_re.db"
) -> List[Dict]:
    """
    Get analysis notes for a driver.

    Args:
        driver_id: Driver UUID
        note_type: Optional filter by type
        priority: Optional filter by priority
        db_path: Path to SQLite database

    Returns:
        List of notes with associated entity details
    """
    tools = AnalysisTools(db_path)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        # Build query with filters
        query = """
            SELECT
                n.id,
                n.driver_id,
                n.session_id,
                n.title,
                n.content,
                n.note_type,
                n.priority,
                n.rva,
                n.va,
                n.created_at,
                f.name as function_name,
                f.rva as function_rva,
                io.name as ioctl_name,
                io.code as ioctl_code
            FROM analysis_notes n
            LEFT JOIN functions f ON n.related_function_id = f.id
            LEFT JOIN ioctls io ON n.related_ioctl_id = io.id
            WHERE n.driver_id = ?
        """
        params = [driver_id]

        if note_type:
            query += " AND n.note_type = ?"
            params.append(note_type)

        if priority:
            query += " AND n.priority = ?"
            params.append(priority)

        query += " ORDER BY n.created_at DESC"

        cursor.execute(query, params)
        rows = cursor.fetchall()

        notes = []
        for row in rows:
            note = {
                "note_id": row["id"],
                "driver_id": row["driver_id"],
                "session_id": row["session_id"],
                "title": row["title"],
                "content": row["content"],
                "note_type": row["note_type"],
                "priority": row["priority"],
                "rva": row["rva"],
                "va": row["va"],
                "created_at": row["created_at"]
            }

            if row["function_name"]:
                note["related_function"] = {
                    "name": row["function_name"],
                    "rva": row["function_rva"]
                }

            if row["ioctl_name"]:
                note["related_ioctl"] = {
                    "name": row["ioctl_name"],
                    "code": row["ioctl_code"]
                }

            notes.append(note)

        return notes

    finally:
        conn.close()


async def generate_analysis_report(
    driver_id: str,
    format: str = "markdown",
    db_path: str = "./data/driver_re.db"
) -> str:
    """
    Generate a comprehensive analysis report.

    Args:
        driver_id: Driver UUID
        format: Output format (markdown, html, json)
        db_path: Path to SQLite database

    Returns:
        Formatted report as string

    Report includes:
        - Driver overview (file info, hashes, metadata)
        - All IOCTLs with vulnerability status
        - Dangerous APIs used
        - All vulnerabilities found
        - Attack chains
        - Analysis notes organized by type/priority
        - Statistics
    """
    tools = AnalysisTools(db_path)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        # Get driver info
        cursor.execute("SELECT * FROM drivers WHERE id = ?", (driver_id,))
        driver = dict(cursor.fetchone())

        # Get IOCTLs
        cursor.execute("""
            SELECT id, name, code, description, is_vulnerable,
                   vulnerability_type, vulnerability_severity
            FROM ioctls
            WHERE driver_id = ?
            ORDER BY code
        """, (driver_id,))
        ioctls = [dict(row) for row in cursor.fetchall()]

        # Get dangerous imports
        cursor.execute("""
            SELECT function_name, dll_name, category, danger_reason
            FROM imports
            WHERE driver_id = ? AND is_dangerous = 1
            ORDER BY function_name
        """, (driver_id,))
        dangerous_apis = [dict(row) for row in cursor.fetchall()]

        # Get vulnerabilities
        cursor.execute("""
            SELECT id, title, vulnerability_class, severity,
                   description, exploitation_difficulty
            FROM vulnerabilities
            WHERE driver_id = ?
            ORDER BY
                CASE severity
                    WHEN 'critical' THEN 1
                    WHEN 'high' THEN 2
                    WHEN 'medium' THEN 3
                    WHEN 'low' THEN 4
                    ELSE 5
                END
        """, (driver_id,))
        vulnerabilities = [dict(row) for row in cursor.fetchall()]

        # Get attack chains
        cursor.execute("""
            SELECT id, name, attack_goal, initial_access, final_privilege
            FROM attack_chains
            WHERE driver_id = ?
        """, (driver_id,))
        attack_chains = [dict(row) for row in cursor.fetchall()]

        # Get notes by type
        cursor.execute("""
            SELECT note_type, priority, title, content, rva
            FROM analysis_notes
            WHERE driver_id = ?
            ORDER BY
                CASE priority
                    WHEN 'high' THEN 1
                    WHEN 'medium' THEN 2
                    WHEN 'low' THEN 3
                END,
                created_at DESC
        """, (driver_id,))
        notes = [dict(row) for row in cursor.fetchall()]

        # Get statistics
        cursor.execute("""
            SELECT
                (SELECT COUNT(*) FROM imports WHERE driver_id = ?) as import_count,
                (SELECT COUNT(*) FROM exports WHERE driver_id = ?) as export_count,
                (SELECT COUNT(*) FROM ioctls WHERE driver_id = ?) as ioctl_count,
                (SELECT COUNT(*) FROM functions WHERE driver_id = ?) as function_count,
                (SELECT COUNT(*) FROM vulnerabilities WHERE driver_id = ?) as vuln_count
        """, (driver_id, driver_id, driver_id, driver_id, driver_id))
        stats = dict(cursor.fetchone())

        # Format report based on requested format
        if format == "json":
            return json.dumps({
                "driver": driver,
                "ioctls": ioctls,
                "dangerous_apis": dangerous_apis,
                "vulnerabilities": vulnerabilities,
                "attack_chains": attack_chains,
                "notes": notes,
                "statistics": stats
            }, indent=2, default=str)

        elif format == "markdown":
            return _generate_markdown_report(
                driver, ioctls, dangerous_apis, vulnerabilities,
                attack_chains, notes, stats
            )

        elif format == "html":
            # Convert markdown to HTML (basic implementation)
            md_report = _generate_markdown_report(
                driver, ioctls, dangerous_apis, vulnerabilities,
                attack_chains, notes, stats
            )
            # In production, use markdown library for proper conversion
            return f"<pre>{md_report}</pre>"

        else:
            raise ValueError(f"Unsupported format: {format}")

    finally:
        conn.close()


def _generate_markdown_report(
    driver: Dict,
    ioctls: List[Dict],
    dangerous_apis: List[Dict],
    vulnerabilities: List[Dict],
    attack_chains: List[Dict],
    notes: List[Dict],
    stats: Dict
) -> str:
    """Generate markdown formatted report."""

    lines = []

    # Header
    lines.append(f"# Driver Analysis Report: {driver['analyzed_name'] or driver['original_name']}")
    lines.append("")
    lines.append(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    lines.append("")

    # Driver Overview
    lines.append("## Driver Overview")
    lines.append("")
    lines.append(f"- **File Name:** {driver['original_name']}")
    lines.append(f"- **File Size:** {driver['file_size']:,} bytes")
    lines.append(f"- **MD5:** {driver['md5']}")
    lines.append(f"- **SHA256:** {driver['sha256']}")
    lines.append(f"- **Image Base:** 0x{driver['image_base']:X}")
    lines.append(f"- **Entry Point:** 0x{driver['entry_point_rva']:X}")
    if driver['file_description']:
        lines.append(f"- **Description:** {driver['file_description']}")
    if driver['company_name']:
        lines.append(f"- **Company:** {driver['company_name']}")
    if driver['file_version']:
        lines.append(f"- **Version:** {driver['file_version']}")
    lines.append("")

    # Statistics
    lines.append("## Statistics")
    lines.append("")
    lines.append(f"- **Imports:** {stats['import_count']}")
    lines.append(f"- **Exports:** {stats['export_count']}")
    lines.append(f"- **IOCTLs:** {stats['ioctl_count']}")
    lines.append(f"- **Functions:** {stats['function_count']}")
    lines.append(f"- **Vulnerabilities:** {stats['vuln_count']}")
    lines.append("")

    # IOCTLs
    if ioctls:
        lines.append("## IOCTLs")
        lines.append("")
        for ioctl in ioctls:
            lines.append(f"### {ioctl['name']}")
            if ioctl['code']:
                lines.append(f"- **Code:** 0x{ioctl['code']:X}")
            if ioctl['description']:
                lines.append(f"- **Description:** {ioctl['description']}")
            if ioctl['is_vulnerable']:
                lines.append(f"- **Vulnerable:** YES ({ioctl['vulnerability_severity']})")
                lines.append(f"- **Type:** {ioctl['vulnerability_type']}")
            lines.append("")

    # Dangerous APIs
    if dangerous_apis:
        lines.append("## Dangerous APIs")
        lines.append("")
        for api in dangerous_apis:
            lines.append(f"- **{api['dll_name']}!{api['function_name']}**")
            lines.append(f"  - Category: {api['category']}")
            if api['danger_reason']:
                lines.append(f"  - Reason: {api['danger_reason']}")
        lines.append("")

    # Vulnerabilities
    if vulnerabilities:
        lines.append("## Vulnerabilities")
        lines.append("")
        for vuln in vulnerabilities:
            lines.append(f"### [{vuln['severity'].upper()}] {vuln['title']}")
            lines.append("")
            lines.append(f"- **Class:** {vuln['vulnerability_class']}")
            lines.append(f"- **Difficulty:** {vuln['exploitation_difficulty']}")
            lines.append("")
            lines.append(vuln['description'])
            lines.append("")

    # Attack Chains
    if attack_chains:
        lines.append("## Attack Chains")
        lines.append("")
        for chain in attack_chains:
            lines.append(f"### {chain['name']}")
            lines.append(f"- **Goal:** {chain['attack_goal']}")
            lines.append(f"- **Initial Access:** {chain['initial_access']}")
            lines.append(f"- **Final Privilege:** {chain['final_privilege']}")
            lines.append("")

    # Notes by type
    if notes:
        lines.append("## Analysis Notes")
        lines.append("")

        # Group by type
        for note_type in ["finding", "todo", "question", "observation"]:
            type_notes = [n for n in notes if n["note_type"] == note_type]
            if type_notes:
                lines.append(f"### {note_type.title()}s")
                lines.append("")
                for note in type_notes:
                    priority_marker = {
                        "high": "High Priority",
                        "medium": "Medium Priority",
                        "low": "Low Priority"
                    }.get(note["priority"], "")

                    title = note["title"] or note["content"][:50]
                    lines.append(f"**[{priority_marker}] {title}**")
                    if note["rva"]:
                        lines.append(f"  - Address: 0x{note['rva']:X}")
                    if note["title"] and note["content"] != note["title"]:
                        lines.append(f"  - {note['content']}")
                    lines.append("")

    return "\n".join(lines)


async def convert_address(
    driver_id: str,
    address: int,
    from_type: str,
    to_type: str,
    db_path: str = "./data/driver_re.db"
) -> int:
    """
    Convert between address types (RVA, VA, file offset).

    Args:
        driver_id: Driver UUID
        address: Address to convert
        from_type: Source type (rva, va, file_offset)
        to_type: Destination type (rva, va, file_offset)
        db_path: Path to SQLite database

    Returns:
        Converted address

    Conversions:
        - RVA = Relative Virtual Address (offset from image base)
        - VA = Virtual Address (absolute in memory)
        - file_offset = Offset in PE file on disk

    Examples:
        convert_address(driver_id, 0x1000, "rva", "va")
        convert_address(driver_id, 0x140001000, "va", "rva")
    """
    tools = AnalysisTools(db_path)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        # Get driver and section info
        cursor.execute(
            "SELECT image_base, entry_point_rva FROM drivers WHERE id = ?",
            (driver_id,)
        )
        driver = cursor.fetchone()
        if not driver:
            raise ValueError(f"Driver not found: {driver_id}")

        image_base = driver["image_base"]

        # Convert to RVA first (common intermediate)
        if from_type == "rva":
            rva = address
        elif from_type == "va":
            rva = address - image_base
        elif from_type == "file_offset":
            # Need to find which section this offset belongs to
            cursor.execute("""
                SELECT virtual_address, raw_address
                FROM sections
                WHERE driver_id = ?
                    AND raw_address <= ?
                    AND raw_address + raw_size > ?
                ORDER BY raw_address
                LIMIT 1
            """, (driver_id, address, address))
            section = cursor.fetchone()
            if not section:
                raise ValueError(f"File offset 0x{address:X} not in any section")

            offset_in_section = address - section["raw_address"]
            rva = section["virtual_address"] + offset_in_section
        else:
            raise ValueError(f"Unknown from_type: {from_type}")

        # Convert from RVA to target type
        if to_type == "rva":
            return rva
        elif to_type == "va":
            return image_base + rva
        elif to_type == "file_offset":
            # Find which section this RVA belongs to
            cursor.execute("""
                SELECT virtual_address, raw_address
                FROM sections
                WHERE driver_id = ?
                    AND virtual_address <= ?
                    AND virtual_address + virtual_size > ?
                ORDER BY virtual_address
                LIMIT 1
            """, (driver_id, rva, rva))
            section = cursor.fetchone()
            if not section:
                raise ValueError(f"RVA 0x{rva:X} not in any section")

            offset_in_section = rva - section["virtual_address"]
            return section["raw_address"] + offset_in_section
        else:
            raise ValueError(f"Unknown to_type: {to_type}")

    finally:
        conn.close()


async def get_api_info(
    api_name: str,
    db_path: str = "./data/driver_re.db"
) -> Dict:
    """
    Get documentation and security info for a Windows kernel API.

    Args:
        api_name: API function name (e.g., "MmMapIoSpace")
        db_path: Path to SQLite database

    Returns:
        Dictionary containing:
            - description: What the API does
            - parameters: Parameter descriptions
            - return_value: Return value description
            - security_relevance: Why this matters for security
            - common_misuse: Common vulnerability patterns
            - msdn_url: Link to official documentation

    Note: This requires a pre-populated API reference database.
    For MVP, returns basic info from imports table if available.
    """
    tools = AnalysisTools(db_path)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        # Try to find API info from imports (if we've categorized it)
        cursor.execute("""
            SELECT DISTINCT
                function_name,
                dll_name,
                category,
                subcategory,
                description,
                is_dangerous,
                danger_reason,
                msdn_url
            FROM imports
            WHERE function_name = ?
            LIMIT 1
        """, (api_name,))

        row = cursor.fetchone()

        if row:
            return {
                "api_name": row["function_name"],
                "dll": row["dll_name"],
                "category": row["category"],
                "subcategory": row["subcategory"],
                "description": row["description"],
                "is_dangerous": bool(row["is_dangerous"]),
                "danger_reason": row["danger_reason"],
                "msdn_url": row["msdn_url"] or f"https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/{api_name.lower()}"
            }
        else:
            # Return basic stub
            return {
                "api_name": api_name,
                "description": "No information available",
                "is_dangerous": False,
                "msdn_url": f"https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/{api_name.lower()}"
            }

    finally:
        conn.close()


async def compare_drivers(
    driver_id_1: str,
    driver_id_2: str,
    db_path: str = "./data/driver_re.db"
) -> Dict:
    """
    Compare two drivers for similarities.

    Args:
        driver_id_1: First driver UUID
        driver_id_2: Second driver UUID
        db_path: Path to SQLite database

    Returns:
        Dictionary containing:
            - shared_imports: List of common imports
            - shared_exports: List of common exports
            - similar_ioctls: IOCTLs with similar names/codes
            - similar_vulnerabilities: Similar vulnerability patterns

    Use cases:
        - Identify driver families
        - Find code reuse
        - Compare vulnerability patterns
    """
    tools = AnalysisTools(db_path)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        # Get driver names
        cursor.execute("SELECT original_name FROM drivers WHERE id IN (?, ?)", (driver_id_1, driver_id_2))
        drivers = cursor.fetchall()
        if len(drivers) != 2:
            raise ValueError("One or both drivers not found")

        # Shared imports
        cursor.execute("""
            SELECT i1.function_name, i1.dll_name
            FROM imports i1
            JOIN imports i2 ON i1.function_name = i2.function_name AND i1.dll_name = i2.dll_name
            WHERE i1.driver_id = ? AND i2.driver_id = ?
            ORDER BY i1.function_name
        """, (driver_id_1, driver_id_2))
        shared_imports = [{"function": row["function_name"], "dll": row["dll_name"]} for row in cursor.fetchall()]

        # Shared exports
        cursor.execute("""
            SELECT e1.function_name
            FROM exports e1
            JOIN exports e2 ON e1.function_name = e2.function_name
            WHERE e1.driver_id = ? AND e2.driver_id = ?
            ORDER BY e1.function_name
        """, (driver_id_1, driver_id_2))
        shared_exports = [row["function_name"] for row in cursor.fetchall()]

        # Similar IOCTL codes
        cursor.execute("""
            SELECT
                io1.name as name1,
                io1.code as code1,
                io2.name as name2,
                io2.code as code2
            FROM ioctls io1
            JOIN ioctls io2 ON io1.code = io2.code
            WHERE io1.driver_id = ? AND io2.driver_id = ?
            ORDER BY io1.code
        """, (driver_id_1, driver_id_2))
        similar_ioctls = []
        for row in cursor.fetchall():
            similar_ioctls.append({
                "code": row["code1"],
                "driver1_name": row["name1"],
                "driver2_name": row["name2"]
            })

        # Similar vulnerabilities (by class)
        cursor.execute("""
            SELECT
                v1.vulnerability_class,
                v1.title as title1,
                v2.title as title2
            FROM vulnerabilities v1
            JOIN vulnerabilities v2 ON v1.vulnerability_class = v2.vulnerability_class
            WHERE v1.driver_id = ? AND v2.driver_id = ?
            ORDER BY v1.vulnerability_class
        """, (driver_id_1, driver_id_2))
        similar_vulnerabilities = []
        for row in cursor.fetchall():
            similar_vulnerabilities.append({
                "class": row["vulnerability_class"],
                "driver1_title": row["title1"],
                "driver2_title": row["title2"]
            })

        return {
            "driver1": drivers[0]["original_name"],
            "driver2": drivers[1]["original_name"],
            "shared_imports": shared_imports,
            "shared_imports_count": len(shared_imports),
            "shared_exports": shared_exports,
            "shared_exports_count": len(shared_exports),
            "similar_ioctls": similar_ioctls,
            "similar_vulnerabilities": similar_vulnerabilities
        }

    finally:
        conn.close()


async def get_statistics(
    driver_id: Optional[str] = None,
    db_path: str = "./data/driver_re.db"
) -> Dict:
    """
    Get statistics for a driver or all drivers.

    Args:
        driver_id: Optional driver UUID (None = all drivers)
        db_path: Path to SQLite database

    Returns:
        Dictionary with comprehensive statistics:
            - Driver counts (if all drivers)
            - Import/export counts
            - IOCTL count
            - Vulnerability breakdown by severity
            - API category breakdown
            - Analysis coverage metrics
    """
    tools = AnalysisTools(db_path)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        stats = {}

        if driver_id:
            # Single driver statistics
            cursor.execute("""
                SELECT
                    (SELECT COUNT(*) FROM imports WHERE driver_id = ?) as import_count,
                    (SELECT COUNT(*) FROM imports WHERE driver_id = ? AND is_dangerous = 1) as dangerous_import_count,
                    (SELECT COUNT(*) FROM exports WHERE driver_id = ?) as export_count,
                    (SELECT COUNT(*) FROM ioctls WHERE driver_id = ?) as ioctl_count,
                    (SELECT COUNT(*) FROM ioctls WHERE driver_id = ? AND is_vulnerable = 1) as vulnerable_ioctl_count,
                    (SELECT COUNT(*) FROM functions WHERE driver_id = ?) as function_count,
                    (SELECT COUNT(*) FROM vulnerabilities WHERE driver_id = ?) as vuln_count,
                    (SELECT COUNT(*) FROM analysis_notes WHERE driver_id = ?) as note_count
            """, (driver_id, driver_id, driver_id, driver_id, driver_id, driver_id, driver_id, driver_id))

            row = cursor.fetchone()
            stats["totals"] = dict(row)

            # Vulnerability breakdown
            cursor.execute("""
                SELECT severity, COUNT(*) as count
                FROM vulnerabilities
                WHERE driver_id = ?
                GROUP BY severity
            """, (driver_id,))
            stats["vulnerabilities_by_severity"] = {row["severity"]: row["count"] for row in cursor.fetchall()}

            # Import categories
            cursor.execute("""
                SELECT category, COUNT(*) as count
                FROM imports
                WHERE driver_id = ? AND category IS NOT NULL
                GROUP BY category
                ORDER BY count DESC
            """, (driver_id,))
            stats["imports_by_category"] = {row["category"]: row["count"] for row in cursor.fetchall()}

        else:
            # All drivers statistics
            cursor.execute("""
                SELECT
                    (SELECT COUNT(*) FROM drivers) as driver_count,
                    (SELECT COUNT(*) FROM imports) as total_imports,
                    (SELECT COUNT(*) FROM exports) as total_exports,
                    (SELECT COUNT(*) FROM ioctls) as total_ioctls,
                    (SELECT COUNT(*) FROM ioctls WHERE is_vulnerable = 1) as total_vulnerable_ioctls,
                    (SELECT COUNT(*) FROM vulnerabilities) as total_vulnerabilities
            """)
            row = cursor.fetchone()
            stats["totals"] = dict(row)

            # Vulnerability breakdown across all drivers
            cursor.execute("""
                SELECT severity, COUNT(*) as count
                FROM vulnerabilities
                GROUP BY severity
            """)
            stats["vulnerabilities_by_severity"] = {row["severity"]: row["count"] for row in cursor.fetchall()}

            # Driver analysis status
            cursor.execute("""
                SELECT analysis_status, COUNT(*) as count
                FROM drivers
                GROUP BY analysis_status
            """)
            stats["drivers_by_status"] = {row["analysis_status"]: row["count"] for row in cursor.fetchall()}

        return stats

    finally:
        conn.close()
