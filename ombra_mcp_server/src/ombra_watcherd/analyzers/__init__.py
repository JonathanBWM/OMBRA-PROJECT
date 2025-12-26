"""
Analyzers package - pluggable code analysis modules

Each analyzer examines code for specific concerns:
- hypervisor: VMCS, EPT, exit handlers, stealth
- consistency: function signatures, unused code
- security: signature exposure, hardcoded values
"""

from .base import BaseAnalyzer
from .hypervisor import HypervisorAnalyzer
from .consistency import ConsistencyAnalyzer
from .security import SecurityAnalyzer

from typing import List
from ..database import ProjectBrainDB


def get_all_analyzers(db: ProjectBrainDB) -> List[BaseAnalyzer]:
    """Get all available analyzers."""
    return [
        HypervisorAnalyzer(db),
        ConsistencyAnalyzer(db),
        SecurityAnalyzer(db),
    ]


__all__ = [
    "BaseAnalyzer",
    "HypervisorAnalyzer",
    "ConsistencyAnalyzer",
    "SecurityAnalyzer",
    "get_all_analyzers",
]
