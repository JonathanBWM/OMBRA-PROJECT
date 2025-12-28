"""
OMBRA Project Management Dashboard - FastAPI Backend

Provides REST API endpoints for the web dashboard on port 1337.
Wraps the MCP tools for HTTP access.
"""

import asyncio
import json
from typing import Optional, Any
from pathlib import Path

from fastapi import FastAPI, HTTPException, Query
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from fastapi.responses import HTMLResponse
from pydantic import BaseModel

# Import MCP tools and database
import sys
sys.path.insert(0, str(Path(__file__).parent.parent.parent / "src"))
from project_mcp.tools import ALL_TOOLS
from project_mcp.database import sync_init_db

# Initialize database on startup
sync_init_db()

app = FastAPI(
    title="OMBRA Project Management Dashboard",
    description="External brain for agent intelligence",
    version="1.0.0"
)

# CORS for frontend
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


# ═══════════════════════════════════════════════════════════════════════════════
# Pydantic Models
# ═══════════════════════════════════════════════════════════════════════════════

class ComponentCreate(BaseModel):
    name: str
    component_type: str
    description: str
    root_path: str
    language: str = "c"
    build_system: str = "msbuild"


class ComponentUpdate(BaseModel):
    description: Optional[str] = None
    language: Optional[str] = None
    build_system: Optional[str] = None


class FeatureCreate(BaseModel):
    name: str
    description: str
    category: str
    priority: str = "P2"
    components: Optional[list[int]] = None
    estimated_hours: Optional[int] = None
    blocked_by: Optional[list[int]] = None
    specification: Optional[str] = None


class FeatureUpdate(BaseModel):
    status: Optional[str] = None
    implementation_percentage: Optional[int] = None
    notes: Optional[str] = None


class TaskCreate(BaseModel):
    title: str
    description: str
    task_type: str
    priority: str = "P2"
    feature_id: Optional[int] = None
    component_id: Optional[int] = None
    assignee: Optional[str] = None
    due_date: Optional[str] = None


class TaskUpdate(BaseModel):
    status: Optional[str] = None
    assignee: Optional[str] = None
    notes: Optional[str] = None


class EpicCreate(BaseModel):
    name: str
    description: str
    priority: str = "P2"


class CommentCreate(BaseModel):
    content: str
    author: str = "claude"


class ScanRequest(BaseModel):
    root_path: str
    extensions: list[str] = [".c", ".h", ".cpp", ".hpp"]
    exclude_dirs: list[str] = ["build", "out", ".git", "node_modules"]


class FileLinkRequest(BaseModel):
    file_id: int
    feature_id: int
    role: str = "implementation"


class FunctionLinkRequest(BaseModel):
    function_id: int
    feature_id: int
    role: str = "core"


class StubResolveRequest(BaseModel):
    stub_id: int
    resolved_by: str = "claude"
    notes: str = ""


class RecommendationAction(BaseModel):
    recommendation_id: int
    action: str  # accept, reject, defer
    notes: str = ""


# ═══════════════════════════════════════════════════════════════════════════════
# Dashboard Routes
# ═══════════════════════════════════════════════════════════════════════════════

@app.get("/api/dashboard")
async def get_dashboard():
    """Get complete dashboard overview."""
    result = await ALL_TOOLS['get_dashboard_overview']()
    return result


@app.get("/api/dashboard/health")
async def get_health():
    """Get codebase health metrics."""
    result = await ALL_TOOLS['get_codebase_health']()
    return result


@app.get("/api/dashboard/activity")
async def get_activity(limit: int = Query(default=20, le=100)):
    """Get recent activity feed."""
    result = await ALL_TOOLS['get_activity_feed'](limit=limit)
    return result


@app.get("/api/dashboard/priority")
async def get_priority():
    """Get priority items requiring attention."""
    result = await ALL_TOOLS['get_priority_items']()
    return result


@app.get("/api/dashboard/tree")
async def get_tree():
    """Get component tree hierarchy."""
    result = await ALL_TOOLS['get_component_tree']()
    return result


@app.get("/api/dashboard/feature-matrix")
async def get_feature_matrix():
    """Get feature implementation matrix."""
    result = await ALL_TOOLS['get_feature_matrix']()
    return result


# ═══════════════════════════════════════════════════════════════════════════════
# Component Routes
# ═══════════════════════════════════════════════════════════════════════════════

@app.get("/api/components")
async def list_components():
    """List all components."""
    result = await ALL_TOOLS['list_components']()
    return result


@app.post("/api/components")
async def create_component(data: ComponentCreate):
    """Register a new component."""
    result = await ALL_TOOLS['register_component'](
        name=data.name,
        component_type=data.component_type,
        description=data.description,
        root_path=data.root_path,
        language=data.language,
        build_system=data.build_system
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


@app.get("/api/components/{component_id}")
async def get_component(component_id: int):
    """Get component details."""
    result = await ALL_TOOLS['get_component_status'](component_id=component_id)
    if not result.get("component"):
        raise HTTPException(status_code=404, detail="Component not found")
    return result


@app.patch("/api/components/{component_id}")
async def update_component(component_id: int, data: ComponentUpdate):
    """Update component."""
    update_data = {k: v for k, v in data.model_dump().items() if v is not None}
    result = await ALL_TOOLS['update_component'](
        component_id=component_id,
        **update_data
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


@app.delete("/api/components/{component_id}")
async def delete_component(component_id: int, force: bool = False):
    """Delete component."""
    result = await ALL_TOOLS['delete_component'](
        component_id=component_id,
        force=force
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


# ═══════════════════════════════════════════════════════════════════════════════
# Feature Routes
# ═══════════════════════════════════════════════════════════════════════════════

@app.get("/api/features")
async def search_features(
    status: Optional[str] = None,
    priority: Optional[str] = None,
    category: Optional[str] = None,
    component_id: Optional[int] = None,
    search: Optional[str] = None
):
    """Search and filter features."""
    result = await ALL_TOOLS['search_features'](
        status=status,
        priority=priority,
        category=category,
        component_id=component_id,
        search_text=search
    )
    return result


@app.post("/api/features")
async def create_feature(data: FeatureCreate):
    """Create a new feature."""
    result = await ALL_TOOLS['create_feature'](
        name=data.name,
        description=data.description,
        category=data.category,
        priority=data.priority,
        components=data.components,
        estimated_hours=data.estimated_hours,
        blocked_by=data.blocked_by,
        specification=data.specification
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


@app.get("/api/features/{feature_id}")
async def get_feature(feature_id: int):
    """Get feature details."""
    result = await ALL_TOOLS['get_feature'](feature_id=feature_id)
    if not result.get("feature"):
        raise HTTPException(status_code=404, detail="Feature not found")
    return result


@app.patch("/api/features/{feature_id}")
async def update_feature(feature_id: int, data: FeatureUpdate):
    """Update feature status."""
    result = await ALL_TOOLS['update_feature_status'](
        feature_id=feature_id,
        status=data.status,
        implementation_percentage=data.implementation_percentage,
        notes=data.notes
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


@app.get("/api/features/{feature_id}/files")
async def get_feature_files(feature_id: int):
    """Get files implementing a feature."""
    result = await ALL_TOOLS['get_feature_files'](feature_id=feature_id)
    return result


@app.get("/api/features/{feature_id}/implemented")
async def check_feature_implemented(feature_id: int):
    """Check if feature is fully implemented."""
    result = await ALL_TOOLS['is_feature_implemented'](feature_id=feature_id)
    return result


@app.get("/api/features/locate/{feature_name}")
async def locate_feature(feature_name: str):
    """Find where a feature is implemented."""
    result = await ALL_TOOLS['where_is_feature'](feature_name=feature_name)
    return result


@app.post("/api/features/{feature_id}/link-file")
async def link_file_to_feature(feature_id: int, data: FileLinkRequest):
    """Link a file to a feature."""
    result = await ALL_TOOLS['link_file_to_feature'](
        file_id=data.file_id,
        feature_id=feature_id,
        role=data.role
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


@app.post("/api/features/{feature_id}/link-function")
async def link_function_to_feature(feature_id: int, data: FunctionLinkRequest):
    """Link a function to a feature."""
    result = await ALL_TOOLS['link_function_to_feature'](
        function_id=data.function_id,
        feature_id=feature_id,
        role=data.role
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


# ═══════════════════════════════════════════════════════════════════════════════
# File Routes
# ═══════════════════════════════════════════════════════════════════════════════

@app.get("/api/files")
async def list_files(
    component_id: Optional[int] = None,
    extension: Optional[str] = None,
    has_stubs: Optional[bool] = None
):
    """List indexed files."""
    result = await ALL_TOOLS['list_files'](
        component_id=component_id,
        extension=extension,
        has_stubs=has_stubs
    )
    return result


@app.get("/api/files/search")
async def search_files(query: str):
    """Full-text search files."""
    result = await ALL_TOOLS['search_files'](query=query)
    return result


@app.get("/api/files/{file_id}")
async def get_file(file_id: int):
    """Get file details."""
    result = await ALL_TOOLS['get_file_info'](file_id=file_id)
    if not result.get("file"):
        raise HTTPException(status_code=404, detail="File not found")
    return result


@app.post("/api/files/index")
async def index_file(file_path: str, component_id: Optional[int] = None):
    """Index a single file."""
    result = await ALL_TOOLS['index_file'](
        file_path=file_path,
        component_id=component_id
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


@app.post("/api/files/index-directory")
async def index_directory(data: ScanRequest):
    """Index a directory of files."""
    result = await ALL_TOOLS['index_directory'](
        root_path=data.root_path,
        extensions=data.extensions,
        exclude_dirs=data.exclude_dirs
    )
    return result


# ═══════════════════════════════════════════════════════════════════════════════
# Function Routes
# ═══════════════════════════════════════════════════════════════════════════════

@app.get("/api/functions")
async def list_functions(
    file_id: Optional[int] = None,
    is_stub: Optional[bool] = None
):
    """List functions."""
    result = await ALL_TOOLS['list_functions'](
        file_id=file_id,
        is_stub=is_stub
    )
    return result


@app.get("/api/functions/search")
async def search_functions(query: str):
    """Search functions by name or signature."""
    result = await ALL_TOOLS['search_functions'](query=query)
    return result


@app.get("/api/functions/{function_id}")
async def get_function(function_id: int):
    """Get function details."""
    result = await ALL_TOOLS['get_function'](function_id=function_id)
    if not result.get("function"):
        raise HTTPException(status_code=404, detail="Function not found")
    return result


@app.get("/api/functions/{function_id}/callers")
async def get_callers(function_id: int):
    """Get functions that call this function."""
    result = await ALL_TOOLS['get_function_callers'](function_id=function_id)
    return result


@app.get("/api/functions/{function_id}/callees")
async def get_callees(function_id: int):
    """Get functions called by this function."""
    result = await ALL_TOOLS['get_function_callees'](function_id=function_id)
    return result


@app.get("/api/functions/find/{name}")
async def find_function(name: str):
    """Find function by name."""
    result = await ALL_TOOLS['find_function'](name=name)
    return result


# ═══════════════════════════════════════════════════════════════════════════════
# Stub Routes
# ═══════════════════════════════════════════════════════════════════════════════

@app.get("/api/stubs")
async def list_stubs(
    component_id: Optional[int] = None,
    feature_id: Optional[int] = None,
    resolved: Optional[bool] = None
):
    """List stubs."""
    result = await ALL_TOOLS['list_stubs'](
        component_id=component_id,
        feature_id=feature_id,
        resolved=resolved
    )
    return result


@app.get("/api/stubs/summary")
async def get_stub_summary():
    """Get stub summary by component."""
    result = await ALL_TOOLS['get_stub_summary']()
    return result


@app.get("/api/stubs/percentage")
async def get_stub_percentage():
    """Get overall stub percentage."""
    result = await ALL_TOOLS['get_stub_percentage']()
    return result


@app.post("/api/stubs/detect")
async def detect_stubs(file_id: int):
    """Detect stubs in a file."""
    result = await ALL_TOOLS['detect_stubs'](file_id=file_id)
    return result


@app.post("/api/stubs/{stub_id}/resolve")
async def resolve_stub(stub_id: int, data: StubResolveRequest):
    """Mark a stub as resolved."""
    result = await ALL_TOOLS['resolve_stub'](
        stub_id=stub_id,
        resolved_by=data.resolved_by,
        notes=data.notes
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


# ═══════════════════════════════════════════════════════════════════════════════
# Task Routes
# ═══════════════════════════════════════════════════════════════════════════════

@app.get("/api/tasks")
async def list_tasks(
    status: Optional[str] = None,
    priority: Optional[str] = None,
    feature_id: Optional[int] = None,
    epic_id: Optional[int] = None,
    assignee: Optional[str] = None
):
    """List tasks with filters."""
    result = await ALL_TOOLS['list_tasks'](
        status=status,
        priority=priority,
        feature_id=feature_id,
        epic_id=epic_id,
        assignee=assignee
    )
    return result


@app.post("/api/tasks")
async def create_task(data: TaskCreate):
    """Create a new task."""
    result = await ALL_TOOLS['create_task'](
        title=data.title,
        description=data.description,
        task_type=data.task_type,
        priority=data.priority,
        feature_id=data.feature_id,
        component_id=data.component_id,
        assignee=data.assignee,
        due_date=data.due_date
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


@app.get("/api/tasks/{task_id}")
async def get_task(task_id: int):
    """Get task details."""
    result = await ALL_TOOLS['get_task'](task_id=task_id)
    if not result.get("task"):
        raise HTTPException(status_code=404, detail="Task not found")
    return result


@app.patch("/api/tasks/{task_id}")
async def update_task(task_id: int, data: TaskUpdate):
    """Update task status."""
    result = await ALL_TOOLS['update_task_status'](
        task_id=task_id,
        status=data.status,
        assignee=data.assignee,
        notes=data.notes
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


@app.post("/api/tasks/{task_id}/comments")
async def add_comment(task_id: int, data: CommentCreate):
    """Add comment to task."""
    result = await ALL_TOOLS['add_task_comment'](
        task_id=task_id,
        content=data.content,
        author=data.author
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


# ═══════════════════════════════════════════════════════════════════════════════
# Epic Routes
# ═══════════════════════════════════════════════════════════════════════════════

@app.get("/api/epics")
async def list_epics():
    """List all epics."""
    result = await ALL_TOOLS['list_epics']()
    return result


@app.post("/api/epics")
async def create_epic(data: EpicCreate):
    """Create a new epic."""
    result = await ALL_TOOLS['create_epic'](
        name=data.name,
        description=data.description,
        priority=data.priority
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


@app.get("/api/epics/{epic_id}/progress")
async def get_epic_progress(epic_id: int):
    """Get epic progress."""
    result = await ALL_TOOLS['get_epic_progress'](epic_id=epic_id)
    return result


@app.post("/api/epics/{epic_id}/tasks/{task_id}")
async def add_task_to_epic(epic_id: int, task_id: int, order: int = 0):
    """Add task to epic."""
    result = await ALL_TOOLS['add_task_to_epic'](
        epic_id=epic_id,
        task_id=task_id,
        order=order
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


# ═══════════════════════════════════════════════════════════════════════════════
# Analysis Routes
# ═══════════════════════════════════════════════════════════════════════════════

@app.post("/api/analysis/scan")
async def full_scan(data: ScanRequest):
    """Run full codebase scan."""
    result = await ALL_TOOLS['full_codebase_scan'](
        root_path=data.root_path,
        extensions=data.extensions,
        exclude_dirs=data.exclude_dirs
    )
    return result


@app.post("/api/analysis/incremental")
async def incremental_scan(paths: list[str]):
    """Run incremental scan on specific paths."""
    result = await ALL_TOOLS['incremental_scan'](changed_paths=paths)
    return result


@app.post("/api/analysis/recommendations")
async def generate_recommendations(component_id: Optional[int] = None):
    """Generate recommendations."""
    result = await ALL_TOOLS['generate_recommendations'](component_id=component_id)
    return result


@app.get("/api/analysis/recommendations")
async def list_recommendations(
    status: Optional[str] = None,
    severity: Optional[str] = None
):
    """List recommendations."""
    result = await ALL_TOOLS['list_recommendations'](
        status=status,
        severity=severity
    )
    return result


@app.post("/api/analysis/recommendations/{recommendation_id}/accept")
async def accept_recommendation(recommendation_id: int, notes: str = ""):
    """Accept a recommendation."""
    result = await ALL_TOOLS['accept_recommendation'](
        recommendation_id=recommendation_id,
        notes=notes
    )
    if not result.get("success"):
        raise HTTPException(status_code=400, detail=result.get("error"))
    return result


# ═══════════════════════════════════════════════════════════════════════════════
# Impact Analysis Routes
# ═══════════════════════════════════════════════════════════════════════════════

@app.get("/api/impact/{file_path:path}")
async def analyze_impact(file_path: str):
    """Analyze impact of changes to a file."""
    result = await ALL_TOOLS['analyze_impact'](file_path=file_path)
    return result


@app.get("/api/dependencies/{entity_type}/{entity_id}")
async def get_dependencies(entity_type: str, entity_id: int):
    """Get dependency graph for an entity."""
    result = await ALL_TOOLS['get_dependency_graph'](
        entity_type=entity_type,
        entity_id=entity_id
    )
    return result


@app.get("/api/impact/what-uses/{function_name}")
async def what_uses_this(function_name: str):
    """Find what uses a function."""
    result = await ALL_TOOLS['what_uses_this'](name=function_name)
    return result


@app.get("/api/impact/what-does-use/{function_name}")
async def what_does_use(function_name: str):
    """Find what a function uses."""
    result = await ALL_TOOLS['what_does_this_use'](name=function_name)
    return result


# ═══════════════════════════════════════════════════════════════════════════════
# Health Check
# ═══════════════════════════════════════════════════════════════════════════════

@app.get("/health")
async def health_check():
    """Health check endpoint."""
    return {"status": "healthy", "service": "project-mcp-dashboard"}


@app.get("/api/stats")
async def get_stats():
    """Get database statistics."""
    from project_mcp.database import count
    return {
        "components": await count("components"),
        "features": await count("features"),
        "files": await count("files"),
        "functions": await count("functions"),
        "stubs": await count("stubs"),
        "tasks": await count("tasks"),
        "epics": await count("epics")
    }


# ═══════════════════════════════════════════════════════════════════════════════
# Main
# ═══════════════════════════════════════════════════════════════════════════════

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=1337)
