"""
Ghidra integration module.

Provides bridge to GhidraMCP server for bidirectional sync between
database and Ghidra reverse engineering environment.
"""

from .bridge import GhidraMCPClient

__all__ = ["GhidraMCPClient"]
