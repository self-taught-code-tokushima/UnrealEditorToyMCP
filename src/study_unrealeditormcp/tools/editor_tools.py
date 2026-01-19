"""
Editor Tools for Study UnrealEditorMCP.

This module provides tools for interacting with the Unreal Editor via HTTP REST API.
"""

import logging
from typing import Dict, Any

from mcp.server.fastmcp import FastMCP

from ..connection import get_connection

logger = logging.getLogger("StudyUnrealEditorMCP")


def register_editor_tools(mcp: FastMCP):
    """Register editor tools with the MCP server."""

    @mcp.tool()
    def ping() -> Dict[str, Any]:
        """Ping the Unreal Engine to check connection.

        Returns:
            Dictionary containing:
            - success: Whether the ping succeeded
            - message: Response message from Unreal Engine
        """
        try:
            conn = get_connection()
            if not conn:
                return {
                    "success": False,
                    "error": "Failed to get Unreal connection"
                }

            response = conn.call_tool("ping", {})

            if not response:
                return {
                    "success": False,
                    "error": "No response from Unreal Engine"
                }

            # Response format: {"success": true, "message": "...", "data": {"message": "pong"}}
            if response.get("success"):
                data = response.get("data", {})
                return {
                    "success": True,
                    "message": data.get("message", "pong")
                }

            return {
                "success": False,
                "error": response.get("error", "Unknown error")
            }

        except Exception as e:
            logger.error(f"Error pinging Unreal: {e}")
            return {
                "success": False,
                "error": str(e)
            }
