"""
Analyzers package - pluggable code analysis modules

Each analyzer examines code for specific concerns:
- hypervisor: VMCS, EPT, exit handlers, stealth
- consistency: function signatures, unused code
- security: signature exposure, hardcoded values
- concept: pattern-based concept implementation detection
"""

from .base import BaseAnalyzer
from .hypervisor import HypervisorAnalyzer
from .consistency import ConsistencyAnalyzer
from .security import SecurityAnalyzer
from .concept import ConceptAnalyzer

from typing import List
from ..database import ProjectBrainDB


def get_all_analyzers(db: ProjectBrainDB) -> List[BaseAnalyzer]:
    """Get all available analyzers."""
    analyzers = [
        HypervisorAnalyzer(db),
        ConsistencyAnalyzer(db),
        SecurityAnalyzer(db),
    ]

    # ConceptAnalyzer uses separate ConceptsDB
    concept_analyzer = ConceptAnalyzer()
    analyzers.append(concept_analyzer)

    return analyzers


__all__ = [
    "BaseAnalyzer",
    "HypervisorAnalyzer",
    "ConsistencyAnalyzer",
    "SecurityAnalyzer",
    "ConceptAnalyzer",
    "get_all_analyzers",
]
