"""
Get actors in level tool.
"""

from typing import Dict, Any

from .base import EditorTool


class GetActorsInLevelTool(EditorTool):
    """Get all actors in the current editor level.

    This tool retrieves information about all actors in the currently
    loaded level in the Unreal Editor, including their names, classes,
    and transform information (location, rotation, scale).
    """

    @property
    def name(self) -> str:
        """Get the tool name."""
        return "get_actors_in_level"

    @property
    def description(self) -> str:
        """Get the tool description."""
        return """Get all actors in the current editor level.

Returns:
    Dictionary containing:
    - success: Whether the operation succeeded
    - actors: List of actor information (name, class, location, rotation, scale)
    - count: Number of actors returned"""

    def execute(self) -> Dict[str, Any]:
        """Execute the get actors command.

        Returns:
            Dictionary containing:
            - success: Whether the operation succeeded
            - actors: List of actor information
            - count: Number of actors
            - error: Error message (if failed)
        """
        response = self.call_unreal_tool("get_actors_in_level", {})

        if response.get("success"):
            data = response.get("data", {})
            return {
                "success": True,
                "actors": data.get("actors", []),
                "count": data.get("count", 0)
            }

        return {
            "success": False,
            "error": response.get("error", "Unknown error")
        }
