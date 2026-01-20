"""
Unreal Engine connection module.
Handles HTTP REST API communication with the UE5 plugin.
"""

import logging
import os
from typing import Dict, Any, Optional

import httpx

logger = logging.getLogger("UnrealEditorMCP")

# Configuration - can be overridden via environment variables
UNREAL_BASE_URL = os.getenv("UNREAL_BASE_URL", "http://localhost:3000")
REQUEST_TIMEOUT = float(os.getenv("UNREAL_REQUEST_TIMEOUT", "30.0"))


class UnrealConnection:
    """Connection to an Unreal Engine instance via HTTP REST API."""

    def __init__(self, base_url: str = None, timeout: float = None):
        """Initialize the connection.

        Args:
            base_url: Base URL for the Unreal Editor HTTP API (default from env or http://localhost:3000)
            timeout: Request timeout in seconds (default from env or 30.0)
        """
        self.base_url = (base_url or UNREAL_BASE_URL).rstrip("/")
        self.timeout = timeout or REQUEST_TIMEOUT
        self.client: Optional[httpx.Client] = None

    def _get_client(self) -> httpx.Client:
        """Get or create HTTP client."""
        if self.client is None:
            self.client = httpx.Client(timeout=self.timeout)
        return self.client

    def close(self):
        """Close the HTTP client."""
        if self.client:
            self.client.close()
            self.client = None

    def check_status(self) -> Optional[Dict[str, Any]]:
        """Check the server status.

        Returns:
            Status information dictionary, or None on error
        """
        try:
            client = self._get_client()
            response = client.get(f"{self.base_url}/mcp/status")
            response.raise_for_status()
            return response.json()
        except Exception as e:
            logger.error(f"Error checking status: {e}")
            return None

    def list_tools(self) -> Optional[Dict[str, Any]]:
        """List available tools.

        Returns:
            Tools list dictionary, or None on error
        """
        try:
            client = self._get_client()
            response = client.get(f"{self.base_url}/mcp/tools")
            response.raise_for_status()
            return response.json()
        except Exception as e:
            logger.error(f"Error listing tools: {e}")
            return None

    def call_tool(self, tool_name: str, params: Dict[str, Any] = None) -> Optional[Dict[str, Any]]:
        """Call a tool on Unreal Engine.

        Args:
            tool_name: The name of the tool to execute
            params: Optional parameters for the tool

        Returns:
            The response dictionary from Unreal Engine, or None on error
        """
        try:
            client = self._get_client()
            url = f"{self.base_url}/mcp/tool/{tool_name}"
            payload = params or {}

            logger.debug(f"Calling tool '{tool_name}' with params: {payload}")

            response = client.post(url, json=payload)
            response.raise_for_status()

            result = response.json()
            logger.debug(f"Response from Unreal: {result}")

            # Handle error responses
            if not result.get("success", False):
                error_message = result.get("error") or result.get("message", "Unknown error")
                logger.error(f"Unreal error: {error_message}")

            return result

        except httpx.HTTPStatusError as e:
            logger.error(f"HTTP error calling tool '{tool_name}': {e.response.status_code} - {e.response.text}")
            return {
                "success": False,
                "error": f"HTTP {e.response.status_code}: {e.response.text}"
            }
        except Exception as e:
            logger.error(f"Error calling tool '{tool_name}': {e}")
            return {
                "success": False,
                "error": str(e)
            }


# Global connection instance
_connection: Optional[UnrealConnection] = None


def get_connection() -> UnrealConnection:
    """Get or create the global Unreal connection."""
    global _connection
    if _connection is None:
        _connection = UnrealConnection()
    return _connection


def reset_connection():
    """Reset the global connection."""
    global _connection
    if _connection:
        _connection.close()
    _connection = None
