"""Driver RE MCP - Windows kernel driver reverse engineering MCP server"""

__version__ = "1.0.0"

from .server import app, main
from .config import settings

__all__ = ['app', 'main', 'settings', '__version__']
