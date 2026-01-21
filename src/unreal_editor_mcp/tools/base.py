"""
Base class for Unreal Editor tools.

This module provides the abstract base class for implementing editor tools
using the Command pattern.
"""

import logging
from abc import ABC, abstractmethod
from typing import Dict, Any, Optional

from ..connection import get_connection

logger = logging.getLogger("UnrealEditorMCP")


class EditorTool(ABC):
    """Abstract base class for editor tools.

    Each tool represents a command that can be executed on the Unreal Engine
    via the HTTP REST API. Tools are registered with the MCP server and can
    be invoked by clients.
    """

    @property
    @abstractmethod
    def name(self) -> str:
        """Get the tool name (used in MCP registration).

        Returns:
            Tool name (e.g., "ping", "get_actors_in_level")
        """
        pass

    @property
    @abstractmethod
    def description(self) -> str:
        """Get the tool description (shown in tool lists).

        Returns:
            Human-readable description
        """
        pass

    @abstractmethod
    def execute(self, **kwargs) -> Dict[str, Any]:
        """Execute the tool with given parameters.

        Args:
            **kwargs: Tool-specific parameters

        Returns:
            Dictionary containing the execution result
        """
        pass

    def call_unreal_tool(self, tool_name: str, params: Dict[str, Any] = None) -> Dict[str, Any]:
        """Call an Unreal Engine tool via HTTP API.

        This is a helper method that handles connection management and error handling.

        Args:
            tool_name: Name of the tool to call in Unreal Engine
            params: Optional parameters to pass to the tool

        Returns:
            Dictionary containing:
            - success: Whether the operation succeeded
            - data: Response data from Unreal Engine (if successful)
            - error: Error message (if failed)
        """
        try:
            conn = get_connection()
            if not conn:
                return {
                    "success": False,
                    "error": "Failed to get Unreal connection"
                }

            response = conn.call_tool(tool_name, params or {})

            if not response:
                return {
                    "success": False,
                    "error": "No response from Unreal Engine"
                }

            return response

        except Exception as e:
            logger.error(f"Error calling Unreal tool '{tool_name}': {e}")
            return {
                "success": False,
                "error": str(e)
            }
