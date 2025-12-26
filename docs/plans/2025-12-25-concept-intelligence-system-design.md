# Concept Intelligence System Design

**Date**: December 25, 2025
**Status**: Approved for Implementation

---

## Overview

A system that extracts abstract hypervisor concepts from legacy documentation, cross-references with Intel SDM, and continuously analyzes the current codebase to identify implementation gaps and issues.

### Goals

1. Extract ~80-120 discrete concepts from old project docs
2. Track implementation status via pattern detection with optional human verification
3. Generate hypervisor-specific findings (not generic lint)
4. Provide both continuous daemon analysis and on-demand MCP tools

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                     Concept Intelligence System                  │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐       │
│  │ Old Project  │    │  Intel SDM   │    │   Current    │       │
│  │    Docs      │    │   Database   │    │   Codebase   │       │
│  └──────┬───────┘    └──────┬───────┘    └──────┬───────┘       │
│         │                   │                   │                │
│         ▼                   ▼                   ▼                │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │                   Concept Database                        │   │
│  │  - 80-120 concepts with patterns, SDM refs, dependencies │   │
│  └──────────────────────────────────────────────────────────┘   │
│         │                                       │                │
│         ▼                                       ▼                │
│  ┌──────────────┐                      ┌──────────────┐         │
│  │ ombra-watcherd│                      │  MCP Tools   │         │
│  │   (daemon)   │                      │ (on-demand)  │         │
│  │              │                      │              │         │
│  │ - Continuous │                      │ - Deep analysis│        │
│  │ - Fast       │                      │ - Gap reports │         │
│  │ - Findings   │                      │ - Suggestions │         │
│  └──────────────┘                      └──────────────┘         │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## Concept Database Schema

```sql
CREATE TABLE concepts (
    id TEXT PRIMARY KEY,           -- e.g., "TSC_OFFSET_COMPENSATION"
    category TEXT,                 -- e.g., "timing", "vmx", "ept", "stealth"
    name TEXT,                     -- Human readable: "TSC Offset Compensation"
    description TEXT,              -- What this concept is and why it matters
    source_doc TEXT,               -- Which old doc it came from
    sdm_refs TEXT,                 -- JSON: ["Vol 3C 24.6.5", "Appendix B"]

    -- Detection patterns (JSON arrays)
    required_patterns TEXT,        -- Patterns that MUST exist for implementation
    optional_patterns TEXT,        -- Patterns that MAY exist (enhance detection)
    anti_patterns TEXT,            -- Patterns that indicate WRONG implementation

    -- Intel SDM cross-references
    vmcs_fields TEXT,              -- JSON: ["0x2010", "0x2012"] - relevant VMCS fields
    exit_reasons TEXT,             -- JSON: [10, 31, 48] - relevant exit reasons
    msrs TEXT,                     -- JSON: ["0xE7", "0xE8"] - relevant MSRs

    -- Status tracking
    implementation_status TEXT,    -- "not_started", "partial", "complete", "verified"
    confidence REAL,               -- 0.0-1.0 from pattern detection
    verified_by_annotation BOOL,   -- True if human confirmed via annotation
    implementation_files TEXT,     -- JSON: files where implementation was detected

    -- Metadata
    priority TEXT,                 -- "critical", "high", "medium", "low"
    anti_cheat_relevance TEXT,     -- JSON: ["battleye", "eac", "vanguard"]

    -- Dependencies
    depends_on TEXT,               -- JSON: ["OTHER_CONCEPT_ID", ...]
    phase_order INTEGER            -- For multi-phase concepts (1, 2, 3...)
);

CREATE TABLE concept_findings (
    id INTEGER PRIMARY KEY,
    concept_id TEXT,
    file_path TEXT,
    line_number INTEGER,
    finding_type TEXT,             -- "missing", "partial", "antipattern", "complete"
    message TEXT,
    severity TEXT,                 -- "critical", "warning", "info"
    detected_at TIMESTAMP,
    dismissed BOOLEAN DEFAULT FALSE,
    dismissed_reason TEXT,
    FOREIGN KEY (concept_id) REFERENCES concepts(id)
);

CREATE TABLE concept_annotations (
    id INTEGER PRIMARY KEY,
    concept_id TEXT,
    file_path TEXT,
    line_number INTEGER,
    annotation_type TEXT,          -- "verified", "override", "wip"
    notes TEXT,
    created_at TIMESTAMP,
    FOREIGN KEY (concept_id) REFERENCES concepts(id)
);
```

---

## Concept Extraction

### Source Documents

| Document | Estimated Concepts |
|----------|-------------------|
| `01_TIMING_ANTI_DETECTION.md` | 8-12 |
| `02_CPUID_SPOOFING.md` | 6-10 |
| `03_VMEXIT_HANDLERS.md` | 15-20 |
| `04_EPT_NPT_MEMORY.md` | 10-15 |
| `05_HARDWARE_SPOOFING.md` | 8-12 |
| `06_BYOVD_KERNEL_LOADING.md` | 6-10 |
| `07_ZEROHVCI_HYPERV.md` | 8-12 |
| `08_IDT_VMCALL.md` | 5-8 |
| `09_ANTICHEAT_EVASION.md` | 10-15 |
| `10_DRIVER_MAPPER_MEMORY.md` | 8-12 |
| `DETECTION_VECTORS_AND_MITIGATIONS.md` | 10-15 |
| Other research docs | 10-15 |
| **Total** | **80-120** |

### Example Concepts

**Timing Category:**
- `TSC_OFFSET_COMPENSATION` - Adjust TSC offset on every VMexit
- `APERF_MPERF_VIRTUALIZATION` - Shadow performance MSRs
- `TIMING_STATE_PER_VCPU` - Cache-line aligned per-CPU state
- `TSC_CAPTURE_AT_ENTRY` - `__rdtsc()` must be first in handler

**VMX Category:**
- `CPUID_EXIT_HANDLING` - Exit reason 10 dispatch
- `VMREAD_UD_INJECTION` - Inject #UD for VMX instruction hiding
- `EPT_VIOLATION_HANDLING` - Exit reason 48 qualification parsing
- `MSR_BITMAP_HANDLER_MATCH` - MSRs in bitmap have handlers
- `RIP_ADVANCEMENT` - Advance RIP by instruction length

**EPT Category:**
- `EPT_IDENTITY_MAPPING` - Physical memory identity mapped
- `EPT_HOOK_EXECUTE_ONLY` - Hooks use execute-only pages
- `INVEPT_AFTER_MODIFY` - Call INVEPT after EPT changes
- `EPT_LARGE_PAGE_OPTIMIZATION` - Use 2MB/1GB pages

**Stealth Category:**
- `CPUID_HYPERVISOR_BIT` - Clear CPUID.1.ECX[31]
- `VMX_INSTRUCTION_UD` - All VMX instructions inject #UD
- `PE_HEADER_ELIMINATION` - Zero PE headers in memory

---

## Pattern Engine

### Three-Tier Analysis

**Tier 1: Syntactic (Fast)**
```python
PATTERN_TYPES = {
    "regex": r"__rdtsc\(\)",
    "ast": "call:__vmx_vmwrite(0x2010, *)",
    "sequence": ["rdtsc", "vmwrite:0x2010"],
    "proximity": {"patterns": [...], "lines": 20},
    "file_scope": "*.asm"
}
```

**Tier 2: Semantic (Cross-File)**
```python
INVARIANTS = [
    {
        "type": "msr_bitmap_handler_match",
        "bitmap_pattern": r"msr_bitmap\[.*\] = 1",
        "handler_pattern": r"case\s+{msr_id}:",
        "error": "MSR {msr_id} in bitmap but no handler"
    },
    {
        "type": "vmcs_field_control_bit",
        "field_write": r"vmwrite.*{field}",
        "control_dependency": "SDM_LOOKUP:{field}.requires_control",
        "error": "VMCS field {field} requires control bit {control}"
    }
]
```

**Tier 3: Architectural (State Machines)**
```python
STATE_MACHINES = {
    "ept_hook_flow": {
        "states": ["primary_ept", "violation", "shadow_ept", "single_step", "restore"],
        "transitions": [...],
        "error": "EPT hook state machine incomplete"
    }
}

PHASE_TRACKING = {
    "ZEROHVCI_HIJACK": {
        "phases": ["preflight", "kernel_rw", "kforge", "hv_discovery",
                  "pattern_scan", "payload_map", "trampoline", "patch"],
        "dependencies": {
            "kforge": ["kernel_rw"],
            "pattern_scan": ["hv_discovery"],
            "patch": ["trampoline", "payload_map"]
        }
    }
}
```

### Confidence Scoring

| Patterns Matched | Confidence |
|------------------|------------|
| All required, no anti-patterns | 0.95 |
| All required, has anti-patterns | 0.60 |
| Some required missing | 0.40 |
| Only optional patterns | 0.20 |
| Human verified via annotation | 1.00 |

---

## Daemon Integration

Add `ConceptAnalyzer` to existing `ombra-watcherd`:

```python
class ConceptAnalyzer(BaseAnalyzer):
    name = "concept"
    file_patterns = ["*.c", "*.h", "*.asm"]

    def analyze(self, file_path: Path, content: str) -> List[Finding]:
        findings = []

        for concept in self.load_concepts():
            # Tier 1: Syntactic
            matched = self.check_patterns(content, concept.required_patterns)
            missing = self.get_missing_patterns(content, concept.required_patterns)
            anti = self.check_patterns(content, concept.anti_patterns)

            if matched and missing:
                findings.append(Finding(
                    type="concept_partial",
                    concept_id=concept.id,
                    message=f"{concept.name}: Found {matched}, missing {missing}",
                    severity="warning"
                ))

            if anti:
                findings.append(Finding(
                    type="concept_antipattern",
                    concept_id=concept.id,
                    message=f"{concept.name}: Anti-pattern - {anti}",
                    severity="critical"
                ))

        return findings

    def cross_file_analysis(self) -> List[Finding]:
        """Run after all files analyzed - Tier 2 semantic checks."""
        # MSR bitmap vs handler matching
        # VMCS field vs control bit validation
        # etc.
```

---

## MCP Tools

### Query Tools

```python
@tool
async def get_concept(concept_id: str) -> dict:
    """Full details on a concept with SDM cross-references."""

@tool
async def list_concepts(
    category: str = None,
    status: str = None,
    priority: str = None
) -> List[dict]:
    """List concepts with filtering."""

@tool
async def check_concept_coverage(concept_id: str) -> dict:
    """Deep analysis with gap report."""
```

### Gap Analysis Tools

```python
@tool
async def get_implementation_gaps(category: str = None) -> List[dict]:
    """What's missing? Sorted by priority."""

@tool
async def suggest_next_work() -> dict:
    """Based on dependencies and priorities, what to implement next."""
```

### Annotation Tools

```python
@tool
async def verify_concept(concept_id: str, file: str, line: int) -> dict:
    """Mark concept as verified at location."""

@tool
async def dismiss_concept_finding(finding_id: int, reason: str) -> dict:
    """Dismiss false positive."""
```

---

## Intel SDM Integration

Cross-reference concepts with existing SDM database:

```python
class SDMLinker:
    def enrich_concept(self, concept: dict) -> dict:
        # Add VMCS field details from SDM
        for field_id in concept.get("vmcs_fields", []):
            field = self.sdm_db.get_vmcs_field(field_id)
            concept["sdm_vmcs_details"].append({
                "name": field["name"],
                "valid_values": field["valid_values"],
                "required_controls": field["dependencies"],
                "sdm_section": field["sdm_section"]
            })

        # Add exit reason details
        # Add MSR details
        return concept

    def validate_implementation(self, concept_id: str, code: str) -> List[Finding]:
        """Validate code against SDM requirements."""
        # Check VMCS field dependencies
        # Check valid value ranges
        # Check MSR bitmap coverage
```

---

## Implementation Order

| Phase | Component | Description |
|-------|-----------|-------------|
| 1 | Schema & Database | Create `concepts.db` with full schema |
| 2 | Concept Extraction | Parse 10+ docs, populate ~100 concepts |
| 3 | Pattern Engine | Tier 1 syntactic matching |
| 4 | Daemon Integration | Add `ConceptAnalyzer` to ombra-watcherd |
| 5 | MCP Tools | Query and gap analysis tools |
| 6 | Semantic Analysis | Tier 2 cross-file invariants |
| 7 | SDM Integration | Link concepts to SDM database |

---

## File Structure

```
ombra_mcp_server/
├── src/
│   ├── ombra_mcp/
│   │   └── tools/
│   │       ├── concepts.py          # MCP tools for concept queries
│   │       └── sdm_linker.py        # SDM cross-reference
│   └── ombra_watcherd/
│       ├── analyzers/
│       │   ├── concept.py           # Concept pattern analyzer
│       │   └── semantic.py          # Cross-file invariant checker
│       ├── extractors/
│       │   └── doc_parser.py        # Extract concepts from old docs
│       └── concepts.db              # Concept database
```

---

## Success Criteria

When complete, the system should:

1. **Track 80-120 concepts** extracted from old project documentation
2. **Continuously analyze** code changes via daemon
3. **Generate hypervisor-specific findings** like:
   - "VMCS field 0x2010 written but TSC_OFFSETTING control bit not set"
   - "EPT modified at ept.c:89 but no INVEPT call in function"
   - "MSR 0xE7 in bitmap but no RDMSR handler case"
4. **Provide gap analysis** showing what's implemented vs. missing
5. **Cross-reference Intel SDM** for authoritative validation
6. **Support hybrid verification** with pattern detection + optional annotations

---

*Design approved December 25, 2025*
