# tests/test_doc_parser.py
import pytest
from ombra_watcherd.extractors.doc_parser import extract_concepts_from_doc

SAMPLE_DOC = """
# TIMING & TSC ANTI-DETECTION

## Overview

The timing subsystem provides anti-detection capabilities.

1. **TSC Offset Compensation**: Capturing TSC at VMExit entry
2. **APERF/MPERF Virtualization**: Compensating performance MSRs

### Key Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| `VMCS_FIELD_TSC_OFFSET_FULL` | `0x2010` | VMCS field for TSC offset |
| `IA32_MPERF` | `0xE7` | Maximum Performance Frequency |
| `IA32_APERF` | `0xE8` | Actual Performance Frequency |

## Critical Data Structures

```cpp
struct VcpuTimingState {
    u64 exit_entry_tsc;
    volatile u64 accumulated_overhead;
    u64 aperf_offset;
    u64 mperf_offset;
};
```
"""


def test_extract_basic_concepts():
    concepts = extract_concepts_from_doc(SAMPLE_DOC, "01_TIMING.md")

    assert len(concepts) >= 2

    # Should extract TSC concept
    tsc_concepts = [c for c in concepts if "TSC" in c["id"]]
    assert len(tsc_concepts) >= 1

    # Should extract VMCS field reference
    vmcs_refs = [c for c in concepts if c.get("vmcs_fields")]
    assert any("0x2010" in str(c.get("vmcs_fields", [])) for c in concepts)


def test_extract_msr_references():
    concepts = extract_concepts_from_doc(SAMPLE_DOC, "01_TIMING.md")

    # Should find MSR references
    all_msrs = []
    for c in concepts:
        all_msrs.extend(c.get("msrs", []))

    # Check for IA32_MPERF or hex value
    assert len(all_msrs) > 0 or any(c.get("msrs") for c in concepts)
