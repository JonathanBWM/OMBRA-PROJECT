# 5-Minute Quickstart

## Step 1: Install

```bash
cd ombra_hypervisor_toolkit/mcp_server
chmod +x setup.sh
./setup.sh
```

## Step 2: Get Paths

After setup completes, note these paths:
```bash
# Python executable
echo "$(pwd)/.venv/bin/python"

# Source directory  
echo "$(pwd)/src"
```

## Step 3: Configure Claude Desktop

Edit `~/Library/Application Support/Claude/claude_desktop_config.json`:

```json
{
  "mcpServers": {
    "ombra": {
      "command": "/YOUR/PATH/mcp_server/.venv/bin/python",
      "args": ["-m", "ombra_mcp.server"],
      "env": {
        "PYTHONPATH": "/YOUR/PATH/mcp_server/src"
      }
    }
  }
}
```

## Step 4: Restart Claude Desktop

Completely quit and reopen Claude Desktop.

## Step 5: Verify

Ask Claude:
> "What's the VMCS encoding for GUEST_RIP?"

Expected response includes: `0x681E`

## Done! 🎉

Try these next:
- "Look up exit reason 48"
- "Generate a CPUID exit handler"
- "What MSRs are virtualized?"
