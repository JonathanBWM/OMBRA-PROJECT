"""
OmbraWatcherd - Living codebase intelligence daemon

A background service that watches the PROJECT-OMBRA codebase,
analyzes changes in real-time, and maintains project state.
"""

__version__ = "0.1.0"

from .daemon import OmbraWatcherd
from .database import ProjectBrainDB

__all__ = ["OmbraWatcherd", "ProjectBrainDB"]
