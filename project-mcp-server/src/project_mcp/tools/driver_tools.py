"""
Driver Analysis Tools

Provides MCP tools for:
- Importing driver scanner JSON output
- Retrieving driver analysis details
- Listing drivers by capability tier
- Managing analysis queue priority
- Tracking analysis progress
- Generating automation scripts for driver-re-mcp
"""

import json
from datetime import datetime
from pathlib import Path
from typing import Optional

from ..database import fetch_one, fetch_all, insert, update, delete, count


async def import_driver_analysis(json_path: str) -> dict:
    """
    Import scanner JSON output into the database.

    Parses the scanner output from ombra-driver-scanner and stores:
    - Driver metadata (path, name, hashes)
    - Capability scores and classification
    - Detected APIs and IOCTLs
    - Blocklist status
    - Certificate information
    - Exploitation hints

    Args:
        json_path: Absolute path to scanner JSON output file

    Returns:
        {
            "success": bool,
            "drivers_imported": int,
            "driver_ids": [int, ...],
            "errors": [str, ...]
        }
    """
    # Read and parse JSON
    path = Path(json_path)
    if not path.exists():
        return {
            "success": False,
            "drivers_imported": 0,
            "driver_ids": [],
            "errors": [f"File not found: {json_path}"]
        }

    try:
        with open(path, 'r') as f:
            data = json.load(f)
    except json.JSONDecodeError as e:
        return {
            "success": False,
            "drivers_imported": 0,
            "driver_ids": [],
            "errors": [f"JSON parse error: {str(e)}"]
        }

    # Extract metadata
    scan_metadata = data.get("scan_metadata", {})
    candidates = data.get("top_candidates", [])

    driver_ids = []
    errors = []

    # Import each driver
    for candidate in candidates:
        try:
            driver_id = await _import_single_driver(candidate, scan_metadata)
            driver_ids.append(driver_id)
        except Exception as e:
            errors.append(f"Failed to import {candidate.get('name', 'unknown')}: {str(e)}")

    # Log import
    await insert("audit_log", {
        "entity_type": "driver_import",
        "entity_id": 0,
        "action": "import",
        "new_value": json.dumps({
            "json_path": json_path,
            "drivers_imported": len(driver_ids),
            "timestamp": scan_metadata.get("timestamp")
        }),
        "performed_by": "agent",
    })

    return {
        "success": len(errors) == 0,
        "drivers_imported": len(driver_ids),
        "driver_ids": driver_ids,
        "errors": errors
    }


async def _import_single_driver(candidate: dict, scan_metadata: dict) -> int:
    """Import a single driver from scanner output."""
    # Check if driver already exists by hash
    hash_sha256 = candidate.get("hash_sha256", "")
    existing = await fetch_one(
        "SELECT id FROM drivers WHERE hash_sha256 = ?",
        (hash_sha256,)
    )

    capabilities = candidate.get("capabilities", {})
    scores = candidate.get("scores", {})
    blocklist = candidate.get("blocklist_status", {})
    cert = candidate.get("certificate", {})
    forensic = candidate.get("forensic_risk", {})
    exploitation = candidate.get("exploitation_hints", {})

    # Determine capability tier
    tier = _calculate_capability_tier(candidate.get("final_score", 0))

    # Prepare driver data
    driver_data = {
        "driver_path": candidate.get("path", ""),
        "driver_name": candidate.get("name", ""),
        "hash_sha256": hash_sha256,
        "hash_md5": candidate.get("hash_md5", ""),

        # Scanner results
        "capability_tier": tier,
        "final_score": candidate.get("final_score", 0.0),
        "classification": candidate.get("classification", ""),

        # Score breakdown (JSON)
        "capability_score": scores.get("capability", 0),
        "accessibility_score": scores.get("accessibility", 0),
        "obscurity_score": scores.get("obscurity", 0),
        "stealth_score": scores.get("stealth", 0),

        # Capabilities (booleans)
        "has_physical_memory": capabilities.get("physical_memory", {}).get("present", False),
        "has_msr_access": capabilities.get("msr_access", {}).get("present", False),
        "has_process_control": capabilities.get("process_control", {}).get("present", False),
        "has_module_loading": capabilities.get("module_loading", {}).get("present", False),
        "has_cr_access": capabilities.get("cr_access", {}).get("present", False),
        "has_port_io": capabilities.get("port_io", {}).get("present", False),

        # Driver family
        "driver_family": capabilities.get("driver_family", "unknown"),

        # Dangerous APIs found (JSON array)
        "dangerous_apis": json.dumps(_collect_dangerous_apis(capabilities)),

        # Detected IOCTLs (JSON array)
        "detected_ioctls": json.dumps(exploitation.get("suspected_ioctls", [])),

        # Blocklist status
        "on_loldrivers": blocklist.get("loldrivers", False),
        "on_msdbx": blocklist.get("msdbx", False),

        # Certificate info
        "is_signed": cert.get("valid", False),
        "signer_name": cert.get("signer", ""),
        "cert_revoked": cert.get("revoked", False),

        # Forensic risk indicators
        "ff25_count": forensic.get("ff25_count", 0),
        "has_gshandler": forensic.get("has_gshandler", False),
        "debug_string_count": forensic.get("debug_strings", 0),

        # Analysis status
        "analysis_status": "pending",
        "priority": _calculate_priority(tier),

        # Scanner metadata
        "scanned_at": scan_metadata.get("timestamp"),
        "scanner_version": scan_metadata.get("loldrivers_version", ""),
    }

    if existing:
        # Update existing driver
        driver_id = existing["id"]
        await update("drivers", driver_data, "id = ?", (driver_id,))

        # Log update
        await insert("audit_log", {
            "entity_type": "driver",
            "entity_id": driver_id,
            "action": "update",
            "new_value": json.dumps({"hash": hash_sha256, "score": driver_data["final_score"]}),
            "performed_by": "agent",
        })
    else:
        # Insert new driver
        driver_id = await insert("drivers", driver_data)

        # Log creation
        await insert("audit_log", {
            "entity_type": "driver",
            "entity_id": driver_id,
            "action": "create",
            "new_value": json.dumps({"name": driver_data["driver_name"], "tier": tier}),
            "performed_by": "agent",
        })

    return driver_id


def _collect_dangerous_apis(capabilities: dict) -> list[str]:
    """Collect all dangerous APIs from capability sections."""
    apis = []

    for capability_type in ["physical_memory", "msr_access", "cr_access",
                            "port_io", "process_control", "module_loading"]:
        cap = capabilities.get(capability_type, {})
        apis.extend(cap.get("apis_found", []))

    return list(set(apis))  # Deduplicate


def _calculate_capability_tier(score: float) -> str:
    """Calculate capability tier based on final score."""
    if score >= 90:
        return "S"
    elif score >= 80:
        return "A"
    elif score >= 70:
        return "B"
    elif score >= 60:
        return "C"
    else:
        return "D"


def _calculate_priority(tier: str) -> int:
    """Calculate numeric priority from tier (1=highest, 5=lowest)."""
    tier_priority = {
        "S": 1,
        "A": 2,
        "B": 3,
        "C": 4,
        "D": 5
    }
    return tier_priority.get(tier, 5)


async def get_driver_analysis(driver_name_or_hash: str) -> dict:
    """
    Get full driver analysis details.

    Retrieves driver information by name or SHA256 hash.

    Args:
        driver_name_or_hash: Driver filename or SHA256 hash

    Returns:
        {
            "driver": {...},  # Full driver record
            "analysis_tasks": [...],  # Suggested analysis tasks
            "related_drivers": [...],  # Other drivers in same family
            "mcp_script": str  # Generated automation script
        }
    """
    # Try to find by name first
    driver = await fetch_one(
        "SELECT * FROM drivers WHERE driver_name = ?",
        (driver_name_or_hash,)
    )

    # If not found, try hash
    if not driver:
        driver = await fetch_one(
            "SELECT * FROM drivers WHERE hash_sha256 = ?",
            (driver_name_or_hash,)
        )

    if not driver:
        return {"error": f"Driver not found: {driver_name_or_hash}"}

    # Parse JSON fields
    driver["dangerous_apis"] = json.loads(driver.get("dangerous_apis", "[]"))
    driver["detected_ioctls"] = json.loads(driver.get("detected_ioctls", "[]"))

    # Generate analysis tasks
    analysis_tasks = _generate_analysis_tasks(driver)

    # Find related drivers (same family)
    related_drivers = await fetch_all(
        """
        SELECT id, driver_name, capability_tier, final_score, analysis_status
        FROM drivers
        WHERE driver_family = ? AND id != ?
        ORDER BY final_score DESC
        LIMIT 5
        """,
        (driver["driver_family"], driver["id"])
    )

    # Generate MCP automation script
    mcp_script = _generate_mcp_python_script(driver, analysis_tasks)

    return {
        "driver": driver,
        "analysis_tasks": analysis_tasks,
        "related_drivers": related_drivers,
        "mcp_script": mcp_script
    }


def _generate_analysis_tasks(driver: dict) -> list[str]:
    """Generate suggested analysis tasks based on capabilities."""
    tasks = []

    # Always do these
    tasks.append("load_driver")
    tasks.append("analyze_imports")
    tasks.append("analyze_exports")

    # Capability-specific tasks
    if driver.get("has_module_loading"):
        tasks.append("trace_ldr_load_handler")
        tasks.append("find_module_loading_ioctls")
        tasks.append("analyze_symbol_resolution_path")

    if driver.get("has_physical_memory"):
        tasks.append("trace_physical_memory_apis")
        tasks.append("find_phys_read_write_ioctls")
        tasks.append("analyze_mmmapiospace_usage")

    if driver.get("has_msr_access"):
        tasks.append("trace_msr_apis")
        tasks.append("find_msr_read_write_ioctls")
        tasks.append("check_msr_validation")

    if driver.get("has_process_control"):
        tasks.append("trace_process_apis")
        tasks.append("analyze_callback_registration")

    # IOCTL analysis
    if driver.get("detected_ioctls"):
        tasks.append("decode_all_ioctls")
        tasks.append("trace_dispatch_handler")
        tasks.append("map_ioctl_to_capability")

    # Family-specific
    family = driver.get("driver_family", "")
    if family in ["VirtualBox", "LDPlayer/Ld9Box"]:
        tasks.append("analyze_vbox_ldr_protocol")
        tasks.append("find_validation_checks")
        tasks.append("document_bypass_requirements")

    # Final documentation
    tasks.append("document_vulnerabilities")
    tasks.append("create_attack_chains")
    tasks.append("generate_exploitation_report")

    return tasks


def _generate_mcp_python_script(driver: dict, analysis_tasks: list[str]) -> str:
    """Generate Python script for driver-re-mcp automation."""
    name = driver.get("driver_name", "unknown")
    path = driver.get("driver_path", "")
    hash_sha256 = driver.get("hash_sha256", "")
    dangerous_apis = driver.get("dangerous_apis", [])
    ioctls = driver.get("detected_ioctls", [])

    script_lines = [
        "#!/usr/bin/env python3",
        f'"""Auto-generated MCP analysis script for {name}"""',
        "",
        "import asyncio",
        "from driver_re_mcp import DriverREMCP",
        "",
        "async def analyze_driver():",
        "    mcp = DriverREMCP()",
        "    await mcp.connect()",
        "",
        "    # Load driver",
        "    driver_id = await mcp.load_driver(",
        f'        path="{path}",',
        f'        name="{name}",',
        f'        hash_sha256="{hash_sha256}"',
        "    )",
        '    print(f"Loaded driver: {driver_id}")',
        "",
        "    # Sync from Ghidra",
        "    await mcp.sync_from_ghidra(driver_id)",
        "",
        "    # Analyze dangerous imports",
        "    imports = await mcp.analyze_dangerous_imports(driver_id)",
        '    print(f"Found {len(imports)} dangerous imports")',
        "",
        "    # Trace call paths to dangerous APIs",
        "    dangerous_apis = [",
    ]

    for api in dangerous_apis[:20]:  # Limit to 20 most important
        script_lines.append(f'        "{api}",')

    script_lines.extend([
        "    ]",
        "",
        "    for api in dangerous_apis:",
        "        paths = await mcp.trace_call_path(",
        "            driver_id,",
        '            from_func="IRP_MJ_DEVICE_CONTROL",',
        "            to_func=api,",
        "            max_depth=20",
        "        )",
        "        if paths:",
        '            print(f"Found {len(paths)} paths to {api}")',
        "",
        "    # Decode and add IOCTLs",
        "    ioctls = [",
    ])

    for ioctl in ioctls[:30]:  # Limit to 30 IOCTLs
        # Clean IOCTL string (remove quotes if present)
        clean_ioctl = ioctl.replace('"', '').replace("'", "")
        script_lines.append(f'        "{clean_ioctl}",')

    script_lines.extend([
        "    ]",
        "",
        "    for ioctl in ioctls:",
        "        await mcp.add_ioctl(driver_id, code=ioctl)",
        "",
        "    # Document vulnerabilities",
    ])

    if driver.get("has_module_loading"):
        script_lines.extend([
            "    await mcp.add_vulnerability(",
            "        driver_id,",
            '        name="Arbitrary Module Loading",',
            '        severity="critical",',
            '        vuln_type="code_execution",',
            '        description="Driver supports loading arbitrary kernel modules via IOCTL"',
            "    )",
            "",
        ])

    if driver.get("has_physical_memory"):
        script_lines.extend([
            "    await mcp.add_vulnerability(",
            "        driver_id,",
            '        name="Physical Memory Read/Write",',
            '        severity="critical",',
            '        vuln_type="memory_corruption",',
            '        description="Driver provides arbitrary physical memory access via IOCTL"',
            "    )",
            "",
        ])

    if driver.get("has_msr_access"):
        script_lines.extend([
            "    await mcp.add_vulnerability(",
            "        driver_id,",
            '        name="MSR Read/Write",',
            '        severity="high",',
            '        vuln_type="privilege_escalation",',
            '        description="Driver allows MSR register access"',
            "    )",
            "",
        ])

    script_lines.extend([
        "    # Create attack chain",
        "    chain_id = await mcp.create_attack_chain(",
        "        driver_id,",
        f'        name="BYOVD Hypervisor Deployment via {name}",',
        '        attack_goal="code_execution",',
        '        initial_access="user",',
        '        final_privilege="kernel"',
        "    )",
        "",
        "    # Generate report",
        '    report = await mcp.generate_report(driver_id, format="markdown")',
        f'    with open("{name}_analysis.md", "w") as f:',
        "        f.write(report)",
        f'    print(f"Report saved to {name}_analysis.md")',
        "",
        "    await mcp.disconnect()",
        "",
        'if __name__ == "__main__":',
        "    asyncio.run(analyze_driver())",
    ])

    return "\n".join(script_lines)


async def list_drivers_by_tier(tier: str = "S", min_score: float = 0.0) -> dict:
    """
    List drivers by capability tier with optional score filter.

    Args:
        tier: Capability tier (S, A, B, C, D) or 'all'
        min_score: Minimum final score to include

    Returns:
        {
            "total": int,
            "tier": str,
            "drivers": [
                {
                    "id": int,
                    "driver_name": str,
                    "capability_tier": str,
                    "final_score": float,
                    "driver_family": str,
                    "analysis_status": str,
                    "priority": int,
                    "scanned_at": str
                }
            ]
        }
    """
    # Build query
    query = "SELECT * FROM drivers WHERE final_score >= ?"
    params = [min_score]

    if tier.upper() != "ALL":
        query += " AND capability_tier = ?"
        params.append(tier.upper())

    query += " ORDER BY final_score DESC, priority ASC"

    drivers = await fetch_all(query, tuple(params))

    # Parse JSON fields for each driver
    for driver in drivers:
        driver["dangerous_apis"] = json.loads(driver.get("dangerous_apis", "[]"))
        driver["detected_ioctls"] = json.loads(driver.get("detected_ioctls", "[]"))

    return {
        "total": len(drivers),
        "tier": tier.upper(),
        "drivers": drivers
    }


async def get_mcp_analysis_queue() -> dict:
    """
    Get priority-sorted analysis queue for MCP consumption.

    Returns drivers that haven't been analyzed yet, sorted by priority.

    Returns:
        {
            "queue_size": int,
            "tier_breakdown": {"S": int, "A": int, "B": int, "C": int, "D": int},
            "queue": [
                {
                    "rank": int,
                    "driver_id": int,
                    "driver_name": str,
                    "capability_tier": str,
                    "final_score": float,
                    "priority": int,
                    "driver_family": str,
                    "has_module_loading": bool,
                    "has_physical_memory": bool,
                    "analysis_status": str
                }
            ]
        }
    """
    # Get unanalyzed or pending drivers
    drivers = await fetch_all(
        """
        SELECT *
        FROM drivers
        WHERE analysis_status IN ('pending', 'in_progress')
        ORDER BY priority ASC, final_score DESC
        """
    )

    # Parse JSON fields
    for driver in drivers:
        driver["dangerous_apis"] = json.loads(driver.get("dangerous_apis", "[]"))
        driver["detected_ioctls"] = json.loads(driver.get("detected_ioctls", "[]"))

    # Calculate tier breakdown
    tier_breakdown = {"S": 0, "A": 0, "B": 0, "C": 0, "D": 0}
    for driver in drivers:
        tier = driver.get("capability_tier", "D")
        tier_breakdown[tier] = tier_breakdown.get(tier, 0) + 1

    # Add rank to each driver
    for idx, driver in enumerate(drivers, 1):
        driver["rank"] = idx

    return {
        "queue_size": len(drivers),
        "tier_breakdown": tier_breakdown,
        "queue": drivers
    }


async def update_driver_status(
    driver_id: int,
    status: str,
    notes: Optional[str] = None
) -> dict:
    """
    Update driver analysis status.

    Args:
        driver_id: Driver ID
        status: New status (pending, in_progress, analyzed, failed)
        notes: Optional status change notes

    Returns:
        {"success": bool, "driver_id": int, "old_status": str, "new_status": str}
    """
    # Get current driver
    driver = await fetch_one("SELECT id, analysis_status FROM drivers WHERE id = ?", (driver_id,))

    if not driver:
        return {
            "success": False,
            "error": f"Driver {driver_id} not found"
        }

    old_status = driver["analysis_status"]

    # Update status
    update_data = {"analysis_status": status}

    # Set timestamps based on status
    now = datetime.now().isoformat()

    if status == "in_progress" and old_status == "pending":
        update_data["analysis_started_at"] = now

    if status == "analyzed" and old_status != "analyzed":
        update_data["analysis_completed_at"] = now

    await update("drivers", update_data, "id = ?", (driver_id,))

    # Log status change
    await insert("audit_log", {
        "entity_type": "driver",
        "entity_id": driver_id,
        "action": "status_change",
        "old_value": json.dumps({"status": old_status}),
        "new_value": json.dumps({"status": status, "notes": notes}),
        "performed_by": "agent",
    })

    return {
        "success": True,
        "driver_id": driver_id,
        "old_status": old_status,
        "new_status": status
    }


async def generate_analysis_script(driver_id: int) -> dict:
    """
    Generate Python automation script for driver-re-mcp analysis.

    Creates a complete Python script that automates the driver analysis
    workflow using the driver-re-mcp server.

    Args:
        driver_id: Driver ID

    Returns:
        {
            "success": bool,
            "driver_name": str,
            "script": str,
            "script_path": str  # Suggested save location
        }
    """
    # Get driver
    driver = await fetch_one("SELECT * FROM drivers WHERE id = ?", (driver_id,))

    if not driver:
        return {
            "success": False,
            "error": f"Driver {driver_id} not found"
        }

    # Parse JSON fields
    driver["dangerous_apis"] = json.loads(driver.get("dangerous_apis", "[]"))
    driver["detected_ioctls"] = json.loads(driver.get("detected_ioctls", "[]"))

    # Generate tasks and script
    analysis_tasks = _generate_analysis_tasks(driver)
    script = _generate_mcp_python_script(driver, analysis_tasks)

    driver_name = driver.get("driver_name", "unknown")
    script_path = f"analyze_{driver_name}.py"

    return {
        "success": True,
        "driver_name": driver_name,
        "script": script,
        "script_path": script_path
    }


# Export tools dictionary
DRIVER_TOOLS = {
    "import_driver_analysis": import_driver_analysis,
    "get_driver_analysis": get_driver_analysis,
    "list_drivers_by_tier": list_drivers_by_tier,
    "get_mcp_analysis_queue": get_mcp_analysis_queue,
    "update_driver_status": update_driver_status,
    "generate_analysis_script": generate_analysis_script,
}
