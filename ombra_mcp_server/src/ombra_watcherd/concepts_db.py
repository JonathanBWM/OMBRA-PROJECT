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
    # JSON fields that need serialization/deserialization
    JSON_FIELDS = [
        "required_patterns", "optional_patterns", "anti_patterns",
        "vmcs_fields", "exit_reasons", "msrs", "sdm_refs",
        "implementation_files", "anti_cheat_relevance", "depends_on"
    ]

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

    def add_concept(self, concept: Dict[str, Any]) -> None:
        """Add or update a concept."""
        # Work on a copy to avoid mutating caller's data
        concept = dict(concept)

        # Convert lists to JSON
        for field in self.JSON_FIELDS:
            if field in concept and isinstance(concept[field], list):
                concept[field] = json.dumps(concept[field])

        fields = list(concept.keys())
        placeholders = ", ".join(["?" for _ in fields])
        field_names = ", ".join(fields)

        sql = f"""
            INSERT OR REPLACE INTO concepts ({field_names})
            VALUES ({placeholders})
        """
        self.execute(sql, tuple(concept.values()))
        self.commit()

    def get_concept(self, concept_id: str) -> Optional[Dict[str, Any]]:
        """Get a concept by ID."""
        row = self.execute(
            "SELECT * FROM concepts WHERE id = ?", (concept_id,)
        ).fetchone()

        if row is None:
            return None

        result = dict(row)

        # Parse JSON fields back to lists
        for field in self.JSON_FIELDS:
            if result.get(field):
                try:
                    result[field] = json.loads(result[field])
                except json.JSONDecodeError:
                    pass

        return result

    def list_concepts(
        self,
        category: Optional[str] = None,
        status: Optional[str] = None,
        priority: Optional[str] = None
    ) -> List[Dict[str, Any]]:
        """List concepts with optional filters."""
        sql = "SELECT * FROM concepts WHERE 1=1"
        params = []

        if category:
            sql += " AND category = ?"
            params.append(category)
        if status:
            sql += " AND implementation_status = ?"
            params.append(status)
        if priority:
            sql += " AND priority = ?"
            params.append(priority)

        # Order by semantic priority, then name
        sql += """ ORDER BY
            CASE priority
                WHEN 'critical' THEN 1
                WHEN 'high' THEN 2
                WHEN 'medium' THEN 3
                WHEN 'low' THEN 4
                ELSE 5
            END,
            name
        """

        rows = self.execute(sql, tuple(params)).fetchall()

        # Deserialize JSON fields for consistency with get_concept()
        results = []
        for row in rows:
            result = dict(row)
            for field in self.JSON_FIELDS:
                if result.get(field):
                    try:
                        result[field] = json.loads(result[field])
                    except json.JSONDecodeError:
                        pass
            results.append(result)

        return results

    def add_finding(
        self,
        concept_id: str,
        file_path: str,
        line_number: int,
        finding_type: str,
        message: str,
        severity: str = "info"
    ) -> int:
        """Add a finding for a concept."""
        cursor = self.execute(
            """
            INSERT INTO concept_findings
            (concept_id, file_path, line_number, finding_type, message, severity)
            VALUES (?, ?, ?, ?, ?, ?)
            """,
            (concept_id, file_path, line_number, finding_type, message, severity)
        )
        self.commit()
        return cursor.lastrowid

    def get_findings(
        self,
        concept_id: Optional[str] = None,
        file_path: Optional[str] = None,
        severity: Optional[str] = None,
        include_dismissed: bool = False
    ) -> List[Dict[str, Any]]:
        """Query findings with filters."""
        sql = "SELECT * FROM concept_findings WHERE 1=1"
        params = []

        if not include_dismissed:
            sql += " AND dismissed = 0"
        if concept_id:
            sql += " AND concept_id = ?"
            params.append(concept_id)
        if file_path:
            sql += " AND file_path LIKE ?"
            params.append(f"%{file_path}%")
        if severity:
            sql += " AND severity = ?"
            params.append(severity)

        sql += " ORDER BY detected_at DESC"

        rows = self.execute(sql, tuple(params)).fetchall()
        return [dict(row) for row in rows]

    def dismiss_finding(self, finding_id: int, reason: str) -> bool:
        """Dismiss a finding as false positive."""
        self.execute(
            "UPDATE concept_findings SET dismissed = 1, dismissed_reason = ? WHERE id = ?",
            (reason, finding_id)
        )
        self.commit()
        return True

    def add_annotation(
        self,
        concept_id: str,
        file_path: str,
        line_number: int,
        annotation_type: str,
        notes: Optional[str] = None
    ) -> int:
        """Add an annotation (human verification) for a concept."""
        cursor = self.execute(
            """
            INSERT INTO concept_annotations
            (concept_id, file_path, line_number, annotation_type, notes)
            VALUES (?, ?, ?, ?, ?)
            """,
            (concept_id, file_path, line_number, annotation_type, notes)
        )
        annotation_id = cursor.lastrowid

        # Update concept verification status
        if annotation_type == "verified":
            self.execute(
                "UPDATE concepts SET verified_by_annotation = 1 WHERE id = ?",
                (concept_id,)
            )

        self.commit()  # Single commit at end for atomic transaction
        return annotation_id

    def get_annotations(
        self,
        concept_id: Optional[str] = None,
        file_path: Optional[str] = None,
        annotation_type: Optional[str] = None
    ) -> List[Dict[str, Any]]:
        """Query annotations with filters."""
        sql = "SELECT * FROM concept_annotations WHERE 1=1"
        params = []

        if concept_id:
            sql += " AND concept_id = ?"
            params.append(concept_id)
        if file_path:
            sql += " AND file_path LIKE ?"
            params.append(f"%{file_path}%")
        if annotation_type:
            sql += " AND annotation_type = ?"
            params.append(annotation_type)

        sql += " ORDER BY created_at DESC"

        rows = self.execute(sql, tuple(params)).fetchall()
        return [dict(row) for row in rows]
