"""
OMBRA Project Management MCP Server

The external brain for agent intelligence.

This MCP server provides 45+ tools for:
- File and function indexing
- Feature-to-file linkage
- Stub detection and tracking
- Task and epic management
- Impact analysis
- Codebase health metrics
- Dashboard data aggregation

Entry point: project-mcp command or python -m project_mcp.server
"""

import asyncio
import json
import logging
from typing import Any, Callable, Coroutine

from mcp.server import Server
from mcp.types import Tool, TextContent
from pydantic import BaseModel

from .database import sync_init_db, close_connection
from .tools import ALL_TOOLS

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Create MCP server instance
server = Server("project-mcp")


def create_tool_schema(func: Callable) -> dict:
    """Extract JSON schema from function annotations and docstring."""
    import inspect

    sig = inspect.signature(func)
    properties = {}
    required = []

    for name, param in sig.parameters.items():
        if name in ('self', 'cls'):
            continue

        prop = {"type": "string"}  # Default type

        # Infer type from annotation
        if param.annotation != inspect.Parameter.empty:
            ann = param.annotation
            if ann == int:
                prop = {"type": "integer"}
            elif ann == float:
                prop = {"type": "number"}
            elif ann == bool:
                prop = {"type": "boolean"}
            elif ann == str:
                prop = {"type": "string"}
            elif hasattr(ann, '__origin__'):
                # Handle Optional, List, etc.
                origin = getattr(ann, '__origin__', None)
                if origin is list:
                    prop = {"type": "array", "items": {"type": "string"}}
                elif origin is dict:
                    prop = {"type": "object"}

        # Check if required
        if param.default == inspect.Parameter.empty:
            required.append(name)

        # Add description from parameter name
        prop["description"] = name.replace("_", " ").title()
        properties[name] = prop

    return {
        "type": "object",
        "properties": properties,
        "required": required,
    }


@server.list_tools()
async def list_tools() -> list[Tool]:
    """List all available tools."""
    tools = []

    for name, func in ALL_TOOLS.items():
        # Get docstring for description
        doc = func.__doc__ or f"{name} tool"
        description = doc.split('\n')[0].strip()

        # Create schema from function
        schema = create_tool_schema(func)

        tools.append(Tool(
            name=name,
            description=description,
            inputSchema=schema,
        ))

    return tools


@server.call_tool()
async def call_tool(name: str, arguments: dict) -> list[TextContent]:
    """Execute a tool and return results."""
    if name not in ALL_TOOLS:
        return [TextContent(
            type="text",
            text=json.dumps({
                "error": f"Unknown tool: {name}",
                "available_tools": list(ALL_TOOLS.keys()),
            }, indent=2)
        )]

    func = ALL_TOOLS[name]

    try:
        # Call the async function
        result = await func(**arguments)

        # Convert result to JSON
        if isinstance(result, (dict, list)):
            text = json.dumps(result, indent=2, default=str)
        else:
            text = str(result)

        return [TextContent(type="text", text=text)]

    except TypeError as e:
        # Argument error
        return [TextContent(
            type="text",
            text=json.dumps({
                "error": f"Invalid arguments for {name}: {str(e)}",
                "tool": name,
            }, indent=2)
        )]
    except Exception as e:
        # General error
        logger.exception(f"Error executing tool {name}")
        return [TextContent(
            type="text",
            text=json.dumps({
                "error": str(e),
                "tool": name,
                "type": type(e).__name__,
            }, indent=2)
        )]


async def run_server():
    """Run the MCP server."""
    from mcp.server.stdio import stdio_server

    # Initialize database on startup
    logger.info("Initializing Project Management MCP database...")
    sync_init_db()
    logger.info(f"Database initialized. Loaded {len(ALL_TOOLS)} tools.")

    # Run the server
    async with stdio_server() as (read_stream, write_stream):
        await server.run(
            read_stream,
            write_stream,
            server.create_initialization_options(),
        )

    # Cleanup
    await close_connection()


def main():
    """Entry point for the project-mcp command."""
    try:
        asyncio.run(run_server())
    except KeyboardInterrupt:
        logger.info("Server shutdown requested")
    except Exception as e:
        logger.exception("Server error")
        raise


if __name__ == "__main__":
    main()
