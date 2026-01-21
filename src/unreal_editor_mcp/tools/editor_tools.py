"""
Editor Tools for UnrealEditorMCP.

This module provides the registration function for editor tools using
the Command pattern with a tool registry.
"""

import logging

from mcp.server.fastmcp import FastMCP

from .registry import ToolRegistry
from .ping_tool import PingTool
from .get_actors_tool import GetActorsInLevelTool
from .execute_python_tool import ExecutePythonTool

logger = logging.getLogger("UnrealEditorMCP")


def register_editor_tools(mcp: FastMCP):
    """Register editor tools with the MCP server.

    This function creates a tool registry, registers all available tools,
    and then registers them with the FastMCP server.

    To add a new tool:
    1. Create a new tool class that inherits from EditorTool
    2. Add the import statement above
    3. Add registry.register_tool(YourNewTool()) below

    Args:
        mcp: FastMCP server instance
    """
    # Create tool registry
    registry = ToolRegistry()
    logger.info(f"Registering editor tools")

    # Register all tools
    registry.register_tool(PingTool())
    registry.register_tool(GetActorsInLevelTool())
    registry.register_tool(ExecutePythonTool())

    # Register all tools with FastMCP
    registry.register_with_mcp(mcp)

    logger.info(f"Registered {registry.get_tool_count()} editor tools")
