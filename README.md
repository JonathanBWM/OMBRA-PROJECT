# OmbraHypervisor Development Toolkit

A complete development environment for building OmbraHypervisor — a UEFI bootkit and Hyper-V hijacking framework for authorized security research.

## 📦 Package Contents

```
ombra_hypervisor_toolkit/
├── README.md                    # This file
├── QUICKSTART.md               # 5-minute setup guide
│
├── mcp_server/                  # OmbraMCP - Claude AI Integration
│   ├── pyproject.toml          # Python package config
│   ├── setup.sh                # One-click installer
│   ├── scripts/
│   │   └── prepare_intel_sdm.py # Intel SDM database builder
│   └── src/ombra_mcp/
│       ├── __init__.py
│       ├── server.py           # Main MCP server (27 tools)
│       ├── tools/              # Modular tool implementations
│       │   ├── __init__.py
│       │   ├── vmcs_validator.py
│       │   ├── binary_scanner.py
│       │   ├── timing_simulator.py
│       │   └── anticheat_intel.py
│       └── data/               # Reference databases
│           ├── intel_sdm.db    # Pre-built Intel SDM database
│           ├── vmcs_fields.h   # Auto-generated C header
│           ├── signatures.json # Anti-cheat signatures
│           └── ld9boxsup_ioctls.json
│
├── prompts/                     # Claude Code System Prompts
│   ├── OmbraHypervisor_Claude_Code_Prompt.md      # Base prompt
│   └── OmbraHypervisor_Claude_Code_Prompt_MCP.md  # MCP-enhanced
│
└── docs/
    ├── SPEC.md                 # Full MCP specification
    └── PROJECT_STRUCTURE.md    # Recommended project layout
```

## 🚀 Quick Setup (macOS)

```bash
# 1. Navigate to MCP server
cd mcp_server

# 2. Run setup script
chmod +x setup.sh
./setup.sh

# 3. Note the paths printed at the end for Claude Desktop config
```

## 🔧 Claude Desktop Configuration

Edit `~/Library/Application Support/Claude/claude_desktop_config.json`:

```json
{
  "mcpServers": {
    "ombra": {
      "command": "/absolute/path/to/mcp_server/.venv/bin/python",
      "args": ["-m", "ombra_mcp.server"],
      "env": {
        "PYTHONPATH": "/absolute/path/to/mcp_server/src"
      }
    }
  }
}
```

## 🛠 Available MCP Tools (27 Total)

### Intel SDM Reference
| Tool | Description |
|------|-------------|
| `vmcs_field_lookup` | Get VMCS field encoding by name |
| `vmcs_field_search` | Search fields by pattern |
| `exit_reason_lookup` | Decode VM-exit reasons |
| `msr_lookup` | MSR address/description |
| `exception_info` | Exception vector details |
| `sdm_search` | Full-text SDM search |

### Code Generation
| Tool | Description |
|------|-------------|
| `generate_vmcs_accessors` | Type-safe VMREAD/VMWRITE |
| `generate_exit_handler` | Exit handler skeleton |
| `generate_ept_tables` | EPT paging structures |
| `generate_msr_bitmap` | MSR intercept bitmap |
| `generate_hook_template` | Function hook templates |
| `scaffold_component` | Full component scaffolding |

### Analysis & Validation
| Tool | Description |
|------|-------------|
| `scan_binary_signatures` | Detect anti-cheat patterns |
| `validate_vmcs_state` | Check VMCS consistency |
| `simulate_rdtsc` | RDTSC timing analysis |
| `get_anticheat_signatures` | Detection signature DB |

## 📋 Requirements

- **macOS** (Apple Silicon or Intel)
- **Python 3.10+**
- **Claude Desktop** or **Claude Code**

For actual hypervisor builds:
- Windows 10/11 (VM or bare metal)
- Visual Studio 2022
- Windows Driver Kit (WDK)

## 🔒 Legal Notice

This toolkit is for **authorized security research only**.

---

*Built for OmbraHypervisor — Ring -1 security research*
