"""Database schema for Driver RE MCP"""

SCHEMA = """
-- Drivers table
CREATE TABLE IF NOT EXISTS drivers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    filename TEXT NOT NULL UNIQUE,
    md5 TEXT NOT NULL,
    sha256 TEXT NOT NULL UNIQUE,
    file_size INTEGER NOT NULL,
    status TEXT DEFAULT 'pending',
    description TEXT,
    vendor TEXT,
    version TEXT,
    signing_cert TEXT,
    is_signed BOOLEAN DEFAULT 0,
    is_vulnerable BOOLEAN DEFAULT 0,
    ghidra_project TEXT,
    base_address INTEGER,
    entry_point INTEGER,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- IOCTLs table
CREATE TABLE IF NOT EXISTS ioctls (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    driver_id INTEGER NOT NULL,
    code INTEGER NOT NULL,
    name TEXT,
    description TEXT,
    handler_address INTEGER,
    handler_name TEXT,
    input_buffer_type TEXT,
    output_buffer_type TEXT,
    input_size INTEGER,
    output_size INTEGER,
    requires_admin BOOLEAN DEFAULT 0,
    validation_notes TEXT,
    is_vulnerable BOOLEAN DEFAULT 0,
    vulnerability_description TEXT,
    FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE,
    UNIQUE(driver_id, code)
);

-- Functions table
CREATE TABLE IF NOT EXISTS functions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    driver_id INTEGER NOT NULL,
    address INTEGER NOT NULL,
    name TEXT NOT NULL,
    decompiled_code TEXT,
    disassembly TEXT,
    description TEXT,
    is_export BOOLEAN DEFAULT 0,
    is_import BOOLEAN DEFAULT 0,
    signature TEXT,
    call_convention TEXT,
    FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE,
    UNIQUE(driver_id, address)
);

-- Imports table
CREATE TABLE IF NOT EXISTS imports (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    driver_id INTEGER NOT NULL,
    module_name TEXT NOT NULL,
    function_name TEXT NOT NULL,
    ordinal INTEGER,
    is_by_ordinal BOOLEAN DEFAULT 0,
    category TEXT,
    risk_level TEXT,
    description TEXT,
    FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE
);

-- Exports table
CREATE TABLE IF NOT EXISTS exports (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    driver_id INTEGER NOT NULL,
    address INTEGER NOT NULL,
    name TEXT NOT NULL,
    ordinal INTEGER,
    description TEXT,
    is_forwarded BOOLEAN DEFAULT 0,
    forward_name TEXT,
    FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE,
    UNIQUE(driver_id, address)
);

-- Structures table
CREATE TABLE IF NOT EXISTS structures (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    driver_id INTEGER NOT NULL,
    name TEXT NOT NULL,
    size INTEGER NOT NULL,
    definition TEXT NOT NULL,
    description TEXT,
    is_ioctl_buffer BOOLEAN DEFAULT 0,
    FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE,
    UNIQUE(driver_id, name)
);

-- IOCTL-Structure mapping
CREATE TABLE IF NOT EXISTS ioctl_structures (
    ioctl_id INTEGER NOT NULL,
    structure_id INTEGER NOT NULL,
    is_input BOOLEAN DEFAULT 1,
    is_output BOOLEAN DEFAULT 0,
    FOREIGN KEY (ioctl_id) REFERENCES ioctls(id) ON DELETE CASCADE,
    FOREIGN KEY (structure_id) REFERENCES structures(id) ON DELETE CASCADE,
    PRIMARY KEY (ioctl_id, structure_id)
);

-- Vulnerabilities table
CREATE TABLE IF NOT EXISTS vulnerabilities (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    driver_id INTEGER NOT NULL,
    vuln_type TEXT NOT NULL,
    severity TEXT NOT NULL,
    title TEXT NOT NULL,
    description TEXT NOT NULL,
    affected_function TEXT,
    affected_address INTEGER,
    root_cause TEXT,
    exploitation_notes TEXT,
    exploit_difficulty TEXT,
    poc_code TEXT,
    cve_id TEXT,
    discovered_by TEXT,
    discovered_date TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE
);

-- Attack chains table
CREATE TABLE IF NOT EXISTS attack_chains (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    driver_id INTEGER NOT NULL,
    name TEXT NOT NULL,
    description TEXT NOT NULL,
    steps TEXT NOT NULL,  -- JSON array
    vulnerability_ids TEXT NOT NULL,  -- JSON array of vuln IDs
    success_rate REAL,
    requirements TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE
);

-- Cross-references table
CREATE TABLE IF NOT EXISTS xrefs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    driver_id INTEGER NOT NULL,
    from_address INTEGER NOT NULL,
    to_address INTEGER NOT NULL,
    xref_type TEXT NOT NULL,
    from_function TEXT,
    to_function TEXT,
    instruction TEXT,
    FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE
);

-- Strings table
CREATE TABLE IF NOT EXISTS strings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    driver_id INTEGER NOT NULL,
    address INTEGER NOT NULL,
    value TEXT NOT NULL,
    encoding TEXT DEFAULT 'ascii',
    length INTEGER NOT NULL,
    FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE
);

-- Analysis sessions table
CREATE TABLE IF NOT EXISTS analysis_sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    driver_id INTEGER NOT NULL,
    analyst TEXT NOT NULL,
    started_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ended_at TIMESTAMP,
    goals TEXT,
    findings_summary TEXT,
    FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE
);

-- Analysis notes table
CREATE TABLE IF NOT EXISTS analysis_notes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    session_id INTEGER NOT NULL,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    note_type TEXT NOT NULL,
    content TEXT NOT NULL,
    related_address INTEGER,
    related_function TEXT,
    tags TEXT,  -- JSON array
    FOREIGN KEY (session_id) REFERENCES analysis_sessions(id) ON DELETE CASCADE
);

-- API categories table (for import categorization)
CREATE TABLE IF NOT EXISTS api_categories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    category_name TEXT NOT NULL UNIQUE,
    description TEXT,
    risk_level TEXT,
    example_apis TEXT  -- JSON array
);

-- Indexes for performance
CREATE INDEX IF NOT EXISTS idx_drivers_status ON drivers(status);
CREATE INDEX IF NOT EXISTS idx_drivers_sha256 ON drivers(sha256);
CREATE INDEX IF NOT EXISTS idx_ioctls_driver ON ioctls(driver_id);
CREATE INDEX IF NOT EXISTS idx_ioctls_code ON ioctls(code);
CREATE INDEX IF NOT EXISTS idx_ioctls_vulnerable ON ioctls(is_vulnerable);
CREATE INDEX IF NOT EXISTS idx_functions_driver ON functions(driver_id);
CREATE INDEX IF NOT EXISTS idx_functions_address ON functions(address);
CREATE INDEX IF NOT EXISTS idx_functions_name ON functions(name);
CREATE INDEX IF NOT EXISTS idx_imports_driver ON imports(driver_id);
CREATE INDEX IF NOT EXISTS idx_imports_function ON imports(function_name);
CREATE INDEX IF NOT EXISTS idx_imports_category ON imports(category);
CREATE INDEX IF NOT EXISTS idx_exports_driver ON exports(driver_id);
CREATE INDEX IF NOT EXISTS idx_structures_driver ON structures(driver_id);
CREATE INDEX IF NOT EXISTS idx_vulnerabilities_driver ON vulnerabilities(driver_id);
CREATE INDEX IF NOT EXISTS idx_vulnerabilities_type ON vulnerabilities(vuln_type);
CREATE INDEX IF NOT EXISTS idx_vulnerabilities_severity ON vulnerabilities(severity);
CREATE INDEX IF NOT EXISTS idx_xrefs_driver ON xrefs(driver_id);
CREATE INDEX IF NOT EXISTS idx_xrefs_from ON xrefs(from_address);
CREATE INDEX IF NOT EXISTS idx_xrefs_to ON xrefs(to_address);
CREATE INDEX IF NOT EXISTS idx_xrefs_type ON xrefs(xref_type);
CREATE INDEX IF NOT EXISTS idx_strings_driver ON strings(driver_id);
CREATE INDEX IF NOT EXISTS idx_analysis_sessions_driver ON analysis_sessions(driver_id);
CREATE INDEX IF NOT EXISTS idx_analysis_notes_session ON analysis_notes(session_id);

-- Full-text search virtual tables
CREATE VIRTUAL TABLE IF NOT EXISTS functions_fts USING fts5(
    driver_id UNINDEXED,
    address UNINDEXED,
    name,
    decompiled_code,
    description,
    content=functions,
    content_rowid=id
);

CREATE VIRTUAL TABLE IF NOT EXISTS analysis_notes_fts USING fts5(
    session_id UNINDEXED,
    note_type,
    content,
    related_function,
    content=analysis_notes,
    content_rowid=id
);

CREATE VIRTUAL TABLE IF NOT EXISTS vulnerabilities_fts USING fts5(
    driver_id UNINDEXED,
    vuln_type,
    title,
    description,
    root_cause,
    exploitation_notes,
    content=vulnerabilities,
    content_rowid=id
);
"""

API_CATEGORIES = [
    ("Memory Management", "Memory allocation and manipulation APIs", "high",
     '["ExAllocatePool", "ExAllocatePoolWithTag", "MmMapIoSpace", "MmMapLockedPages", "ZwAllocateVirtualMemory"]'),

    ("Process/Thread", "Process and thread manipulation", "critical",
     '["PsLookupProcessByProcessId", "KeStackAttachProcess", "ZwOpenProcess", "PsCreateSystemThread"]'),

    ("Registry", "Registry access APIs", "medium",
     '["ZwOpenKey", "ZwQueryValueKey", "ZwSetValueKey", "ZwDeleteKey"]'),

    ("File I/O", "File system operations", "medium",
     '["ZwCreateFile", "ZwReadFile", "ZwWriteFile", "ZwDeleteFile"]'),

    ("Device I/O", "Device communication", "high",
     '["IoBuildDeviceIoControlRequest", "IoCallDriver", "IofCallDriver"]'),

    ("Synchronization", "Locking and synchronization primitives", "low",
     '["KeWaitForSingleObject", "KeInitializeMutex", "ExAcquireFastMutex"]'),

    ("String Manipulation", "String handling functions", "medium",
     '["RtlCopyMemory", "RtlMoveMemory", "RtlCompareMemory", "memcpy", "strcpy"]'),

    ("Security", "Security descriptor and access control", "high",
     '["SeAccessCheck", "SeCaptureSecurityDescriptor", "ObReferenceObjectByHandle"]'),

    ("Network", "Network stack interaction", "medium",
     '["TdiConnect", "TdiSend", "TdiReceive"]'),

    ("Hardware", "Direct hardware access", "critical",
     '["READ_PORT_UCHAR", "WRITE_PORT_UCHAR", "HalSetBusDataByOffset", "__inbyte", "__outbyte"]'),

    ("Debugging", "Debug and trace APIs", "low",
     '["DbgPrint", "DbgPrintEx", "KdDebuggerEnabled"]'),

    ("Exception Handling", "Exception and SEH related", "medium",
     '["__try", "__except", "RtlCaptureContext", "RtlUnwind"]'),

    ("Object Management", "Object manager APIs", "medium",
     '["ObReferenceObject", "ObDereferenceObject", "ObOpenObjectByPointer"]'),

    ("System Information", "System query and configuration", "low",
     '["ZwQuerySystemInformation", "ZwSetSystemInformation"]'),

    ("Token Manipulation", "Access token operations", "critical",
     '["SeQueryAuthenticationIdToken", "PsReferencePrimaryToken", "ZwOpenProcessToken"]'),

    ("Physical Memory", "Direct physical memory access", "critical",
     '["MmMapIoSpace", "MmGetPhysicalAddress", "MmGetVirtualForPhysical"]'),

    ("MDL Operations", "Memory Descriptor List functions", "high",
     '["IoAllocateMdl", "MmProbeAndLockPages", "MmBuildMdlForNonPagedPool"]'),

    ("Callback Registration", "Notification callbacks", "high",
     '["PsSetCreateProcessNotifyRoutine", "PsSetLoadImageNotifyRoutine", "CmRegisterCallback"]'),

    ("Cryptography", "Cryptographic functions", "medium",
     '["BCryptOpenAlgorithmProvider", "BCryptEncrypt", "BCryptDecrypt"]'),

    ("Virtualization", "Hypervisor and virtualization", "critical",
     '["__vmx_on", "__vmx_vmcall", "__svm_vmrun", "HvCallSwitchVirtualAddressSpace"]'),
]
