"""
Tool registry for managing Unreal Editor tools.

This module provides the registry pattern for managing and registering
editor tools with the FastMCP server.
"""

import inspect
import logging
from typing import Dict, List

from mcp.server.fastmcp import FastMCP

from .base import EditorTool

logger = logging.getLogger("UnrealEditorMCP")


class ToolRegistry:
    """Registry for managing editor tools.

    The registry maintains a collection of tools and provides methods
    to register them with the FastMCP server.
    """

    def __init__(self):
        """Initialize the tool registry."""
        self._tools: Dict[str, EditorTool] = {}

    def register_tool(self, tool: EditorTool) -> None:
        """Register a new tool.

        Args:
            tool: Tool instance to register
        """
        if not isinstance(tool, EditorTool):
            logger.warning(f"Attempted to register invalid tool: {tool}")
            return

        tool_name = tool.name
        if tool_name in self._tools:
            logger.warning(f"Tool '{tool_name}' is already registered, overwriting")

        self._tools[tool_name] = tool
        logger.info(f"Registered tool '{tool_name}'")

    def get_tool(self, tool_name: str) -> EditorTool:
        """Get a tool by name.

        Args:
            tool_name: Name of the tool to retrieve

        Returns:
            Tool instance, or None if not found
        """
        return self._tools.get(tool_name)

    def get_all_tools(self) -> List[EditorTool]:
        """Get all registered tools.

        Returns:
            List of all tool instances
        """
        return list(self._tools.values())

    def has_tool(self, tool_name: str) -> bool:
        """Check if a tool exists.

        Args:
            tool_name: Name of the tool to check

        Returns:
            True if tool exists, False otherwise
        """
        return tool_name in self._tools

    def get_tool_count(self) -> int:
        """Get the number of registered tools.

        Returns:
            Number of tools
        """
        return len(self._tools)

    def register_with_mcp(self, mcp: FastMCP) -> None:
        """Register all tools with the FastMCP server.

        This method creates MCP tool decorators for all registered tools,
        enabling them to be called via the MCP protocol.

        Args:
            mcp: FastMCP server instance
        """
        for tool in self._tools.values():
            # Create a wrapper function that calls the tool's execute method
            # We need to create a proper function with the same signature as execute
            def make_tool_handler(tool_instance: EditorTool):
                # Get the signature of the execute method
                sig = inspect.signature(tool_instance.execute)

                # Remove 'self' parameter from signature if present
                params = [
                    param for name, param in sig.parameters.items()
                    if name != 'self'
                ]

                # Create a new signature without 'self'
                new_sig = sig.replace(parameters=params)

                # Create a wrapper function with the correct signature
                def wrapper(**kwargs):
                    return tool_instance.execute(**kwargs)

                # Set function metadata
                wrapper.__name__ = tool_instance.name
                wrapper.__doc__ = tool_instance.description
                wrapper.__signature__ = new_sig
                wrapper.__annotations__ = {
                    name: param.annotation
                    for name, param in new_sig.parameters.items()
                }
                if sig.return_annotation != inspect.Signature.empty:
                    wrapper.__annotations__['return'] = sig.return_annotation

                return wrapper

            # Get the handler function
            handler = make_tool_handler(tool)

            # Register with MCP using the decorator
            mcp.tool()(handler)

        logger.info(f"Registered {self.get_tool_count()} tools with FastMCP server")
