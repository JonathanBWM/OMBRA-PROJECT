# OmbraHypervisor Project Structure

## Complete Directory Layout

```
OmbraHypervisor/
│
├── 📄 OmbraHypervisor_Claude_Code_Prompt.md        # Standalone prompt (no MCP)
├── 📄 OmbraHypervisor_Claude_Code_Prompt_With_MCP.md  # Prompt with MCP guidance
│
├── 📁 ombra_mcp_server/                            # MCP Server Package
│   ├── 📄 README.md                                # Quick start guide
│   ├── 📄 SPEC.md                                  # Full specification
│   ├── 📄 pyproject.toml                           # Python package config
│   ├── 📄 setup.sh                                 # One-command install script
│   │
│   ├── 📁 scripts/
│   │   └── 📄 prepare_intel_sdm.py                 # Database preparation script
│   │
│   └── 📁 src/
│       └── 📁 ombra_mcp/
│           ├── 📄 __init__.py                      # Package init
│           ├── 📄 server.py                        # Main MCP server (25+ tools)
│           ├── 📄 additional_tools.py              # Driver & signature tools
│           │
│           └── 📁 data/
│               ├── 📄 .gitkeep
│               ├── 📄 intel_sdm.db                 # [Generated] SQLite database
│               ├── 📄 ld9boxsup_ioctls.json        # Vulnerable driver IOCTLs
│               └── 📄 signatures.json              # Detection patterns to avoid
│
└── 📁 hypervisor/                                  # [TO BE BUILT] Main project
    ├── 📁 usermode/
    │   ├── 📄 main.c
    │   ├── 📄 driver_interface.c
    │   ├── 📄 driver_interface.h
    │   └── 📄 payload_loader.c
    │
    ├── 📁 hypervisor/
    │   ├── 📄 entry.c
    │   ├── 📄 vmx.c
    │   ├── 📄 vmx.h
    │   ├── 📄 vmcs.c
    │   ├── 📄 vmcs.h
    │   ├── 📄 ept.c
    │   ├── 📄 ept.h
    │   ├── 📄 exit_dispatch.c
    │   ├── 📄 exit_dispatch.h
    │   ├── 📄 timing.c
    │   ├── 📄 timing.h
    │   │
    │   ├── 📁 handlers/
    │   │   ├── 📄 handlers.h
    │   │   ├── 📄 cpuid.c
    │   │   ├── 📄 rdtsc.c
    │   │   ├── 📄 msr.c
    │   │   ├── 📄 cr_access.c
    │   │   ├── 📄 ept_violation.c
    │   │   ├── 📄 vmcall.c
    │   │   ├── 📄 exception.c
    │   │   └── 📄 io.c
    │   │
    │   └── 📁 asm/
    │       ├── 📄 vmexit.asm
    │       ├── 📄 intrinsics.asm
    │       └── 📄 segment.asm
    │
    ├── 📁 shared/
    │   ├── 📄 types.h
    │   ├── 📄 vmcs_fields.h                        # ← generate_vmcs_header()
    │   ├── 📄 msr_defs.h                           # ← generate_msr_header()
    │   ├── 📄 exit_reasons.h
    │   ├── 📄 ept_defs.h                           # ← generate_ept_structures()
    │   └── 📄 cpu_defs.h
    │
    ├── 📁 build/
    │   ├── 📄 Makefile
    │   └── 📄 build.bat
    │
    └── 📁 docs/
        ├── 📄 vmcs_reference.md
        ├── 📄 exit_handling.md
        └── 📄 detection_vectors.md
```

---

## What's Ready Now vs What Claude Code Will Build

### ✅ Ready Now (Download These)

```
OmbraHypervisor/
├── OmbraHypervisor_Claude_Code_Prompt.md           ✅ Ready
├── OmbraHypervisor_Claude_Code_Prompt_With_MCP.md  ✅ Ready
│
└── ombra_mcp_server/                               ✅ Ready
    ├── README.md                                   ✅ Ready
    ├── SPEC.md                                     ✅ Ready
    ├── pyproject.toml                              ✅ Ready
    ├── setup.sh                                    ✅ Ready
    ├── scripts/prepare_intel_sdm.py                ✅ Ready
    └── src/ombra_mcp/
        ├── __init__.py                             ✅ Ready
        ├── server.py                               ✅ Ready
        ├── additional_tools.py                     ✅ Ready
        └── data/
            ├── ld9boxsup_ioctls.json               ✅ Ready
            └── signatures.json                     ✅ Ready
```

### 🔨 To Be Built (Claude Code Will Generate)

```
hypervisor/                                         🔨 Claude Code builds this
├── usermode/                                       🔨 Phase 2
├── hypervisor/                                     🔨 Phase 3-5
├── shared/                                         🔨 Phase 1 (MCP generates)
├── build/                                          🔨 Phase 6
└── docs/                                           🔨 Ongoing
```

---

## File Counts

| Component | Files | Status |
|-----------|-------|--------|
| MCP Server | 10 files | ✅ Complete |
| Prompts | 2 files | ✅ Complete |
| Hypervisor Usermode | 4 files | 🔨 To build |
| Hypervisor Core | 12 files | 🔨 To build |
| Hypervisor Handlers | 9 files | 🔨 To build |
| Hypervisor ASM | 3 files | 🔨 To build |
| Shared Headers | 6 files | 🔨 To build (MCP generates) |
| Build System | 2 files | 🔨 To build |
| **Total** | **48 files** | |

---

## Setup Workflow

```bash
# Step 1: Download all files from Claude outputs

# Step 2: Set up MCP server
cd ombra_mcp_server
./setup.sh

# Step 3: Create hypervisor project directory
mkdir -p ../hypervisor/{usermode,hypervisor/handlers,hypervisor/asm,shared,build,docs}

# Step 4: Configure Claude Code
mkdir -p ../hypervisor/.claude
cat > ../hypervisor/.claude/settings.json << 'EOF'
{
  "mcp": {
    "servers": {
      "ombra": {
        "command": "python",
        "args": ["-m", "ombra_mcp.server"],
        "cwd": "../ombra_mcp_server"
      }
    }
  }
}
EOF

# Step 5: Start Claude Code with the prompt
cd ../hypervisor
# Open Claude Code, paste OmbraHypervisor_Claude_Code_Prompt_With_MCP.md
```

---

## MCP Server Tools Quick Reference

### Intel SDM (6 tools)
| Tool | Input | Output |
|------|-------|--------|
| `vmcs_field` | "GUEST_RIP" | `{encoding: 0x681E, ...}` |
| `vmcs_fields_by_category` | "guest_state" | List of all guest fields |
| `exit_reason` | 48 | EPT violation details |
| `exit_qualification_format` | 48 | Bit layout |
| `msr_info` | "IA32_VMX_BASIC" | MSR specification |
| `search_sdm` | "EPT identity" | Relevant excerpts |

### Code Generation (6 tools)
| Tool | Output |
|------|--------|
| `generate_vmcs_header` | Complete vmcs_fields.h |
| `generate_msr_header` | Complete msr_defs.h |
| `generate_exit_handler_skeleton` | exit_dispatch.c |
| `generate_handler_template(N)` | Handler for exit N |
| `generate_ept_structures` | ept_defs.h |
| `generate_asm_stub("vmexit")` | vmexit.asm |

### Driver Interface (4 tools)
| Tool | Output |
|------|--------|
| `ld9boxsup_ioctl_info` | IOCTL specification |
| `list_available_ioctls` | All IOCTLs |
| `get_driver_workflow` | Step-by-step guide |
| `generate_ioctl_wrapper` | C wrapper function |

### Detection (5 tools)
| Tool | Output |
|------|--------|
| `check_for_signatures` | Pattern matches |
| `get_timing_thresholds` | Detection limits |
| `get_cpuid_detection_info` | Spoofing requirements |
| `get_detection_recommendations` | Best practices |
| `generate_random_pool_tag` | Safe tag |

### Project Memory (4 tools)
| Tool | Output |
|------|--------|
| `get_implementation_status` | Progress tracking |
| `update_status` | Update file status |
| `add_implementation_note` | Document decisions |
| `get_known_issues` | Blockers list |
