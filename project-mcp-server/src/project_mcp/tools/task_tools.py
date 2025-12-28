"""
Task and Epic Management Tools

Provides MCP tools for:
- Creating and managing tasks
- Creating and tracking epics
- Linking tasks to features/components
- Task status updates and comments
- Epic progress tracking
"""

import json
from datetime import datetime
from typing import Optional

from ..database import fetch_one, fetch_all, insert, update, delete, count


async def create_task(
    title: str,
    description: str,
    task_type: str = "task",
    feature_id: Optional[int] = None,
    component_id: Optional[int] = None,
    priority: str = "P2",
    estimated_hours: Optional[float] = None,
    affected_files: Optional[list[int]] = None,
    blocked_by: Optional[list[int]] = None,
    tags: Optional[list[str]] = None,
) -> dict:
    """
    Create a new task.

    Args:
        title: Task title
        description: Detailed description
        task_type: task, bug, improvement, research, documentation
        feature_id: Link to feature
        component_id: Link to component
        priority: P0, P1, P2, P3
        estimated_hours: Estimated effort
        affected_files: List of file IDs
        blocked_by: List of task IDs blocking this
        tags: List of tags

    Returns:
        {"task_id": int, "title": str, "status": str}
    """
    # Prepare data
    data = {
        "title": title,
        "description": description,
        "task_type": task_type,
        "priority": priority,
        "status": "todo",
    }

    if feature_id:
        data["feature_id"] = feature_id

    if component_id:
        data["component_id"] = component_id

    if estimated_hours:
        data["estimated_hours"] = estimated_hours

    # JSON encode array fields
    if affected_files:
        data["affected_files"] = json.dumps(affected_files)

    if blocked_by:
        data["blocked_by_tasks"] = json.dumps(blocked_by)

    if tags:
        data["tags"] = json.dumps(tags)

    # Insert
    task_id = await insert("tasks", data)

    # Log creation
    await insert("audit_log", {
        "entity_type": "task",
        "entity_id": task_id,
        "action": "create",
        "new_value": json.dumps({"title": title, "priority": priority}),
        "performed_by": "agent",
    })

    return {
        "task_id": task_id,
        "title": title,
        "status": "todo",
    }


async def update_task_status(
    task_id: int,
    status: str,
    actual_hours: Optional[float] = None,
    notes: Optional[str] = None,
) -> dict:
    """
    Update task status.

    Args:
        task_id: Task ID
        status: New status (todo, in_progress, review, testing, done, blocked, cancelled)
        actual_hours: Actual hours spent (if completing)
        notes: Status change notes

    Returns:
        {"success": bool, "task_id": int, "old_status": str, "new_status": str}
    """
    # Get current task
    task = await fetch_one("SELECT id, status FROM tasks WHERE id = ?", (task_id,))

    if not task:
        return {
            "success": False,
            "error": f"Task {task_id} not found"
        }

    old_status = task["status"]

    # Prepare update data
    data = {"status": status}

    # Set timestamps based on status
    now = datetime.now().isoformat()

    if status == "in_progress" and old_status == "todo":
        data["started_at"] = now

    if status == "done" and old_status != "done":
        data["completed_at"] = now
        if actual_hours:
            data["actual_hours"] = actual_hours

    # Update
    await update("tasks", data, "id = ?", (task_id,))

    # Add comment if notes provided
    if notes:
        await insert("task_comments", {
            "task_id": task_id,
            "author": "agent",
            "content": notes,
            "comment_type": "status_change",
        })

    # Log status change
    await insert("audit_log", {
        "entity_type": "task",
        "entity_id": task_id,
        "action": "status_change",
        "old_value": json.dumps({"status": old_status}),
        "new_value": json.dumps({"status": status}),
        "performed_by": "agent",
    })

    return {
        "success": True,
        "task_id": task_id,
        "old_status": old_status,
        "new_status": status,
    }


async def list_tasks(
    status: Optional[str] = None,
    feature_id: Optional[int] = None,
    component_id: Optional[int] = None,
    assigned_to: Optional[str] = None,
    priority: Optional[str] = None,
) -> dict:
    """
    List tasks with filtering.

    Args:
        status: Filter by status
        feature_id: Filter by feature
        component_id: Filter by component
        assigned_to: Filter by assignee
        priority: Filter by priority

    Returns:
        {
            "total": int,
            "tasks": [
                {
                    "id": int,
                    "title": str,
                    "status": str,
                    "priority": str,
                    "task_type": str,
                    "feature_id": int,
                    "component_id": int,
                    "created_at": str,
                    "estimated_hours": float,
                    "actual_hours": float
                }
            ]
        }
    """
    # Build query
    query = "SELECT * FROM tasks WHERE 1=1"
    params = []

    if status:
        query += " AND status = ?"
        params.append(status)

    if feature_id:
        query += " AND feature_id = ?"
        params.append(feature_id)

    if component_id:
        query += " AND component_id = ?"
        params.append(component_id)

    if assigned_to:
        query += " AND assigned_to = ?"
        params.append(assigned_to)

    if priority:
        query += " AND priority = ?"
        params.append(priority)

    query += " ORDER BY priority, created_at DESC"

    tasks = await fetch_all(query, tuple(params))

    # Parse JSON fields
    for task in tasks:
        if task.get("affected_files"):
            task["affected_files"] = json.loads(task["affected_files"])
        if task.get("blocked_by_tasks"):
            task["blocked_by_tasks"] = json.loads(task["blocked_by_tasks"])
        if task.get("tags"):
            task["tags"] = json.loads(task["tags"])

    return {
        "total": len(tasks),
        "tasks": tasks,
    }


async def get_task(task_id: int) -> dict:
    """
    Get full task details including comments.

    Args:
        task_id: Task ID

    Returns:
        {
            "task": {...},
            "comments": [...],
            "feature": {...},
            "component": {...}
        }
    """
    # Get task
    task = await fetch_one("SELECT * FROM tasks WHERE id = ?", (task_id,))

    if not task:
        return {"error": f"Task {task_id} not found"}

    # Parse JSON fields
    if task.get("affected_files"):
        task["affected_files"] = json.loads(task["affected_files"])
    if task.get("blocked_by_tasks"):
        task["blocked_by_tasks"] = json.loads(task["blocked_by_tasks"])
    if task.get("blocking_tasks"):
        task["blocking_tasks"] = json.loads(task["blocking_tasks"])
    if task.get("tags"):
        task["tags"] = json.loads(task["tags"])

    # Get comments
    comments = await fetch_all(
        "SELECT * FROM task_comments WHERE task_id = ? ORDER BY created_at",
        (task_id,)
    )

    # Get feature if linked
    feature = None
    if task.get("feature_id"):
        feature = await fetch_one(
            "SELECT id, name, status FROM features WHERE id = ?",
            (task["feature_id"],)
        )

    # Get component if linked
    component = None
    if task.get("component_id"):
        component = await fetch_one(
            "SELECT id, name, component_type FROM components WHERE id = ?",
            (task["component_id"],)
        )

    return {
        "task": task,
        "comments": comments,
        "feature": feature,
        "component": component,
    }


async def add_task_comment(
    task_id: int,
    content: str,
    author: str = "agent",
    comment_type: str = "comment",
) -> dict:
    """
    Add a comment to a task.

    Args:
        task_id: Task ID
        content: Comment content
        author: Comment author
        comment_type: comment, status_change, code_review, blocker

    Returns:
        {"success": bool, "comment_id": int}
    """
    # Verify task exists
    task = await fetch_one("SELECT id FROM tasks WHERE id = ?", (task_id,))

    if not task:
        return {
            "success": False,
            "error": f"Task {task_id} not found"
        }

    # Insert comment
    comment_id = await insert("task_comments", {
        "task_id": task_id,
        "author": author,
        "content": content,
        "comment_type": comment_type,
    })

    return {
        "success": True,
        "comment_id": comment_id,
    }


async def create_epic(
    name: str,
    description: str,
    priority: str = "P1",
    target_date: Optional[str] = None,
) -> dict:
    """
    Create a new epic.

    Args:
        name: Epic name
        description: Detailed description
        priority: P0, P1, P2, P3
        target_date: Target completion date (ISO format)

    Returns:
        {"epic_id": int, "name": str}
    """
    data = {
        "name": name,
        "description": description,
        "priority": priority,
        "status": "open",
    }

    if target_date:
        data["target_date"] = target_date

    epic_id = await insert("epics", data)

    # Log creation
    await insert("audit_log", {
        "entity_type": "epic",
        "entity_id": epic_id,
        "action": "create",
        "new_value": json.dumps({"name": name, "priority": priority}),
        "performed_by": "agent",
    })

    return {
        "epic_id": epic_id,
        "name": name,
    }


async def add_task_to_epic(epic_id: int, task_id: int) -> dict:
    """
    Link a task to an epic.

    Args:
        epic_id: Epic ID
        task_id: Task ID

    Returns:
        {"success": bool}
    """
    # Verify epic exists
    epic = await fetch_one("SELECT id FROM epics WHERE id = ?", (epic_id,))
    if not epic:
        return {"success": False, "error": f"Epic {epic_id} not found"}

    # Verify task exists
    task = await fetch_one("SELECT id FROM tasks WHERE id = ?", (task_id,))
    if not task:
        return {"success": False, "error": f"Task {task_id} not found"}

    # Update task
    await update("tasks", {"epic_id": epic_id}, "id = ?", (task_id,))

    # Update epic task counts
    await _update_epic_counts(epic_id)

    return {"success": True}


async def get_epic_progress(epic_id: int) -> dict:
    """
    Get epic with all tasks and progress percentage.

    Args:
        epic_id: Epic ID

    Returns:
        {
            "epic": {...},
            "tasks": [...],
            "progress_percentage": float,
            "total_tasks": int,
            "completed_tasks": int
        }
    """
    # Get epic
    epic = await fetch_one("SELECT * FROM epics WHERE id = ?", (epic_id,))

    if not epic:
        return {"error": f"Epic {epic_id} not found"}

    # Get tasks
    tasks = await fetch_all(
        "SELECT * FROM tasks WHERE epic_id = ? ORDER BY priority, created_at",
        (epic_id,)
    )

    # Calculate progress
    total_tasks = len(tasks)
    completed_tasks = sum(1 for t in tasks if t["status"] == "done")
    progress = (completed_tasks / total_tasks * 100) if total_tasks > 0 else 0.0

    return {
        "epic": epic,
        "tasks": tasks,
        "progress_percentage": round(progress, 2),
        "total_tasks": total_tasks,
        "completed_tasks": completed_tasks,
    }


async def list_epics(status: Optional[str] = None) -> dict:
    """
    List all epics.

    Args:
        status: Filter by status (open, in_progress, completed, cancelled)

    Returns:
        {
            "total": int,
            "epics": [
                {
                    "id": int,
                    "name": str,
                    "status": str,
                    "priority": str,
                    "total_tasks": int,
                    "completed_tasks": int,
                    "progress_percentage": float
                }
            ]
        }
    """
    query = "SELECT * FROM epics WHERE 1=1"
    params = []

    if status:
        query += " AND status = ?"
        params.append(status)

    query += " ORDER BY priority, created_at DESC"

    epics = await fetch_all(query, tuple(params))

    # Calculate progress for each epic
    for epic in epics:
        tasks = await fetch_all(
            "SELECT status FROM tasks WHERE epic_id = ?",
            (epic["id"],)
        )
        total = len(tasks)
        completed = sum(1 for t in tasks if t["status"] == "done")
        epic["progress_percentage"] = round(
            (completed / total * 100) if total > 0 else 0.0,
            2
        )

    return {
        "total": len(epics),
        "epics": epics,
    }


async def _update_epic_counts(epic_id: int) -> None:
    """Update epic task counts."""
    tasks = await fetch_all(
        "SELECT status FROM tasks WHERE epic_id = ?",
        (epic_id,)
    )

    total = len(tasks)
    completed = sum(1 for t in tasks if t["status"] == "done")

    await update(
        "epics",
        {
            "total_tasks": total,
            "completed_tasks": completed,
        },
        "id = ?",
        (epic_id,)
    )


# Export tools dictionary
TASK_TOOLS = {
    "create_task": create_task,
    "update_task_status": update_task_status,
    "list_tasks": list_tasks,
    "get_task": get_task,
    "add_task_comment": add_task_comment,
    "create_epic": create_epic,
    "add_task_to_epic": add_task_to_epic,
    "get_epic_progress": get_epic_progress,
    "list_epics": list_epics,
}
