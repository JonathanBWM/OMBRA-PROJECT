-- ============================================
-- DRIVER RE MCP DATABASE SCHEMA (SQLite)
-- ============================================
-- Adapted from PostgreSQL schema for SQLite
-- Uses INTEGER PRIMARY KEY for auto-increment
-- TEXT instead of VARCHAR
-- REAL instead of FLOAT
-- JSON stored as TEXT
-- No arrays - use separate tables or JSON
-- No GENERATED ALWAYS AS columns - use triggers or app logic
-- ============================================

-- ============================================
-- DRIVER METADATA
-- ============================================
CREATE TABLE IF NOT EXISTS drivers (
    id                  TEXT PRIMARY KEY,

    -- Identification
    original_name       TEXT NOT NULL,
    analyzed_name       TEXT,

    -- File Hashes
    md5                 TEXT NOT NULL,
    sha1                TEXT NOT NULL,
    sha256              TEXT NOT NULL UNIQUE,
    imphash             TEXT,

    -- PE Metadata
    file_size           INTEGER NOT NULL,
    image_base          INTEGER NOT NULL,
    entry_point_rva     INTEGER NOT NULL,
    size_of_image       INTEGER NOT NULL,
    timestamp           INTEGER,
    machine             INTEGER NOT NULL,
    subsystem           INTEGER NOT NULL,
    characteristics     INTEGER NOT NULL,

    -- Version Info
    file_version        TEXT,
    product_version     TEXT,
    company_name        TEXT,
    product_name        TEXT,
    file_description    TEXT,

    -- Build Info
    build_path          TEXT,
    pdb_path            TEXT,

    -- Analysis State
    analysis_status     TEXT DEFAULT 'pending',

    -- Metadata
    notes               TEXT,
    tags                TEXT, -- JSON array
    created_at          TEXT DEFAULT CURRENT_TIMESTAMP,
    updated_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_drivers_sha256 ON drivers(sha256);
CREATE INDEX IF NOT EXISTS idx_drivers_md5 ON drivers(md5);
CREATE INDEX IF NOT EXISTS idx_drivers_name ON drivers(original_name);

-- ============================================
-- PE SECTIONS
-- ============================================
CREATE TABLE IF NOT EXISTS sections (
    id                  TEXT PRIMARY KEY,
    driver_id           TEXT NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,

    name                TEXT NOT NULL,
    virtual_address     INTEGER NOT NULL,
    virtual_size        INTEGER NOT NULL,
    raw_address         INTEGER NOT NULL,
    raw_size            INTEGER NOT NULL,
    characteristics     INTEGER NOT NULL,
    entropy             REAL,

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_sections_driver ON sections(driver_id);

-- ============================================
-- IMPORTS (NT KERNEL APIs)
-- ============================================
CREATE TABLE IF NOT EXISTS imports (
    id                  TEXT PRIMARY KEY,
    driver_id           TEXT NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,

    dll_name            TEXT NOT NULL,
    function_name       TEXT,
    ordinal             INTEGER,
    hint                INTEGER,

    -- IAT Info
    iat_rva             INTEGER,

    -- Categorization
    category            TEXT,
    subcategory         TEXT,

    -- Security relevance
    is_dangerous        INTEGER DEFAULT 0,
    danger_reason       TEXT,

    -- Documentation
    description         TEXT,

    -- Usage analysis
    usage_count         INTEGER DEFAULT 0,

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_imports_driver ON imports(driver_id);
CREATE INDEX IF NOT EXISTS idx_imports_function ON imports(function_name);
CREATE INDEX IF NOT EXISTS idx_imports_dll ON imports(dll_name);
CREATE INDEX IF NOT EXISTS idx_imports_category ON imports(category);
CREATE INDEX IF NOT EXISTS idx_imports_dangerous ON imports(is_dangerous) WHERE is_dangerous = 1;

-- ============================================
-- EXPORTS
-- ============================================
CREATE TABLE IF NOT EXISTS exports (
    id                  TEXT PRIMARY KEY,
    driver_id           TEXT NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,

    function_name       TEXT,
    ordinal             INTEGER NOT NULL,
    rva                 INTEGER NOT NULL,

    -- Categorization
    prefix              TEXT,
    category            TEXT,

    -- Analysis
    is_dangerous        INTEGER DEFAULT 0,
    danger_reason       TEXT,

    -- Function signature
    return_type         TEXT,
    parameters          TEXT, -- JSON
    calling_convention  TEXT,

    -- Documentation
    description         TEXT,

    -- Decompiled code
    decompiled_code     TEXT,

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_exports_driver ON exports(driver_id);
CREATE INDEX IF NOT EXISTS idx_exports_function ON exports(function_name);
CREATE INDEX IF NOT EXISTS idx_exports_prefix ON exports(prefix);
CREATE INDEX IF NOT EXISTS idx_exports_category ON exports(category);
CREATE INDEX IF NOT EXISTS idx_exports_dangerous ON exports(is_dangerous) WHERE is_dangerous = 1;

-- ============================================
-- IOCTLS (CRITICAL)
-- ============================================
CREATE TABLE IF NOT EXISTS ioctls (
    id                  TEXT PRIMARY KEY,
    driver_id           TEXT NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,

    -- IOCTL Identity
    name                TEXT NOT NULL,
    code                INTEGER,
    code_hex            TEXT,

    -- CTL_CODE components
    device_type         INTEGER,
    function_code       INTEGER,
    method              INTEGER,
    access              INTEGER,

    -- Handler Info
    handler_rva         INTEGER,
    handler_function_id TEXT REFERENCES functions(id),

    -- Input/Output
    input_struct_id     TEXT REFERENCES structures(id),
    output_struct_id    TEXT REFERENCES structures(id),
    min_input_size      INTEGER,
    max_input_size      INTEGER,
    min_output_size     INTEGER,
    max_output_size     INTEGER,

    -- Security Analysis
    requires_admin      INTEGER,
    requires_session    INTEGER DEFAULT 1,

    -- Vulnerability Assessment
    is_vulnerable       INTEGER DEFAULT 0,
    vulnerability_type  TEXT,
    vulnerability_severity TEXT,
    vulnerability_description TEXT,
    exploitation_notes  TEXT,
    cve_ids             TEXT, -- JSON array

    -- Documentation
    description         TEXT,

    -- Fast path info
    has_fast_path       INTEGER DEFAULT 0,

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_ioctls_driver ON ioctls(driver_id);
CREATE INDEX IF NOT EXISTS idx_ioctls_name ON ioctls(name);
CREATE INDEX IF NOT EXISTS idx_ioctls_code ON ioctls(code);
CREATE INDEX IF NOT EXISTS idx_ioctls_vulnerable ON ioctls(is_vulnerable) WHERE is_vulnerable = 1;

-- ============================================
-- DATA STRUCTURES
-- ============================================
CREATE TABLE IF NOT EXISTS structures (
    id                  TEXT PRIMARY KEY,
    driver_id           TEXT REFERENCES drivers(id) ON DELETE CASCADE,

    name                TEXT NOT NULL,
    size                INTEGER,
    alignment           INTEGER,

    -- Type
    struct_type         TEXT,

    -- Definition
    definition_c        TEXT,
    definition_json     TEXT, -- JSON

    -- Source
    source              TEXT,

    description         TEXT,

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_structures_driver ON structures(driver_id);
CREATE INDEX IF NOT EXISTS idx_structures_name ON structures(name);
CREATE INDEX IF NOT EXISTS idx_structures_type ON structures(struct_type);

-- ============================================
-- STRUCTURE MEMBERS
-- ============================================
CREATE TABLE IF NOT EXISTS structure_members (
    id                  TEXT PRIMARY KEY,
    structure_id        TEXT NOT NULL REFERENCES structures(id) ON DELETE CASCADE,

    name                TEXT NOT NULL,
    offset              INTEGER NOT NULL,
    size                INTEGER NOT NULL,
    type_name           TEXT,

    -- Nested structure reference
    nested_struct_id    TEXT REFERENCES structures(id),

    -- Array info
    is_array            INTEGER DEFAULT 0,
    array_count         INTEGER,

    -- Pointer info
    is_pointer          INTEGER DEFAULT 0,
    pointer_depth       INTEGER DEFAULT 0,

    -- Bitfield info
    is_bitfield         INTEGER DEFAULT 0,
    bit_offset          INTEGER,
    bit_size            INTEGER,

    description         TEXT,

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_struct_members_struct ON structure_members(structure_id);

-- ============================================
-- FUNCTIONS (ANALYZED)
-- ============================================
CREATE TABLE IF NOT EXISTS functions (
    id                  TEXT PRIMARY KEY,
    driver_id           TEXT NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,

    -- Identity
    name                TEXT,
    rva                 INTEGER NOT NULL,
    size                INTEGER,

    -- Source
    source              TEXT,

    -- Signature
    return_type         TEXT,
    parameters          TEXT, -- JSON
    calling_convention  TEXT DEFAULT 'fastcall',

    -- Analysis
    is_entry_point      INTEGER DEFAULT 0,
    is_dispatch         INTEGER DEFAULT 0,
    dispatch_type       TEXT,

    -- Code
    assembly            TEXT,
    decompiled          TEXT,
    pseudocode          TEXT,

    -- Annotations
    annotations         TEXT, -- JSON

    -- Complexity metrics
    cyclomatic_complexity INTEGER,
    basic_block_count   INTEGER,
    instruction_count   INTEGER,

    -- Ghidra sync
    ghidra_synced       INTEGER DEFAULT 0,
    ghidra_sync_time    TEXT,

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP,
    updated_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_functions_driver ON functions(driver_id);
CREATE INDEX IF NOT EXISTS idx_functions_name ON functions(name);
CREATE INDEX IF NOT EXISTS idx_functions_rva ON functions(rva);
CREATE INDEX IF NOT EXISTS idx_functions_dispatch ON functions(is_dispatch) WHERE is_dispatch = 1;

-- ============================================
-- CROSS-REFERENCES
-- ============================================
CREATE TABLE IF NOT EXISTS xrefs (
    id                  TEXT PRIMARY KEY,
    driver_id           TEXT NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,

    -- From
    from_rva            INTEGER NOT NULL,
    from_function_id    TEXT REFERENCES functions(id),

    -- To
    to_rva              INTEGER NOT NULL,
    to_function_id      TEXT REFERENCES functions(id),
    to_import_id        TEXT REFERENCES imports(id),
    to_export_id        TEXT REFERENCES exports(id),

    -- Type
    xref_type           TEXT NOT NULL,

    -- Context
    instruction         TEXT,

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_xrefs_driver ON xrefs(driver_id);
CREATE INDEX IF NOT EXISTS idx_xrefs_from_func ON xrefs(from_function_id);
CREATE INDEX IF NOT EXISTS idx_xrefs_to_func ON xrefs(to_function_id);
CREATE INDEX IF NOT EXISTS idx_xrefs_to_import ON xrefs(to_import_id);
CREATE INDEX IF NOT EXISTS idx_xrefs_from_rva ON xrefs(from_rva);
CREATE INDEX IF NOT EXISTS idx_xrefs_to_rva ON xrefs(to_rva);

-- ============================================
-- GLOBAL VARIABLES
-- ============================================
CREATE TABLE IF NOT EXISTS globals (
    id                  TEXT PRIMARY KEY,
    driver_id           TEXT NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,

    name                TEXT,
    rva                 INTEGER NOT NULL,
    size                INTEGER,

    type_name           TEXT,
    structure_id        TEXT REFERENCES structures(id),

    -- Initial value
    initial_value_hex   TEXT,

    -- Section
    section_name        TEXT,

    -- Security relevance
    is_sensitive        INTEGER DEFAULT 0,
    sensitivity_reason  TEXT,

    description         TEXT,

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_globals_driver ON globals(driver_id);
CREATE INDEX IF NOT EXISTS idx_globals_name ON globals(name);
CREATE INDEX IF NOT EXISTS idx_globals_rva ON globals(rva);

-- ============================================
-- STRINGS
-- ============================================
CREATE TABLE IF NOT EXISTS strings (
    id                  TEXT PRIMARY KEY,
    driver_id           TEXT NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,

    value               TEXT NOT NULL,
    rva                 INTEGER NOT NULL,
    length              INTEGER NOT NULL,

    encoding            TEXT,
    section_name        TEXT,

    -- Categorization
    category            TEXT,

    -- References
    reference_count     INTEGER DEFAULT 0,

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_strings_driver ON strings(driver_id);
CREATE INDEX IF NOT EXISTS idx_strings_category ON strings(category);

-- Full-text search for strings
CREATE VIRTUAL TABLE IF NOT EXISTS strings_fts USING fts5(
    value,
    content=strings,
    content_rowid=rowid
);

-- ============================================
-- VULNERABILITIES
-- ============================================
CREATE TABLE IF NOT EXISTS vulnerabilities (
    id                  TEXT PRIMARY KEY,
    driver_id           TEXT NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,

    -- Identity
    title               TEXT NOT NULL,
    vulnerability_class TEXT NOT NULL,

    -- Severity
    severity            TEXT NOT NULL,
    cvss_score          REAL,
    cvss_vector         TEXT,

    -- CVE
    cve_id              TEXT,
    cve_url             TEXT,

    -- Affected components
    affected_ioctl_id   TEXT REFERENCES ioctls(id),
    affected_function_id TEXT REFERENCES functions(id),
    affected_export_id  TEXT REFERENCES exports(id),

    -- Description
    description         TEXT NOT NULL,
    technical_details   TEXT,

    -- Exploitation
    exploitation_difficulty TEXT,
    exploitation_requirements TEXT,
    exploitation_steps  TEXT, -- JSON

    -- PoC
    poc_code            TEXT,
    poc_language        TEXT,

    -- Mitigations
    mitigations         TEXT,

    -- References
    vuln_references     TEXT, -- JSON

    -- Status
    status              TEXT DEFAULT 'confirmed',

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP,
    updated_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_vulns_driver ON vulnerabilities(driver_id);
CREATE INDEX IF NOT EXISTS idx_vulns_class ON vulnerabilities(vulnerability_class);
CREATE INDEX IF NOT EXISTS idx_vulns_severity ON vulnerabilities(severity);
CREATE INDEX IF NOT EXISTS idx_vulns_cve ON vulnerabilities(cve_id);

-- ============================================
-- ATTACK CHAINS
-- ============================================
CREATE TABLE IF NOT EXISTS attack_chains (
    id                  TEXT PRIMARY KEY,
    driver_id           TEXT NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,

    name                TEXT NOT NULL,
    description         TEXT,

    -- Goal
    attack_goal         TEXT,

    -- Steps (ordered)
    steps               TEXT NOT NULL, -- JSON

    -- Requirements
    initial_access      TEXT,

    -- Impact
    final_privilege     TEXT,

    -- PoC
    poc_code            TEXT,

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_attack_chains_driver ON attack_chains(driver_id);
CREATE INDEX IF NOT EXISTS idx_attack_chains_goal ON attack_chains(attack_goal);

-- ============================================
-- API CATEGORIES (Reference Table)
-- ============================================
CREATE TABLE IF NOT EXISTS api_categories (
    id                  TEXT PRIMARY KEY,

    category            TEXT NOT NULL UNIQUE,
    subcategory         TEXT,

    description         TEXT,
    security_relevance  TEXT,

    -- Common patterns
    common_misuse       TEXT,
    security_notes      TEXT,

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

-- ============================================
-- ANALYSIS SESSIONS
-- ============================================
CREATE TABLE IF NOT EXISTS analysis_sessions (
    id                  TEXT PRIMARY KEY,
    driver_id           TEXT NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,

    name                TEXT,

    -- Ghidra project info
    ghidra_project_path TEXT,
    ghidra_project_name TEXT,

    -- Session state
    status              TEXT DEFAULT 'active',

    -- Notes
    notes               TEXT,

    started_at          TEXT DEFAULT CURRENT_TIMESTAMP,
    ended_at            TEXT,

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_sessions_driver ON analysis_sessions(driver_id);

-- ============================================
-- ANALYSIS NOTES
-- ============================================
CREATE TABLE IF NOT EXISTS analysis_notes (
    id                  TEXT PRIMARY KEY,
    driver_id           TEXT NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,
    session_id          TEXT REFERENCES analysis_sessions(id),

    -- What this note is about
    related_function_id TEXT REFERENCES functions(id),
    related_ioctl_id    TEXT REFERENCES ioctls(id),
    related_vuln_id     TEXT REFERENCES vulnerabilities(id),

    -- Address if applicable
    rva                 INTEGER,

    -- Note content
    title               TEXT,
    content             TEXT NOT NULL,

    -- Priority/Type
    note_type           TEXT,
    priority            TEXT,

    created_at          TEXT DEFAULT CURRENT_TIMESTAMP,
    updated_at          TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_notes_driver ON analysis_notes(driver_id);
CREATE INDEX IF NOT EXISTS idx_notes_function ON analysis_notes(related_function_id);
CREATE INDEX IF NOT EXISTS idx_notes_ioctl ON analysis_notes(related_ioctl_id);

-- ============================================
-- TRIGGERS FOR UPDATED_AT
-- ============================================
CREATE TRIGGER IF NOT EXISTS update_drivers_timestamp
AFTER UPDATE ON drivers
BEGIN
    UPDATE drivers SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS update_functions_timestamp
AFTER UPDATE ON functions
BEGIN
    UPDATE functions SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS update_vulnerabilities_timestamp
AFTER UPDATE ON vulnerabilities
BEGIN
    UPDATE vulnerabilities SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER IF NOT EXISTS update_notes_timestamp
AFTER UPDATE ON analysis_notes
BEGIN
    UPDATE analysis_notes SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

-- ============================================
-- SYNC FTS INDEX FOR STRINGS
-- ============================================
CREATE TRIGGER IF NOT EXISTS strings_fts_insert AFTER INSERT ON strings BEGIN
    INSERT INTO strings_fts(rowid, value) VALUES (NEW.rowid, NEW.value);
END;

CREATE TRIGGER IF NOT EXISTS strings_fts_delete AFTER DELETE ON strings BEGIN
    DELETE FROM strings_fts WHERE rowid = OLD.rowid;
END;

CREATE TRIGGER IF NOT EXISTS strings_fts_update AFTER UPDATE ON strings BEGIN
    UPDATE strings_fts SET value = NEW.value WHERE rowid = NEW.rowid;
END;
