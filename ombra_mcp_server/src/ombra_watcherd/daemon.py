"""
OmbraWatcherd Daemon - Filesystem watcher and analysis coordinator

Watches the PROJECT-OMBRA directory for changes and triggers
analysis when relevant files are modified.
"""

import logging
import time
import threading
from pathlib import Path
from queue import Queue, Empty
from typing import Optional, Set, Dict, Any, List
from datetime import datetime

from watchdog.observers import Observer
from watchdog.events import (
    FileSystemEventHandler,
    FileCreatedEvent,
    FileModifiedEvent,
    FileDeletedEvent,
    FileMovedEvent
)

from .database import ProjectBrainDB


# Configure logging
LOG_PATH = Path.home() / "Library" / "Logs" / "ombra-watcherd.log"
LOG_PATH.parent.mkdir(parents=True, exist_ok=True)

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler(LOG_PATH),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger("ombra-watcherd")


# Patterns to ignore
IGNORE_PATTERNS = {
    ".git",
    ".venv",
    "venv",
    "__pycache__",
    ".pyc",
    "build",
    "dist",
    ".egg-info",
    ".DS_Store",
    "*.log",
    "*.tmp",
    "node_modules",
}

# Extensions we care about
RELEVANT_EXTENSIONS = {
    ".c", ".h", ".cpp", ".hpp",  # C/C++
    ".asm", ".s",                 # Assembly
    ".py",                        # Python
    ".md",                        # Markdown docs
    ".json",                      # Config
}


class ChangeHandler(FileSystemEventHandler):
    """Handles filesystem events and queues files for analysis."""

    def __init__(self, queue: Queue, watch_path: Path):
        super().__init__()
        self.queue = queue
        self.watch_path = watch_path

    def _should_ignore(self, path: Path) -> bool:
        """Check if path should be ignored."""
        path_str = str(path)

        for pattern in IGNORE_PATTERNS:
            if pattern in path_str:
                return True

        if path.suffix not in RELEVANT_EXTENSIONS:
            return True

        return False

    def _queue_file(self, path: Path, event_type: str):
        """Queue a file for analysis."""
        if not self._should_ignore(path):
            self.queue.put({
                "path": path,
                "event": event_type,
                "time": datetime.now()
            })
            logger.debug(f"Queued {event_type}: {path}")

    def on_created(self, event):
        if not event.is_directory:
            self._queue_file(Path(event.src_path), "created")

    def on_modified(self, event):
        if not event.is_directory:
            self._queue_file(Path(event.src_path), "modified")

    def on_deleted(self, event):
        if not event.is_directory:
            self._queue_file(Path(event.src_path), "deleted")

    def on_moved(self, event):
        if not event.is_directory:
            self._queue_file(Path(event.dest_path), "moved")


class OmbraWatcherd:
    """
    Main daemon class that coordinates watching and analysis.
    """

    def __init__(
        self,
        watch_path: Path = Path("/Users/jonathanmcclintock/PROJECT-OMBRA"),
        db_path: Optional[Path] = None,
        debounce_seconds: float = 0.5
    ):
        self.watch_path = watch_path
        self.db = ProjectBrainDB(db_path)
        self.debounce_seconds = debounce_seconds

        self.queue: Queue = Queue()
        self.observer: Optional[Observer] = None
        self.processor_thread: Optional[threading.Thread] = None
        self.running = False

        # Track pending files for debouncing
        self._pending: Dict[Path, datetime] = {}
        self._pending_lock = threading.Lock()

        # Lazy-load analyzers
        self._analyzers = None

    @property
    def analyzers(self):
        """Lazy-load analyzers to avoid circular imports."""
        if self._analyzers is None:
            from .analyzers import get_all_analyzers
            self._analyzers = get_all_analyzers(self.db)
        return self._analyzers

    def run(self):
        """Start the daemon and run until stopped."""
        logger.info(f"Starting OmbraWatcherd, watching: {self.watch_path}")

        self.running = True
        self.db.set_state("daemon_started", datetime.now().isoformat())
        self.db.set_state("daemon_pid", threading.get_ident())

        # Start file watcher
        self.observer = Observer()
        handler = ChangeHandler(self.queue, self.watch_path)
        self.observer.schedule(handler, str(self.watch_path), recursive=True)
        self.observer.start()

        # Start processor thread
        self.processor_thread = threading.Thread(target=self._process_loop, daemon=True)
        self.processor_thread.start()

        # Initial scan
        logger.info("Running initial scan...")
        self.scan_all()

        # Keep running
        try:
            while self.running:
                time.sleep(1)
        except KeyboardInterrupt:
            self.stop()

    def stop(self):
        """Stop the daemon gracefully."""
        logger.info("Stopping OmbraWatcherd...")
        self.running = False

        if self.observer:
            self.observer.stop()
            self.observer.join(timeout=5)

        self.db.set_state("daemon_stopped", datetime.now().isoformat())
        logger.info("OmbraWatcherd stopped")

    def _process_loop(self):
        """Process queued file changes with debouncing."""
        while self.running:
            try:
                item = self.queue.get(timeout=0.1)
                path = item["path"]
                event_time = item["time"]

                # Debounce: track when we last saw this file
                with self._pending_lock:
                    self._pending[path] = event_time

                # Wait for debounce period
                time.sleep(self.debounce_seconds)

                # Check if this is still the latest event for this file
                with self._pending_lock:
                    if self._pending.get(path) == event_time:
                        del self._pending[path]
                        self._analyze_file(path, item["event"])

            except Empty:
                continue
            except Exception as e:
                logger.error(f"Error processing queue: {e}", exc_info=True)

    def _analyze_file(self, path: Path, event_type: str):
        """Run analyzers on a single file."""
        if event_type == "deleted":
            # Clear findings for deleted file
            self.db.clear_stale_findings(str(path))
            logger.info(f"Cleared findings for deleted file: {path}")
            return

        if not path.exists():
            return

        logger.info(f"Analyzing: {path}")

        # Clear old findings for this file
        self.db.clear_stale_findings(str(path))

        # Run each analyzer
        findings_count = 0
        for analyzer in self.analyzers:
            if analyzer.should_analyze(path):
                try:
                    findings = analyzer.analyze(path)
                    findings_count += len(findings)
                    for finding in findings:
                        self.db.add_finding(
                            file=str(path),
                            severity=finding["severity"],
                            type_=finding["type"],
                            check_id=finding["check_id"],
                            message=finding["message"],
                            line=finding.get("line"),
                            suggested_fix=finding.get("suggested_fix")
                        )
                except Exception as e:
                    logger.error(f"Analyzer {analyzer.name} failed on {path}: {e}")

        if findings_count > 0:
            logger.info(f"Found {findings_count} issues in {path}")

        # Update last scan time
        self.db.set_state("last_scan", datetime.now().isoformat())
        self.db.set_state("last_file_scanned", str(path))

    def scan_all(self) -> Dict[str, Any]:
        """Scan all files in the watch path."""
        logger.info(f"Full scan of {self.watch_path}")

        files_scanned = 0
        total_findings = 0

        for ext in RELEVANT_EXTENSIONS:
            for path in self.watch_path.rglob(f"*{ext}"):
                # Skip ignored paths
                if any(pattern in str(path) for pattern in IGNORE_PATTERNS):
                    continue

                self._analyze_file(path, "scan")
                files_scanned += 1

        # Get summary
        health = self.db.get_project_health()
        total_findings = health["total_findings"]

        self.db.set_state("last_full_scan", datetime.now().isoformat())
        self.db.set_state("files_scanned", files_scanned)

        result = {
            "files_scanned": files_scanned,
            "findings_count": total_findings,
            "critical_count": health["critical_count"],
            "warning_count": health["findings"].get("warning", 0),
            "info_count": health["findings"].get("info", 0),
        }

        logger.info(f"Full scan complete: {files_scanned} files, {total_findings} findings")
        return result

    def scan_path(self, path: Path) -> Dict[str, Any]:
        """Scan a specific file or directory."""
        if path.is_file():
            self._analyze_file(path, "manual_scan")
            return {"files_scanned": 1}

        files_scanned = 0
        for ext in RELEVANT_EXTENSIONS:
            for file_path in path.rglob(f"*{ext}"):
                if any(pattern in str(file_path) for pattern in IGNORE_PATTERNS):
                    continue
                self._analyze_file(file_path, "manual_scan")
                files_scanned += 1

        return {"files_scanned": files_scanned}
