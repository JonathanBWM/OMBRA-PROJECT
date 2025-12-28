"""
MCP Tool Modules

All tools are organized by category:
- component_tools: Component registration and status
- feature_tools: Feature management and status tracking
- file_tools: File indexing and search
- function_tools: Function indexing and lookup
- stub_tools: Stub detection and tracking
- task_tools: Task and epic management
- analysis_tools: Codebase analysis and recommendations
- impact_tools: Impact analysis and dependency tracking
- dashboard_tools: Dashboard data aggregation
- driver_tools: Driver scanner integration and analysis
- sync_tools: Cross-MCP synchronization
"""

from .component_tools import COMPONENT_TOOLS
from .feature_tools import FEATURE_TOOLS
from .file_tools import FILE_TOOLS
from .function_tools import FUNCTION_TOOLS
from .stub_tools import STUB_TOOLS
from .task_tools import TASK_TOOLS
from .analysis_tools import ANALYSIS_TOOLS
from .impact_tools import IMPACT_TOOLS
from .dashboard_tools import DASHBOARD_TOOLS
from .driver_tools import DRIVER_TOOLS

# Aggregate all tools
ALL_TOOLS = {}
ALL_TOOLS.update(COMPONENT_TOOLS)
ALL_TOOLS.update(FEATURE_TOOLS)
ALL_TOOLS.update(FILE_TOOLS)
ALL_TOOLS.update(FUNCTION_TOOLS)
ALL_TOOLS.update(STUB_TOOLS)
ALL_TOOLS.update(TASK_TOOLS)
ALL_TOOLS.update(ANALYSIS_TOOLS)
ALL_TOOLS.update(IMPACT_TOOLS)
ALL_TOOLS.update(DASHBOARD_TOOLS)
ALL_TOOLS.update(DRIVER_TOOLS)

__all__ = [
    "ALL_TOOLS",
    "COMPONENT_TOOLS",
    "FEATURE_TOOLS",
    "FILE_TOOLS",
    "FUNCTION_TOOLS",
    "STUB_TOOLS",
    "TASK_TOOLS",
    "ANALYSIS_TOOLS",
    "IMPACT_TOOLS",
    "DASHBOARD_TOOLS",
    "DRIVER_TOOLS",
]
