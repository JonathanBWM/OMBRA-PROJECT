"""
Launchd Service Manager - macOS service lifecycle management

Handles:
- Installing/uninstalling launchd plist
- Starting/stopping the daemon
- Checking daemon status
"""

import os
import subprocess
import plistlib
from pathlib import Path
from typing import Dict, Any, Optional

from .database import ProjectBrainDB, DEFAULT_DB_PATH


PLIST_PATH = Path.home() / "Library" / "LaunchAgents" / "com.ombra.watcherd.plist"
LOG_PATH = Path.home() / "Library" / "Logs" / "ombra-watcherd.log"

# Auto-detect project root from package location or environment variable
# __file__ is src/ombra_watcherd/launchd.py -> .parent.parent.parent.parent = PROJECT-OMBRA
def _get_project_root() -> Path:
    """Detect project root via environment or package location."""
    if "OMBRA_PROJECT_ROOT" in os.environ:
        return Path(os.environ["OMBRA_PROJECT_ROOT"])
    # Fallback: derive from package location (src/ombra_watcherd/launchd.py)
    return Path(__file__).resolve().parent.parent.parent.parent

WATCH_PATH = _get_project_root()

# Get the Python interpreter from the ombra_mcp_server venv
# __file__ is src/ombra_watcherd/launchd.py, so .parent.parent.parent is ombra_mcp_server/
PYTHON_PATH = Path(__file__).parent.parent.parent / ".venv" / "bin" / "python"


class LaunchdManager:
    """
    Manages the ombra-watcherd launchd service.
    """

    def __init__(self):
        self.plist_path = PLIST_PATH
        self.label = "com.ombra.watcherd"

    def _generate_plist(self) -> Dict[str, Any]:
        """Generate launchd plist configuration."""
        return {
            "Label": self.label,
            "ProgramArguments": [
                str(PYTHON_PATH),
                "-m", "ombra_watcherd",
                "run",
                "--watch-path", str(WATCH_PATH),
            ],
            "RunAtLoad": True,
            "KeepAlive": True,
            "StandardOutPath": str(LOG_PATH),
            "StandardErrorPath": str(LOG_PATH),
            "WorkingDirectory": str(WATCH_PATH),
            "EnvironmentVariables": {
                "PYTHONUNBUFFERED": "1",
            },
            "ThrottleInterval": 10,  # Don't restart more than once per 10 seconds
        }

    def install(self) -> bool:
        """Install and start the launchd service."""
        try:
            # Create plist
            self.plist_path.parent.mkdir(parents=True, exist_ok=True)

            plist = self._generate_plist()
            with open(self.plist_path, "wb") as f:
                plistlib.dump(plist, f)

            # Set permissions
            os.chmod(self.plist_path, 0o644)

            # Load the service
            result = subprocess.run(
                ["launchctl", "load", str(self.plist_path)],
                capture_output=True,
                text=True
            )

            if result.returncode != 0:
                # Might already be loaded, try unload then load
                subprocess.run(
                    ["launchctl", "unload", str(self.plist_path)],
                    capture_output=True
                )
                result = subprocess.run(
                    ["launchctl", "load", str(self.plist_path)],
                    capture_output=True,
                    text=True
                )

            return result.returncode == 0

        except Exception as e:
            print(f"Failed to install: {e}")
            return False

    def uninstall(self) -> bool:
        """Stop and remove the launchd service."""
        try:
            # Unload
            subprocess.run(
                ["launchctl", "unload", str(self.plist_path)],
                capture_output=True
            )

            # Remove plist
            if self.plist_path.exists():
                self.plist_path.unlink()

            return True

        except Exception as e:
            print(f"Failed to uninstall: {e}")
            return False

    def start(self) -> bool:
        """Start the daemon."""
        try:
            result = subprocess.run(
                ["launchctl", "start", self.label],
                capture_output=True,
                text=True
            )
            return result.returncode == 0

        except Exception as e:
            print(f"Failed to start: {e}")
            return False

    def stop(self) -> bool:
        """Stop the daemon."""
        try:
            result = subprocess.run(
                ["launchctl", "stop", self.label],
                capture_output=True,
                text=True
            )
            return result.returncode == 0

        except Exception as e:
            print(f"Failed to stop: {e}")
            return False

    def status(self) -> Dict[str, Any]:
        """Get daemon status."""
        status = {
            "state": "unknown",
            "pid": None,
            "installed": self.plist_path.exists(),
        }

        # Check launchctl list
        try:
            result = subprocess.run(
                ["launchctl", "list", self.label],
                capture_output=True,
                text=True
            )

            if result.returncode == 0:
                # Parse output
                lines = result.stdout.strip().split("\n")
                if len(lines) > 0:
                    # Format: PID\tStatus\tLabel
                    # or just label info if not running
                    for line in lines:
                        if self.label in line:
                            parts = line.split("\t")
                            if len(parts) >= 1 and parts[0].isdigit():
                                status["pid"] = int(parts[0])
                                status["state"] = "running"
                            elif parts[0] == "-":
                                status["state"] = "stopped"
                            break
            else:
                status["state"] = "not_loaded"

        except Exception:
            pass

        # Get last scan info from database
        try:
            db = ProjectBrainDB()
            status["last_scan"] = db.get_state("last_scan")
            health = db.get_project_health()
            status["findings_count"] = health.get("total_findings", 0)
        except Exception:
            pass

        return status

    def get_logs(self, lines: int = 50) -> str:
        """Get recent log output."""
        try:
            if LOG_PATH.exists():
                result = subprocess.run(
                    ["tail", f"-{lines}", str(LOG_PATH)],
                    capture_output=True,
                    text=True
                )
                return result.stdout
        except Exception:
            pass
        return ""
