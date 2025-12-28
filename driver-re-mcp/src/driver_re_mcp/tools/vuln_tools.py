"""
Vulnerability management tools for driver reverse engineering.
Handles vulnerability tracking, exploitation documentation, and attack chain composition.
"""

import uuid
from typing import Dict, List, Optional
from datetime import datetime
import json


async def add_vulnerability(
    driver_id: str,
    title: str,
    vulnerability_class: str,
    severity: str,
    description: str,
    technical_details: Optional[str] = None,
    affected_ioctl_id: Optional[str] = None,
    affected_function_id: Optional[str] = None,
    exploitation_difficulty: Optional[str] = None,
    exploitation_requirements: Optional[str] = None,
    exploitation_steps: Optional[List[Dict]] = None,
    poc_code: Optional[str] = None,
    poc_language: Optional[str] = None,
    cve_id: Optional[str] = None,
    cvss_score: Optional[float] = None,
    mitigations: Optional[str] = None,
    references: Optional[List[Dict]] = None
) -> Dict:
    """
    Add a vulnerability finding to the database.

    Args:
        driver_id: Driver UUID
        title: Vulnerability title (e.g., "Arbitrary Physical Memory Read via IOCTL 0x12345")
        vulnerability_class: Type (arbitrary_read, arbitrary_write, code_exec, info_leak, dos, etc.)
        severity: Severity level (critical, high, medium, low, info)
        description: High-level vulnerability description
        technical_details: In-depth technical analysis
        affected_ioctl_id: IOCTL UUID if vulnerability is in IOCTL handler
        affected_function_id: Function UUID containing the vulnerability
        exploitation_difficulty: How hard to exploit (trivial, easy, moderate, hard, theoretical)
        exploitation_requirements: Prerequisites for exploitation
        exploitation_steps: Ordered list of exploitation steps [{'step': 1, 'description': '...'}, ...]
        poc_code: Proof-of-concept code
        poc_language: PoC language (c, cpp, python, etc.)
        cve_id: CVE identifier if assigned
        cvss_score: CVSS score (0.0-10.0)
        mitigations: Mitigation/remediation guidance
        references: External references [{'title': '...', 'url': '...'}, ...]

    Returns:
        Vulnerability details including UUID
    """
    from ..database.connection import get_db_connection
    from ..embeddings.provider import generate_embedding

    vuln_id = str(uuid.uuid4())
    now = datetime.utcnow().isoformat()

    # Validate severity
    valid_severities = ['critical', 'high', 'medium', 'low', 'info']
    if severity.lower() not in valid_severities:
        raise ValueError(f"Invalid severity. Must be one of: {', '.join(valid_severities)}")

    # Validate exploitation_difficulty
    if exploitation_difficulty:
        valid_difficulties = ['trivial', 'easy', 'moderate', 'hard', 'theoretical']
        if exploitation_difficulty.lower() not in valid_difficulties:
            raise ValueError(f"Invalid difficulty. Must be one of: {', '.join(valid_difficulties)}")

    # Generate embedding for semantic search
    embedding_text = f"Vulnerability: {title}. {description}. Technical: {technical_details or ''}. Class: {vulnerability_class}"
    embedding = await generate_embedding(embedding_text)

    conn = await get_db_connection()

    # Insert vulnerability
    await conn.execute("""
        INSERT INTO vulnerabilities (
            id, driver_id, title, vulnerability_class, severity,
            cvss_score, cve_id,
            affected_ioctl_id, affected_function_id,
            description, technical_details,
            exploitation_difficulty, exploitation_requirements,
            exploitation_steps,
            poc_code, poc_language,
            mitigations, references,
            status, created_at, updated_at, embedding
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    """, (
        vuln_id, driver_id, title, vulnerability_class, severity.lower(),
        cvss_score, cve_id,
        affected_ioctl_id, affected_function_id,
        description, technical_details,
        exploitation_difficulty, exploitation_requirements,
        json.dumps(exploitation_steps) if exploitation_steps else None,
        poc_code, poc_language,
        mitigations, json.dumps(references) if references else None,
        'confirmed', now, now, json.dumps(embedding)
    ))

    # If affected IOCTL, mark it as vulnerable
    if affected_ioctl_id:
        await conn.execute("""
            UPDATE ioctls
            SET is_vulnerable = 1,
                vulnerability_type = ?,
                vulnerability_severity = ?,
                vulnerability_description = ?
            WHERE id = ?
        """, (vulnerability_class, severity.lower(), description, affected_ioctl_id))

    await conn.commit()

    return {
        'vuln_id': vuln_id,
        'title': title,
        'severity': severity.lower(),
        'vulnerability_class': vulnerability_class,
        'cve_id': cve_id,
        'affected_ioctl_id': affected_ioctl_id,
        'affected_function_id': affected_function_id
    }


async def get_vulnerability(vuln_id: str) -> Dict:
    """
    Get full vulnerability details.

    Returns:
    - Complete vulnerability information
    - Affected IOCTL details
    - Affected function details
    - Exploitation steps
    - PoC code
    - Related attack chains

    Args:
        vuln_id: Vulnerability UUID

    Returns:
        Complete vulnerability details
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    # Get vulnerability
    cursor = await conn.execute(
        "SELECT * FROM vulnerabilities WHERE id = ?",
        (vuln_id,)
    )
    row = await cursor.fetchone()
    if not row:
        raise ValueError(f"Vulnerability {vuln_id} not found")

    vuln = dict(row)

    # Parse JSON fields
    vuln['exploitation_steps'] = json.loads(vuln['exploitation_steps']) if vuln['exploitation_steps'] else []
    vuln['references'] = json.loads(vuln['references']) if vuln['references'] else []

    # Get affected IOCTL details
    if vuln['affected_ioctl_id']:
        cursor = await conn.execute("""
            SELECT id, name, code, code_hex, handler_rva,
                   min_input_size, max_input_size
            FROM ioctls WHERE id = ?
        """, (vuln['affected_ioctl_id'],))
        row = await cursor.fetchone()
        if row:
            vuln['affected_ioctl'] = dict(row)

    # Get affected function details
    if vuln['affected_function_id']:
        cursor = await conn.execute("""
            SELECT id, name, rva, va, decompiled
            FROM functions WHERE id = ?
        """, (vuln['affected_function_id'],))
        row = await cursor.fetchone()
        if row:
            vuln['affected_function'] = dict(row)

    # Get related attack chains
    cursor = await conn.execute("""
        SELECT id, name, attack_goal, description
        FROM attack_chains
        WHERE driver_id = ? AND steps LIKE ?
    """, (vuln['driver_id'], f'%{vuln_id}%'))

    vuln['related_attack_chains'] = [dict(r) for r in await cursor.fetchall()]

    return vuln


async def list_vulnerabilities(
    driver_id: Optional[str] = None,
    severity: Optional[str] = None,
    vulnerability_class: Optional[str] = None,
    status: Optional[str] = None
) -> List[Dict]:
    """
    List vulnerabilities with optional filtering.

    Args:
        driver_id: Filter by driver UUID
        severity: Filter by severity (critical, high, medium, low, info)
        vulnerability_class: Filter by class (arbitrary_read, arbitrary_write, etc.)
        status: Filter by status (suspected, confirmed, exploited, patched)

    Returns:
        List of vulnerability summaries
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    query = """
        SELECT
            v.id, v.title, v.vulnerability_class, v.severity,
            v.cvss_score, v.cve_id, v.status,
            i.name as affected_ioctl_name,
            f.name as affected_function_name
        FROM vulnerabilities v
        LEFT JOIN ioctls i ON v.affected_ioctl_id = i.id
        LEFT JOIN functions f ON v.affected_function_id = f.id
        WHERE 1=1
    """
    params = []

    if driver_id:
        query += " AND v.driver_id = ?"
        params.append(driver_id)

    if severity:
        query += " AND v.severity = ?"
        params.append(severity.lower())

    if vulnerability_class:
        query += " AND v.vulnerability_class = ?"
        params.append(vulnerability_class)

    if status:
        query += " AND v.status = ?"
        params.append(status)

    query += " ORDER BY v.severity DESC, v.cvss_score DESC"

    cursor = await conn.execute(query, params)
    vulns = [dict(row) for row in await cursor.fetchall()]

    return vulns


async def create_attack_chain(
    driver_id: str,
    name: str,
    attack_goal: str,
    steps: List[Dict],
    initial_access: str,
    final_privilege: str,
    description: Optional[str] = None,
    poc_code: Optional[str] = None
) -> Dict:
    """
    Create an attack chain combining multiple vulnerabilities/IOCTLs.

    Attack chains show how multiple primitives can be chained together
    to achieve a goal (e.g., read primitive + write primitive = code execution).

    Args:
        driver_id: Driver UUID
        name: Attack chain name (e.g., "Arbitrary Kernel Code Execution via Physical Memory R/W")
        attack_goal: Goal (privilege_escalation, code_execution, info_leak, persistence, etc.)
        steps: Ordered steps [{'order': 1, 'vuln_id': '...', 'ioctl_id': '...', 'description': '...'}, ...]
        initial_access: Starting privilege level (user, admin, system, any)
        final_privilege: Final privilege achieved (system, kernel, hypervisor)
        description: Overview of the attack chain
        poc_code: Complete proof-of-concept code

    Returns:
        Attack chain details including UUID
    """
    from ..database.connection import get_db_connection
    from ..embeddings.provider import generate_embedding

    chain_id = str(uuid.uuid4())
    now = datetime.utcnow().isoformat()

    # Validate steps format
    for step in steps:
        if 'order' not in step or 'description' not in step:
            raise ValueError("Each step must have 'order' and 'description' fields")

    # Generate embedding
    embedding_text = f"Attack chain: {name}. Goal: {attack_goal}. {description or ''}. Steps: {json.dumps(steps)}"
    embedding = await generate_embedding(embedding_text)

    conn = await get_db_connection()

    # Insert attack chain
    await conn.execute("""
        INSERT INTO attack_chains (
            id, driver_id, name, description,
            attack_goal, steps,
            initial_access, final_privilege,
            poc_code, created_at, embedding
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    """, (
        chain_id, driver_id, name, description,
        attack_goal, json.dumps(steps),
        initial_access, final_privilege,
        poc_code, now, json.dumps(embedding)
    ))

    await conn.commit()

    return {
        'chain_id': chain_id,
        'name': name,
        'attack_goal': attack_goal,
        'step_count': len(steps),
        'initial_access': initial_access,
        'final_privilege': final_privilege
    }


async def get_attack_chains(
    driver_id: str,
    attack_goal: Optional[str] = None
) -> List[Dict]:
    """
    Get all attack chains for a driver.

    Args:
        driver_id: Driver UUID
        attack_goal: Filter by goal (privilege_escalation, code_execution, etc.)

    Returns:
        List of attack chains with expanded details
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    query = """
        SELECT id, name, description, attack_goal,
               steps, initial_access, final_privilege, poc_code
        FROM attack_chains
        WHERE driver_id = ?
    """
    params = [driver_id]

    if attack_goal:
        query += " AND attack_goal = ?"
        params.append(attack_goal)

    query += " ORDER BY name"

    cursor = await conn.execute(query, params)
    chains = []

    for row in await cursor.fetchall():
        chain = dict(row)
        chain['steps'] = json.loads(chain['steps'])

        # Expand step details (vuln/ioctl info)
        for step in chain['steps']:
            if 'vuln_id' in step:
                cursor2 = await conn.execute(
                    "SELECT id, title, severity FROM vulnerabilities WHERE id = ?",
                    (step['vuln_id'],)
                )
                vuln_row = await cursor2.fetchone()
                if vuln_row:
                    step['vuln_info'] = dict(vuln_row)

            if 'ioctl_id' in step:
                cursor2 = await conn.execute(
                    "SELECT id, name, code_hex FROM ioctls WHERE id = ?",
                    (step['ioctl_id'],)
                )
                ioctl_row = await cursor2.fetchone()
                if ioctl_row:
                    step['ioctl_info'] = dict(ioctl_row)

        chains.append(chain)

    return chains
