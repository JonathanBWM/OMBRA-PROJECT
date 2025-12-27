#!/usr/bin/env python3
"""
Test the project status summary function
"""

import sys
from pathlib import Path

# Add src to path
sys.path.insert(0, str(Path(__file__).parent / "src"))

from ombra_mcp.tools.code_generator import get_project_implementation_status

if __name__ == "__main__":
    status = get_project_implementation_status()
    print(status["summary"])
