#!/bin/bash
# OmbraMCP Quick Setup Script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=========================================="
echo "  OmbraMCP — Hypervisor Development MCP  "
echo "=========================================="
echo ""

# Check Python version
python_version=$(python3 --version 2>&1 | cut -d' ' -f2 | cut -d'.' -f1,2)
required_version="3.10"

if [ "$(printf '%s\n' "$required_version" "$python_version" | sort -V | head -n1)" != "$required_version" ]; then
    echo "Error: Python 3.10+ required (found $python_version)"
    exit 1
fi

echo "[1/5] Creating virtual environment..."
python3 -m venv .venv
source .venv/bin/activate

echo "[2/5] Installing dependencies..."
pip install --upgrade pip --quiet
pip install -e . --quiet

echo "[3/5] Preparing Intel SDM database..."
python scripts/prepare_intel_sdm.py

echo "[4/5] Creating data directory..."
mkdir -p src/ombra_mcp/data

echo "[5/5] Verifying installation..."
python -c "from ombra_mcp.server import app; print('✓ Server module loads correctly')"

PYTHON_PATH="$SCRIPT_DIR/.venv/bin/python"
SOURCE_PATH="$SCRIPT_DIR/src"

echo ""
echo "=========================================="
echo "  Setup Complete!                        "
echo "=========================================="
echo ""
echo "IMPORTANT PATHS (copy these):"
echo "  Python: $PYTHON_PATH"
echo "  Source: $SOURCE_PATH"
echo ""
echo "Claude Desktop Configuration:"
echo "  File: ~/Library/Application Support/Claude/claude_desktop_config.json"
echo ""
cat << EOF
{
  "mcpServers": {
    "ombra": {
      "command": "$PYTHON_PATH",
      "args": ["-m", "ombra_mcp.server"],
      "env": {
        "PYTHONPATH": "$SOURCE_PATH"
      }
    }
  }
}
EOF
echo ""
echo "Restart Claude Desktop after adding this configuration."
echo ""
