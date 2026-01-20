"""
Test script for UnrealEditorMCP server.

This script verifies that the MCP server can:
- Start up correctly
- Connect to Unreal Engine
- List available tools
- Execute tools (ping test)
"""

import asyncio
import sys
from mcp import ClientSession, StdioServerParameters
from mcp.client.stdio import stdio_client


async def test_mcp_server():
    """Test the MCP server."""
    print("=" * 60)
    print("UnrealEditorMCP - Server Test")
    print("=" * 60)

    server_params = StdioServerParameters(
        command="uv",
        args=["run", "unreal-editor-mcp"],
    )

    try:
        async with stdio_client(server_params) as (read, write):
            async with ClientSession(read, write) as session:
                # Initialize
                print("\n[1/3] Initializing MCP session...")
                await session.initialize()
                print("[OK] Session initialized successfully")

                # List tools
                print("\n[2/3] Listing available tools...")
                tools = await session.list_tools()
                print(f"[OK] Found {len(tools.tools)} tool(s):")
                for tool in tools.tools:
                    # Extract first line of description for cleaner output
                    desc_lines = tool.description.split('\n')
                    first_line = desc_lines[0] if desc_lines else tool.description
                    print(f"  - {tool.name}: {first_line}")

                # Call ping tool
                print("\n[3/3] Testing 'ping' tool...")
                result = await session.call_tool("ping", arguments={})
                print("[OK] Tool executed successfully")
                print("\nResult:")
                for content in result.content:
                    if hasattr(content, 'text'):
                        print(f"  {content.text}")
                    else:
                        print(f"  {content}")

                print("\n" + "=" * 60)
                print("All tests passed!")
                print("=" * 60)
                return 0

    except Exception as e:
        print(f"\n[ERROR] {e}", file=sys.stderr)
        print("\n" + "=" * 60)
        print("Tests failed!")
        print("=" * 60)
        return 1


def main():
    """Main entry point."""
    return asyncio.run(test_mcp_server())


if __name__ == "__main__":
    sys.exit(main())
