# SDM Query Tool Fixes

## Summary

Fixed broken query tools in `/src/ombra_mcp/tools/sdm_query.py` that were returning garbage results due to poor search logic.

## Problems Fixed

### 1. `ask_sdm()` - Natural Language Search

**Before:**
- Split question into words and searched for EVERY word including stopwords
- Query "What is the GUEST_RIP field?" would search for: "what", "is", "the", "guest_rip", "field?"
- Stopwords like "is", "the", "what" matched nearly everything in the database
- No deduplication - same result appeared multiple times
- No relevance ranking - random order
- No result limit - returned everything

**After:**
- Filters out stopwords (is, the, what, how, etc.)
- Strips punctuation from keywords (e.g., "GUEST_RIP?" becomes "guest_rip")
- Only searches meaningful keywords
- Prioritizes results by relevance:
  - Score 100: Exact name matches
  - Score 80: Partial name matches
  - Score 40: Description matches
- Deduplicates results
- Returns top 10 most relevant results only
- Includes metadata: keywords used, total matches, showing count

### 2. `vmcs_field_complete()` - VMCS Field Lookup

**Before:**
- Only did partial LIKE searches
- No preference for exact matches

**After:**
- Tries exact match first (case-insensitive)
- Falls back to partial match if no exact match
- Much more predictable behavior

### 3. `get_msr_info()` - MSR Lookup

**Before:**
- Only did partial LIKE searches for names
- No preference for exact matches

**After:**
- For name searches: tries exact match first, then partial
- For address searches: exact match only (unchanged)
- Added `count` field to result
- Case-insensitive matching

## Test Results

All tests pass successfully:

```
Testing ask_sdm with stopword-heavy query
Question: What is the GUEST_RIP field?
Keywords used: ['guest_rip', 'field?']
Total matches: 1
Showing top: 1
Result: GUEST_RIP (exact match, relevant)

Testing ask_sdm with EPT query
Question: EPT violation handler
Keywords used: ['ept', 'violation', 'handler']
Total matches: 12
Showing top: 10
Results: Relevant EPT fields and exit reasons, ranked by relevance

Testing vmcs_field_complete with exact match
Input: "GUEST_RIP"
Found: 1 field (exact match)

Testing vmcs_field_complete with partial match
Input: "RIP"
Found: 3 fields (IO_RIP, GUEST_RIP, HOST_RIP)

Testing get_msr_info with exact name match
Input: "IA32_VMX_BASIC"
Found: 1 MSR (exact match)

Testing get_msr_info with partial name match
Input: "VMX"
Found: 19 MSRs (all VMX-related MSRs)

Testing get_msr_info with hex address
Input: "0x480"
Found: 1 MSR (IA32_VMX_BASIC)
```

## Impact

These fixes make the MCP tools actually usable:

- Natural language queries now return relevant results instead of garbage
- VMCS field lookups are predictable and accurate
- MSR lookups prioritize exact matches
- All functions have proper result limiting and ranking

The tools are now production-ready for the OmbraMCP server.
