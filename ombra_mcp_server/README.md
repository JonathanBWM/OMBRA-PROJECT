# OmbraMCP Server

Model Context Protocol server providing hypervisor development tools for Claude.

## Installation

```bash
chmod +x setup.sh
./setup.sh
```

## Manual Installation

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -e .
python scripts/prepare_intel_sdm.py
```

## Testing

```bash
source .venv/bin/activate
python -c "from ombra_mcp.server import app; print('OK')"
```

## Structure

```
src/ombra_mcp/
├── server.py           # Main MCP server (27 tools)
├── tools/              # Modular implementations
│   ├── vmcs_validator.py
│   ├── binary_scanner.py
│   ├── timing_simulator.py
│   └── anticheat_intel.py
└── data/
    ├── intel_sdm.db    # Intel SDM database
    ├── signatures.json # Anti-cheat signatures
    └── vmcs_fields.h   # Generated C header
```

## Regenerate Intel SDM Database

```bash
python scripts/prepare_intel_sdm.py
```

This builds `intel_sdm.db` with:
- 167 VMCS field encodings
- 66 VM-exit reasons
- 35 MSRs
- 20 exception vectors
