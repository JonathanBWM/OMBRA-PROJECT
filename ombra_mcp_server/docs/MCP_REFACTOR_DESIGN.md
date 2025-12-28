# OmbraMCP Database Refactor Design

**Version:** 1.0
**Date:** December 26, 2025
**Author:** ENI

---

## Executive Summary

The current MCP server uses a monolithic approach with hardcoded Python dicts and two databases (`intel_sdm.db`, `project_brain.db`). As we ingest community intel from UC and other sources, we need a proper taxonomy that enables:

1. **Semantic organization** - Intel goes where it belongs
2. **Proper querying** - Ask questions in the right context
3. **Update capabilities** - Add new intel programmatically
4. **Cross-referencing** - Link related intel across categories

---

## Database Architecture

```
ombra_mcp/data/
├── intel_sdm.db          # Intel SDM reference (existing, keep as-is)
├── vergilius.db          # Windows kernel structures (existing, keep as-is)
├── project_brain.db      # Project state tracking (existing, refactor)
├── anticheat_intel.db    # NEW: Anti-cheat detection methods & bypasses
├── evasion_techniques.db # NEW: Bypass chains, hiding methods
├── byovd_drivers.db      # NEW: Vulnerable driver arsenal
└── uc_intel.db           # Community intel (at UC-SCRAPER, link to it)
```

---

## 1. anticheat_intel.db Schema

Stores detection methods, timing thresholds, bypasses, and signatures per anti-cheat.

```sql
-- Anti-cheat products
CREATE TABLE anticheats (
    id INTEGER PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,           -- "EAC", "BattlEye", "Vanguard"
    full_name TEXT,                       -- "Easy Anti-Cheat"
    vendor TEXT,                          -- "Epic Games"
    kernel_driver TEXT,                   -- "EasyAntiCheat.sys"
    ring0 BOOLEAN DEFAULT TRUE,
    last_updated TEXT,                    -- ISO timestamp
    notes TEXT
);

-- Detection methods used by anti-cheats
CREATE TABLE detection_methods (
    id INTEGER PRIMARY KEY,
    anticheat_id INTEGER REFERENCES anticheats(id),
    method_id TEXT NOT NULL,              -- "EAC-001", "BE-003"
    category TEXT NOT NULL,               -- "timing", "cpuid", "msr", "memory", "behavioral"
    name TEXT NOT NULL,                   -- "PML4E Kernel Scan"
    description TEXT NOT NULL,
    technique TEXT,                       -- How they detect
    threshold_value REAL,                 -- e.g., 500
    threshold_unit TEXT,                  -- "cycles", "ms", "bytes"
    check_frequency TEXT,                 -- "continuous", "periodic", "on_event"
    severity TEXT DEFAULT 'high',         -- "low", "medium", "high", "critical"
    first_seen TEXT,                      -- When we discovered this
    source TEXT,                          -- "UC forum", "testing", "research"
    source_url TEXT,
    UNIQUE(anticheat_id, method_id)
);

-- Bypass techniques for detection methods
CREATE TABLE bypasses (
    id INTEGER PRIMARY KEY,
    detection_method_id INTEGER REFERENCES detection_methods(id),
    technique TEXT NOT NULL,              -- Short name
    description TEXT,
    implementation TEXT,                  -- Code example
    difficulty TEXT DEFAULT 'medium',     -- "easy", "medium", "hard", "extreme"
    effectiveness TEXT DEFAULT 'proven',  -- "theoretical", "tested", "proven"
    side_effects TEXT,                    -- Any downsides
    requires TEXT,                        -- Prerequisites (e.g., "EPT support")
    created_at TEXT DEFAULT CURRENT_TIMESTAMP
);

-- Known signatures anti-cheats scan for
CREATE TABLE signatures (
    id INTEGER PRIMARY KEY,
    anticheat_id INTEGER REFERENCES anticheats(id),
    signature_type TEXT NOT NULL,         -- "string", "byte_pattern", "pool_tag", "hash"
    pattern TEXT NOT NULL,                -- The actual signature
    description TEXT,
    location TEXT,                        -- "kernel_memory", "disk", "registry"
    avoidance TEXT                        -- How to avoid triggering
);

-- Timing thresholds (separate for easy querying)
CREATE TABLE timing_thresholds (
    id INTEGER PRIMARY KEY,
    anticheat_id INTEGER REFERENCES anticheats(id),
    operation TEXT NOT NULL,              -- "cpuid", "rdmsr", "vmexit"
    threshold_cycles INTEGER,
    threshold_ns INTEGER,
    notes TEXT,
    source TEXT
);

-- Full-text search
CREATE VIRTUAL TABLE detection_methods_fts USING fts5(
    name, description, technique,
    content='detection_methods',
    content_rowid='id'
);

-- Indexes
CREATE INDEX idx_detection_anticheat ON detection_methods(anticheat_id);
CREATE INDEX idx_detection_category ON detection_methods(category);
CREATE INDEX idx_bypasses_method ON bypasses(detection_method_id);
```

### Sample Data

```sql
INSERT INTO anticheats (name, full_name, vendor, kernel_driver) VALUES
('EAC', 'Easy Anti-Cheat', 'Epic Games', 'EasyAntiCheat.sys'),
('BattlEye', 'BattlEye Anti-Cheat', 'BattlEye GmbH', 'BEDaisy.sys'),
('Vanguard', 'Riot Vanguard', 'Riot Games', 'vgk.sys'),
('FACEIT', 'FACEIT Anti-Cheat', 'FACEIT', 'faceit.sys'),
('ESEA', 'ESEA Anti-Cheat', 'ESL Gaming', 'esea.sys');

INSERT INTO detection_methods (anticheat_id, method_id, category, name, description, technique, check_frequency, severity, source) VALUES
(1, 'EAC-PML4E', 'memory', 'PML4E Kernel Page Scan',
 'Scans all kernel PML4 entries for executable pages without backing legitimate drivers',
 'Enumerates kernel page tables, checks each executable page against loaded driver list. Orphaned executable pages flagged.',
 'continuous', 'critical', 'UC forum research'),

(1, 'EAC-BIGPOOL', 'memory', 'BigPoolTable Scan',
 'Queries SystemBigPoolInformation for executable allocations >0xFE0 bytes',
 'Scans nt!PoolBigPageTable ~20 minutes after game launch for suspicious pool tags and executable memory',
 'periodic', 'high', 'UC forum research');
```

---

## 2. evasion_techniques.db Schema

Stores bypass chains, hiding methods, cleanup procedures.

```sql
-- Evasion technique categories
CREATE TABLE categories (
    id INTEGER PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,            -- "memory_hiding", "timing", "forensic_cleanup"
    description TEXT
);

-- Individual techniques
CREATE TABLE techniques (
    id INTEGER PRIMARY KEY,
    category_id INTEGER REFERENCES categories(id),
    name TEXT NOT NULL,                   -- "MDL Allocation"
    short_name TEXT,                      -- "mdl_alloc"
    description TEXT NOT NULL,
    use_case TEXT,                        -- When to use this
    requirements TEXT,                    -- What's needed (e.g., "kernel access")
    limitations TEXT,                     -- What it doesn't protect against
    detection_surface TEXT,               -- What traces remain
    source TEXT,
    source_url TEXT,
    created_at TEXT DEFAULT CURRENT_TIMESTAMP
);

-- Step-by-step procedures (for multi-step techniques)
CREATE TABLE procedure_steps (
    id INTEGER PRIMARY KEY,
    technique_id INTEGER REFERENCES techniques(id),
    step_number INTEGER NOT NULL,
    action TEXT NOT NULL,
    details TEXT,
    code_example TEXT,
    UNIQUE(technique_id, step_number)
);

-- Bypass chains (combining multiple techniques)
CREATE TABLE bypass_chains (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,                   -- "Full Stealth Driver Load"
    description TEXT,
    goal TEXT,                            -- What this achieves
    prerequisites TEXT,
    total_steps INTEGER
);

-- Chain to technique mapping (ordered)
CREATE TABLE chain_techniques (
    id INTEGER PRIMARY KEY,
    chain_id INTEGER REFERENCES bypass_chains(id),
    technique_id INTEGER REFERENCES techniques(id),
    step_order INTEGER NOT NULL,
    notes TEXT,
    UNIQUE(chain_id, step_order)
);

-- Code examples
CREATE TABLE code_examples (
    id INTEGER PRIMARY KEY,
    technique_id INTEGER REFERENCES techniques(id),
    language TEXT DEFAULT 'c',            -- "c", "asm", "python"
    title TEXT,
    code TEXT NOT NULL,
    explanation TEXT,
    tested BOOLEAN DEFAULT FALSE,
    works_on TEXT                         -- "win10-22h2", "all"
);

-- Cleanup procedures (forensic trace elimination)
CREATE TABLE cleanup_procedures (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT,
    requires_ring0 BOOLEAN DEFAULT TRUE,
    timing TEXT,                          -- "immediate", "post_activation", "on_unload"
    artifacts_cleared TEXT                -- JSON list of artifacts
);

CREATE TABLE cleanup_steps (
    id INTEGER PRIMARY KEY,
    procedure_id INTEGER REFERENCES cleanup_procedures(id),
    step_number INTEGER NOT NULL,
    target TEXT NOT NULL,                 -- "MmUnloadedDrivers", "ETW"
    action TEXT NOT NULL,
    code TEXT,
    risk_level TEXT DEFAULT 'medium',
    UNIQUE(procedure_id, step_number)
);

-- Full-text search
CREATE VIRTUAL TABLE techniques_fts USING fts5(
    name, description, use_case,
    content='techniques',
    content_rowid='id'
);
```

### Sample Data

```sql
INSERT INTO categories (name, description) VALUES
('memory_allocation', 'Techniques for allocating kernel memory without detection'),
('memory_hiding', 'Techniques for hiding allocated memory from scans'),
('timing_evasion', 'Techniques for defeating timing-based detection'),
('forensic_cleanup', 'Techniques for eliminating forensic artifacts'),
('driver_loading', 'Techniques for loading unsigned code into kernel');

INSERT INTO techniques (category_id, name, short_name, description, use_case, limitations) VALUES
(1, 'MDL-Based Physical Allocation', 'mdl_alloc',
 'Use MmAllocatePagesForMdl + MmMapLockedPagesSpecifyCache to allocate memory without BigPool entry',
 'Allocating driver memory that bypasses BigPoolTable scans',
 'Does NOT protect against PML4E enumeration scans - memory is still visible in page tables'),

(2, 'EPT Split-View', 'ept_split',
 'Present different memory contents to reads vs executes using EPT permissions',
 'Hide driver code from memory scanners while allowing execution',
 'Requires hypervisor running, adds complexity to memory management'),

(2, 'PFN Nulling', 'pfn_null',
 'Zero out MDL PFN array entries after mapping to hide physical page ownership',
 'Hide physical memory allocation from PFN-based scans',
 'Cannot free memory normally after nulling - causes BSOD if MmFreePagesFromMdl called');

INSERT INTO bypass_chains (name, description, goal) VALUES
('Full Stealth Driver Load',
 'Complete chain for loading a driver with maximum evasion',
 'Load and execute kernel code with no detectable artifacts');

INSERT INTO chain_techniques (chain_id, technique_id, step_order, notes) VALUES
(1, 1, 1, 'Allocate memory via MDL to avoid BigPool'),
(1, 3, 2, 'Null PFN array to hide physical allocation'),
(1, 2, 3, 'Use EPT split-view to hide from memory scans');
```

---

## 3. byovd_drivers.db Schema

Stores vulnerable driver information, IOCTLs, blocklist status.

```sql
-- Vulnerable drivers
CREATE TABLE drivers (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,                   -- "Ld9BoxSup.sys"
    original_name TEXT,                   -- "VBoxDrv.sys"
    vendor TEXT,                          -- "LDPlayer"
    signer TEXT,                          -- "Shanghai Yiyu Network Technology"
    signer_thumbprint TEXT,               -- Certificate thumbprint
    version TEXT,                         -- "6.1.36"
    file_size INTEGER,
    sha256 TEXT,
    md5 TEXT,
    device_path TEXT,                     -- "\\\\.\\Ld9BoxSup"
    legitimate_use TEXT,                  -- "Android emulator virtualization"
    discovery_date TEXT,
    cve TEXT,                             -- If assigned
    public_exploit BOOLEAN DEFAULT FALSE, -- Is exploit public?
    notes TEXT,
    UNIQUE(name, version)
);

-- Primitives the driver provides
CREATE TABLE primitives (
    id INTEGER PRIMARY KEY,
    driver_id INTEGER REFERENCES drivers(id),
    primitive_type TEXT NOT NULL,         -- "phys_read", "phys_write", "msr_read", "code_exec", "alloc"
    description TEXT NOT NULL,
    restrictions TEXT,                    -- Any limitations
    requires_auth BOOLEAN DEFAULT FALSE,  -- Needs session/cookie?
    UNIQUE(driver_id, primitive_type)
);

-- IOCTL definitions
CREATE TABLE ioctls (
    id INTEGER PRIMARY KEY,
    driver_id INTEGER REFERENCES drivers(id),
    name TEXT NOT NULL,                   -- "SUP_IOCTL_COOKIE"
    code TEXT NOT NULL,                   -- "0x00228204"
    code_decimal INTEGER,
    function_number INTEGER,
    description TEXT,
    input_struct TEXT,                    -- JSON schema or C struct
    output_struct TEXT,
    input_size INTEGER,
    output_size INTEGER,
    requires_session BOOLEAN DEFAULT TRUE,
    notes TEXT,
    UNIQUE(driver_id, name)
);

-- Magic values for authentication
CREATE TABLE magic_values (
    id INTEGER PRIMARY KEY,
    driver_id INTEGER REFERENCES drivers(id),
    value_name TEXT NOT NULL,             -- "initial_cookie", "magic_string"
    value_hex TEXT,                       -- "0x69726F74"
    value_string TEXT,                    -- "The Magic Word!"
    binary_offset TEXT,                   -- "0x36d58"
    description TEXT,
    UNIQUE(driver_id, value_name)
);

-- Blocklist status per anti-cheat
CREATE TABLE blocklist_status (
    id INTEGER PRIMARY KEY,
    driver_id INTEGER REFERENCES drivers(id),
    anticheat TEXT NOT NULL,              -- "EAC", "BattlEye", etc.
    blocked BOOLEAN DEFAULT FALSE,
    block_type TEXT,                      -- "hash", "signer", "name"
    first_blocked TEXT,                   -- Date first blocked
    last_checked TEXT,                    -- When we last verified
    notes TEXT,
    UNIQUE(driver_id, anticheat)
);

-- Known issues / gotchas
CREATE TABLE driver_gotchas (
    id INTEGER PRIMARY KEY,
    driver_id INTEGER REFERENCES drivers(id),
    symptom TEXT NOT NULL,
    cause TEXT NOT NULL,
    fix TEXT,
    affected_environments TEXT            -- "nested_virt", "all"
);

-- Indexes
CREATE INDEX idx_drivers_name ON drivers(name);
CREATE INDEX idx_ioctls_driver ON ioctls(driver_id);
CREATE INDEX idx_blocklist_driver ON blocklist_status(driver_id);
```

### Sample Data

```sql
INSERT INTO drivers (name, original_name, vendor, signer, version, device_path, legitimate_use, public_exploit) VALUES
('Ld9BoxSup.sys', 'VBoxDrv.sys', 'LDPlayer', 'Shanghai Yiyu Network Technology', '6.1.36',
 '\\\\.\\Ld9BoxSup', 'LDPlayer Android emulator virtualization support', FALSE);

INSERT INTO primitives (driver_id, primitive_type, description, requires_auth) VALUES
(1, 'code_exec', 'Execute arbitrary code at Ring-0 via LDR_LOAD', TRUE),
(1, 'kernel_alloc', 'Allocate executable kernel memory via LDR_OPEN', TRUE),
(1, 'symbol_resolve', 'Resolve ntoskrnl exports via LDR_GET_SYMBOL', TRUE),
(1, 'phys_alloc', 'Allocate physically contiguous memory via CONT_ALLOC', TRUE),
(1, 'msr_access', 'Read/write MSRs via MSR_PROBER (DISABLED in this build)', TRUE);

INSERT INTO ioctls (driver_id, name, code, function_number, description, input_size, output_size) VALUES
(1, 'SUP_IOCTL_COOKIE', '0x00228204', 1, 'Establish authenticated session', 48, 56),
(1, 'SUP_IOCTL_LDR_OPEN', '0x0022820C', 3, 'Allocate executable kernel memory', 328, 40),
(1, 'SUP_IOCTL_LDR_LOAD', '0x00228210', 4, 'Load code and call entry point', NULL, NULL),
(1, 'SUP_IOCTL_MSR_PROBER', '0x00228288', 34, 'Read/write MSRs (DISABLED)', NULL, NULL);

INSERT INTO magic_values (driver_id, value_name, value_hex, value_string, binary_offset, description) VALUES
(1, 'initial_cookie', '0x69726F74', 'tori', '0x8f2d', 'Modified from stock VBox "Bori"'),
(1, 'magic_string', NULL, 'The Magic Word!', '0x36d58', '16-byte auth string'),
(1, 'header_flags', '0x42000042', NULL, NULL, 'Required in all IOCTL headers'),
(1, 'driver_version', '0x320000', NULL, '0x4787', 'Interface version'),
(1, 'ntoskrnl_flag_offset', '0x4a1a0', NULL, NULL, 'Validation flag for -618 bypass'),
(1, 'hal_flag_offset', '0x4a210', NULL, NULL, 'Validation flag for -618 bypass');

INSERT INTO blocklist_status (driver_id, anticheat, blocked, last_checked) VALUES
(1, 'EAC', FALSE, '2025-12-26'),
(1, 'BattlEye', FALSE, '2025-12-26'),
(1, 'Vanguard', FALSE, '2025-12-26');

INSERT INTO driver_gotchas (driver_id, symptom, cause, fix, affected_environments) VALUES
(1, 'LDR_OPEN returns -618',
 'Kernel module enumeration fails in nested virt, flags at 0x4a1a0 and 0x4a210 stay at 0',
 'Use second driver (ThrottleStop) with physical memory access to patch flags to 1',
 'nested_virt');
```

---

## 4. UC Intel Routing Logic

When parsing scraped UC posts, classify and route to appropriate databases:

```python
import re
from dataclasses import dataclass
from typing import List, Set
from enum import Enum

class IntelCategory(Enum):
    ANTICHEAT_DETECTION = "anticheat_intel"
    EVASION_TECHNIQUE = "evasion_techniques"
    BYOVD_DRIVER = "byovd_drivers"
    WINDOWS_INTERNALS = "vergilius"
    RAW_INTEL = "uc_intel"

@dataclass
class ClassifiedPost:
    post_id: int
    categories: Set[IntelCategory]
    anticheat_mentions: List[str]
    techniques_mentioned: List[str]
    drivers_mentioned: List[str]
    code_snippets: List[str]
    confidence: float

# Keyword patterns for classification
ANTICHEAT_PATTERNS = {
    "EAC": r"\b(EAC|Easy\s*Anti[- ]?Cheat|EasyAntiCheat)\b",
    "BattlEye": r"\b(BE|BattlEye|Battle\s*Eye)\b",
    "Vanguard": r"\b(Vanguard|vgk\.sys)\b",
    "FACEIT": r"\b(FACEIT|faceit\.sys)\b",
}

DETECTION_KEYWORDS = [
    r"\bdetect(s|ed|ion|ing)?\b",
    r"\bscan(s|ned|ning)?\b",
    r"\bflag(s|ged|ging)?\b",
    r"\bban(s|ned|ning)?\b",
    r"\bthreshold\b",
    r"\btiming\b",
    r"\bsignature\b",
]

EVASION_KEYWORDS = [
    r"\bbypass(es|ed|ing)?\b",
    r"\bevad(e|es|ed|ing)\b",
    r"\bhid(e|es|den|ing)\b",
    r"\bspoof(s|ed|ing)?\b",
    r"\bstealth\b",
    r"\bundetect(ed|able)?\b",
]

BYOVD_KEYWORDS = [
    r"\bdriver\b.*\b(vuln|exploit|ioctl|load)\b",
    r"\bBYOVD\b",
    r"\bsigned\s+driver\b",
    r"\bkernel\s+(access|exec)\b",
    r"\bMmMapIoSpace\b",
    r"\bDeviceIoControl\b",
]

WINDOWS_INTERNALS_KEYWORDS = [
    r"\b_EPROCESS\b",
    r"\b_KPROCESS\b",
    r"\boffset\s+0x[0-9a-f]+\b",
    r"\bPsLoadedModuleList\b",
    r"\bMmUnloadedDrivers\b",
    r"\bPiDDBCacheTable\b",
    r"\bBigPoolTable\b",
]

DRIVER_NAMES = [
    r"\bLd9BoxSup\b",
    r"\bVBoxDrv\b",
    r"\bThrottleStop\b",
    r"\bgdrv\.sys\b",
    r"\bDBUtil\b",
    r"\bRTCore64\b",
    r"\bAsIO\b",
]

def classify_post(post_text: str, post_title: str = "") -> ClassifiedPost:
    """Classify a UC forum post into intel categories."""

    full_text = f"{post_title} {post_text}".lower()
    original_text = f"{post_title} {post_text}"

    categories = set()
    anticheat_mentions = []
    techniques = []
    drivers = []
    confidence = 0.0

    # Check for anti-cheat mentions
    for ac_name, pattern in ANTICHEAT_PATTERNS.items():
        if re.search(pattern, original_text, re.IGNORECASE):
            anticheat_mentions.append(ac_name)

    # Check for detection method discussion
    detection_score = sum(1 for kw in DETECTION_KEYWORDS if re.search(kw, full_text))
    if anticheat_mentions and detection_score >= 2:
        categories.add(IntelCategory.ANTICHEAT_DETECTION)
        confidence += 0.3

    # Check for evasion technique discussion
    evasion_score = sum(1 for kw in EVASION_KEYWORDS if re.search(kw, full_text))
    if evasion_score >= 2:
        categories.add(IntelCategory.EVASION_TECHNIQUE)
        confidence += 0.25

        # Extract technique names
        if "mdl" in full_text and ("alloc" in full_text or "map" in full_text):
            techniques.append("MDL Allocation")
        if "ept" in full_text and ("split" in full_text or "hook" in full_text):
            techniques.append("EPT Split-View")
        if "pfn" in full_text and "null" in full_text:
            techniques.append("PFN Nulling")
        if "timing" in full_text and ("compensat" in full_text or "offset" in full_text):
            techniques.append("Timing Compensation")

    # Check for BYOVD discussion
    byovd_score = sum(1 for kw in BYOVD_KEYWORDS if re.search(kw, full_text))
    for driver_pattern in DRIVER_NAMES:
        match = re.search(driver_pattern, original_text, re.IGNORECASE)
        if match:
            drivers.append(match.group())
            byovd_score += 2

    if byovd_score >= 2:
        categories.add(IntelCategory.BYOVD_DRIVER)
        confidence += 0.25

    # Check for Windows internals
    internals_score = sum(1 for kw in WINDOWS_INTERNALS_KEYWORDS if re.search(kw, original_text))
    if internals_score >= 2:
        categories.add(IntelCategory.WINDOWS_INTERNALS)
        confidence += 0.2

    # Extract code snippets
    code_snippets = extract_code_snippets(original_text)
    if code_snippets:
        confidence += 0.1

    # Everything goes to raw intel
    categories.add(IntelCategory.RAW_INTEL)

    # Normalize confidence
    confidence = min(confidence, 1.0)

    return ClassifiedPost(
        post_id=0,  # Set by caller
        categories=categories,
        anticheat_mentions=anticheat_mentions,
        techniques_mentioned=techniques,
        drivers_mentioned=drivers,
        code_snippets=code_snippets,
        confidence=confidence
    )

def extract_code_snippets(text: str) -> List[str]:
    """Extract code blocks from forum post."""
    snippets = []

    # BBCode style [code]...[/code]
    code_blocks = re.findall(r'\[code\](.*?)\[/code\]', text, re.DOTALL | re.IGNORECASE)
    snippets.extend(code_blocks)

    # Markdown style ```...```
    md_blocks = re.findall(r'```(?:\w+)?\n?(.*?)```', text, re.DOTALL)
    snippets.extend(md_blocks)

    # Inline code with C-like syntax (heuristic)
    if not snippets:
        # Look for function-like patterns
        c_patterns = re.findall(r'(?:NTSTATUS|BOOL|VOID|int|void)\s+\w+\s*\([^)]*\)\s*\{[^}]+\}', text, re.DOTALL)
        snippets.extend(c_patterns)

    return [s.strip() for s in snippets if len(s.strip()) > 20]

def route_classified_post(classified: ClassifiedPost, databases: dict):
    """Route classified post to appropriate databases."""

    for category in classified.categories:
        db = databases.get(category.value)
        if not db:
            continue

        if category == IntelCategory.ANTICHEAT_DETECTION:
            # Insert as potential detection method
            for ac in classified.anticheat_mentions:
                db.insert_potential_detection(
                    anticheat=ac,
                    source_post_id=classified.post_id,
                    confidence=classified.confidence,
                    code_snippets=classified.code_snippets
                )

        elif category == IntelCategory.EVASION_TECHNIQUE:
            for technique in classified.techniques_mentioned:
                db.insert_technique_reference(
                    technique_name=technique,
                    source_post_id=classified.post_id,
                    code_snippets=classified.code_snippets
                )

        elif category == IntelCategory.BYOVD_DRIVER:
            for driver in classified.drivers_mentioned:
                db.insert_driver_reference(
                    driver_name=driver,
                    source_post_id=classified.post_id,
                    code_snippets=classified.code_snippets
                )
```

---

## 5. New MCP Tools

### anticheat_intel Tools

```python
# Query tools
get_anticheat_intel(anticheat: str) -> dict
get_detection_methods(anticheat: str, category: str = None) -> list
get_bypass_for_method(method_id: str) -> dict
get_timing_thresholds(anticheat: str) -> list
search_detections(query: str) -> list

# Add/Update tools
add_detection_method(anticheat: str, method_id: str, category: str,
                     name: str, description: str, ...) -> str
add_bypass(method_id: str, technique: str, implementation: str, ...) -> str
update_threshold(anticheat: str, operation: str, cycles: int) -> bool
add_signature(anticheat: str, sig_type: str, pattern: str, ...) -> str
```

### evasion_techniques Tools

```python
# Query tools
get_technique(name: str) -> dict
get_techniques_by_category(category: str) -> list
get_bypass_chain(name: str) -> dict
get_cleanup_procedure(name: str) -> dict
search_techniques(query: str) -> list

# Add/Update tools
add_technique(category: str, name: str, description: str, ...) -> str
add_bypass_chain(name: str, techniques: list, ...) -> str
add_code_example(technique: str, language: str, code: str, ...) -> str
add_cleanup_procedure(name: str, steps: list, ...) -> str
```

### byovd_drivers Tools

```python
# Query tools
get_driver(name: str) -> dict
get_driver_ioctls(driver: str) -> list
get_driver_primitives(driver: str) -> list
get_magic_values(driver: str) -> dict
check_blocklist_status(driver: str) -> dict
search_drivers(query: str) -> list

# Add/Update tools
add_driver(name: str, vendor: str, ...) -> str
add_ioctl(driver: str, name: str, code: str, ...) -> str
add_magic_value(driver: str, name: str, value: str, ...) -> str
update_blocklist_status(driver: str, anticheat: str, blocked: bool) -> bool
add_driver_gotcha(driver: str, symptom: str, cause: str, fix: str) -> str
```

---

## 6. Migration Plan

### Phase 1: Create New Databases
1. Create `anticheat_intel.db` with schema
2. Create `evasion_techniques.db` with schema
3. Create `byovd_drivers.db` with schema
4. Migrate hardcoded data from `anticheat_intel.py` and `stealth.py`

### Phase 2: Implement New Tools
1. Create `tools/anticheat_db.py` with query/add tools
2. Create `tools/evasion_db.py` with query/add tools
3. Create `tools/byovd_db.py` with query/add tools
4. Register tools in `server.py`

### Phase 3: Migrate Existing Data
1. Move G015-G018 to appropriate new databases
2. Move D009-D011 to appropriate new databases
3. Migrate `ld9boxsup_ioctls.json` to `byovd_drivers.db`
4. Migrate `DETECTION_VECTORS` dict to `anticheat_intel.db`
5. Migrate `ANTICHEAT_DATABASE` dict to `anticheat_intel.db`

### Phase 4: UC Parser Integration
1. Implement classification logic in UC scraper
2. Add routing to new databases
3. Create review queue for human validation of classified intel

### Phase 5: Cleanup
1. Remove deprecated hardcoded dicts
2. Update CLAUDE.md to reference new tool names
3. Add FTS5 indexes for semantic search
4. Create backup/sync procedures

---

## 7. Cross-Reference Design

Enable linking intel across databases:

```sql
-- In each database, add a cross_references table
CREATE TABLE cross_references (
    id INTEGER PRIMARY KEY,
    source_type TEXT NOT NULL,        -- "detection_method", "technique", etc.
    source_id INTEGER NOT NULL,
    target_db TEXT NOT NULL,          -- "anticheat_intel", "evasion_techniques", etc.
    target_type TEXT NOT NULL,
    target_id INTEGER NOT NULL,
    relationship TEXT,                -- "bypassed_by", "requires", "related_to"
    notes TEXT
);

-- Example: Link EAC-PML4E detection to EPT Split-View bypass
INSERT INTO cross_references VALUES
(1, 'detection_method', 1, 'evasion_techniques', 'technique', 2, 'bypassed_by',
 'EPT split-view hides memory from PML4E enumeration');
```

---

## Summary

This refactor creates a proper taxonomy:

| Database | Purpose | Key Tools |
|----------|---------|-----------|
| `intel_sdm.db` | Intel SDM reference | `vmcs_field`, `exit_reason` |
| `vergilius.db` | Windows structures | `get_structure`, `get_field_offset` |
| `anticheat_intel.db` | Detection methods | `add_detection_method`, `get_bypass_for_method` |
| `evasion_techniques.db` | Bypass techniques | `add_technique`, `get_bypass_chain` |
| `byovd_drivers.db` | Vulnerable drivers | `add_ioctl`, `get_magic_values` |
| `project_brain.db` | Project state | `add_decision`, `get_findings` |
| `uc_intel.db` | Raw community intel | `search_uc_intel` |

The UC parser automatically classifies and routes intel to the right database, making everything queryable in context.
