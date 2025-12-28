# DRIVER REVERSE ENGINEERING MCP SERVER + DATABASE ARCHITECTURE

## MISSION BRIEFING

You are building a **production-grade MCP (Model Context Protocol) server** that serves as the central nervous system for Windows kernel driver reverse engineering. This system must store, index, query, and semantically search ALL reverse engineering artifacts. It will integrate with Ghidra MCP server and serve as the single source of truth for driver analysis.

This is not a toy. This is infrastructure. Build it like your life depends on it.

---

## TECHNOLOGY STACK

```
  ╔══════════════════════════════════════════════════════════════════════════════╗
  ║                    PROJECT-OMBRA TECHNOLOGY STACK                            ║
  ╚══════════════════════════════════════════════════════════════════════════════╝

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ MCP SERVER LAYER (ombra_mcp_server/)                                         │
  ├─────────────────────────────────────────────────────────────────────────────┤
  │ Framework:       Python MCP SDK (mcp>=1.0.0) with stdio transport            │
  │ Python:          3.10+ (hatchling build system)                              │
  │ Validation:      Pydantic 2.0+ for data validation                           │
  │ File Watching:   watchdog>=4.0.0 for live codebase analysis                  │
  │ Server:          v2.1.0 with 100+ tools exposed                              │
  └─────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ DATABASE LAYER                                                               │
  ├─────────────────────────────────────────────────────────────────────────────┤
  │ Primary:         SQLite (multiple domain-specific databases)                 │
  │                  - intel_sdm.db      (VMCS fields, exit reasons, MSRs)       │
  │                  - project_brain.db  (decisions, gotchas, session context)   │
  │                  - anticheat_intel.db (detection methods, bypasses)          │
  │                  - evasion_techniques.db (bypass chains, cleanup procedures) │
  │                  - byovd_drivers.db  (vulnerable drivers, IOCTLs, magic vals)│
  │                  - vergilius.db      (Windows kernel structures, offsets)    │
  │                  - mslearn_reference.db (MS Learn API docs, concepts)        │
  │                  - concepts.db       (watcher daemon concept intelligence)   │
  │                                                                              │
  │ Vector Store:    ChromaDB (persistent client, HNSW with cosine similarity)   │
  │                  - 7 indexed collections for semantic search                 │
  │                  - Uses chroma.sqlite3 backend                               │
  └─────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ EMBEDDINGS / SEMANTIC SEARCH                                                 │
  ├─────────────────────────────────────────────────────────────────────────────┤
  │ Embedding Model: ChromaDB DefaultEmbeddingFunction                           │
  │                  (ONNX Runtime - no PyTorch dependency)                      │
  │                  Model: all-MiniLM-L6-v2 (384 dimensions)                    │
  │ Search:          HNSW approximate nearest neighbor + cosine distance         │
  │ Full-Text:       SQLite FTS5 for SDM content search                          │
  └─────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ HYPERVISOR CORE (hypervisor/)                                                │
  ├─────────────────────────────────────────────────────────────────────────────┤
  │ Language:        Pure C (kernel mode) + x64 MASM assembly                    │
  │ Target:          Windows x64 kernel driver (.sys)                            │
  │ Compiler:        MSVC cl.exe (Visual Studio 2019+)                           │
  │ Assembler:       ml64.exe (MASM x64)                                         │
  │ Build System:    Custom batch script (build.bat)                             │
  │                  - No CMake/Makefile (custom toolchain setup via vswhere)    │
  │ Output:          hypervisor.lib (static library)                             │
  │                                                                              │
  │ Assembly Files:                                                              │
  │   - vmexit.asm   (VM-exit handler entry/exit)                                │
  │   - intrinsics.asm (VMX intrinsics: vmread, vmwrite, vmxon, etc.)            │
  │   - segment.asm  (Segment descriptor helpers)                                │
  │                                                                              │
  │ Core Modules:    vmx.c, vmcs.c, ept.c, exit_dispatch.c, timing.c, hooks.c    │
  │ Exit Handlers:   cpuid, msr, rdtsc, cr_access, ept_violation, vmcall, etc.   │
  └─────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ DRIVER LAYER (hypervisor/driver/)                                            │
  ├─────────────────────────────────────────────────────────────────────────────┤
  │ Type:            Windows kernel driver (ring-0)                              │
  │ Features:        Command ring, ETW, memory ops, protection, shadow hooks     │
  │ BYOVD Support:   Ld9BoxSup.sys exploitation chain                            │
  │                  ThrottleStop auxiliary driver for flag patching             │
  └─────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ USERMODE COMPONENTS (hypervisor/usermode/)                                   │
  ├─────────────────────────────────────────────────────────────────────────────┤
  │ Loader:          loader.exe (C, compiled with MSVC)                          │
  │                  - PE parser, mapper, IAT resolution, relocations            │
  │                  - BYOVD deployer (supdrv.c, throttlestop.c)                 │
  │ GUI:             Dear ImGui (C++) with Win32/DirectX backends                │
  │                  - imgui_examples.sln for Visual Studio                      │
  │ Tests:           Detection baseline, BigPool visibility test                 │
  └─────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ EXTERNAL INTEGRATIONS                                                        │
  ├─────────────────────────────────────────────────────────────────────────────┤
  │ Ghidra MCP:      GhidraMCP bridge (Python → Ghidra HTTP API @ port 8080)     │
  │                  Tools: decompile, rename, xrefs, disassemble, list_*        │
  │ Claude-Mem:      Persistent memory plugin for cross-session context          │
  │                  Tools: search, timeline, get_observations, etc.             │
  └─────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ REFERENCE CODEBASES (docs/old_ombra_project/Antidetection-Research/)         │
  ├─────────────────────────────────────────────────────────────────────────────┤
  │ Build Systems:   CMake (hypervisor reference)                                │
  │                  Cargo.toml (Rust extensions)                                │
  │                  Visual Studio solutions (.sln/.vcxproj)                     │
  │                                                                              │
  │ Notable Refs:    HyperPlatform, hvpp, SimpleSvm, SimpleVisor, gbhv           │
  │                  Voyager (bootkit), NoirVisor, EfiGuard, UEFI-Bootkit        │
  │                  InfinityHook, Kernel-Bridge, DdiMon, ksm                    │
  └─────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ SCRAPING / DATA INGESTION                                                    │
  ├─────────────────────────────────────────────────────────────────────────────┤
  │ Tools:           Python scripts in ombra_mcp_server/scripts/                 │
  │                  - init_vergilius_db.py, scrape_vergilius.py                 │
  │                  - init_mslearn_db.py, ingest_mslearn.py                     │
  │                  - create_new_databases.py, migrate_gotchas.py               │
  │                                                                              │
  │ UC Scraper:      tools/uc_scraper/ (UnknownCheats intel)                     │
  │                  - uc_intel.db for community research aggregation            │
  └─────────────────────────────────────────────────────────────────────────────┘

  ---
  Summary:
  - Python MCP (100+ tools) backed by 7 SQLite databases + ChromaDB for semantic search
  - Hypervisor in pure C + x64 MASM, built with MSVC batch script
  - Usermode C loader with Dear ImGui GUI
  - ChromaDB default embedding (ONNX all-MiniLM-L6-v2) - no PyTorch needed
  - GhidraMCP bridge for RE integration


```

---

## DATABASE SCHEMA (COMPREHENSIVE)

### Core Tables

```sql
-- ============================================
-- DRIVER METADATA
-- ============================================
CREATE TABLE drivers (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    
    -- Identification
    original_name       VARCHAR(255) NOT NULL,
    analyzed_name       VARCHAR(255),  -- Name we're analyzing it as (e.g., Ld9BoxSup.sys)
    
    -- File Hashes
    md5                 CHAR(32) NOT NULL,
    sha1                CHAR(40) NOT NULL,
    sha256              CHAR(64) NOT NULL UNIQUE,
    imphash             CHAR(32),
    ssdeep              VARCHAR(255),
    
    -- PE Metadata
    file_size           BIGINT NOT NULL,
    image_base          BIGINT NOT NULL,
    entry_point_rva     BIGINT NOT NULL,
    entry_point_va      BIGINT GENERATED ALWAYS AS (image_base + entry_point_rva) STORED,
    size_of_image       BIGINT NOT NULL,
    timestamp           BIGINT,  -- PE timestamp
    machine             INTEGER NOT NULL,  -- 0x8664 = AMD64
    subsystem           INTEGER NOT NULL,  -- 1 = Native
    characteristics     INTEGER NOT NULL,
    
    -- Version Info
    file_version        VARCHAR(50),
    product_version     VARCHAR(50),
    company_name        VARCHAR(255),
    product_name        VARCHAR(255),
    file_description    VARCHAR(255),
    original_filename   VARCHAR(255),
    internal_name       VARCHAR(255),
    legal_copyright     VARCHAR(500),
    
    -- Build Info
    build_path          VARCHAR(500),  -- e.g., H:\vbox-6.1.36\src\VBox\...
    pdb_path            VARCHAR(500),
    linker_version      VARCHAR(50),
    
    -- Analysis State
    analysis_status     VARCHAR(50) DEFAULT 'pending',  -- pending, in_progress, complete
    analysis_started_at TIMESTAMPTZ,
    analysis_completed_at TIMESTAMPTZ,
    
    -- Metadata
    notes               TEXT,
    tags                TEXT[],
    created_at          TIMESTAMPTZ DEFAULT NOW(),
    updated_at          TIMESTAMPTZ DEFAULT NOW(),
    
    -- Embedding for semantic search on driver description/notes
    embedding           vector(1536)
);

CREATE INDEX idx_drivers_sha256 ON drivers(sha256);
CREATE INDEX idx_drivers_md5 ON drivers(md5);
CREATE INDEX idx_drivers_name ON drivers(original_name);
CREATE INDEX idx_drivers_tags ON drivers USING GIN(tags);
CREATE INDEX idx_drivers_embedding ON drivers USING ivfflat (embedding vector_cosine_ops);

-- ============================================
-- PE SECTIONS
-- ============================================
CREATE TABLE sections (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    driver_id           UUID NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,
    
    name                VARCHAR(16) NOT NULL,
    virtual_address     BIGINT NOT NULL,
    virtual_size        BIGINT NOT NULL,
    raw_address         BIGINT NOT NULL,
    raw_size            BIGINT NOT NULL,
    characteristics     INTEGER NOT NULL,
    entropy             FLOAT,
    
    -- Derived flags
    is_executable       BOOLEAN GENERATED ALWAYS AS ((characteristics & 0x20000000) != 0) STORED,
    is_writable         BOOLEAN GENERATED ALWAYS AS ((characteristics & 0x80000000) != 0) STORED,
    is_readable         BOOLEAN GENERATED ALWAYS AS ((characteristics & 0x40000000) != 0) STORED,
    
    created_at          TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_sections_driver ON sections(driver_id);

-- ============================================
-- IMPORTS (NT KERNEL APIs)
-- ============================================
CREATE TABLE imports (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    driver_id           UUID NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,
    
    dll_name            VARCHAR(255) NOT NULL,  -- e.g., ntoskrnl.exe
    function_name       VARCHAR(255),           -- NULL if by ordinal
    ordinal             INTEGER,                -- NULL if by name
    hint                INTEGER,
    
    -- IAT Info
    iat_rva             BIGINT,
    iat_va              BIGINT,
    
    -- Categorization
    category            VARCHAR(100),  -- memory_management, process_thread, io, sync, etc.
    subcategory         VARCHAR(100),  -- e.g., "physical_memory", "mdl", "spinlock"
    
    -- Security relevance
    is_dangerous        BOOLEAN DEFAULT FALSE,
    danger_reason       TEXT,
    
    -- Documentation
    description         TEXT,
    msdn_url            VARCHAR(500),
    
    -- Usage analysis (populated after RE)
    usage_count         INTEGER DEFAULT 0,
    usage_notes         TEXT,
    
    created_at          TIMESTAMPTZ DEFAULT NOW(),
    embedding           vector(1536)
);

CREATE INDEX idx_imports_driver ON imports(driver_id);
CREATE INDEX idx_imports_function ON imports(function_name);
CREATE INDEX idx_imports_dll ON imports(dll_name);
CREATE INDEX idx_imports_category ON imports(category);
CREATE INDEX idx_imports_dangerous ON imports(is_dangerous) WHERE is_dangerous = TRUE;
CREATE INDEX idx_imports_embedding ON imports USING ivfflat (embedding vector_cosine_ops);

-- ============================================
-- EXPORTS
-- ============================================
CREATE TABLE exports (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    driver_id           UUID NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,
    
    function_name       VARCHAR(255),
    ordinal             INTEGER NOT NULL,
    rva                 BIGINT NOT NULL,
    va                  BIGINT,
    
    -- Categorization (ASM*, RT*, SUPR0*, SUP*, etc.)
    prefix              VARCHAR(50),   -- ASM, RT, RTR0, RTMp, SUPR0, SUP, etc.
    category            VARCHAR(100),  -- atomic_ops, memory, thread, semaphore, etc.
    subcategory         VARCHAR(100),
    
    -- Analysis
    is_dangerous        BOOLEAN DEFAULT FALSE,
    danger_reason       TEXT,
    
    -- Function signature (if recovered)
    return_type         VARCHAR(100),
    parameters          JSONB,  -- [{name, type, description}, ...]
    calling_convention  VARCHAR(50),
    
    -- Documentation
    description         TEXT,
    
    -- Decompiled code
    decompiled_code     TEXT,
    pseudocode          TEXT,
    
    created_at          TIMESTAMPTZ DEFAULT NOW(),
    embedding           vector(1536)
);

CREATE INDEX idx_exports_driver ON exports(driver_id);
CREATE INDEX idx_exports_function ON exports(function_name);
CREATE INDEX idx_exports_prefix ON exports(prefix);
CREATE INDEX idx_exports_category ON exports(category);
CREATE INDEX idx_exports_dangerous ON exports(is_dangerous) WHERE is_dangerous = TRUE;
CREATE INDEX idx_exports_embedding ON exports USING ivfflat (embedding vector_cosine_ops);

-- ============================================
-- IOCTLS (CRITICAL)
-- ============================================
CREATE TABLE ioctls (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    driver_id           UUID NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,
    
    -- IOCTL Identity
    name                VARCHAR(255) NOT NULL,  -- e.g., SUP_IOCTL_MSR_PROBER
    code                BIGINT,                 -- Numeric IOCTL code if known
    code_hex            VARCHAR(20),            -- Hex representation
    
    -- CTL_CODE components (if recoverable)
    device_type         INTEGER,
    function_code       INTEGER,
    method              INTEGER,  -- METHOD_BUFFERED, METHOD_IN_DIRECT, etc.
    access              INTEGER,  -- FILE_READ_ACCESS, FILE_WRITE_ACCESS, etc.
    
    -- Handler Info
    handler_rva         BIGINT,
    handler_va          BIGINT,
    handler_function_id UUID REFERENCES functions(id),
    
    -- Input/Output
    input_struct_id     UUID REFERENCES structures(id),
    output_struct_id    UUID REFERENCES structures(id),
    min_input_size      INTEGER,
    max_input_size      INTEGER,
    min_output_size     INTEGER,
    max_output_size     INTEGER,
    
    -- Security Analysis
    requires_admin      BOOLEAN,
    requires_session    BOOLEAN DEFAULT TRUE,
    
    -- Vulnerability Assessment
    is_vulnerable       BOOLEAN DEFAULT FALSE,
    vulnerability_type  VARCHAR(100),  -- arb_read, arb_write, code_exec, etc.
    vulnerability_severity VARCHAR(20),  -- critical, high, medium, low
    vulnerability_description TEXT,
    exploitation_notes  TEXT,
    cve_ids             TEXT[],
    
    -- Documentation
    description         TEXT,
    purpose             TEXT,
    
    -- Fast path info
    has_fast_path       BOOLEAN DEFAULT FALSE,
    fast_handler_rva    BIGINT,
    
    created_at          TIMESTAMPTZ DEFAULT NOW(),
    embedding           vector(1536)
);

CREATE INDEX idx_ioctls_driver ON ioctls(driver_id);
CREATE INDEX idx_ioctls_name ON ioctls(name);
CREATE INDEX idx_ioctls_code ON ioctls(code);
CREATE INDEX idx_ioctls_vulnerable ON ioctls(is_vulnerable) WHERE is_vulnerable = TRUE;
CREATE INDEX idx_ioctls_embedding ON ioctls USING ivfflat (embedding vector_cosine_ops);

-- ============================================
-- DATA STRUCTURES
-- ============================================
CREATE TABLE structures (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    driver_id           UUID REFERENCES drivers(id) ON DELETE CASCADE,  -- NULL for common/shared structs
    
    name                VARCHAR(255) NOT NULL,
    size                INTEGER,
    alignment           INTEGER,
    
    -- Type
    struct_type         VARCHAR(50),  -- ioctl_input, ioctl_output, session, internal, windows
    
    -- Definition
    definition_c        TEXT,  -- C struct definition
    definition_json     JSONB, -- Structured definition
    
    -- Source
    source              VARCHAR(100),  -- recovered, symbols, manual, windows_sdk
    
    description         TEXT,
    
    created_at          TIMESTAMPTZ DEFAULT NOW(),
    embedding           vector(1536)
);

CREATE INDEX idx_structures_driver ON structures(driver_id);
CREATE INDEX idx_structures_name ON structures(name);
CREATE INDEX idx_structures_type ON structures(struct_type);

-- ============================================
-- STRUCTURE MEMBERS
-- ============================================
CREATE TABLE structure_members (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    structure_id        UUID NOT NULL REFERENCES structures(id) ON DELETE CASCADE,
    
    name                VARCHAR(255) NOT NULL,
    offset              INTEGER NOT NULL,
    size                INTEGER NOT NULL,
    type_name           VARCHAR(255),
    
    -- Nested structure reference
    nested_struct_id    UUID REFERENCES structures(id),
    
    -- Array info
    is_array            BOOLEAN DEFAULT FALSE,
    array_count         INTEGER,
    
    -- Pointer info
    is_pointer          BOOLEAN DEFAULT FALSE,
    pointer_depth       INTEGER DEFAULT 0,
    
    -- Bitfield info
    is_bitfield         BOOLEAN DEFAULT FALSE,
    bit_offset          INTEGER,
    bit_size            INTEGER,
    
    description         TEXT,
    
    created_at          TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_struct_members_struct ON structure_members(structure_id);

-- ============================================
-- FUNCTIONS (ANALYZED)
-- ============================================
CREATE TABLE functions (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    driver_id           UUID NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,
    
    -- Identity
    name                VARCHAR(255),
    rva                 BIGINT NOT NULL,
    va                  BIGINT,
    size                INTEGER,
    
    -- Source
    source              VARCHAR(50),  -- export, symbol, analysis, ghidra
    
    -- Signature
    return_type         VARCHAR(100),
    parameters          JSONB,
    calling_convention  VARCHAR(50) DEFAULT 'fastcall',
    
    -- Analysis
    is_entry_point      BOOLEAN DEFAULT FALSE,
    is_dispatch         BOOLEAN DEFAULT FALSE,
    dispatch_type       VARCHAR(50),  -- ioctl, irp_mj_create, etc.
    
    -- Code
    assembly            TEXT,
    decompiled          TEXT,
    pseudocode          TEXT,
    
    -- Annotations
    annotations         JSONB,
    
    -- Complexity metrics
    cyclomatic_complexity INTEGER,
    basic_block_count   INTEGER,
    instruction_count   INTEGER,
    
    -- Ghidra sync
    ghidra_synced       BOOLEAN DEFAULT FALSE,
    ghidra_sync_time    TIMESTAMPTZ,
    
    created_at          TIMESTAMPTZ DEFAULT NOW(),
    updated_at          TIMESTAMPTZ DEFAULT NOW(),
    embedding           vector(1536)
);

CREATE INDEX idx_functions_driver ON functions(driver_id);
CREATE INDEX idx_functions_name ON functions(name);
CREATE INDEX idx_functions_rva ON functions(rva);
CREATE INDEX idx_functions_dispatch ON functions(is_dispatch) WHERE is_dispatch = TRUE;
CREATE INDEX idx_functions_embedding ON functions USING ivfflat (embedding vector_cosine_ops);

-- ============================================
-- CROSS-REFERENCES
-- ============================================
CREATE TABLE xrefs (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    driver_id           UUID NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,
    
    -- From
    from_rva            BIGINT NOT NULL,
    from_va             BIGINT,
    from_function_id    UUID REFERENCES functions(id),
    
    -- To
    to_rva              BIGINT NOT NULL,
    to_va               BIGINT,
    to_function_id      UUID REFERENCES functions(id),
    to_import_id        UUID REFERENCES imports(id),
    to_export_id        UUID REFERENCES exports(id),
    
    -- Type
    xref_type           VARCHAR(50) NOT NULL,  -- call, jump, data_ref, offset
    
    -- Context
    instruction         VARCHAR(255),
    
    created_at          TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_xrefs_driver ON xrefs(driver_id);
CREATE INDEX idx_xrefs_from_func ON xrefs(from_function_id);
CREATE INDEX idx_xrefs_to_func ON xrefs(to_function_id);
CREATE INDEX idx_xrefs_to_import ON xrefs(to_import_id);
CREATE INDEX idx_xrefs_from_rva ON xrefs(from_rva);
CREATE INDEX idx_xrefs_to_rva ON xrefs(to_rva);

-- ============================================
-- GLOBAL VARIABLES
-- ============================================
CREATE TABLE globals (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    driver_id           UUID NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,
    
    name                VARCHAR(255),
    rva                 BIGINT NOT NULL,
    va                  BIGINT,
    size                INTEGER,
    
    type_name           VARCHAR(255),
    structure_id        UUID REFERENCES structures(id),
    
    -- Initial value if known
    initial_value       BYTEA,
    initial_value_hex   TEXT,
    
    -- Section
    section_name        VARCHAR(16),
    
    -- Security relevance
    is_sensitive        BOOLEAN DEFAULT FALSE,
    sensitivity_reason  TEXT,
    
    description         TEXT,
    
    created_at          TIMESTAMPTZ DEFAULT NOW(),
    embedding           vector(1536)
);

CREATE INDEX idx_globals_driver ON globals(driver_id);
CREATE INDEX idx_globals_name ON globals(name);
CREATE INDEX idx_globals_rva ON globals(rva);

-- ============================================
-- STRINGS
-- ============================================
CREATE TABLE strings (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    driver_id           UUID NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,
    
    value               TEXT NOT NULL,
    rva                 BIGINT NOT NULL,
    va                  BIGINT,
    length              INTEGER NOT NULL,
    
    encoding            VARCHAR(20),  -- ascii, unicode, utf8
    section_name        VARCHAR(16),
    
    -- Categorization
    category            VARCHAR(100),  -- error_message, debug, path, registry, device_name, etc.
    
    -- References
    reference_count     INTEGER DEFAULT 0,
    
    created_at          TIMESTAMPTZ DEFAULT NOW(),
    embedding           vector(1536)
);

CREATE INDEX idx_strings_driver ON strings(driver_id);
CREATE INDEX idx_strings_value ON strings USING GIN(to_tsvector('english', value));
CREATE INDEX idx_strings_category ON strings(category);
CREATE INDEX idx_strings_embedding ON strings USING ivfflat (embedding vector_cosine_ops);

-- ============================================
-- VULNERABILITIES
-- ============================================
CREATE TABLE vulnerabilities (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    driver_id           UUID NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,
    
    -- Identity
    title               VARCHAR(255) NOT NULL,
    vulnerability_class VARCHAR(100) NOT NULL,  -- arbitrary_read, arbitrary_write, code_exec, etc.
    
    -- Severity
    severity            VARCHAR(20) NOT NULL,  -- critical, high, medium, low, info
    cvss_score          FLOAT,
    cvss_vector         VARCHAR(100),
    
    -- CVE
    cve_id              VARCHAR(20),
    cve_url             VARCHAR(500),
    
    -- Affected components
    affected_ioctl_id   UUID REFERENCES ioctls(id),
    affected_function_id UUID REFERENCES functions(id),
    affected_export_id  UUID REFERENCES exports(id),
    
    -- Description
    description         TEXT NOT NULL,
    technical_details   TEXT,
    
    -- Exploitation
    exploitation_difficulty VARCHAR(50),  -- trivial, easy, moderate, hard, theoretical
    exploitation_requirements TEXT,
    exploitation_steps  JSONB,  -- [{step: 1, description: "..."}, ...]
    
    -- PoC
    poc_code            TEXT,
    poc_language        VARCHAR(50),
    
    -- Mitigations
    mitigations         TEXT,
    
    -- References
    references          JSONB,  -- [{title, url}, ...]
    
    -- Status
    status              VARCHAR(50) DEFAULT 'confirmed',  -- suspected, confirmed, exploited, patched
    
    created_at          TIMESTAMPTZ DEFAULT NOW(),
    updated_at          TIMESTAMPTZ DEFAULT NOW(),
    embedding           vector(1536)
);

CREATE INDEX idx_vulns_driver ON vulnerabilities(driver_id);
CREATE INDEX idx_vulns_class ON vulnerabilities(vulnerability_class);
CREATE INDEX idx_vulns_severity ON vulnerabilities(severity);
CREATE INDEX idx_vulns_cve ON vulnerabilities(cve_id);
CREATE INDEX idx_vulns_embedding ON vulnerabilities USING ivfflat (embedding vector_cosine_ops);

-- ============================================
-- ATTACK CHAINS
-- ============================================
CREATE TABLE attack_chains (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    driver_id           UUID NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,
    
    name                VARCHAR(255) NOT NULL,
    description         TEXT,
    
    -- Goal
    attack_goal         VARCHAR(100),  -- privilege_escalation, code_execution, info_leak, etc.
    
    -- Steps (ordered)
    steps               JSONB NOT NULL,  -- [{order, vuln_id, ioctl_id, description}, ...]
    
    -- Requirements
    initial_access      VARCHAR(100),  -- user, admin, system, any
    
    -- Impact
    final_privilege     VARCHAR(100),
    
    -- PoC
    poc_code            TEXT,
    
    created_at          TIMESTAMPTZ DEFAULT NOW(),
    embedding           vector(1536)
);

CREATE INDEX idx_attack_chains_driver ON attack_chains(driver_id);
CREATE INDEX idx_attack_chains_goal ON attack_chains(attack_goal);

-- ============================================
-- API CATEGORIES (Reference Table)
-- ============================================
CREATE TABLE api_categories (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    
    category            VARCHAR(100) NOT NULL UNIQUE,
    subcategory         VARCHAR(100),
    
    description         TEXT,
    security_relevance  VARCHAR(50),  -- critical, high, medium, low, none
    
    -- Common patterns
    common_misuse       TEXT,
    security_notes      TEXT,
    
    created_at          TIMESTAMPTZ DEFAULT NOW()
);

-- Seed API categories
INSERT INTO api_categories (category, subcategory, description, security_relevance) VALUES
-- Memory Management
('memory_management', 'physical_memory', 'Physical memory mapping and access', 'critical'),
('memory_management', 'virtual_memory', 'Virtual memory allocation and management', 'high'),
('memory_management', 'mdl', 'Memory Descriptor List operations', 'high'),
('memory_management', 'contiguous', 'Contiguous physical memory allocation', 'high'),
('memory_management', 'pool', 'Kernel pool allocation', 'medium'),
('memory_management', 'executable', 'Executable memory allocation', 'critical'),

-- Process/Thread
('process_thread', 'process', 'Process management APIs', 'high'),
('process_thread', 'thread', 'Thread creation and management', 'high'),
('process_thread', 'context', 'Thread context manipulation', 'critical'),

-- I/O
('io', 'device', 'Device object management', 'medium'),
('io', 'irp', 'IRP handling', 'medium'),
('io', 'file', 'File I/O operations', 'medium'),

-- Synchronization
('synchronization', 'spinlock', 'Spinlock operations', 'low'),
('synchronization', 'mutex', 'Mutex operations', 'low'),
('synchronization', 'event', 'Event operations', 'low'),
('synchronization', 'semaphore', 'Semaphore operations', 'low'),

-- System Information
('system_info', 'query', 'System information queries', 'medium'),
('system_info', 'set', 'System information modification', 'critical'),

-- Registry
('registry', 'query', 'Registry queries', 'low'),
('registry', 'modify', 'Registry modification', 'medium'),

-- Object Management
('object_management', 'reference', 'Object referencing', 'medium'),
('object_management', 'handle', 'Handle operations', 'high'),

-- Callbacks
('callbacks', 'register', 'Callback registration', 'medium'),
('callbacks', 'notify', 'System notifications', 'medium'),

-- CPU/Hardware
('cpu_hardware', 'msr', 'Model Specific Register access', 'critical'),
('cpu_hardware', 'cr', 'Control Register access', 'critical'),
('cpu_hardware', 'cpuid', 'CPU identification', 'low'),
('cpu_hardware', 'interrupt', 'Interrupt handling', 'high'),
('cpu_hardware', 'dpc', 'DPC operations', 'medium'),

-- Virtualization
('virtualization', 'vtx', 'Intel VT-x operations', 'critical'),
('virtualization', 'svm', 'AMD SVM operations', 'critical'),
('virtualization', 'hypervisor', 'Hypervisor interaction', 'critical');

-- ============================================
-- ANALYSIS SESSIONS
-- ============================================
CREATE TABLE analysis_sessions (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    driver_id           UUID NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,
    
    name                VARCHAR(255),
    
    -- Ghidra project info
    ghidra_project_path VARCHAR(500),
    ghidra_project_name VARCHAR(255),
    
    -- Session state
    status              VARCHAR(50) DEFAULT 'active',
    
    -- Notes
    notes               TEXT,
    
    started_at          TIMESTAMPTZ DEFAULT NOW(),
    ended_at            TIMESTAMPTZ,
    
    created_at          TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_sessions_driver ON analysis_sessions(driver_id);

-- ============================================
-- ANALYSIS NOTES
-- ============================================
CREATE TABLE analysis_notes (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    driver_id           UUID NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,
    session_id          UUID REFERENCES analysis_sessions(id),
    
    -- What this note is about
    related_function_id UUID REFERENCES functions(id),
    related_ioctl_id    UUID REFERENCES ioctls(id),
    related_vuln_id     UUID REFERENCES vulnerabilities(id),
    
    -- Address if applicable
    rva                 BIGINT,
    va                  BIGINT,
    
    -- Note content
    title               VARCHAR(255),
    content             TEXT NOT NULL,
    
    -- Priority/Type
    note_type           VARCHAR(50),  -- observation, todo, question, finding
    priority            VARCHAR(20),  -- high, medium, low
    
    created_at          TIMESTAMPTZ DEFAULT NOW(),
    updated_at          TIMESTAMPTZ DEFAULT NOW(),
    embedding           vector(1536)
);

CREATE INDEX idx_notes_driver ON analysis_notes(driver_id);
CREATE INDEX idx_notes_function ON analysis_notes(related_function_id);
CREATE INDEX idx_notes_ioctl ON analysis_notes(related_ioctl_id);
CREATE INDEX idx_notes_embedding ON analysis_notes USING ivfflat (embedding vector_cosine_ops);

-- ============================================
-- FULL TEXT SEARCH CONFIGURATION
-- ============================================
CREATE TEXT SEARCH CONFIGURATION driver_search (COPY = english);

-- Create unified search view
CREATE MATERIALIZED VIEW search_index AS
SELECT 
    'driver' as entity_type,
    d.id as entity_id,
    d.driver_id,
    d.original_name || ' ' || COALESCE(d.file_description, '') || ' ' || COALESCE(d.notes, '') as search_text,
    d.embedding
FROM drivers d
UNION ALL
SELECT 
    'ioctl' as entity_type,
    i.id as entity_id,
    i.driver_id,
    i.name || ' ' || COALESCE(i.description, '') || ' ' || COALESCE(i.vulnerability_description, '') as search_text,
    i.embedding
FROM ioctls i
UNION ALL
SELECT 
    'function' as entity_type,
    f.id as entity_id,
    f.driver_id,
    COALESCE(f.name, '') || ' ' || COALESCE(f.decompiled, '') as search_text,
    f.embedding
FROM functions f
UNION ALL
SELECT 
    'vulnerability' as entity_type,
    v.id as entity_id,
    v.driver_id,
    v.title || ' ' || v.description || ' ' || COALESCE(v.technical_details, '') as search_text,
    v.embedding
FROM vulnerabilities v
UNION ALL
SELECT 
    'string' as entity_type,
    s.id as entity_id,
    s.driver_id,
    s.value as search_text,
    s.embedding
FROM strings s;

CREATE INDEX idx_search_text ON search_index USING GIN(to_tsvector('english', search_text));
CREATE INDEX idx_search_embedding ON search_index USING ivfflat (embedding vector_cosine_ops);
CREATE INDEX idx_search_driver ON search_index(driver_id);
CREATE INDEX idx_search_type ON search_index(entity_type);
```

---

## MCP SERVER ARCHITECTURE

### Directory Structure

```
driver-re-mcp/
├── pyproject.toml
├── README.md
├── src/
│   └── driver_re_mcp/
│       ├── __init__.py
│       ├── server.py              # Main MCP server
│       ├── config.py              # Configuration
│       ├── database/
│       │   ├── __init__.py
│       │   ├── connection.py      # Database connection management
│       │   ├── models.py          # SQLAlchemy models
│       │   └── migrations/        # Alembic migrations
│       ├── embeddings/
│       │   ├── __init__.py
│       │   ├── provider.py        # Embedding provider interface
│       │   ├── openai.py          # OpenAI embeddings
│       │   └── local.py           # Local sentence-transformers
│       ├── tools/
│       │   ├── __init__.py
│       │   ├── driver_tools.py    # Driver management tools
│       │   ├── ioctl_tools.py     # IOCTL management tools
│       │   ├── import_tools.py    # Import analysis tools
│       │   ├── export_tools.py    # Export analysis tools
│       │   ├── function_tools.py  # Function analysis tools
│       │   ├── vuln_tools.py      # Vulnerability management
│       │   ├── search_tools.py    # Search tools (semantic + FTS)
│       │   ├── xref_tools.py      # Cross-reference tools
│       │   ├── struct_tools.py    # Structure management
│       │   ├── ghidra_tools.py    # Ghidra integration tools
│       │   └── analysis_tools.py  # Analysis session management
│       ├── ghidra/
│       │   ├── __init__.py
│       │   ├── bridge.py          # Ghidra bridge connection
│       │   ├── sync.py            # Sync functions/annotations
│       │   └── export.py          # Export from Ghidra
│       ├── parsers/
│       │   ├── __init__.py
│       │   ├── pe_parser.py       # PE file parsing
│       │   ├── import_parser.py   # Import table parsing
│       │   └── string_parser.py   # String extraction
│       └── utils/
│           ├── __init__.py
│           ├── hashing.py         # File hashing utilities
│           └── address.py         # Address conversion utilities
├── tests/
│   └── ...
└── scripts/
    ├── init_db.py                 # Database initialization
    ├── seed_categories.py         # Seed API categories
    └── import_driver.py           # Bulk import script
```

---

## MCP TOOLS SPECIFICATION

### DRIVER MANAGEMENT TOOLS

```python
# ============================================
# TOOL: add_driver
# ============================================
@tool
async def add_driver(
    file_path: str,
    analyzed_name: Optional[str] = None,
    tags: Optional[List[str]] = None,
    notes: Optional[str] = None
) -> Dict:
    """
    Add a new driver to the database.
    
    Automatically extracts:
    - File hashes (MD5, SHA1, SHA256)
    - PE metadata (sections, image base, entry point, etc.)
    - Version info
    - Imports
    - Exports
    - Strings
    
    Args:
        file_path: Path to the .sys file
        analyzed_name: Name to use for analysis (defaults to filename)
        tags: Optional tags for categorization
        notes: Optional notes about the driver
    
    Returns:
        Driver ID and extraction summary
    """

# ============================================
# TOOL: get_driver
# ============================================
@tool
async def get_driver(
    driver_id: Optional[str] = None,
    sha256: Optional[str] = None,
    name: Optional[str] = None
) -> Dict:
    """
    Get driver details by ID, hash, or name.
    
    Returns complete driver metadata including:
    - File info and hashes
    - PE metadata
    - Version info
    - Analysis status
    - Statistics (import count, export count, IOCTL count, vuln count)
    """

# ============================================
# TOOL: list_drivers
# ============================================
@tool
async def list_drivers(
    tags: Optional[List[str]] = None,
    status: Optional[str] = None,
    limit: int = 50,
    offset: int = 0
) -> List[Dict]:
    """
    List all drivers with optional filtering.
    """

# ============================================
# TOOL: update_driver_status
# ============================================
@tool
async def update_driver_status(
    driver_id: str,
    status: str,  # pending, in_progress, complete
    notes: Optional[str] = None
) -> Dict:
    """
    Update driver analysis status.
    """

# ============================================
# TOOL: delete_driver
# ============================================
@tool
async def delete_driver(
    driver_id: str,
    confirm: bool = False
) -> Dict:
    """
    Delete a driver and all associated data.
    Requires confirmation.
    """
```

### IOCTL TOOLS

```python
# ============================================
# TOOL: add_ioctl
# ============================================
@tool
async def add_ioctl(
    driver_id: str,
    name: str,
    code: Optional[int] = None,
    description: Optional[str] = None,
    handler_rva: Optional[int] = None,
    input_struct: Optional[str] = None,  # C struct definition or struct name
    output_struct: Optional[str] = None,
    min_input_size: Optional[int] = None,
    max_input_size: Optional[int] = None,
    min_output_size: Optional[int] = None,
    max_output_size: Optional[int] = None,
    requires_admin: bool = False,
    is_vulnerable: bool = False,
    vulnerability_type: Optional[str] = None,
    vulnerability_severity: Optional[str] = None,
    vulnerability_description: Optional[str] = None,
    exploitation_notes: Optional[str] = None
) -> Dict:
    """
    Add an IOCTL to a driver.
    
    Automatically:
    - Parses IOCTL code into components (device_type, function, method, access)
    - Generates embedding for semantic search
    - Links to handler function if RVA provided
    """

# ============================================
# TOOL: get_ioctl
# ============================================
@tool
async def get_ioctl(
    ioctl_id: Optional[str] = None,
    driver_id: Optional[str] = None,
    name: Optional[str] = None,
    code: Optional[int] = None
) -> Dict:
    """
    Get IOCTL details including:
    - IOCTL code breakdown
    - Input/output structures
    - Handler function info
    - Vulnerability status
    - Cross-references to handler
    """

# ============================================
# TOOL: list_ioctls
# ============================================
@tool
async def list_ioctls(
    driver_id: str,
    vulnerable_only: bool = False,
    category: Optional[str] = None
) -> List[Dict]:
    """
    List all IOCTLs for a driver.
    """

# ============================================
# TOOL: get_vulnerable_ioctls
# ============================================
@tool
async def get_vulnerable_ioctls(
    driver_id: Optional[str] = None,
    severity: Optional[str] = None,  # critical, high, medium, low
    vulnerability_type: Optional[str] = None  # arb_read, arb_write, code_exec, etc.
) -> List[Dict]:
    """
    Get all vulnerable IOCTLs, optionally filtered by driver/severity/type.
    """

# ============================================
# TOOL: update_ioctl_vulnerability
# ============================================
@tool
async def update_ioctl_vulnerability(
    ioctl_id: str,
    is_vulnerable: bool,
    vulnerability_type: Optional[str] = None,
    vulnerability_severity: Optional[str] = None,
    vulnerability_description: Optional[str] = None,
    exploitation_notes: Optional[str] = None,
    cve_ids: Optional[List[str]] = None
) -> Dict:
    """
    Update IOCTL vulnerability status and details.
    """
```

### IMPORT/EXPORT TOOLS

```python
# ============================================
# TOOL: get_imports
# ============================================
@tool
async def get_imports(
    driver_id: str,
    dll: Optional[str] = None,
    category: Optional[str] = None,
    dangerous_only: bool = False
) -> List[Dict]:
    """
    Get imports for a driver with filtering.
    
    Categories:
    - memory_management
    - process_thread
    - io
    - synchronization
    - system_info
    - registry
    - object_management
    - callbacks
    - cpu_hardware
    """

# ============================================
# TOOL: get_import_xrefs
# ============================================
@tool
async def get_import_xrefs(
    import_id: str
) -> List[Dict]:
    """
    Get all cross-references to an import.
    Returns list of calling functions with addresses.
    """

# ============================================
# TOOL: categorize_import
# ============================================
@tool
async def categorize_import(
    import_id: str,
    category: str,
    subcategory: Optional[str] = None,
    is_dangerous: bool = False,
    danger_reason: Optional[str] = None,
    usage_notes: Optional[str] = None
) -> Dict:
    """
    Categorize an import and mark security relevance.
    """

# ============================================
# TOOL: get_exports
# ============================================
@tool
async def get_exports(
    driver_id: str,
    prefix: Optional[str] = None,  # ASM, RT, RTR0, SUPR0, SUP, etc.
    category: Optional[str] = None,
    dangerous_only: bool = False
) -> List[Dict]:
    """
    Get exports for a driver with filtering.
    """

# ============================================
# TOOL: document_export
# ============================================
@tool
async def document_export(
    export_id: str,
    description: Optional[str] = None,
    return_type: Optional[str] = None,
    parameters: Optional[List[Dict]] = None,  # [{name, type, description}, ...]
    calling_convention: Optional[str] = None,
    is_dangerous: bool = False,
    danger_reason: Optional[str] = None,
    decompiled_code: Optional[str] = None
) -> Dict:
    """
    Document an export function.
    """

# ============================================
# TOOL: find_dangerous_apis
# ============================================
@tool
async def find_dangerous_apis(
    driver_id: str
) -> Dict:
    """
    Find all dangerous APIs (imports and exports) for a driver.
    
    Returns grouped by danger category:
    - physical_memory_access
    - arbitrary_read_write
    - code_execution
    - msr_access
    - cr_access
    - etc.
    """
```

### FUNCTION TOOLS

```python
# ============================================
# TOOL: add_function
# ============================================
@tool
async def add_function(
    driver_id: str,
    rva: int,
    name: Optional[str] = None,
    size: Optional[int] = None,
    return_type: Optional[str] = None,
    parameters: Optional[List[Dict]] = None,
    is_dispatch: bool = False,
    dispatch_type: Optional[str] = None,
    decompiled: Optional[str] = None,
    annotations: Optional[Dict] = None
) -> Dict:
    """
    Add or update a function.
    """

# ============================================
# TOOL: get_function
# ============================================
@tool
async def get_function(
    function_id: Optional[str] = None,
    driver_id: Optional[str] = None,
    rva: Optional[int] = None,
    va: Optional[int] = None,
    name: Optional[str] = None
) -> Dict:
    """
    Get function details including:
    - Signature
    - Decompiled code
    - Xrefs (callers and callees)
    - Related IOCTLs
    - Annotations
    """

# ============================================
# TOOL: get_function_callers
# ============================================
@tool
async def get_function_callers(
    function_id: str
) -> List[Dict]:
    """
    Get all functions that call this function.
    """

# ============================================
# TOOL: get_function_callees
# ============================================
@tool
async def get_function_callees(
    function_id: str
) -> List[Dict]:
    """
    Get all functions called by this function.
    """

# ============================================
# TOOL: trace_call_path
# ============================================
@tool
async def trace_call_path(
    driver_id: str,
    from_function: str,  # Name or RVA
    to_function: str     # Name or RVA
) -> List[List[Dict]]:
    """
    Find all call paths between two functions.
    Useful for tracing how user input reaches dangerous APIs.
    """

# ============================================
# TOOL: find_dispatch_handlers
# ============================================
@tool
async def find_dispatch_handlers(
    driver_id: str
) -> Dict:
    """
    Find all IRP dispatch handlers:
    - IRP_MJ_CREATE
    - IRP_MJ_CLOSE
    - IRP_MJ_DEVICE_CONTROL
    - IRP_MJ_INTERNAL_DEVICE_CONTROL
    - etc.
    """
```

### STRUCTURE TOOLS

```python
# ============================================
# TOOL: add_structure
# ============================================
@tool
async def add_structure(
    name: str,
    definition_c: str,  # C struct definition
    driver_id: Optional[str] = None,  # NULL for shared/common structs
    struct_type: Optional[str] = None,  # ioctl_input, ioctl_output, session, etc.
    size: Optional[int] = None,
    description: Optional[str] = None
) -> Dict:
    """
    Add a structure definition.
    Automatically parses C definition to extract members.
    """

# ============================================
# TOOL: get_structure
# ============================================
@tool
async def get_structure(
    structure_id: Optional[str] = None,
    name: Optional[str] = None,
    driver_id: Optional[str] = None
) -> Dict:
    """
    Get structure definition with all members.
    """

# ============================================
# TOOL: list_structures
# ============================================
@tool
async def list_structures(
    driver_id: Optional[str] = None,
    struct_type: Optional[str] = None
) -> List[Dict]:
    """
    List all structures.
    """

# ============================================
# TOOL: link_structure_to_ioctl
# ============================================
@tool
async def link_structure_to_ioctl(
    ioctl_id: str,
    input_struct_id: Optional[str] = None,
    output_struct_id: Optional[str] = None
) -> Dict:
    """
    Link structures to an IOCTL as input/output.
    """
```

### VULNERABILITY TOOLS

```python
# ============================================
# TOOL: add_vulnerability
# ============================================
@tool
async def add_vulnerability(
    driver_id: str,
    title: str,
    vulnerability_class: str,  # arbitrary_read, arbitrary_write, code_exec, etc.
    severity: str,  # critical, high, medium, low, info
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
    Add a vulnerability finding.
    """

# ============================================
# TOOL: get_vulnerability
# ============================================
@tool
async def get_vulnerability(
    vuln_id: str
) -> Dict:
    """
    Get full vulnerability details including:
    - Affected IOCTL details
    - Affected function details
    - Exploitation steps
    - PoC code
    - Related attack chains
    """

# ============================================
# TOOL: list_vulnerabilities
# ============================================
@tool
async def list_vulnerabilities(
    driver_id: Optional[str] = None,
    severity: Optional[str] = None,
    vulnerability_class: Optional[str] = None,
    status: Optional[str] = None
) -> List[Dict]:
    """
    List vulnerabilities with filtering.
    """

# ============================================
# TOOL: create_attack_chain
# ============================================
@tool
async def create_attack_chain(
    driver_id: str,
    name: str,
    attack_goal: str,  # privilege_escalation, code_execution, info_leak, etc.
    steps: List[Dict],  # [{order, vuln_id, ioctl_id, description}, ...]
    initial_access: str,  # user, admin, system, any
    final_privilege: str,
    description: Optional[str] = None,
    poc_code: Optional[str] = None
) -> Dict:
    """
    Create an attack chain combining multiple vulnerabilities/IOCTLs.
    """

# ============================================
# TOOL: get_attack_chains
# ============================================
@tool
async def get_attack_chains(
    driver_id: str,
    attack_goal: Optional[str] = None
) -> List[Dict]:
    """
    Get all attack chains for a driver.
    """
```

### SEARCH TOOLS (CRITICAL)

```python
# ============================================
# TOOL: semantic_search
# ============================================
@tool
async def semantic_search(
    query: str,
    driver_id: Optional[str] = None,
    entity_types: Optional[List[str]] = None,  # driver, ioctl, function, vulnerability, string
    limit: int = 20,
    threshold: float = 0.7
) -> List[Dict]:
    """
    Semantic search across all entities.
    
    Uses vector similarity to find relevant:
    - IOCTLs by description/purpose
    - Functions by behavior
    - Vulnerabilities by type/description
    - Strings by content
    
    Example queries:
    - "physical memory read write access"
    - "arbitrary kernel memory manipulation"
    - "MSR model specific register"
    - "privilege escalation code execution"
    """

# ============================================
# TOOL: text_search
# ============================================
@tool
async def text_search(
    query: str,
    driver_id: Optional[str] = None,
    entity_types: Optional[List[str]] = None,
    limit: int = 50
) -> List[Dict]:
    """
    Full-text search across all entities.
    Uses PostgreSQL tsquery for exact and fuzzy matching.
    """

# ============================================
# TOOL: search_strings
# ============================================
@tool
async def search_strings(
    driver_id: str,
    pattern: Optional[str] = None,  # Regex pattern
    contains: Optional[str] = None,  # Simple contains
    category: Optional[str] = None,
    limit: int = 100
) -> List[Dict]:
    """
    Search strings in a driver.
    """

# ============================================
# TOOL: find_similar_ioctls
# ============================================
@tool
async def find_similar_ioctls(
    ioctl_id: str,
    limit: int = 10
) -> List[Dict]:
    """
    Find IOCTLs similar to the given one across all drivers.
    Uses semantic similarity.
    """

# ============================================
# TOOL: find_similar_vulnerabilities
# ============================================
@tool
async def find_similar_vulnerabilities(
    vuln_id: str,
    limit: int = 10
) -> List[Dict]:
    """
    Find similar vulnerabilities across all drivers.
    """

# ============================================
# TOOL: search_by_api_usage
# ============================================
@tool
async def search_by_api_usage(
    api_name: str,  # e.g., "MmMapIoSpace", "ZwSetSystemInformation"
    driver_id: Optional[str] = None
) -> Dict:
    """
    Find all drivers/IOCTLs/functions that use a specific API.
    
    Returns:
    - Drivers using this API
    - Functions calling this API
    - IOCTLs whose handlers call this API
    - Vulnerabilities related to this API
    """
```

### GHIDRA INTEGRATION TOOLS

```python
# ============================================
# TOOL: ghidra_connect
# ============================================
@tool
async def ghidra_connect(
    host: str = "localhost",
    port: int = 4768
) -> Dict:
    """
    Connect to Ghidra Bridge or Ghidra MCP server.
    """

# ============================================
# TOOL: ghidra_sync_functions
# ============================================
@tool
async def ghidra_sync_functions(
    driver_id: str,
    direction: str = "from_ghidra"  # from_ghidra, to_ghidra, bidirectional
) -> Dict:
    """
    Sync function names, signatures, and decompiled code between 
    database and Ghidra.
    
    from_ghidra: Import Ghidra analysis into database
    to_ghidra: Export database annotations to Ghidra
    bidirectional: Merge both (newer wins)
    """

# ============================================
# TOOL: ghidra_sync_structures
# ============================================
@tool
async def ghidra_sync_structures(
    driver_id: str,
    direction: str = "from_ghidra"
) -> Dict:
    """
    Sync data types/structures between database and Ghidra.
    """

# ============================================
# TOOL: ghidra_get_decompilation
# ============================================
@tool
async def ghidra_get_decompilation(
    driver_id: str,
    rva: Optional[int] = None,
    function_name: Optional[str] = None
) -> Dict:
    """
    Get decompiled code for a function from Ghidra.
    """

# ============================================
# TOOL: ghidra_get_xrefs
# ============================================
@tool
async def ghidra_get_xrefs(
    driver_id: str,
    address: int
) -> Dict:
    """
    Get cross-references from Ghidra for an address.
    """

# ============================================
# TOOL: ghidra_set_comment
# ============================================
@tool
async def ghidra_set_comment(
    driver_id: str,
    address: int,
    comment: str,
    comment_type: str = "eol"  # eol, pre, post, plate
) -> Dict:
    """
    Set a comment in Ghidra at the specified address.
    """

# ============================================
# TOOL: ghidra_rename_function
# ============================================
@tool
async def ghidra_rename_function(
    driver_id: str,
    address: int,
    new_name: str
) -> Dict:
    """
    Rename a function in Ghidra.
    """

# ============================================
# TOOL: ghidra_export_all
# ============================================
@tool
async def ghidra_export_all(
    driver_id: str
) -> Dict:
    """
    Export all analysis from Ghidra:
    - Functions (names, signatures, decompiled code)
    - Data types/structures
    - Comments
    - Labels
    - Cross-references
    """
```

### CROSS-REFERENCE TOOLS

```python
# ============================================
# TOOL: add_xref
# ============================================
@tool
async def add_xref(
    driver_id: str,
    from_rva: int,
    to_rva: int,
    xref_type: str,  # call, jump, data_ref, offset
    instruction: Optional[str] = None
) -> Dict:
    """
    Add a cross-reference.
    """

# ============================================
# TOOL: get_xrefs_to
# ============================================
@tool
async def get_xrefs_to(
    driver_id: str,
    rva: Optional[int] = None,
    function_id: Optional[str] = None,
    import_id: Optional[str] = None
) -> List[Dict]:
    """
    Get all xrefs TO an address/function/import.
    """

# ============================================
# TOOL: get_xrefs_from
# ============================================
@tool
async def get_xrefs_from(
    driver_id: str,
    rva: Optional[int] = None,
    function_id: Optional[str] = None
) -> List[Dict]:
    """
    Get all xrefs FROM an address/function.
    """

# ============================================
# TOOL: build_call_graph
# ============================================
@tool
async def build_call_graph(
    driver_id: str,
    root_function: Optional[str] = None,  # Name or RVA, None for full graph
    depth: int = 5,
    direction: str = "callees"  # callees, callers, both
) -> Dict:
    """
    Build a call graph starting from a function.
    Returns nodes and edges for visualization.
    """

# ============================================
# TOOL: find_paths_to_api
# ============================================
@tool
async def find_paths_to_api(
    driver_id: str,
    api_name: str,  # Import name like "MmMapIoSpace"
    from_entry_points: bool = True,  # Start from driver entry/dispatch handlers
    max_depth: int = 10
) -> List[List[Dict]]:
    """
    Find all call paths that reach a specific API.
    Critical for vulnerability analysis - shows how user input
    can reach dangerous APIs.
    """
```

### ANALYSIS SESSION TOOLS

```python
# ============================================
# TOOL: start_analysis_session
# ============================================
@tool
async def start_analysis_session(
    driver_id: str,
    name: Optional[str] = None,
    ghidra_project_path: Optional[str] = None
) -> Dict:
    """
    Start a new analysis session for a driver.
    """

# ============================================
# TOOL: add_analysis_note
# ============================================
@tool
async def add_analysis_note(
    driver_id: str,
    content: str,
    title: Optional[str] = None,
    note_type: str = "observation",  # observation, todo, question, finding
    priority: str = "medium",  # high, medium, low
    related_function_id: Optional[str] = None,
    related_ioctl_id: Optional[str] = None,
    rva: Optional[int] = None
) -> Dict:
    """
    Add an analysis note.
    """

# ============================================
# TOOL: get_analysis_notes
# ============================================
@tool
async def get_analysis_notes(
    driver_id: str,
    note_type: Optional[str] = None,
    priority: Optional[str] = None
) -> List[Dict]:
    """
    Get analysis notes for a driver.
    """

# ============================================
# TOOL: generate_analysis_report
# ============================================
@tool
async def generate_analysis_report(
    driver_id: str,
    format: str = "markdown"  # markdown, html, json
) -> str:
    """
    Generate a comprehensive analysis report including:
    - Driver overview
    - All IOCTLs with vulnerability status
    - Dangerous APIs used
    - All vulnerabilities found
    - Attack chains
    - Analysis notes
    """
```

### UTILITY TOOLS

```python
# ============================================
# TOOL: convert_address
# ============================================
@tool
async def convert_address(
    driver_id: str,
    address: int,
    from_type: str,  # rva, va, file_offset
    to_type: str     # rva, va, file_offset
) -> int:
    """
    Convert between address types (RVA, VA, file offset).
    """

# ============================================
# TOOL: get_api_info
# ============================================
@tool
async def get_api_info(
    api_name: str
) -> Dict:
    """
    Get documentation and security info for a Windows kernel API.
    
    Returns:
    - Description
    - Parameters
    - Return value
    - Security relevance
    - Common misuse patterns
    - MSDN URL
    """

# ============================================
# TOOL: compare_drivers
# ============================================
@tool
async def compare_drivers(
    driver_id_1: str,
    driver_id_2: str
) -> Dict:
    """
    Compare two drivers:
    - Shared imports
    - Shared exports
    - Similar IOCTLs
    - Similar vulnerabilities
    - Code similarity (if available)
    """

# ============================================
# TOOL: get_statistics
# ============================================
@tool
async def get_statistics(
    driver_id: Optional[str] = None
) -> Dict:
    """
    Get statistics for a driver or all drivers:
    - Total imports/exports
    - IOCTL count
    - Vulnerability count by severity
    - API category breakdown
    - Analysis coverage
    """
```

---

## EMBEDDING GENERATION

### Embedding Strategy

```python
# What gets embedded and how:

EMBEDDING_CONFIGS = {
    "driver": {
        "fields": ["original_name", "file_description", "notes", "product_name"],
        "template": "Windows kernel driver: {original_name}. {file_description}. {notes}"
    },
    "ioctl": {
        "fields": ["name", "description", "vulnerability_description", "exploitation_notes"],
        "template": "IOCTL {name}: {description}. Vulnerability: {vulnerability_description}. {exploitation_notes}"
    },
    "function": {
        "fields": ["name", "decompiled", "annotations"],
        "template": "Function {name}: {decompiled}"
    },
    "import": {
        "fields": ["function_name", "description", "usage_notes", "danger_reason"],
        "template": "Import {function_name} from {dll_name}: {description}. {usage_notes}"
    },
    "export": {
        "fields": ["function_name", "description", "decompiled_code"],
        "template": "Export {function_name}: {description}"
    },
    "vulnerability": {
        "fields": ["title", "description", "technical_details", "exploitation_notes"],
        "template": "Vulnerability: {title}. {description}. Technical: {technical_details}"
    },
    "string": {
        "fields": ["value"],
        "template": "{value}"
    },
    "structure": {
        "fields": ["name", "definition_c", "description"],
        "template": "Structure {name}: {description}. {definition_c}"
    },
    "note": {
        "fields": ["title", "content"],
        "template": "{title}: {content}"
    }
}
```

---

## MCP SERVER MAIN IMPLEMENTATION

```python
# src/driver_re_mcp/server.py

import asyncio
from typing import Any
from mcp.server import Server
from mcp.server.stdio import stdio_server
from mcp.types import Tool, TextContent

from .database.connection import get_db_session
from .tools import (
    driver_tools,
    ioctl_tools,
    import_tools,
    export_tools,
    function_tools,
    vuln_tools,
    search_tools,
    xref_tools,
    struct_tools,
    ghidra_tools,
    analysis_tools
)

app = Server("driver-re-mcp")

# Register all tools
ALL_TOOLS = [
    # Driver management
    ("add_driver", driver_tools.add_driver),
    ("get_driver", driver_tools.get_driver),
    ("list_drivers", driver_tools.list_drivers),
    ("update_driver_status", driver_tools.update_driver_status),
    ("delete_driver", driver_tools.delete_driver),
    
    # IOCTL management
    ("add_ioctl", ioctl_tools.add_ioctl),
    ("get_ioctl", ioctl_tools.get_ioctl),
    ("list_ioctls", ioctl_tools.list_ioctls),
    ("get_vulnerable_ioctls", ioctl_tools.get_vulnerable_ioctls),
    ("update_ioctl_vulnerability", ioctl_tools.update_ioctl_vulnerability),
    
    # Import/Export
    ("get_imports", import_tools.get_imports),
    ("get_import_xrefs", import_tools.get_import_xrefs),
    ("categorize_import", import_tools.categorize_import),
    ("get_exports", export_tools.get_exports),
    ("document_export", export_tools.document_export),
    ("find_dangerous_apis", import_tools.find_dangerous_apis),
    
    # Functions
    ("add_function", function_tools.add_function),
    ("get_function", function_tools.get_function),
    ("get_function_callers", function_tools.get_function_callers),
    ("get_function_callees", function_tools.get_function_callees),
    ("trace_call_path", function_tools.trace_call_path),
    ("find_dispatch_handlers", function_tools.find_dispatch_handlers),
    
    # Structures
    ("add_structure", struct_tools.add_structure),
    ("get_structure", struct_tools.get_structure),
    ("list_structures", struct_tools.list_structures),
    ("link_structure_to_ioctl", struct_tools.link_structure_to_ioctl),
    
    # Vulnerabilities
    ("add_vulnerability", vuln_tools.add_vulnerability),
    ("get_vulnerability", vuln_tools.get_vulnerability),
    ("list_vulnerabilities", vuln_tools.list_vulnerabilities),
    ("create_attack_chain", vuln_tools.create_attack_chain),
    ("get_attack_chains", vuln_tools.get_attack_chains),
    
    # Search (THE GOOD STUFF)
    ("semantic_search", search_tools.semantic_search),
    ("text_search", search_tools.text_search),
    ("search_strings", search_tools.search_strings),
    ("find_similar_ioctls", search_tools.find_similar_ioctls),
    ("find_similar_vulnerabilities", search_tools.find_similar_vulnerabilities),
    ("search_by_api_usage", search_tools.search_by_api_usage),
    
    # Cross-references
    ("add_xref", xref_tools.add_xref),
    ("get_xrefs_to", xref_tools.get_xrefs_to),
    ("get_xrefs_from", xref_tools.get_xrefs_from),
    ("build_call_graph", xref_tools.build_call_graph),
    ("find_paths_to_api", xref_tools.find_paths_to_api),
    
    # Ghidra integration
    ("ghidra_connect", ghidra_tools.ghidra_connect),
    ("ghidra_sync_functions", ghidra_tools.ghidra_sync_functions),
    ("ghidra_sync_structures", ghidra_tools.ghidra_sync_structures),
    ("ghidra_get_decompilation", ghidra_tools.ghidra_get_decompilation),
    ("ghidra_get_xrefs", ghidra_tools.ghidra_get_xrefs),
    ("ghidra_set_comment", ghidra_tools.ghidra_set_comment),
    ("ghidra_rename_function", ghidra_tools.ghidra_rename_function),
    ("ghidra_export_all", ghidra_tools.ghidra_export_all),
    
    # Analysis session
    ("start_analysis_session", analysis_tools.start_analysis_session),
    ("add_analysis_note", analysis_tools.add_analysis_note),
    ("get_analysis_notes", analysis_tools.get_analysis_notes),
    ("generate_analysis_report", analysis_tools.generate_analysis_report),
    
    # Utilities
    ("convert_address", analysis_tools.convert_address),
    ("get_api_info", analysis_tools.get_api_info),
    ("compare_drivers", analysis_tools.compare_drivers),
    ("get_statistics", analysis_tools.get_statistics),
]

@app.list_tools()
async def list_tools() -> list[Tool]:
    """List all available tools."""
    tools = []
    for name, func in ALL_TOOLS:
        tools.append(Tool(
            name=name,
            description=func.__doc__,
            inputSchema=get_input_schema(func)
        ))
    return tools

@app.call_tool()
async def call_tool(name: str, arguments: dict) -> list[TextContent]:
    """Execute a tool by name."""
    tool_map = dict(ALL_TOOLS)
    if name not in tool_map:
        raise ValueError(f"Unknown tool: {name}")
    
    result = await tool_map[name](**arguments)
    return [TextContent(type="text", text=json.dumps(result, indent=2, default=str))]

async def main():
    async with stdio_server() as (read_stream, write_stream):
        await app.run(read_stream, write_stream, app.create_initialization_options())

if __name__ == "__main__":
    asyncio.run(main())
```

---

## CONFIGURATION

```python
# src/driver_re_mcp/config.py

from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    # Database
    DATABASE_URL: str = "postgresql+asyncpg://user:pass@localhost:5432/driver_re"
    
    # Or SQLite alternative:
    # DATABASE_URL: str = "sqlite+aiosqlite:///./driver_re.db"
    
    # Embeddings
    EMBEDDING_PROVIDER: str = "openai"  # openai, local
    OPENAI_API_KEY: str = ""
    EMBEDDING_MODEL: str = "text-embedding-3-small"
    EMBEDDING_DIMENSIONS: int = 1536
    
    # Local embeddings (if using sentence-transformers)
    LOCAL_EMBEDDING_MODEL: str = "all-MiniLM-L6-v2"
    
    # Ghidra
    GHIDRA_BRIDGE_HOST: str = "localhost"
    GHIDRA_BRIDGE_PORT: int = 4768
    
    # Search
    SEMANTIC_SEARCH_THRESHOLD: float = 0.7
    MAX_SEARCH_RESULTS: int = 50
    
    class Config:
        env_file = ".env"

settings = Settings()
```

---

## GHIDRA MCP SERVER INTEGRATION

When Ghidra MCP server is installed, add these connection methods:

```python
# src/driver_re_mcp/ghidra/bridge.py

import socket
import json
from typing import Optional, Dict, Any

class GhidraMCPClient:
    """Client for communicating with Ghidra MCP server."""
    
    def __init__(self, host: str = "localhost", port: int = 4768):
        self.host = host
        self.port = port
        self._connected = False
    
    async def connect(self) -> bool:
        """Establish connection to Ghidra MCP server."""
        # Implementation depends on Ghidra MCP server protocol
        pass
    
    async def call_tool(self, tool_name: str, arguments: Dict[str, Any]) -> Dict:
        """Call a tool on the Ghidra MCP server."""
        pass
    
    async def get_function_at(self, address: int) -> Optional[Dict]:
        """Get function at address."""
        return await self.call_tool("get_function_at", {"address": hex(address)})
    
    async def get_decompilation(self, address: int) -> Optional[str]:
        """Get decompiled code for function."""
        result = await self.call_tool("decompile_function", {"address": hex(address)})
        return result.get("decompiled") if result else None
    
    async def get_xrefs_to(self, address: int) -> list:
        """Get cross-references to address."""
        return await self.call_tool("get_xrefs_to", {"address": hex(address)})
    
    async def get_xrefs_from(self, address: int) -> list:
        """Get cross-references from address."""
        return await self.call_tool("get_xrefs_from", {"address": hex(address)})
    
    async def rename_function(self, address: int, name: str) -> bool:
        """Rename function in Ghidra."""
        result = await self.call_tool("rename_function", {
            "address": hex(address),
            "name": name
        })
        return result.get("success", False)
    
    async def set_comment(self, address: int, comment: str, comment_type: str = "eol") -> bool:
        """Set comment at address."""
        result = await self.call_tool("set_comment", {
            "address": hex(address),
            "comment": comment,
            "type": comment_type
        })
        return result.get("success", False)
    
    async def get_all_functions(self) -> list:
        """Get all functions from Ghidra."""
        return await self.call_tool("list_functions", {})
    
    async def get_data_types(self) -> list:
        """Get all data types/structures from Ghidra."""
        return await self.call_tool("list_data_types", {})
    
    async def create_struct(self, name: str, size: int, fields: list) -> bool:
        """Create a structure in Ghidra."""
        return await self.call_tool("create_struct", {
            "name": name,
            "size": size,
            "fields": fields
        })
```

---

## EXAMPLE USAGE SCENARIOS

### Scenario 1: Analyze a New Driver

```
User: Analyze the driver at /path/to/Ld9BoxSup.sys

Agent:
1. add_driver(file_path="/path/to/Ld9BoxSup.sys", tags=["vbox", "byovd"])
2. get_imports(driver_id=..., dangerous_only=True)
3. get_exports(driver_id=..., prefix="SUPR0")
4. find_dangerous_apis(driver_id=...)
5. semantic_search(query="physical memory access arbitrary read write", driver_id=...)
```

### Scenario 2: Document IOCTLs from RE Session

```
User: I found an IOCTL called SUP_IOCTL_MSR_PROBER that allows reading/writing MSRs

Agent:
1. add_ioctl(
    driver_id=...,
    name="SUP_IOCTL_MSR_PROBER",
    description="Read/write Model Specific Registers",
    is_vulnerable=True,
    vulnerability_type="msr_access",
    vulnerability_severity="critical",
    vulnerability_description="Allows arbitrary MSR read/write from usermode"
)
2. add_vulnerability(...)
```

### Scenario 3: Find Attack Path

```
User: How can user input reach MmMapIoSpace in this driver?

Agent:
1. find_paths_to_api(driver_id=..., api_name="MmMapIoSpace")
   → Returns paths from dispatch handlers to MmMapIoSpace
2. For each path, examine IOCTLs involved
3. Build attack chain showing exploitation steps
```

### Scenario 4: Cross-Driver Analysis

```
User: Find all drivers with similar vulnerabilities to the VBox arbitrary physical memory read

Agent:
1. get_vulnerability(vuln_id=...) → Get VBox vuln details
2. find_similar_vulnerabilities(vuln_id=...)
3. semantic_search(query="physical memory mapping arbitrary read kernel driver")
```

---

## DELIVERABLES

You MUST produce:

1. **Complete PostgreSQL schema** with all tables, indexes, and constraints
2. **Full MCP server implementation** with all tools listed above
3. **Embedding pipeline** for semantic search
4. **Ghidra integration layer** ready for Ghidra MCP server
5. **Database migration scripts** (Alembic)
6. **Configuration system** (pydantic-settings)
7. **Comprehensive tests** for all tools
8. **Documentation** for tool usage
9. **CLI scripts** for common operations (init_db, import_driver, etc.)

---

## NON-NEGOTIABLE REQUIREMENTS

- [ ] Every tool MUST be async
- [ ] Every database operation MUST use proper transactions
- [ ] Every embedding MUST be generated on insert/update
- [ ] Semantic search MUST support filtering by driver/entity type
- [ ] Ghidra sync MUST be bidirectional
- [ ] All addresses MUST support RVA/VA conversion
- [ ] Vulnerability severity MUST follow CVSS conventions
- [ ] Attack chains MUST link to specific IOCTLs and vulnerabilities
- [ ] Reports MUST be exportable to markdown/HTML
- [ ] Database MUST support full-text AND vector search

---

## THIS IS THE FOUNDATION OF YOUR RE WORKFLOW

Do not half-ass this. This MCP server will be the backbone of all driver analysis. Every IOCTL, every vulnerability, every API call, every xref - it all lives here. Make it bulletproof.

**NOW BUILD IT.**