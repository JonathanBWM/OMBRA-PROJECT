"""
Impact Analysis Tools - Dependency tracking and impact assessment

Tools for analyzing change impact across the codebase.
"""

from ..database import fetch_one, fetch_all
import json


async def analyze_impact(
    file_path: str = None,
    function_name: str = None,
    feature_id: int = None
) -> dict:
    """
    Analyze the impact of changes to a file, function, or feature.

    Args:
        file_path: Absolute path to file
        function_name: Name of function to analyze
        feature_id: ID of feature to analyze

    Returns:
        Comprehensive impact report
    """
    impact = {
        "target": {},
        "affected_files": [],
        "affected_features": [],
        "affected_functions": [],
        "dependency_chain": [],
        "risk_level": "unknown",
        "recommendations": []
    }

    # Analyze file impact
    if file_path:
        file_record = await fetch_one(
            "SELECT * FROM files WHERE absolute_path = ?",
            (file_path,)
        )

        if not file_record:
            raise ValueError(f"File not found: {file_path}")

        impact["target"] = {
            "type": "file",
            "path": file_path,
            "id": file_record["id"]
        }

        # Find features using this file
        features = await fetch_all(
            """
            SELECT
                f.id,
                f.name,
                f.priority,
                f.status,
                ff.role
            FROM features f
            JOIN feature_files ff ON f.id = ff.feature_id
            WHERE ff.file_id = ?
            """,
            (file_record["id"],)
        )

        impact["affected_features"] = features

        # Find functions in this file
        functions = await fetch_all(
            "SELECT * FROM functions WHERE file_id = ?",
            (file_record["id"],)
        )

        impact["affected_functions"] = functions

        # Risk assessment based on features
        if any(f["priority"] == "P0" for f in features):
            impact["risk_level"] = "critical"
            impact["recommendations"].append("This file affects P0 features - thorough testing required")
        elif any(f["priority"] == "P1" for f in features):
            impact["risk_level"] = "high"
            impact["recommendations"].append("This file affects P1 features - careful review needed")
        else:
            impact["risk_level"] = "medium"

    # Analyze function impact
    elif function_name:
        # Find all instances of this function
        functions = await fetch_all(
            """
            SELECT
                func.*,
                f.absolute_path,
                f.filename
            FROM functions func
            JOIN files f ON func.file_id = f.id
            WHERE func.name = ?
            """,
            (function_name,)
        )

        if not functions:
            raise ValueError(f"Function not found: {function_name}")

        primary_func = functions[0]

        impact["target"] = {
            "type": "function",
            "name": function_name,
            "id": primary_func["id"],
            "file": primary_func["absolute_path"]
        }

        # Find callers (functions that call this)
        callers = await fetch_all(
            """
            SELECT
                func.id,
                func.name,
                func.signature,
                f.absolute_path,
                func.called_by
            FROM functions func
            JOIN files f ON func.file_id = f.id
            WHERE func.calls LIKE ?
            """,
            (f'%"{function_name}"%',)
        )

        impact["affected_functions"] = callers
        impact["dependency_chain"].append({
            "level": "direct_callers",
            "count": len(callers),
            "functions": [{"name": c["name"], "file": c["absolute_path"]} for c in callers]
        })

        # Find features using this function
        features = await fetch_all(
            """
            SELECT
                f.id,
                f.name,
                f.priority,
                f.status,
                ff.role
            FROM features f
            JOIN feature_functions ff ON f.id = ff.feature_id
            WHERE ff.function_id = ?
            """,
            (primary_func["id"],)
        )

        impact["affected_features"] = features

        # Get files containing callers
        affected_file_ids = set(c["file_id"] for c in callers)
        if affected_file_ids:
            files = await fetch_all(
                f"""
                SELECT id, absolute_path, filename
                FROM files
                WHERE id IN ({','.join('?' * len(affected_file_ids))})
                """,
                tuple(affected_file_ids)
            )
            impact["affected_files"] = files

        # Risk based on caller count and features
        caller_count = len(callers)
        if caller_count > 20:
            impact["risk_level"] = "critical"
            impact["recommendations"].append(f"High-impact function with {caller_count} callers")
        elif caller_count > 10:
            impact["risk_level"] = "high"
            impact["recommendations"].append(f"Medium-impact function with {caller_count} callers")
        elif any(f["priority"] == "P0" for f in features):
            impact["risk_level"] = "high"
            impact["recommendations"].append("Function affects P0 features")
        else:
            impact["risk_level"] = "medium"

    # Analyze feature impact
    elif feature_id:
        feature = await fetch_one(
            "SELECT * FROM features WHERE id = ?",
            (feature_id,)
        )

        if not feature:
            raise ValueError(f"Feature not found: {feature_id}")

        impact["target"] = {
            "type": "feature",
            "id": feature_id,
            "name": feature["name"],
            "priority": feature["priority"]
        }

        # Find implementing files
        files = await fetch_all(
            """
            SELECT
                f.id,
                f.absolute_path,
                f.filename,
                f.line_count,
                ff.role
            FROM files f
            JOIN feature_files ff ON f.id = ff.file_id
            WHERE ff.feature_id = ?
            """,
            (feature_id,)
        )

        impact["affected_files"] = files

        # Find implementing functions
        functions = await fetch_all(
            """
            SELECT
                func.id,
                func.name,
                func.signature,
                func.is_stub,
                f.absolute_path
            FROM functions func
            JOIN feature_functions ff ON func.id = ff.function_id
            JOIN files f ON func.file_id = f.id
            WHERE ff.feature_id = ?
            """,
            (feature_id,)
        )

        impact["affected_functions"] = functions

        # Find blocking relationships
        if feature["blocks"]:
            blocked_feature_ids = json.loads(feature["blocks"])
            blocked_features = await fetch_all(
                f"""
                SELECT id, name, priority, status
                FROM features
                WHERE id IN ({','.join('?' * len(blocked_feature_ids))})
                """,
                tuple(blocked_feature_ids)
            )
            impact["dependency_chain"].append({
                "level": "blocks",
                "features": blocked_features
            })

        if feature["blocked_by"]:
            blocking_feature_ids = json.loads(feature["blocked_by"])
            blocking_features = await fetch_all(
                f"""
                SELECT id, name, priority, status
                FROM features
                WHERE id IN ({','.join('?' * len(blocking_feature_ids))})
                """,
                tuple(blocking_feature_ids)
            )
            impact["dependency_chain"].append({
                "level": "blocked_by",
                "features": blocking_features
            })

        # Risk based on priority and implementation
        stub_count = sum(1 for f in functions if f["is_stub"])
        if feature["priority"] == "P0":
            impact["risk_level"] = "critical"
            impact["recommendations"].append("P0 feature - highest priority")
        elif feature["priority"] == "P1":
            impact["risk_level"] = "high"
        else:
            impact["risk_level"] = "medium"

        if stub_count > 0:
            impact["recommendations"].append(f"{stub_count} stub functions need implementation")

    else:
        raise ValueError("Must specify file_path, function_name, or feature_id")

    return impact


async def what_uses_this(
    file_path: str = None,
    function_name: str = None,
    struct_name: str = None
) -> list[dict]:
    """
    Find what uses a given file, function, or struct.

    Args:
        file_path: Absolute path to file
        function_name: Name of function
        struct_name: Name of struct

    Returns:
        List of dependents
    """
    dependents = []

    if file_path:
        # Find file record
        file_record = await fetch_one(
            "SELECT id FROM files WHERE absolute_path = ?",
            (file_path,)
        )

        if not file_record:
            return []

        # Find features using this file
        features = await fetch_all(
            """
            SELECT
                f.id,
                f.name,
                f.priority,
                f.status,
                ff.role as usage_role
            FROM features f
            JOIN feature_files ff ON f.id = ff.feature_id
            WHERE ff.file_id = ?
            """,
            (file_record["id"],)
        )

        for feat in features:
            dependents.append({
                "type": "feature",
                "id": feat["id"],
                "name": feat["name"],
                "priority": feat["priority"],
                "usage_role": feat["usage_role"]
            })

    elif function_name:
        # Find functions that call this
        callers = await fetch_all(
            """
            SELECT
                func.id,
                func.name,
                func.signature,
                f.absolute_path,
                f.filename
            FROM functions func
            JOIN files f ON func.file_id = f.id
            WHERE func.calls LIKE ?
            """,
            (f'%"{function_name}"%',)
        )

        for caller in callers:
            dependents.append({
                "type": "function",
                "id": caller["id"],
                "name": caller["name"],
                "signature": caller["signature"],
                "file": caller["absolute_path"]
            })

    elif struct_name:
        # Find where struct is referenced (would need code parsing)
        # For now, return empty - this requires more sophisticated analysis
        pass

    return dependents


async def what_does_this_use(
    file_path: str = None,
    function_name: str = None
) -> list[dict]:
    """
    Find what a given file or function depends on.

    Args:
        file_path: Absolute path to file
        function_name: Name of function

    Returns:
        List of dependencies
    """
    dependencies = []

    if file_path:
        # Find file record
        file_record = await fetch_one(
            "SELECT id FROM files WHERE absolute_path = ?",
            (file_path,)
        )

        if not file_record:
            return []

        # Find all functions in this file and their calls
        functions = await fetch_all(
            """
            SELECT id, name, calls
            FROM functions
            WHERE file_id = ?
            """,
            (file_record["id"],)
        )

        all_called_funcs = set()
        for func in functions:
            if func["calls"]:
                calls = json.loads(func["calls"])
                all_called_funcs.update(calls)

        # Find details of called functions
        for func_name in all_called_funcs:
            func_details = await fetch_all(
                """
                SELECT
                    func.id,
                    func.name,
                    func.signature,
                    f.absolute_path
                FROM functions func
                JOIN files f ON func.file_id = f.id
                WHERE func.name = ?
                """,
                (func_name,)
            )

            for detail in func_details:
                dependencies.append({
                    "type": "function",
                    "id": detail["id"],
                    "name": detail["name"],
                    "signature": detail["signature"],
                    "file": detail["absolute_path"]
                })

    elif function_name:
        # Find function and its calls
        functions = await fetch_all(
            """
            SELECT id, name, calls, file_id
            FROM functions
            WHERE name = ?
            """,
            (function_name,)
        )

        if not functions:
            return []

        func = functions[0]
        if func["calls"]:
            calls = json.loads(func["calls"])

            for called_func_name in calls:
                # Find details of called function
                called_details = await fetch_all(
                    """
                    SELECT
                        func.id,
                        func.name,
                        func.signature,
                        f.absolute_path
                    FROM functions func
                    JOIN files f ON func.file_id = f.id
                    WHERE func.name = ?
                    """,
                    (called_func_name,)
                )

                for detail in called_details:
                    dependencies.append({
                        "type": "function",
                        "id": detail["id"],
                        "name": detail["name"],
                        "signature": detail["signature"],
                        "file": detail["absolute_path"]
                    })

    return dependencies


async def get_dependency_graph(
    feature_id: int = None,
    component_id: int = None
) -> dict:
    """
    Build a dependency graph for visualization.

    Args:
        feature_id: ID of feature to graph
        component_id: ID of component to graph

    Returns:
        Graph with nodes and edges
    """
    nodes = []
    edges = []

    if feature_id:
        # Get feature
        feature = await fetch_one(
            "SELECT * FROM features WHERE id = ?",
            (feature_id,)
        )

        if not feature:
            raise ValueError(f"Feature {feature_id} not found")

        # Add feature node
        nodes.append({
            "id": f"feature_{feature_id}",
            "type": "feature",
            "label": feature["name"],
            "priority": feature["priority"],
            "status": feature["status"]
        })

        # Add implementing files as nodes
        files = await fetch_all(
            """
            SELECT
                f.id,
                f.filename,
                f.absolute_path,
                ff.role
            FROM files f
            JOIN feature_files ff ON f.id = ff.file_id
            WHERE ff.feature_id = ?
            """,
            (feature_id,)
        )

        for file_rec in files:
            node_id = f"file_{file_rec['id']}"
            nodes.append({
                "id": node_id,
                "type": "file",
                "label": file_rec["filename"],
                "path": file_rec["absolute_path"]
            })

            edges.append({
                "from": f"feature_{feature_id}",
                "to": node_id,
                "label": file_rec["role"]
            })

        # Add implementing functions
        functions = await fetch_all(
            """
            SELECT
                func.id,
                func.name,
                func.is_stub,
                f.id as file_id
            FROM functions func
            JOIN feature_functions ff ON func.id = ff.function_id
            JOIN files f ON func.file_id = f.id
            WHERE ff.feature_id = ?
            """,
            (feature_id,)
        )

        for func in functions:
            node_id = f"function_{func['id']}"
            nodes.append({
                "id": node_id,
                "type": "function",
                "label": func["name"],
                "is_stub": bool(func["is_stub"])
            })

            # Edge from file to function
            edges.append({
                "from": f"file_{func['file_id']}",
                "to": node_id,
                "label": "contains"
            })

        # Add blocking relationships
        if feature["blocks"]:
            blocked_ids = json.loads(feature["blocks"])
            for blocked_id in blocked_ids:
                blocked = await fetch_one(
                    "SELECT name FROM features WHERE id = ?",
                    (blocked_id,)
                )
                if blocked:
                    node_id = f"feature_{blocked_id}"
                    if node_id not in [n["id"] for n in nodes]:
                        nodes.append({
                            "id": node_id,
                            "type": "feature",
                            "label": blocked["name"]
                        })
                    edges.append({
                        "from": f"feature_{feature_id}",
                        "to": node_id,
                        "label": "blocks",
                        "style": "dashed"
                    })

    elif component_id:
        # Get component
        component = await fetch_one(
            "SELECT * FROM components WHERE id = ?",
            (component_id,)
        )

        if not component:
            raise ValueError(f"Component {component_id} not found")

        # Add component node
        nodes.append({
            "id": f"component_{component_id}",
            "type": "component",
            "label": component["name"]
        })

        # Add files in this component
        files = await fetch_all(
            "SELECT id, filename FROM files WHERE component_id = ?",
            (component_id,)
        )

        for file_rec in files:
            node_id = f"file_{file_rec['id']}"
            nodes.append({
                "id": node_id,
                "type": "file",
                "label": file_rec["filename"]
            })

            edges.append({
                "from": f"component_{component_id}",
                "to": node_id,
                "label": "contains"
            })

    return {
        "nodes": nodes,
        "edges": edges
    }


# Export tools
IMPACT_TOOLS = {
    "analyze_impact": analyze_impact,
    "what_uses_this": what_uses_this,
    "what_does_this_use": what_does_this_use,
    "get_dependency_graph": get_dependency_graph,
}
