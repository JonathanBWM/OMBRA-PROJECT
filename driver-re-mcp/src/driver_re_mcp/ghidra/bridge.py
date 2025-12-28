"""
Ghidra MCP Client Bridge

Connects to GhidraMCP server running on localhost:8080 (HTTP → MCP bridge)
Provides async interface to all Ghidra reverse engineering operations.
"""

import asyncio
import json
from typing import Dict, List, Optional, Any
import aiohttp


class GhidraMCPClient:
    """
    Client for communicating with GhidraMCP server.

    GhidraMCP runs as HTTP bridge on port 8080 by default.
    Translates HTTP requests → MCP tool calls → Ghidra Python API.
    """

    def __init__(self, host: str = "localhost", port: int = 8080):
        self.host = host
        self.port = port
        self.base_url = f"http://{host}:{port}"
        self._session: Optional[aiohttp.ClientSession] = None
        self._connected = False

    async def connect(self) -> bool:
        """
        Establish connection to GhidraMCP server.

        Returns:
            True if connection successful
        """
        try:
            self._session = aiohttp.ClientSession()
            # Test connection with a simple call
            response = await self._session.get(f"{self.base_url}/health")
            if response.status == 200:
                self._connected = True
                return True
            return False
        except Exception as e:
            print(f"Failed to connect to GhidraMCP: {e}")
            return False

    async def disconnect(self):
        """Close connection to GhidraMCP server."""
        if self._session:
            await self._session.close()
            self._session = None
            self._connected = False

    async def call_tool(self, tool_name: str, arguments: Dict[str, Any]) -> Dict:
        """
        Call a tool on the GhidraMCP server.

        Args:
            tool_name: Name of the tool (e.g., 'decompile_function')
            arguments: Tool arguments as dict

        Returns:
            Tool response as dict
        """
        if not self._connected or not self._session:
            raise RuntimeError("Not connected to GhidraMCP. Call connect() first.")

        try:
            response = await self._session.post(
                f"{self.base_url}/tools/{tool_name}",
                json=arguments
            )
            response.raise_for_status()
            return await response.json()
        except Exception as e:
            print(f"GhidraMCP tool call failed ({tool_name}): {e}")
            return {"error": str(e)}

    async def get_function_at(self, address: int) -> Optional[Dict]:
        """
        Get function at specified address.

        Args:
            address: RVA or VA of function

        Returns:
            Function info: {name, address, size, ...}
        """
        result = await self.call_tool("get_function_by_address", {
            "address": hex(address)
        })
        return result if not result.get("error") else None

    async def get_decompilation(self, address: int) -> Optional[str]:
        """
        Get decompiled code for function at address.

        Args:
            address: RVA or VA of function

        Returns:
            Decompiled C code as string
        """
        result = await self.call_tool("decompile_function_by_address", {
            "address": hex(address)
        })
        return result.get("decompiled") if result and not result.get("error") else None

    async def get_xrefs_to(self, address: int) -> List[Dict]:
        """
        Get cross-references TO an address.

        Args:
            address: Target address

        Returns:
            List of xrefs: [{from_address, type, context}, ...]
        """
        result = await self.call_tool("get_xrefs_to", {
            "address": hex(address)
        })
        return result.get("xrefs", []) if result else []

    async def get_xrefs_from(self, address: int) -> List[Dict]:
        """
        Get cross-references FROM an address.

        Args:
            address: Source address

        Returns:
            List of xrefs: [{to_address, type, context}, ...]
        """
        result = await self.call_tool("get_xrefs_from", {
            "address": hex(address)
        })
        return result.get("xrefs", []) if result else []

    async def rename_function(self, address: int, name: str) -> bool:
        """
        Rename function in Ghidra.

        Args:
            address: Function address
            name: New function name

        Returns:
            True if successful
        """
        result = await self.call_tool("rename_function_by_address", {
            "address": hex(address),
            "new_name": name
        })
        return result.get("success", False) if result else False

    async def set_comment(self, address: int, comment: str, comment_type: str = "eol") -> bool:
        """
        Set comment at address in Ghidra.

        Args:
            address: Address for comment
            comment: Comment text
            comment_type: 'eol', 'pre', 'post', or 'plate'

        Returns:
            True if successful
        """
        # GhidraMCP uses separate tools for decompiler vs disassembly comments
        tool_name = "set_decompiler_comment" if comment_type in ["eol", "pre"] else "set_disassembly_comment"

        result = await self.call_tool(tool_name, {
            "address": hex(address),
            "comment": comment
        })
        return result.get("success", False) if result else False

    async def get_all_functions(self) -> List[Dict]:
        """
        Get all functions from Ghidra.

        Returns:
            List of functions: [{name, address, size}, ...]
        """
        result = await self.call_tool("list_functions", {})
        return result.get("functions", []) if result else []

    async def get_data_types(self) -> List[Dict]:
        """
        Get all data types/structures from Ghidra.

        Returns:
            List of data types
        """
        # GhidraMCP doesn't have direct data types listing yet
        # Use list_data_items as fallback
        result = await self.call_tool("list_data_items", {})
        return result.get("data_items", []) if result else []

    async def create_struct(self, name: str, size: int, fields: List[Dict]) -> bool:
        """
        Create a structure in Ghidra.

        Args:
            name: Structure name
            size: Structure size in bytes
            fields: List of fields [{name, offset, type, size}, ...]

        Returns:
            True if successful
        """
        # This would require extending GhidraMCP with struct creation
        # For now, return False as not implemented
        print(f"Warning: Structure creation not yet implemented in GhidraMCP")
        return False

    async def get_imports(self) -> List[Dict]:
        """
        Get all imports from current program.

        Returns:
            List of imports: [{name, address, ordinal}, ...]
        """
        result = await self.call_tool("list_imports", {})
        return result.get("imports", []) if result else []

    async def get_exports(self) -> List[Dict]:
        """
        Get all exports from current program.

        Returns:
            List of exports: [{name, address, ordinal}, ...]
        """
        result = await self.call_tool("list_exports", {})
        return result.get("exports", []) if result else []

    async def get_segments(self) -> List[Dict]:
        """
        Get all memory segments/sections.

        Returns:
            List of segments: [{name, start, end, permissions}, ...]
        """
        result = await self.call_tool("list_segments", {})
        return result.get("segments", []) if result else []

    async def get_strings(self) -> List[Dict]:
        """
        Get all strings from current program.

        Returns:
            List of strings: [{value, address, length}, ...]
        """
        result = await self.call_tool("list_strings", {})
        return result.get("strings", []) if result else []

    async def disassemble_function(self, address: int) -> Optional[str]:
        """
        Get disassembly for function at address.

        Args:
            address: Function address

        Returns:
            Disassembly as string
        """
        result = await self.call_tool("disassemble_function", {
            "address": hex(address)
        })
        return result.get("disassembly") if result and not result.get("error") else None

    async def rename_variable(self, function_address: int, variable_name: str, new_name: str) -> bool:
        """
        Rename a local variable in a function.

        Args:
            function_address: Function containing the variable
            variable_name: Current variable name
            new_name: New variable name

        Returns:
            True if successful
        """
        result = await self.call_tool("rename_variable", {
            "function_address": hex(function_address),
            "old_name": variable_name,
            "new_name": new_name
        })
        return result.get("success", False) if result else False

    async def get_current_program(self) -> Optional[Dict]:
        """
        Get information about currently open program in Ghidra.

        Returns:
            Program info: {name, path, language, compiler, ...}
        """
        result = await self.call_tool("get_current_program", {})
        return result if result and not result.get("error") else None

    async def search_functions_by_name(self, pattern: str) -> List[Dict]:
        """
        Search for functions matching name pattern.

        Args:
            pattern: Regex pattern to match function names

        Returns:
            List of matching functions
        """
        result = await self.call_tool("search_functions_by_name", {
            "pattern": pattern
        })
        return result.get("functions", []) if result else []

    async def get_function_xrefs(self, address: int) -> Dict:
        """
        Get comprehensive xref info for a function.

        Args:
            address: Function address

        Returns:
            Dict with 'callers' and 'callees' lists
        """
        result = await self.call_tool("get_function_xrefs", {
            "address": hex(address)
        })
        return result if result and not result.get("error") else {"callers": [], "callees": []}


async def test_connection():
    """Test GhidraMCP connection."""
    client = GhidraMCPClient()

    print("Connecting to GhidraMCP...")
    if await client.connect():
        print("✓ Connected successfully")

        # Test basic operations
        functions = await client.get_all_functions()
        print(f"✓ Found {len(functions)} functions")

        await client.disconnect()
        print("✓ Disconnected")
    else:
        print("✗ Connection failed")


if __name__ == "__main__":
    asyncio.run(test_connection())
