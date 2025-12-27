# Concept Intelligence System

## Overview

The Concept Intelligence System extracts abstract hypervisor concepts from legacy documentation, tracks their implementation status via pattern detection, and provides intelligent gap analysis to guide development.

## What It Does

1. **Concept Extraction**: Analyzes old project docs to identify ~80-120 discrete hypervisor concepts
2. **Pattern Detection**: Uses regex/AST patterns to detect if concepts are implemented in code
3. **Gap Analysis**: Identifies what's missing and suggests what to work on next
4. **Dependency Tracking**: Understands which concepts depend on others
5. **SDM Cross-Reference**: Links concepts to Intel SDM sections for authoritative specs

## Database Schema

### Tables

- **concepts**: Core concept definitions with patterns, SDM refs, dependencies
- **concept_findings**: Pattern detection results (missing, partial, antipattern, complete)
- **concept_annotations**: Human verification annotations (overrides pattern detection)

### Fields

Each concept has:
- `id`: Unique identifier (e.g., TSC_OFFSET_COMPENSATION)
- `category`: timing, vmx, ept, stealth
- `name`: Human-readable name
- `description`: What this concept is and why it matters
- `required_patterns`: JSON array of regex patterns that MUST exist
- `optional_patterns`: JSON array of patterns that MAY exist
- `anti_patterns`: JSON array of patterns indicating WRONG implementation
- `vmcs_fields`: Relevant VMCS field encodings
- `exit_reasons`: Relevant VMExit reasons
- `msrs`: Relevant MSR addresses
- `implementation_status`: not_started, partial, complete, verified
- `confidence`: 0.0-1.0 from pattern detection
- `priority`: critical, high, medium, low
- `depends_on`: JSON array of concept IDs this depends on
- `phase_order`: Sequential phase number

## MCP Tools

### Query Tools

1. **get_concept(concept_id)**
   - Get full details on a specific concept
   - Returns patterns, SDM refs, status, dependencies

2. **list_concepts(category?, status?, priority?)**
   - List all concepts with optional filtering
   - Sort by priority and phase order

3. **check_concept_coverage(concept_id)**
   - Deep analysis of a concept's implementation
   - Returns which patterns are present/missing
   - Includes findings breakdown

### Gap Analysis Tools

4. **get_implementation_gaps(category?)**
   - List missing or partial implementations
   - Sorted by priority
   - Shows which dependencies are satisfied

5. **suggest_next_work()**
   - AI suggests what to implement next
   - Based on dependencies, priority, phase order
   - Prefers continuing partial work over starting new

### Annotation Tools

6. **verify_concept(concept_id, file, line, notes?)**
   - Mark concept as verified at location
   - Human verification overrides pattern detection

7. **dismiss_concept_finding(finding_id, reason?)**
   - Dismiss false positive findings

## Current Status

**24 concepts seeded** covering:
- **Timing** (4): TSC offset, APERF/MPERF, per-vCPU state, TSC capture
- **VMX** (6): CPUID handling, RIP advancement, MSR bitmap matching, VMCALL auth, XSETBV validation, VMCS validation
- **EPT** (7): Identity mapping, execute-only hooks, EPT violations, INVEPT, large pages, MTRR mapping, page splitting
- **Stealth** (7): Hypervisor bit hiding, VMX hiding, vendor string spoofing, VMREAD #UD injection, PE header elimination, LBR virtualization, SGDT/SIDT WoW64 fix

## How It Works

### Pattern Matching

Concepts define three types of patterns:

**Required patterns** - MUST exist for implementation:
```json
[
  "timing.*OnExitEntry",
  "__rdtsc\\(\\)",
  "VMCS.*TSC_OFFSET"
]
```

**Optional patterns** - MAY exist (enhance confidence):
```json
[
  "exit_entry_tsc",
  "g_timing_states"
]
```

**Anti-patterns** - Indicate WRONG implementation:
```json
[
  "VMResume.*before.*timing",
  "OnExitEntry.*\\n.*\\n.*__rdtsc"
]
```

### Confidence Scoring

| Patterns Matched | Confidence |
|------------------|------------|
| All required, no anti-patterns | 0.95 |
| All required, has anti-patterns | 0.60 |
| Some required missing | 0.40 |
| Only optional patterns | 0.20 |
| Human verified via annotation | 1.00 |

### Dependency Graph

Concepts can depend on others:

```
TSC_OFFSET_COMPENSATION (phase 1)
  └─> APERF_MPERF_VIRTUALIZATION (phase 2)
  └─> TSC_CAPTURE_AT_ENTRY (phase 1)

CPUID_EXIT_HANDLING (phase 1)
  └─> CPUID_HYPERVISOR_BIT (phase 2)
  └─> CPUID_VMX_HIDING (phase 2)
  └─> CPUID_VENDOR_STRING_SPOOFING (phase 2)
  └─> VMCALL_AUTHENTICATION (phase 2)
```

Only suggests work where dependencies are satisfied.

## Usage Examples

### Check what to work on next:
```bash
mcp-cli call ombra/suggest_next_work '{}'
```

### List all critical timing concepts:
```bash
mcp-cli call ombra/list_concepts '{"category":"timing","priority":"critical"}'
```

### Deep dive on a specific concept:
```bash
mcp-cli call ombra/check_concept_coverage '{"concept_id":"TSC_OFFSET_COMPENSATION"}'
```

### See all implementation gaps:
```bash
mcp-cli call ombra/get_implementation_gaps '{}'
```

### Verify a concept implementation:
```bash
mcp-cli call ombra/verify_concept '{
  "concept_id": "TSC_OFFSET_COMPENSATION",
  "file_path": "hypervisor/timing.c",
  "line_number": 45,
  "notes": "Fully implemented with assembly optimization"
}'
```

## Future Enhancements

### Phase 2: Semantic Analysis (Tier 2)
- Cross-file invariant checking
- MSR bitmap vs handler matching
- VMCS field vs control bit validation

### Phase 3: Architectural Analysis (Tier 3)
- State machine tracking
- Multi-phase concept verification
- Call graph analysis

### Phase 4: Daemon Integration
- Continuous pattern scanning via ombra-watcherd
- Real-time finding generation
- Auto-update concept status on code changes

## File Structure

```
ombra_mcp_server/
├── src/
│   ├── ombra_mcp/
│   │   ├── tools/
│   │   │   └── concepts.py          # MCP tools
│   │   └── data/
│   │       └── project_brain.db     # Concept database
│   └── ombra_watcherd/
│       └── database.py              # Database schema
└── scripts/
    └── seed_concepts.py             # Concept seeding script
```

## Contributing

To add new concepts:

1. Edit `scripts/seed_concepts.py`
2. Add concept to `INITIAL_CONCEPTS` array
3. Run: `python3 scripts/seed_concepts.py`

Concept template:
```python
{
    "id": "MY_CONCEPT_ID",
    "category": "timing|vmx|ept|stealth",
    "name": "Human Readable Name",
    "description": "What this is and why it matters",
    "source_doc": "01_TIMING_ANTI_DETECTION.md",
    "sdm_refs": json.dumps(["Vol 3C 24.6.5"]),
    "required_patterns": json.dumps([
        r"pattern1",
        r"pattern2"
    ]),
    "optional_patterns": json.dumps([]),
    "anti_patterns": json.dumps([]),
    "vmcs_fields": json.dumps(["0x2010"]),
    "exit_reasons": json.dumps([10]),
    "msrs": json.dumps(["0xE7"]),
    "implementation_status": "not_started",
    "confidence": 0.0,
    "priority": "critical|high|medium|low",
    "anti_cheat_relevance": json.dumps(["battleye", "eac"]),
    "depends_on": json.dumps(["OTHER_CONCEPT"]),
    "phase_order": 1
}
```

---

*Built for OmbraHypervisor V3 - December 2025*
