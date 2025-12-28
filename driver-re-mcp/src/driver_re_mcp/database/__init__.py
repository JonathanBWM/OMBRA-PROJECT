"""Database layer for Driver RE MCP."""

from .connection import (
    DatabaseManager,
    init_database,
    get_db,
    get_db_session,
    get_db_transaction,
)

from .models import (
    # Driver models
    Driver,
    Section,

    # Import/Export
    Import,
    Export,

    # IOCTL
    IOCTL,

    # Structures
    Structure,
    StructureMember,

    # Functions
    Function,

    # Cross-references
    XRef,

    # Globals
    Global,

    # Strings
    String,

    # Vulnerabilities
    Vulnerability,
    AttackChain,

    # API categories
    APICategory,

    # Analysis
    AnalysisSession,
    AnalysisNote,
)

__all__ = [
    # Connection management
    "DatabaseManager",
    "init_database",
    "get_db",
    "get_db_session",
    "get_db_transaction",

    # Models
    "Driver",
    "Section",
    "Import",
    "Export",
    "IOCTL",
    "Structure",
    "StructureMember",
    "Function",
    "XRef",
    "Global",
    "String",
    "Vulnerability",
    "AttackChain",
    "APICategory",
    "AnalysisSession",
    "AnalysisNote",
]
