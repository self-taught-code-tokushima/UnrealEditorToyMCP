"""
Ping tool for checking Unreal Engine connection.
"""

from typing import Dict, Any

from .base import EditorTool


class PingTool(EditorTool):
    """Ping the Unreal Engine to check connection.

    This tool sends a simple ping request to the Unreal Engine to verify
    that the connection is working properly.
    """

    @property
    def name(self) -> str:
        """Get the tool name."""
        return "ping"

    @property
    def description(self) -> str:
        """Get the tool description."""
        return """Ping the Unreal Engine to check connection.

Returns:
    Dictionary containing:
    - success: Whether the ping succeeded
    - message: Response message from Unreal Engine"""

    def execute(self) -> Dict[str, Any]:
        """Execute the ping command.

        Returns:
            Dictionary containing:
            - success: Whether the ping succeeded
            - message: Response message ("pong" if successful)
            - error: Error message (if failed)
        """
        response = self.call_unreal_tool("ping", {})

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
