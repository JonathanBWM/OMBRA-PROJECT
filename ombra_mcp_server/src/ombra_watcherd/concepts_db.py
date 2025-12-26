"""
Concept Intelligence Database

Stores hypervisor concepts, patterns, findings, and annotations.
"""

import sqlite3
from pathlib import Path
from typing import Optional, List, Dict, Any
import json

SCHEMA = """
CREATE TABLE IF NOT EXISTS concepts (
    id TEXT PRIMARY KEY,
    category TEXT NOT NULL,
    name TEXT NOT NULL,
    description TEXT,
    source_doc TEXT,
    sdm_refs TEXT,

    required_patterns TEXT,
    optional_patterns TEXT,
    anti_patterns TEXT,

    vmcs_fields TEXT,
    exit_reasons TEXT,
    msrs TEXT,

    implementation_status TEXT DEFAULT 'not_started',
    confidence REAL DEFAULT 0.0,
    verified_by_annotation INTEGER DEFAULT 0,
    implementation_files TEXT,

    priority TEXT DEFAULT 'medium',
    anti_cheat_relevance TEXT,

    depends_on TEXT,
    phase_order INTEGER
);

CREATE TABLE IF NOT EXISTS concept_findings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    concept_id TEXT NOT NULL,
    file_path TEXT,
    line_number INTEGER,
    finding_type TEXT NOT NULL,
    message TEXT,
    severity TEXT DEFAULT 'info',
    detected_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    dismissed INTEGER DEFAULT 0,
    dismissed_reason TEXT,
    FOREIGN KEY (concept_id) REFERENCES concepts(id)
);

CREATE TABLE IF NOT EXISTS concept_annotations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    concept_id TEXT NOT NULL,
    file_path TEXT,
    line_number INTEGER,
    annotation_type TEXT NOT NULL,
    notes TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (concept_id) REFERENCES concepts(id)
);

CREATE INDEX IF NOT EXISTS idx_findings_concept ON concept_findings(concept_id);
CREATE INDEX IF NOT EXISTS idx_findings_file ON concept_findings(file_path);
CREATE INDEX IF NOT EXISTS idx_annotations_concept ON concept_annotations(concept_id);
"""


class ConceptsDB:
    def __init__(self, db_path: Optional[Path] = None):
        if db_path is None:
            db_path = Path(__file__).parent / "concepts.db"
        self.db_path = db_path
        self._conn: Optional[sqlite3.Connection] = None
        self._init_db()

    def _init_db(self):
        conn = self._get_conn()
        conn.executescript(SCHEMA)
        conn.commit()

    def _get_conn(self) -> sqlite3.Connection:
        if self._conn is None:
            self._conn = sqlite3.connect(str(self.db_path))
            self._conn.row_factory = sqlite3.Row
        return self._conn

    def execute(self, sql: str, params: tuple = ()) -> sqlite3.Cursor:
        return self._get_conn().execute(sql, params)

    def commit(self):
        self._get_conn().commit()

    def close(self):
        if self._conn:
            self._conn.close()
            self._conn = None
