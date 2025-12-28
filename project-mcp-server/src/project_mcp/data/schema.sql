-- ============================================
-- OMBRA PROJECT MANAGEMENT MCP - SQLITE SCHEMA
-- Adapted from PostgreSQL spec for SQLite compatibility
-- Uses ChromaDB for vector embeddings (separate from SQLite)
-- ============================================

-- ============================================
-- CORE ENTITIES
-- ============================================

-- COMPONENTS: Top-level system components
CREATE TABLE IF NOT EXISTS components (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    description TEXT,
    component_type TEXT NOT NULL,  -- hypervisor, kernel_lib, usermode_api, kernel_driver, vuln_driver, bootloader, payload, common, installer, gui
    root_path TEXT,  -- Base directory for this component
    created_at TEXT DEFAULT (datetime('now')),
    updated_at TEXT DEFAULT (datetime('now')),

    -- Metadata
    language TEXT,  -- c, cpp, asm, mixed, python
    build_system TEXT  -- msbuild, cmake, make, custom
);

-- MODULES: Sub-components within a component
CREATE TABLE IF NOT EXISTS modules (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    component_id INTEGER REFERENCES components(id) ON DELETE CASCADE,
    name TEXT NOT NULL,
    description TEXT,
    path TEXT,  -- Relative to component root

    -- Status
    implementation_status TEXT DEFAULT 'not_started',  -- not_started, in_progress, implemented, needs_review, deprecated
    stub_percentage INTEGER DEFAULT 100,  -- 0 = fully implemented, 100 = fully stubbed

    created_at TEXT DEFAULT (datetime('now')),
    updated_at TEXT DEFAULT (datetime('now')),

    UNIQUE(component_id, name)
);

-- FEATURES: Actual features/capabilities
CREATE TABLE IF NOT EXISTS features (
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    -- Identity
    name TEXT NOT NULL,
    short_name TEXT,  -- For quick reference (e.g., "cpuid_mask")
    description TEXT NOT NULL,

    -- Classification
    category TEXT,  -- detection_evasion, memory_virtualization, process_management, communication, etc.
    subcategory TEXT,
    priority TEXT DEFAULT 'P2',  -- P0, P1, P2, P3

    -- Status
    status TEXT DEFAULT 'not_started',  -- not_started, planned, in_progress, implemented, testing, verified, deprecated
    implementation_percentage INTEGER DEFAULT 0,  -- 0-100
    stub_percentage INTEGER DEFAULT 0,  -- How much is stubbed vs real

    -- Ownership
    assigned_to TEXT,

    -- Timestamps
    created_at TEXT DEFAULT (datetime('now')),
    updated_at TEXT DEFAULT (datetime('now')),
    started_at TEXT,
    completed_at TEXT,

    -- Effort tracking
    estimated_hours INTEGER,
    actual_hours INTEGER,

    -- Dependencies (stored as JSON arrays)
    blocks TEXT,  -- JSON array of feature IDs this blocks
    blocked_by TEXT,  -- JSON array of feature IDs blocking this

    -- Documentation
    specification_doc TEXT,
    design_notes TEXT
);

-- FEATURE_COMPONENTS: Many-to-many feature to component mapping
CREATE TABLE IF NOT EXISTS feature_components (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    feature_id INTEGER REFERENCES features(id) ON DELETE CASCADE,
    component_id INTEGER REFERENCES components(id) ON DELETE CASCADE,
    role TEXT,  -- primary, supporting, testing

    UNIQUE(feature_id, component_id)
);

-- ============================================
-- FILE TRACKING (THE BRAIN)
-- ============================================

-- FILES: Every source file indexed
CREATE TABLE IF NOT EXISTS files (
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    -- Location
    absolute_path TEXT NOT NULL UNIQUE,
    relative_path TEXT,  -- Relative to project root
    filename TEXT NOT NULL,
    extension TEXT,

    -- Ownership
    component_id INTEGER REFERENCES components(id),
    module_id INTEGER REFERENCES modules(id),

    -- Content analysis
    language TEXT,  -- c, cpp, h, hpp, asm, py, etc.
    line_count INTEGER,
    function_count INTEGER,
    struct_count INTEGER,

    -- Quality metrics
    stub_line_count INTEGER DEFAULT 0,
    todo_count INTEGER DEFAULT 0,
    fixme_count INTEGER DEFAULT 0,

    -- Hashes for change detection
    content_hash TEXT,
    last_indexed TEXT,

    -- File modification time for incremental scanning
    mtime REAL,

    created_at TEXT DEFAULT (datetime('now')),
    updated_at TEXT DEFAULT (datetime('now'))
);

-- FUNCTIONS: Every function/procedure indexed
CREATE TABLE IF NOT EXISTS functions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file_id INTEGER REFERENCES files(id) ON DELETE CASCADE,

    -- Identity
    name TEXT NOT NULL,
    signature TEXT,  -- Full signature
    return_type TEXT,

    -- Location
    line_start INTEGER,
    line_end INTEGER,
    column_start INTEGER,

    -- Analysis
    is_stub INTEGER DEFAULT 0,  -- 0 = false, 1 = true
    stub_reason TEXT,  -- "TODO", "NotImplemented", "return 0", etc.
    complexity INTEGER,  -- Cyclomatic complexity

    -- Documentation
    has_doc_comment INTEGER DEFAULT 0,
    doc_comment TEXT,

    -- Linking (stored as JSON arrays)
    calls TEXT,  -- JSON array of function names this calls
    called_by TEXT,  -- JSON array of functions that call this

    created_at TEXT DEFAULT (datetime('now')),
    updated_at TEXT DEFAULT (datetime('now')),

    UNIQUE(file_id, name, line_start)
);

-- STRUCTS: Every struct/type indexed
CREATE TABLE IF NOT EXISTS structs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file_id INTEGER REFERENCES files(id) ON DELETE CASCADE,

    name TEXT NOT NULL,
    definition TEXT,  -- Full struct definition
    size_bytes INTEGER,
    member_count INTEGER,

    line_start INTEGER,
    line_end INTEGER,

    UNIQUE(file_id, name)
);

-- ============================================
-- FEATURE-FILE LINKAGE (CRITICAL)
-- ============================================

-- FEATURE_FILES: Links features to implementing files
CREATE TABLE IF NOT EXISTS feature_files (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    feature_id INTEGER REFERENCES features(id) ON DELETE CASCADE,
    file_id INTEGER REFERENCES files(id) ON DELETE CASCADE,

    -- Role of this file in the feature
    role TEXT,  -- implementation, header, test, example, documentation

    -- Specific locations (JSON arrays)
    relevant_functions TEXT,  -- JSON array of function names in this file for this feature
    line_ranges TEXT,  -- JSON array of line ranges (e.g., ["100-150", "200-250"])

    -- Confidence
    confidence REAL DEFAULT 1.0,  -- 0-1, how confident we are this file relates to feature
    manually_linked INTEGER DEFAULT 0,  -- 0 = auto-detected, 1 = manually linked

    notes TEXT,

    created_at TEXT DEFAULT (datetime('now')),

    UNIQUE(feature_id, file_id)
);

-- FEATURE_FUNCTIONS: Links features to specific functions
CREATE TABLE IF NOT EXISTS feature_functions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    feature_id INTEGER REFERENCES features(id) ON DELETE CASCADE,
    function_id INTEGER REFERENCES functions(id) ON DELETE CASCADE,

    role TEXT,  -- entry_point, helper, callback, handler
    is_stub INTEGER DEFAULT 0,

    UNIQUE(feature_id, function_id)
);

-- ============================================
-- IMPLEMENTATION TRACKING
-- ============================================

-- IMPLEMENTATION_STATUS: Detailed status per feature per component
CREATE TABLE IF NOT EXISTS implementation_status (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    feature_id INTEGER REFERENCES features(id) ON DELETE CASCADE,
    component_id INTEGER REFERENCES components(id) ON DELETE CASCADE,

    status TEXT DEFAULT 'not_started',
    percentage INTEGER DEFAULT 0,
    stub_percentage INTEGER DEFAULT 0,

    -- Evidence (JSON arrays of IDs)
    implementing_files TEXT,  -- JSON array of file IDs
    implementing_functions TEXT,  -- JSON array of function IDs

    -- Verification
    last_verified TEXT,
    verified_by TEXT,  -- agent, human, automated
    verification_notes TEXT,

    -- Issues (JSON arrays)
    blockers TEXT,
    warnings TEXT,

    created_at TEXT DEFAULT (datetime('now')),
    updated_at TEXT DEFAULT (datetime('now')),

    UNIQUE(feature_id, component_id)
);

-- STUBS: Track all detected stubs
CREATE TABLE IF NOT EXISTS stubs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    -- Location
    file_id INTEGER REFERENCES files(id) ON DELETE CASCADE,
    function_id INTEGER REFERENCES functions(id),

    line_number INTEGER,
    stub_type TEXT,  -- todo, fixme, not_implemented, return_zero, return_null, empty_body, placeholder

    -- Content
    stub_text TEXT,  -- The actual stub code
    surrounding_context TEXT,  -- Lines around the stub

    -- Tracking
    detected_at TEXT DEFAULT (datetime('now')),
    resolved_at TEXT,
    resolved_by TEXT,

    -- Linkage
    feature_id INTEGER REFERENCES features(id),  -- Which feature this stub belongs to
    task_id INTEGER,  -- Link to task tracking

    -- Priority
    severity TEXT DEFAULT 'medium'  -- critical, high, medium, low
);

-- ============================================
-- TASK MANAGEMENT (JIRA-LEVEL)
-- ============================================

-- EPICS: High-level goals
CREATE TABLE IF NOT EXISTS epics (
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    name TEXT NOT NULL,
    description TEXT,

    status TEXT DEFAULT 'open',  -- open, in_progress, completed, cancelled
    priority TEXT DEFAULT 'P1',

    -- Ownership
    owner TEXT,

    -- Dates
    created_at TEXT DEFAULT (datetime('now')),
    updated_at TEXT DEFAULT (datetime('now')),
    target_date TEXT,
    completed_at TEXT,

    -- Progress
    total_tasks INTEGER DEFAULT 0,
    completed_tasks INTEGER DEFAULT 0
);

-- TASKS: Individual work items
CREATE TABLE IF NOT EXISTS tasks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    -- Identity
    title TEXT NOT NULL,
    description TEXT,
    task_type TEXT DEFAULT 'task',  -- task, bug, improvement, research, documentation

    -- Hierarchy
    epic_id INTEGER REFERENCES epics(id),
    parent_task_id INTEGER REFERENCES tasks(id),

    -- Status
    status TEXT DEFAULT 'todo',  -- todo, in_progress, review, testing, done, blocked, cancelled
    priority TEXT DEFAULT 'P2',

    -- Assignment
    assigned_to TEXT,

    -- Dates
    created_at TEXT DEFAULT (datetime('now')),
    updated_at TEXT DEFAULT (datetime('now')),
    started_at TEXT,
    completed_at TEXT,
    due_date TEXT,

    -- Effort
    estimated_hours REAL,
    actual_hours REAL,

    -- Linkage
    feature_id INTEGER REFERENCES features(id),
    component_id INTEGER REFERENCES components(id),
    blocking_tasks TEXT,  -- JSON array of task IDs
    blocked_by_tasks TEXT,  -- JSON array of task IDs

    -- Files affected (JSON array of file IDs)
    affected_files TEXT,

    -- Tags (JSON array)
    tags TEXT
);

-- TASK_COMMENTS: Discussion on tasks
CREATE TABLE IF NOT EXISTS task_comments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    task_id INTEGER REFERENCES tasks(id) ON DELETE CASCADE,

    author TEXT,
    content TEXT NOT NULL,

    created_at TEXT DEFAULT (datetime('now')),

    -- Type
    comment_type TEXT DEFAULT 'comment'  -- comment, status_change, code_review, blocker
);

-- ============================================
-- RECOMMENDATIONS ENGINE
-- ============================================

-- RECOMMENDATIONS: AI-generated suggestions
CREATE TABLE IF NOT EXISTS recommendations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    -- Classification
    recommendation_type TEXT,  -- missing_feature, stub_resolution, code_improvement, architecture, performance, security
    severity TEXT DEFAULT 'suggestion',  -- critical, warning, suggestion, info

    -- Content
    title TEXT NOT NULL,
    description TEXT NOT NULL,
    rationale TEXT,  -- Why this is recommended

    -- Affected entities
    affected_feature_id INTEGER REFERENCES features(id),
    affected_component_id INTEGER REFERENCES components(id),
    affected_file_ids TEXT,  -- JSON array
    affected_function_ids TEXT,  -- JSON array

    -- Action
    suggested_action TEXT,
    estimated_effort TEXT,

    -- Status
    status TEXT DEFAULT 'open',  -- open, accepted, rejected, implemented, deferred

    -- Source
    generated_by TEXT,  -- codebase_scan, comparison_analysis, pattern_detection, manual
    generated_at TEXT DEFAULT (datetime('now')),

    -- Resolution
    resolved_at TEXT,
    resolution_notes TEXT
);

-- ============================================
-- CODEBASE ANALYSIS
-- ============================================

-- ANALYSIS_RUNS: Track analysis executions
CREATE TABLE IF NOT EXISTS analysis_runs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    run_type TEXT,  -- full_scan, incremental, feature_specific, stub_detection
    status TEXT DEFAULT 'running',  -- running, completed, failed

    started_at TEXT DEFAULT (datetime('now')),
    completed_at TEXT,

    -- Stats
    files_scanned INTEGER DEFAULT 0,
    functions_indexed INTEGER DEFAULT 0,
    stubs_detected INTEGER DEFAULT 0,
    recommendations_generated INTEGER DEFAULT 0,

    -- Errors (JSON arrays)
    errors TEXT,
    warnings TEXT
);

-- CODE_PATTERNS: Detected patterns in codebase
CREATE TABLE IF NOT EXISTS code_patterns (
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    pattern_type TEXT,  -- stub, todo, anti_pattern, security_issue, performance_issue
    pattern_signature TEXT,  -- Regex pattern

    description TEXT,
    severity TEXT,

    -- Occurrences
    occurrence_count INTEGER DEFAULT 0,
    file_ids TEXT,  -- JSON array

    -- Example
    example_code TEXT,
    example_file_id INTEGER REFERENCES files(id),
    example_line INTEGER,

    last_scanned TEXT
);

-- ============================================
-- CROSS-MCP INTEGRATION
-- ============================================

-- MCP_LINKS: Links to other MCP databases
CREATE TABLE IF NOT EXISTS mcp_links (
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    source_entity_type TEXT,  -- feature, component, file, function
    source_entity_id INTEGER,

    target_mcp TEXT,  -- ombra_mcp_server, driver_re_mcp
    target_entity_type TEXT,
    target_entity_id INTEGER,

    link_type TEXT,  -- implements, references, related, derived_from

    confidence REAL DEFAULT 1.0,

    created_at TEXT DEFAULT (datetime('now'))
);

-- ============================================
-- AUDIT TRAIL
-- ============================================

-- AUDIT_LOG: Track all changes
CREATE TABLE IF NOT EXISTS audit_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    entity_type TEXT NOT NULL,  -- feature, task, file, etc.
    entity_id INTEGER NOT NULL,

    action TEXT NOT NULL,  -- create, update, delete, status_change

    old_value TEXT,  -- JSON
    new_value TEXT,  -- JSON

    performed_by TEXT,  -- agent, human, system
    performed_at TEXT DEFAULT (datetime('now')),

    notes TEXT
);

-- ============================================
-- INDEXES FOR PERFORMANCE
-- ============================================

CREATE INDEX IF NOT EXISTS idx_files_path ON files(absolute_path);
CREATE INDEX IF NOT EXISTS idx_files_component ON files(component_id);
CREATE INDEX IF NOT EXISTS idx_files_extension ON files(extension);
CREATE INDEX IF NOT EXISTS idx_files_mtime ON files(mtime);
CREATE INDEX IF NOT EXISTS idx_functions_name ON functions(name);
CREATE INDEX IF NOT EXISTS idx_functions_file ON functions(file_id);
CREATE INDEX IF NOT EXISTS idx_functions_stub ON functions(is_stub);
CREATE INDEX IF NOT EXISTS idx_features_status ON features(status);
CREATE INDEX IF NOT EXISTS idx_features_priority ON features(priority);
CREATE INDEX IF NOT EXISTS idx_features_category ON features(category);
CREATE INDEX IF NOT EXISTS idx_tasks_status ON tasks(status);
CREATE INDEX IF NOT EXISTS idx_tasks_priority ON tasks(priority);
CREATE INDEX IF NOT EXISTS idx_stubs_file ON stubs(file_id);
CREATE INDEX IF NOT EXISTS idx_stubs_resolved ON stubs(resolved_at);
CREATE INDEX IF NOT EXISTS idx_recommendations_status ON recommendations(status);
CREATE INDEX IF NOT EXISTS idx_analysis_runs_status ON analysis_runs(status);
CREATE INDEX IF NOT EXISTS idx_feature_files_feature ON feature_files(feature_id);
CREATE INDEX IF NOT EXISTS idx_feature_files_file ON feature_files(file_id);
CREATE INDEX IF NOT EXISTS idx_feature_functions_feature ON feature_functions(feature_id);

-- ============================================
-- FULL-TEXT SEARCH (FTS5)
-- ============================================

CREATE VIRTUAL TABLE IF NOT EXISTS files_fts USING fts5(
    filename,
    relative_path,
    content='files',
    content_rowid='id'
);

CREATE VIRTUAL TABLE IF NOT EXISTS functions_fts USING fts5(
    name,
    signature,
    doc_comment,
    content='functions',
    content_rowid='id'
);

CREATE VIRTUAL TABLE IF NOT EXISTS features_fts USING fts5(
    name,
    short_name,
    description,
    design_notes,
    content='features',
    content_rowid='id'
);

CREATE VIRTUAL TABLE IF NOT EXISTS tasks_fts USING fts5(
    title,
    description,
    content='tasks',
    content_rowid='id'
);

-- ============================================
-- TRIGGERS FOR FTS SYNC
-- ============================================

CREATE TRIGGER IF NOT EXISTS files_ai AFTER INSERT ON files BEGIN
    INSERT INTO files_fts(rowid, filename, relative_path) VALUES (new.id, new.filename, new.relative_path);
END;

CREATE TRIGGER IF NOT EXISTS files_ad AFTER DELETE ON files BEGIN
    INSERT INTO files_fts(files_fts, rowid, filename, relative_path) VALUES('delete', old.id, old.filename, old.relative_path);
END;

CREATE TRIGGER IF NOT EXISTS files_au AFTER UPDATE ON files BEGIN
    INSERT INTO files_fts(files_fts, rowid, filename, relative_path) VALUES('delete', old.id, old.filename, old.relative_path);
    INSERT INTO files_fts(rowid, filename, relative_path) VALUES (new.id, new.filename, new.relative_path);
END;

CREATE TRIGGER IF NOT EXISTS functions_ai AFTER INSERT ON functions BEGIN
    INSERT INTO functions_fts(rowid, name, signature, doc_comment) VALUES (new.id, new.name, new.signature, new.doc_comment);
END;

CREATE TRIGGER IF NOT EXISTS functions_ad AFTER DELETE ON functions BEGIN
    INSERT INTO functions_fts(functions_fts, rowid, name, signature, doc_comment) VALUES('delete', old.id, old.name, old.signature, old.doc_comment);
END;

CREATE TRIGGER IF NOT EXISTS functions_au AFTER UPDATE ON functions BEGIN
    INSERT INTO functions_fts(functions_fts, rowid, name, signature, doc_comment) VALUES('delete', old.id, old.name, old.signature, old.doc_comment);
    INSERT INTO functions_fts(rowid, name, signature, doc_comment) VALUES (new.id, new.name, new.signature, new.doc_comment);
END;

CREATE TRIGGER IF NOT EXISTS features_ai AFTER INSERT ON features BEGIN
    INSERT INTO features_fts(rowid, name, short_name, description, design_notes) VALUES (new.id, new.name, new.short_name, new.description, new.design_notes);
END;

CREATE TRIGGER IF NOT EXISTS features_ad AFTER DELETE ON features BEGIN
    INSERT INTO features_fts(features_fts, rowid, name, short_name, description, design_notes) VALUES('delete', old.id, old.name, old.short_name, old.description, old.design_notes);
END;

CREATE TRIGGER IF NOT EXISTS features_au AFTER UPDATE ON features BEGIN
    INSERT INTO features_fts(features_fts, rowid, name, short_name, description, design_notes) VALUES('delete', old.id, old.name, old.short_name, old.description, old.design_notes);
    INSERT INTO features_fts(rowid, name, short_name, description, design_notes) VALUES (new.id, new.name, new.short_name, new.description, new.design_notes);
END;

CREATE TRIGGER IF NOT EXISTS tasks_ai AFTER INSERT ON tasks BEGIN
    INSERT INTO tasks_fts(rowid, title, description) VALUES (new.id, new.title, new.description);
END;

CREATE TRIGGER IF NOT EXISTS tasks_ad AFTER DELETE ON tasks BEGIN
    INSERT INTO tasks_fts(tasks_fts, rowid, title, description) VALUES('delete', old.id, old.title, old.description);
END;

CREATE TRIGGER IF NOT EXISTS tasks_au AFTER UPDATE ON tasks BEGIN
    INSERT INTO tasks_fts(tasks_fts, rowid, title, description) VALUES('delete', old.id, old.title, old.description);
    INSERT INTO tasks_fts(rowid, title, description) VALUES (new.id, new.title, new.description);
END;

-- ============================================
-- UPDATE TIMESTAMP TRIGGERS
-- ============================================

CREATE TRIGGER IF NOT EXISTS update_components_timestamp AFTER UPDATE ON components BEGIN
    UPDATE components SET updated_at = datetime('now') WHERE id = new.id;
END;

CREATE TRIGGER IF NOT EXISTS update_modules_timestamp AFTER UPDATE ON modules BEGIN
    UPDATE modules SET updated_at = datetime('now') WHERE id = new.id;
END;

CREATE TRIGGER IF NOT EXISTS update_features_timestamp AFTER UPDATE ON features BEGIN
    UPDATE features SET updated_at = datetime('now') WHERE id = new.id;
END;

CREATE TRIGGER IF NOT EXISTS update_files_timestamp AFTER UPDATE ON files BEGIN
    UPDATE files SET updated_at = datetime('now') WHERE id = new.id;
END;

CREATE TRIGGER IF NOT EXISTS update_functions_timestamp AFTER UPDATE ON functions BEGIN
    UPDATE functions SET updated_at = datetime('now') WHERE id = new.id;
END;

CREATE TRIGGER IF NOT EXISTS update_tasks_timestamp AFTER UPDATE ON tasks BEGIN
    UPDATE tasks SET updated_at = datetime('now') WHERE id = new.id;
END;

-- ============================================
-- DRIVER ANALYSIS (OMBRA-DRIVER-SCANNER INTEGRATION)
-- ============================================

-- DRIVERS: Scanned driver analysis results
CREATE TABLE IF NOT EXISTS drivers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    -- Driver identification
    driver_path TEXT NOT NULL,
    driver_name TEXT NOT NULL,
    hash_sha256 TEXT UNIQUE NOT NULL,
    hash_md5 TEXT,

    -- Scanner results
    capability_tier TEXT,  -- S, A, B, C, D
    final_score REAL,
    classification TEXT,  -- EXCELLENT, GOOD, FAIR, POOR

    -- Score breakdown
    capability_score INTEGER,
    accessibility_score INTEGER,
    obscurity_score INTEGER,
    stealth_score INTEGER,

    -- Capability flags
    has_physical_memory INTEGER DEFAULT 0,
    has_msr_access INTEGER DEFAULT 0,
    has_process_control INTEGER DEFAULT 0,
    has_module_loading INTEGER DEFAULT 0,
    has_cr_access INTEGER DEFAULT 0,
    has_port_io INTEGER DEFAULT 0,
    has_pci_config INTEGER DEFAULT 0,
    has_virtualization INTEGER DEFAULT 0,

    -- Driver family and patterns
    driver_family TEXT,  -- VirtualBox, Capcom, GDRV, etc.

    -- Dangerous APIs found (JSON array)
    dangerous_apis TEXT,  -- JSON array of API names

    -- Detected IOCTLs (JSON array)
    detected_ioctls TEXT,  -- JSON array of IOCTL codes

    -- Blocklist status
    on_loldrivers INTEGER DEFAULT 0,
    on_msdbx INTEGER DEFAULT 0,

    -- Certificate information
    is_signed INTEGER DEFAULT 0,
    signer_name TEXT,
    cert_revoked INTEGER DEFAULT 0,

    -- Forensic risk indicators
    ff25_count INTEGER DEFAULT 0,
    has_gshandler INTEGER DEFAULT 0,
    debug_string_count INTEGER DEFAULT 0,

    -- Analysis status
    analysis_status TEXT DEFAULT 'pending',  -- pending, in_progress, analyzed, failed
    priority INTEGER DEFAULT 5,  -- 1 = highest (Tier S), 5 = lowest (Tier D)

    -- Timestamps
    scanned_at TEXT,
    analysis_started_at TEXT,
    analysis_completed_at TEXT,

    -- Scanner metadata
    scanner_version TEXT,

    created_at TEXT DEFAULT (datetime('now')),
    updated_at TEXT DEFAULT (datetime('now'))
);

-- ============================================
-- DRIVER ANALYSIS INDEXES
-- ============================================

CREATE INDEX IF NOT EXISTS idx_drivers_hash ON drivers(hash_sha256);
CREATE INDEX IF NOT EXISTS idx_drivers_tier ON drivers(capability_tier);
CREATE INDEX IF NOT EXISTS idx_drivers_score ON drivers(final_score);
CREATE INDEX IF NOT EXISTS idx_drivers_status ON drivers(analysis_status);
CREATE INDEX IF NOT EXISTS idx_drivers_priority ON drivers(priority);
CREATE INDEX IF NOT EXISTS idx_drivers_family ON drivers(driver_family);

-- ============================================
-- DRIVER ANALYSIS TRIGGERS
-- ============================================

CREATE TRIGGER IF NOT EXISTS update_drivers_timestamp AFTER UPDATE ON drivers BEGIN
    UPDATE drivers SET updated_at = datetime('now') WHERE id = new.id;
END;
