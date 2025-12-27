# SDM Query Tools: Before vs After

## The Problem

The natural language query function `ask_sdm()` was fundamentally broken, making the MCP tools nearly useless.

## Example: "What is the GUEST_RIP field?"

### BEFORE (Broken)

```python
# Extracted keywords: ["what", "is", "the", "guest_rip", "field?"]
# Searches for EACH word individually

# Query 1: Search for "what"
SELECT ... WHERE name LIKE '%what%' OR description LIKE '%what%'
# Returns: 0 results (good)

# Query 2: Search for "is"
SELECT ... WHERE name LIKE '%is%' OR description LIKE '%is%'
# Returns: 50+ results (GUEST_INTERRUPTIBILITY_STATE, VM_EXIT_INSTRUCTION_INFO, etc.)

# Query 3: Search for "the"
SELECT ... WHERE name LIKE '%the%' OR description LIKE '%the%'
# Returns: 20+ results (anything with "the" in description)

# Query 4: Search for "guest_rip"
SELECT ... WHERE name LIKE '%guest_rip%' OR description LIKE '%guest_rip%'
# Returns: GUEST_RIP (correct)

# Query 5: Search for "field?"
SELECT ... WHERE name LIKE '%field?%' OR description LIKE '%field?%'
# Returns: 0 results (punctuation)

# Total results: 70+ mostly garbage results, GUEST_RIP buried somewhere in the middle
# No ranking, no deduplication, no limit
```

**Result:** Unusable. Returns everything that contains common words.

### AFTER (Fixed)

```python
# Filter stopwords: ["what", "is", "the"] removed
# Extracted keywords: ["guest_rip", "field?"]

# For keyword "guest_rip":
#   1. Try exact match (score 100)
SELECT ... WHERE LOWER(name) = 'guest_rip'
# Returns: GUEST_RIP

#   2. Try partial name match (score 80) - skipped, already found exact match
#   3. Try description match (score 40) - skipped

# For keyword "field?":
#   1. Try exact match (score 100)
SELECT ... WHERE LOWER(name) = 'field?'
# Returns: nothing

#   2. Try partial name match (score 80)
SELECT ... WHERE LOWER(name) LIKE '%field?%'
# Returns: nothing (punctuation doesn't match)

#   3. Try description match (score 40) - skipped

# Sort by score: [(100, GUEST_RIP)]
# Take top 10: [GUEST_RIP]
```

**Result:**
```json
{
  "question": "What is the GUEST_RIP field?",
  "keywords_used": ["guest_rip", "field?"],
  "total_matches": 1,
  "showing": 1,
  "answers": [
    {
      "type": "vmcs_field",
      "name": "GUEST_RIP",
      "encoding": "0x681e",
      "category": "guest_state",
      "description": "Guest RIP"
    }
  ]
}
```

Perfect. Exactly what the user wanted.

## Example: "EPT violation handler"

### BEFORE (Broken)

```python
# Keywords: ["ept", "violation", "handler"]
# Each searched independently, all results combined

# Returns: 100+ results including:
# - Everything with "ept" in name/description (EPT_POINTER, EPTP_INDEX, etc.)
# - Everything with "violation" (EPT_VIOLATION, violation-related descriptions)
# - Everything with "handler" in description (most exit reasons mention "handler")
# - Duplicates of same result appearing 3 times (matched by all 3 keywords)
# - Random order
# - No limit
```

**Result:** Massive dump of unsorted, duplicated results.

### AFTER (Fixed)

```python
# Keywords: ["ept", "violation", "handler"]
# Stopwords already filtered

# Scored results:
# - EPT_VIOLATION exit reason (matches "ept" + "violation"): 180 points
# - EPT_MISCONFIGURATION exit reason (matches "ept"): 80 points
# - EPT_POINTER field (matches "ept"): 80 points
# - EPTP_INDEX field (matches "ept"): 80 points
# - Various other EPT-related fields: 40-80 points
# - Results mentioning "handler" in description: 40 points

# Deduplicated, sorted by score, top 10 returned
```

**Result:**
```json
{
  "question": "EPT violation handler",
  "keywords_used": ["ept", "violation", "handler"],
  "total_matches": 12,
  "showing": 10,
  "answers": [
    {"type": "vmcs_field", "name": "EPTP_INDEX", ...},
    {"type": "vmcs_field", "name": "EPT_POINTER", ...},
    {"type": "vmcs_field", "name": "EPTP_LIST_ADDRESS", ...},
    {"type": "exit_reason", "name": "EPT_VIOLATION", "reason": 48, ...},
    {"type": "exit_reason", "name": "EPT_MISCONFIGURATION", "reason": 49, ...},
    ...
  ]
}
```

Relevant results, ranked by usefulness, limited to top 10.

## vmcs_field_complete() Improvements

### BEFORE
```python
vmcs_field_complete("RIP")
# Query: SELECT ... WHERE name LIKE '%rip%'
# Returns: [IO_RIP, GUEST_RIP, HOST_RIP, ...] in random order
```

### AFTER
```python
vmcs_field_complete("RIP")
# 1. Try exact: SELECT ... WHERE LOWER(name) = 'rip'
#    Returns: nothing
# 2. Try partial: SELECT ... WHERE LOWER(name) LIKE '%rip%'
#    Returns: [IO_RIP, GUEST_RIP, HOST_RIP]

vmcs_field_complete("GUEST_RIP")
# 1. Try exact: SELECT ... WHERE LOWER(name) = 'guest_rip'
#    Returns: [GUEST_RIP] ✓ stops here
# Doesn't waste time on partial matches
```

**Improvement:** Exact matches prioritized, predictable behavior.

## get_msr_info() Improvements

### BEFORE
```python
get_msr_info("VMX")
# Query: SELECT ... WHERE name LIKE '%vmx%'
# Returns: all 19 VMX-related MSRs in random order
```

### AFTER
```python
get_msr_info("VMX")
# 1. Try exact: SELECT ... WHERE LOWER(name) = 'vmx'
#    Returns: nothing
# 2. Try partial: SELECT ... WHERE LOWER(name) LIKE '%vmx%'
#    Returns: [IA32_VMX_BASIC, IA32_VMX_PINBASED_CTLS, ...] (19 MSRs)

get_msr_info("IA32_VMX_BASIC")
# 1. Try exact: SELECT ... WHERE LOWER(name) = 'ia32_vmx_basic'
#    Returns: [IA32_VMX_BASIC] ✓ stops here
# Much faster, no wasted queries

get_msr_info("0x480")
# Address lookup: SELECT ... WHERE address = 0x480
# Returns: [IA32_VMX_BASIC]
```

**Improvement:** Exact matches prioritized, includes `count` field.

## Summary of Changes

| Function | Before | After |
|----------|--------|-------|
| `ask_sdm()` | Searches all words including stopwords | Filters stopwords, only searches meaningful keywords |
| | No relevance ranking | Scores: 100 (exact), 80 (partial name), 40 (description) |
| | No deduplication | Deduplicates by unique ID |
| | No result limit | Returns top 10 most relevant |
| | Returns garbage | Returns useful results |
| `vmcs_field_complete()` | Only partial LIKE search | Exact match first, then partial |
| `get_msr_info()` | Only partial LIKE search | Exact match first, then partial |
| | No `count` field | Includes `count` field |

## Backwards Compatibility

All changes are backwards compatible:
- Only added new fields to return values (`keywords_used`, `total_matches`, `showing`, `count`)
- Did not remove or rename existing fields
- MCP protocol just serializes dicts to JSON - extra fields are fine
- Existing callers will continue to work
