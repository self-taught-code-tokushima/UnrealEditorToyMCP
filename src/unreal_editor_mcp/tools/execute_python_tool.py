"""
Execute Python script tool.
"""

from typing import Dict, Any

from .base import EditorTool


class ExecutePythonTool(EditorTool):
    """Execute a Python script in the Unreal Editor.

    This tool allows executing arbitrary Python code within the Unreal Editor
    environment. The script is sent to the editor, executed, and the output
    is returned.
    """

    @property
    def name(self) -> str:
        """Get the tool name."""
        return "execute_python"

    @property
    def description(self) -> str:
        """Get the tool description."""
        return """Execute a Python script in the Unreal Editor.

Args:
    script_content: The Python script content to execute
    script_name: Optional name for the script (auto-generated if not provided)

Returns:
    Dictionary containing:
    - success: Whether the script executed successfully
    - output: The output from the Python script execution
    - script_path: Path where the script was saved
    - error: Error message if execution failed"""

    def execute(self, script_content: str, script_name: str = "") -> Dict[str, Any]:
        """Execute a Python script in Unreal Engine.

        Args:
            script_content: The Python script content to execute
            script_name: Optional name for the script (auto-generated if not provided)

        Returns:
            Dictionary containing:
            - success: Whether the script executed successfully
            - output: Script execution output
            - script_path: Path where script was saved
            - error: Error message (if failed)
        """
        params = {"script_content": script_content}
        if script_name:
            params["script_name"] = script_name

        response = self.call_unreal_tool("execute_python", params)

        if response.get("success"):
            data = response.get("data", {})
            return {
                "success": data.get("success", False),
                "output": data.get("output", ""),
                "script_path": data.get("script_path", ""),
                "error": data.get("error", "")
            }

        return {
            "success": False,
            "error": response.get("error", "Unknown error")
        }
