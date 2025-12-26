"""
Project Brain Database - SQLite storage for all project intelligence

Stores: components, findings, suggestions, decisions, gotchas, sessions
"""

import sqlite3
import json
from datetime import datetime
from pathlib import Path
from typing import Optional, List, Dict, Any
from contextlib import contextmanager


DEFAULT_DB_PATH = Path(__file__).parent.parent / "ombra_mcp" / "data" / "project_brain.db"


class ProjectBrainDB:
    """SQLite database for project intelligence."""

    def __init__(self, db_path: Optional[Path] = None):
        self.db_path = db_path or DEFAULT_DB_PATH
        self.db_path.parent.mkdir(parents=True, exist_ok=True)
        self._init_schema()

    @contextmanager
    def _connection(self):
        """Context manager for database connections."""
        conn = sqlite3.connect(self.db_path)
        conn.row_factory = sqlite3.Row
        try:
            yield conn
            conn.commit()
        finally:
            conn.close()

    def _init_schema(self):
        """Initialize database schema."""
        with self._connection() as conn:
            conn.executescript("""
                -- Component tracking
                CREATE TABLE IF NOT EXISTS components (
                    id TEXT PRIMARY KEY,
                    name TEXT NOT NULL,
                    category TEXT,
                    status TEXT,
                    files TEXT,
                    missing TEXT,
                    depends_on TEXT,
                    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                );

                CREATE TABLE IF NOT EXISTS exit_handlers (
                    reason INTEGER PRIMARY KEY,
                    name TEXT NOT NULL,
                    status TEXT,
                    has_stealth INTEGER DEFAULT 0,
                    has_timing INTEGER DEFAULT 0,
                    file TEXT,
                    line INTEGER,
                    notes TEXT,
                    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                );

                CREATE TABLE IF NOT EXISTS vmcs_field_usage (
                    encoding TEXT PRIMARY KEY,
                    name TEXT NOT NULL,
                    is_read INTEGER DEFAULT 0,
                    is_written INTEGER DEFAULT 0,
                    read_locations TEXT,
                    write_locations TEXT,
                    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                );

                -- Findings
                CREATE TABLE IF NOT EXISTS findings (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    file TEXT NOT NULL,
                    line INTEGER,
                    severity TEXT NOT NULL,
                    type TEXT NOT NULL,
                    check_id TEXT NOT NULL,
                    message TEXT NOT NULL,
                    suggested_fix TEXT,
                    dismissed INTEGER DEFAULT 0,
                    dismissed_reason TEXT,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                );

                CREATE TABLE IF NOT EXISTS suggestions (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    priority TEXT NOT NULL,
                    type TEXT NOT NULL,
                    component TEXT,
                    message TEXT NOT NULL,
                    action TEXT,
                    acted_on INTEGER DEFAULT 0,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                );

                -- Knowledge persistence
                CREATE TABLE IF NOT EXISTS decisions (
                    id TEXT PRIMARY KEY,
                    date DATE NOT NULL,
                    topic TEXT NOT NULL,
                    choice TEXT NOT NULL,
                    rationale TEXT,
                    alternatives TEXT,
                    affects TEXT,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                );

                CREATE TABLE IF NOT EXISTS gotchas (
                    id TEXT PRIMARY KEY,
                    symptom TEXT NOT NULL,
                    cause TEXT NOT NULL,
                    fix TEXT NOT NULL,
                    files TEXT,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                );

                CREATE TABLE IF NOT EXISTS sessions (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    started_at TIMESTAMP NOT NULL,
                    ended_at TIMESTAMP,
                    working_on TEXT,
                    context_snapshot TEXT,
                    files_touched TEXT,
                    decisions_made TEXT,
                    findings_addressed TEXT
                );

                -- Daemon state
                CREATE TABLE IF NOT EXISTS daemon_state (
                    key TEXT PRIMARY KEY,
                    value TEXT,
                    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                );

                -- Indexes
                CREATE INDEX IF NOT EXISTS idx_findings_severity
                    ON findings(severity) WHERE NOT dismissed;
                CREATE INDEX IF NOT EXISTS idx_findings_file
                    ON findings(file) WHERE NOT dismissed;
                CREATE INDEX IF NOT EXISTS idx_suggestions_priority
                    ON suggestions(priority) WHERE NOT acted_on;
                CREATE INDEX IF NOT EXISTS idx_exit_handlers_status
                    ON exit_handlers(status);
            """)

    # =========================================================================
    # Findings
    # =========================================================================

    def add_finding(
        self,
        file: str,
        severity: str,
        type_: str,
        check_id: str,
        message: str,
        line: Optional[int] = None,
        suggested_fix: Optional[str] = None
    ) -> int:
        """Add a new finding. Returns the finding ID."""
        with self._connection() as conn:
            # Check if identical finding already exists
            existing = conn.execute("""
                SELECT id FROM findings
                WHERE file = ? AND check_id = ? AND line = ? AND NOT dismissed
            """, (file, check_id, line)).fetchone()

            if existing:
                # Update timestamp
                conn.execute("""
                    UPDATE findings SET updated_at = CURRENT_TIMESTAMP
                    WHERE id = ?
                """, (existing['id'],))
                return existing['id']

            cursor = conn.execute("""
                INSERT INTO findings (file, line, severity, type, check_id, message, suggested_fix)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            """, (file, line, severity, type_, check_id, message, suggested_fix))
            return cursor.lastrowid

    def get_findings(
        self,
        severity: Optional[str] = None,
        file: Optional[str] = None,
        type_: Optional[str] = None,
        include_dismissed: bool = False,
        limit: int = 100
    ) -> List[Dict[str, Any]]:
        """Query findings with optional filters."""
        conditions = []
        params = []

        if not include_dismissed:
            conditions.append("NOT dismissed")

        if severity:
            conditions.append("severity = ?")
            params.append(severity)

        if file:
            conditions.append("file LIKE ?")
            params.append(f"%{file}%")

        if type_:
            conditions.append("type = ?")
            params.append(type_)

        where = "WHERE " + " AND ".join(conditions) if conditions else ""

        with self._connection() as conn:
            rows = conn.execute(f"""
                SELECT * FROM findings {where}
                ORDER BY
                    CASE severity
                        WHEN 'critical' THEN 1
                        WHEN 'warning' THEN 2
                        ELSE 3
                    END,
                    updated_at DESC
                LIMIT ?
            """, params + [limit]).fetchall()
            return [dict(row) for row in rows]

    def dismiss_finding(self, finding_id: int, reason: Optional[str] = None) -> bool:
        """Dismiss a finding as false positive."""
        with self._connection() as conn:
            cursor = conn.execute("""
                UPDATE findings
                SET dismissed = 1, dismissed_reason = ?, updated_at = CURRENT_TIMESTAMP
                WHERE id = ?
            """, (reason, finding_id))
            return cursor.rowcount > 0

    def clear_stale_findings(self, file: str):
        """Clear findings for a file that will be re-scanned."""
        with self._connection() as conn:
            conn.execute("""
                DELETE FROM findings WHERE file = ? AND NOT dismissed
            """, (file,))

    def get_critical_findings(self) -> List[Dict[str, Any]]:
        """Get all critical, non-dismissed findings."""
        return self.get_findings(severity="critical")

    # =========================================================================
    # Components
    # =========================================================================

    def upsert_component(
        self,
        id_: str,
        name: str,
        category: Optional[str] = None,
        status: Optional[str] = None,
        files: Optional[List[str]] = None,
        missing: Optional[List[str]] = None,
        depends_on: Optional[List[str]] = None
    ):
        """Insert or update a component."""
        with self._connection() as conn:
            conn.execute("""
                INSERT INTO components (id, name, category, status, files, missing, depends_on)
                VALUES (?, ?, ?, ?, ?, ?, ?)
                ON CONFLICT(id) DO UPDATE SET
                    name = excluded.name,
                    category = COALESCE(excluded.category, category),
                    status = COALESCE(excluded.status, status),
                    files = COALESCE(excluded.files, files),
                    missing = COALESCE(excluded.missing, missing),
                    depends_on = COALESCE(excluded.depends_on, depends_on),
                    updated_at = CURRENT_TIMESTAMP
            """, (
                id_, name, category, status,
                json.dumps(files) if files else None,
                json.dumps(missing) if missing else None,
                json.dumps(depends_on) if depends_on else None
            ))

    def get_component(self, id_: str) -> Optional[Dict[str, Any]]:
        """Get a component by ID."""
        with self._connection() as conn:
            row = conn.execute(
                "SELECT * FROM components WHERE id = ?", (id_,)
            ).fetchone()
            if row:
                result = dict(row)
                for field in ['files', 'missing', 'depends_on']:
                    if result.get(field):
                        result[field] = json.loads(result[field])
                return result
            return None

    def get_all_components(self) -> List[Dict[str, Any]]:
        """Get all components."""
        with self._connection() as conn:
            rows = conn.execute("SELECT * FROM components ORDER BY category, name").fetchall()
            results = []
            for row in rows:
                result = dict(row)
                for field in ['files', 'missing', 'depends_on']:
                    if result.get(field):
                        result[field] = json.loads(result[field])
                results.append(result)
            return results

    # =========================================================================
    # Exit Handlers
    # =========================================================================

    def upsert_exit_handler(
        self,
        reason: int,
        name: str,
        status: str,
        file: Optional[str] = None,
        line: Optional[int] = None,
        has_stealth: bool = False,
        has_timing: bool = False,
        notes: Optional[str] = None
    ):
        """Insert or update exit handler tracking."""
        with self._connection() as conn:
            conn.execute("""
                INSERT INTO exit_handlers (reason, name, status, file, line, has_stealth, has_timing, notes)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?)
                ON CONFLICT(reason) DO UPDATE SET
                    name = excluded.name,
                    status = excluded.status,
                    file = COALESCE(excluded.file, file),
                    line = COALESCE(excluded.line, line),
                    has_stealth = excluded.has_stealth,
                    has_timing = excluded.has_timing,
                    notes = COALESCE(excluded.notes, notes),
                    updated_at = CURRENT_TIMESTAMP
            """, (reason, name, status, file, line, has_stealth, has_timing, notes))

    def get_exit_handlers(self, status: Optional[str] = None) -> List[Dict[str, Any]]:
        """Get exit handlers, optionally filtered by status."""
        with self._connection() as conn:
            if status:
                rows = conn.execute(
                    "SELECT * FROM exit_handlers WHERE status = ? ORDER BY reason",
                    (status,)
                ).fetchall()
            else:
                rows = conn.execute(
                    "SELECT * FROM exit_handlers ORDER BY reason"
                ).fetchall()
            return [dict(row) for row in rows]

    # =========================================================================
    # VMCS Field Usage
    # =========================================================================

    def update_vmcs_usage(
        self,
        encoding: str,
        name: str,
        is_read: bool = False,
        is_written: bool = False,
        location: Optional[str] = None
    ):
        """Track VMCS field read/write locations."""
        with self._connection() as conn:
            existing = conn.execute(
                "SELECT * FROM vmcs_field_usage WHERE encoding = ?", (encoding,)
            ).fetchone()

            if existing:
                read_locs = json.loads(existing['read_locations'] or '[]')
                write_locs = json.loads(existing['write_locations'] or '[]')

                if is_read and location and location not in read_locs:
                    read_locs.append(location)
                if is_written and location and location not in write_locs:
                    write_locs.append(location)

                conn.execute("""
                    UPDATE vmcs_field_usage SET
                        is_read = is_read OR ?,
                        is_written = is_written OR ?,
                        read_locations = ?,
                        write_locations = ?,
                        updated_at = CURRENT_TIMESTAMP
                    WHERE encoding = ?
                """, (
                    is_read, is_written,
                    json.dumps(read_locs), json.dumps(write_locs),
                    encoding
                ))
            else:
                conn.execute("""
                    INSERT INTO vmcs_field_usage
                    (encoding, name, is_read, is_written, read_locations, write_locations)
                    VALUES (?, ?, ?, ?, ?, ?)
                """, (
                    encoding, name, is_read, is_written,
                    json.dumps([location]) if is_read and location else '[]',
                    json.dumps([location]) if is_written and location else '[]'
                ))

    # =========================================================================
    # Decisions
    # =========================================================================

    def add_decision(
        self,
        topic: str,
        choice: str,
        rationale: Optional[str] = None,
        alternatives: Optional[List[str]] = None,
        affects: Optional[List[str]] = None
    ) -> str:
        """Add a design decision. Returns the decision ID."""
        with self._connection() as conn:
            # Generate next ID
            row = conn.execute(
                "SELECT MAX(CAST(SUBSTR(id, 2) AS INTEGER)) as max_id FROM decisions"
            ).fetchone()
            next_num = (row['max_id'] or 0) + 1
            decision_id = f"D{next_num:03d}"

            conn.execute("""
                INSERT INTO decisions (id, date, topic, choice, rationale, alternatives, affects)
                VALUES (?, date('now'), ?, ?, ?, ?, ?)
            """, (
                decision_id, topic, choice, rationale,
                json.dumps(alternatives) if alternatives else None,
                json.dumps(affects) if affects else None
            ))
            return decision_id

    def get_decision(self, id_: str) -> Optional[Dict[str, Any]]:
        """Get a decision by ID."""
        with self._connection() as conn:
            row = conn.execute(
                "SELECT * FROM decisions WHERE id = ?", (id_,)
            ).fetchone()
            if row:
                result = dict(row)
                for field in ['alternatives', 'affects']:
                    if result.get(field):
                        result[field] = json.loads(result[field])
                return result
            return None

    def list_decisions(
        self,
        topic: Optional[str] = None,
        affects: Optional[str] = None
    ) -> List[Dict[str, Any]]:
        """List decisions with optional filters."""
        conditions = []
        params = []

        if topic:
            conditions.append("topic LIKE ?")
            params.append(f"%{topic}%")

        if affects:
            conditions.append("affects LIKE ?")
            params.append(f"%{affects}%")

        where = "WHERE " + " AND ".join(conditions) if conditions else ""

        with self._connection() as conn:
            rows = conn.execute(f"""
                SELECT * FROM decisions {where} ORDER BY date DESC
            """, params).fetchall()

            results = []
            for row in rows:
                result = dict(row)
                for field in ['alternatives', 'affects']:
                    if result.get(field):
                        result[field] = json.loads(result[field])
                results.append(result)
            return results

    # =========================================================================
    # Gotchas
    # =========================================================================

    def add_gotcha(
        self,
        symptom: str,
        cause: str,
        fix: str,
        files: Optional[List[str]] = None
    ) -> str:
        """Add a gotcha (solved bug). Returns the gotcha ID."""
        with self._connection() as conn:
            row = conn.execute(
                "SELECT MAX(CAST(SUBSTR(id, 2) AS INTEGER)) as max_id FROM gotchas"
            ).fetchone()
            next_num = (row['max_id'] or 0) + 1
            gotcha_id = f"G{next_num:03d}"

            conn.execute("""
                INSERT INTO gotchas (id, symptom, cause, fix, files)
                VALUES (?, ?, ?, ?, ?)
            """, (gotcha_id, symptom, cause, fix, json.dumps(files) if files else None))
            return gotcha_id

    def search_gotchas(self, keyword: str) -> List[Dict[str, Any]]:
        """Search gotchas by keyword."""
        with self._connection() as conn:
            rows = conn.execute("""
                SELECT * FROM gotchas
                WHERE symptom LIKE ? OR cause LIKE ? OR fix LIKE ?
                ORDER BY created_at DESC
            """, (f"%{keyword}%", f"%{keyword}%", f"%{keyword}%")).fetchall()

            results = []
            for row in rows:
                result = dict(row)
                if result.get('files'):
                    result['files'] = json.loads(result['files'])
                results.append(result)
            return results

    # =========================================================================
    # Sessions
    # =========================================================================

    def start_session(self) -> int:
        """Start a new session. Returns session ID."""
        with self._connection() as conn:
            cursor = conn.execute("""
                INSERT INTO sessions (started_at) VALUES (CURRENT_TIMESTAMP)
            """)
            return cursor.lastrowid

    def update_session(
        self,
        session_id: int,
        working_on: Optional[str] = None,
        context_snapshot: Optional[str] = None,
        files_touched: Optional[List[str]] = None
    ):
        """Update current session."""
        with self._connection() as conn:
            updates = []
            params = []

            if working_on is not None:
                updates.append("working_on = ?")
                params.append(working_on)

            if context_snapshot is not None:
                updates.append("context_snapshot = ?")
                params.append(context_snapshot)

            if files_touched is not None:
                updates.append("files_touched = ?")
                params.append(json.dumps(files_touched))

            if updates:
                params.append(session_id)
                conn.execute(f"""
                    UPDATE sessions SET {', '.join(updates)} WHERE id = ?
                """, params)

    def end_session(self, session_id: int):
        """End a session."""
        with self._connection() as conn:
            conn.execute("""
                UPDATE sessions SET ended_at = CURRENT_TIMESTAMP WHERE id = ?
            """, (session_id,))

    def get_last_session(self) -> Optional[Dict[str, Any]]:
        """Get the most recent completed session."""
        with self._connection() as conn:
            row = conn.execute("""
                SELECT * FROM sessions
                WHERE ended_at IS NOT NULL
                ORDER BY ended_at DESC LIMIT 1
            """).fetchone()

            if row:
                result = dict(row)
                for field in ['files_touched', 'decisions_made', 'findings_addressed']:
                    if result.get(field):
                        result[field] = json.loads(result[field])
                return result
            return None

    # =========================================================================
    # Daemon State
    # =========================================================================

    def set_state(self, key: str, value: Any):
        """Set daemon state value."""
        with self._connection() as conn:
            conn.execute("""
                INSERT INTO daemon_state (key, value, updated_at)
                VALUES (?, ?, CURRENT_TIMESTAMP)
                ON CONFLICT(key) DO UPDATE SET
                    value = excluded.value,
                    updated_at = CURRENT_TIMESTAMP
            """, (key, json.dumps(value)))

    def get_state(self, key: str, default: Any = None) -> Any:
        """Get daemon state value."""
        with self._connection() as conn:
            row = conn.execute(
                "SELECT value FROM daemon_state WHERE key = ?", (key,)
            ).fetchone()
            if row:
                return json.loads(row['value'])
            return default

    # =========================================================================
    # Suggestions
    # =========================================================================

    def add_suggestion(
        self,
        priority: str,
        type_: str,
        message: str,
        component: Optional[str] = None,
        action: Optional[str] = None
    ) -> int:
        """Add a suggestion. Returns suggestion ID."""
        with self._connection() as conn:
            # Check for duplicate
            existing = conn.execute("""
                SELECT id FROM suggestions
                WHERE message = ? AND NOT acted_on
            """, (message,)).fetchone()

            if existing:
                return existing['id']

            cursor = conn.execute("""
                INSERT INTO suggestions (priority, type, component, message, action)
                VALUES (?, ?, ?, ?, ?)
            """, (priority, type_, component, message, action))
            return cursor.lastrowid

    def get_suggestions(
        self,
        priority: Optional[str] = None,
        limit: int = 10
    ) -> List[Dict[str, Any]]:
        """Get pending suggestions."""
        conditions = ["NOT acted_on"]
        params = []

        if priority:
            conditions.append("priority = ?")
            params.append(priority)

        where = "WHERE " + " AND ".join(conditions)

        with self._connection() as conn:
            rows = conn.execute(f"""
                SELECT * FROM suggestions {where}
                ORDER BY
                    CASE priority
                        WHEN 'high' THEN 1
                        WHEN 'medium' THEN 2
                        ELSE 3
                    END,
                    created_at DESC
                LIMIT ?
            """, params + [limit]).fetchall()
            return [dict(row) for row in rows]

    def mark_suggestion_acted(self, suggestion_id: int):
        """Mark a suggestion as acted upon."""
        with self._connection() as conn:
            conn.execute("""
                UPDATE suggestions SET acted_on = 1 WHERE id = ?
            """, (suggestion_id,))

    # =========================================================================
    # Statistics
    # =========================================================================

    def get_project_health(self) -> Dict[str, Any]:
        """Get overall project health summary."""
        with self._connection() as conn:
            findings = conn.execute("""
                SELECT severity, COUNT(*) as count
                FROM findings WHERE NOT dismissed
                GROUP BY severity
            """).fetchall()

            components = conn.execute("""
                SELECT status, COUNT(*) as count
                FROM components
                GROUP BY status
            """).fetchall()

            handlers = conn.execute("""
                SELECT status, COUNT(*) as count
                FROM exit_handlers
                GROUP BY status
            """).fetchall()

            return {
                "findings": {row['severity']: row['count'] for row in findings},
                "components": {row['status']: row['count'] for row in components},
                "exit_handlers": {row['status']: row['count'] for row in handlers},
                "total_findings": sum(row['count'] for row in findings),
                "critical_count": next(
                    (row['count'] for row in findings if row['severity'] == 'critical'), 0
                )
            }
