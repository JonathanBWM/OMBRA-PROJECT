# Project Management MCP - Tool Reference

## Analysis Tools (6 tools)

### `full_codebase_scan(project_root, component_mapping=None)`
Performs a complete codebase scan:
- Creates analysis run record
- Scans all source files using regex parser
- Indexes files, functions, structs, and stubs
- Generates recommendations automatically
- Returns comprehensive scan summary

**Use when:** Initial project setup, major refactoring, or full re-index needed

### `incremental_scan(since_timestamp=None)`
Scans only changed files since last scan:
- Checks file mtime against database records
- Re-indexes only modified files
- Much faster than full scan for routine updates
- Returns stats on files processed

**Use when:** Regular updates, CI/CD integration, watching for changes

### `get_codebase_health()`
Overall codebase health metrics:
- Total files, functions, LOC
- Stub percentage calculation
- Feature implementation breakdown
- TODO/FIXME counts
- Open recommendation counts

**Use when:** Dashboard display, status reports, health checks

### `generate_recommendations(scope="all", scope_id=None)`
Generate actionable recommendations:
- Critical stubbed features
- Orphan files not linked to features
- Features with no implementing files
- Scoped to component, feature, or file if specified

**Use when:** After scans, code reviews, planning sessions

### `list_recommendations(status="open", severity=None, recommendation_type=None)`
List and filter recommendations:
- Filter by status (open, accepted, rejected, implemented)
- Filter by severity (critical, warning, suggestion)
- Filter by type (stub_resolution, missing_feature, etc.)

**Use when:** Triaging work, prioritizing tasks

### `accept_recommendation(recommendation_id, create_task=True)`
Accept a recommendation:
- Marks recommendation as accepted
- Optionally creates a task from the recommendation
- Links task to affected feature/component

**Use when:** Converting recommendations to actionable work

## Impact Tools (4 tools)

### `analyze_impact(file_path=None, function_name=None, feature_id=None)`
Comprehensive impact analysis:
- Finds all affected files, features, functions
- Builds dependency chains
- Calculates risk level (critical, high, medium)
- Provides recommendations for changes

**Use when:** Before modifying code, planning refactoring, risk assessment

### `what_uses_this(file_path=None, function_name=None, struct_name=None)`
Find dependents of a given entity:
- Lists features using a file
- Lists functions calling a function
- Returns structured dependent list

**Use when:** Understanding usage, planning deprecation

### `what_does_this_use(file_path=None, function_name=None)`
Find dependencies of a given entity:
- Lists functions called by a file's functions
- Lists functions called by a specific function
- Returns structured dependency list

**Use when:** Understanding dependencies, planning isolation

### `get_dependency_graph(feature_id=None, component_id=None)`
Build visualization-ready dependency graph:
- Returns nodes and edges for graph rendering
- Supports feature or component scope
- Includes blocking relationships
- Ready for D3.js, Cytoscape, etc.

**Use when:** Visualizing architecture, planning work

## Dashboard Tools (5 tools)

### `get_dashboard_overview()`
**Single call for complete dashboard data:**
- Component summary with stats
- Feature breakdown by status/priority
- Stub metrics
- Recent activity (last 10 audit log entries)
- P0/P1 open tasks
- Open recommendations

**Use when:** Main dashboard display (minimizes round trips)

### `get_component_tree()`
Hierarchical component structure:
- Components with nested modules
- Features linked to each component
- File counts per component
- Ready for tree view rendering

**Use when:** Navigation UI, component explorer

### `get_feature_matrix()`
Feature-component relationship matrix:
- Shows which features are in which components
- Implementation status per cell
- Role (primary, supporting, testing)
- Ready for heatmap/table visualization

**Use when:** Coverage analysis, cross-cutting concern tracking

### `get_activity_feed(limit=20)`
Recent activity from audit log:
- Enriched with entity names
- Parsed JSON values
- Chronological order
- Configurable limit

**Use when:** Activity dashboard, change tracking

### `get_priority_items()`
High-priority work requiring attention:
- P0/P1 tasks
- Critical stubs (in priority features)
- Blocked tasks with blocker details
- Unimplemented priority features
- Critical recommendations

**Use when:** Sprint planning, daily standup, priority dashboard

## Implementation Details

### Database Operations
All tools use async database operations:
```python
from ..database import fetch_one, fetch_all, insert, update, count, execute
```

### Parser Integration
Analysis tools use the regex-based parser:
```python
from ..parser import scan_directory, analyze_file
```

### Error Handling
- Tools validate inputs and raise ValueError for invalid IDs
- Analysis scans track errors in run records
- Database operations are transactional

### Performance Optimizations
- Dashboard tools minimize round trips with joined queries
- Incremental scans only process changed files (mtime check)
- Recommendations batch-generated during full scans
- Activity feed uses LIMIT for performance

### JSON Storage
Several fields use JSON for arrays:
- `feature.blocks` / `feature.blocked_by`
- `function.calls` / `function.called_by`
- `task.blocking_tasks` / `task.blocked_by_tasks`
- `recommendation.affected_file_ids` / `affected_function_ids`

Always parse these with `json.loads()` when reading.

## Tool Export Pattern

Each file exports a dict of tools:
```python
ANALYSIS_TOOLS = {
    "full_codebase_scan": full_codebase_scan,
    "incremental_scan": incremental_scan,
    # ...
}
```

These are aggregated in `tools/__init__.py` into `ALL_TOOLS`.
