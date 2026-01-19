"""
Study UnrealEditorMCP Server

FastMCP server for Unreal Engine 5 editor automation via HTTP REST API.
"""

import logging
import sys
from contextlib import asynccontextmanager
from typing import AsyncIterator, Dict, Any

from mcp.server.fastmcp import FastMCP
from dotenv import load_dotenv

from .connection import get_connection, reset_connection

# Load environment variables
load_dotenv()

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('study_unrealeditormcp.log'),
        logging.StreamHandler(sys.stderr)
    ]
)
logger = logging.getLogger("StudyUnrealEditorMCP")


@asynccontextmanager
async def server_lifespan(server: FastMCP) -> AsyncIterator[Dict[str, Any]]:
    """Handle server startup and shutdown."""
    logger.info("Study UnrealEditorMCP server starting up")

    try:
        # Try to connect on startup
        conn = get_connection()
        status = conn.check_status()
        if status:
            logger.info(f"Connected to Unreal Engine: {status.get('projectName')} (Engine {status.get('engineVersion')})")
        else:
            logger.warning("Could not connect to Unreal Engine on startup (will retry per-command)")
    except Exception as e:
        logger.warning(f"Error during startup connection: {e}")

    try:
        yield {}
    finally:
        reset_connection()
        logger.info("Study UnrealEditorMCP server shut down")


# Initialize FastMCP server
mcp = FastMCP(
    "StudyUnrealEditorMCP",
    lifespan=server_lifespan
)

# Import and register tools
from .tools.editor_tools import register_editor_tools

register_editor_tools(mcp)


def main():
    """Main entry point for the MCP server."""
    logger.info("Starting Study UnrealEditorMCP server...")
    mcp.run()


if __name__ == "__main__":
    main()
