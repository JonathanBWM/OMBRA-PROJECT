# Scanner Integration - Testing Checklist

## Pre-Testing Setup

- [ ] Database initialized: `python scripts/init_db.py`
- [ ] MCP server can start: `driver-re-mcp` or `python -m driver_re_mcp`
- [ ] Example JSON files in place: `examples/scanner_*.json`

## Tool Testing

### 1. import_scanner_results

**Test: Valid batch import**
```bash
mcp-cli call driver-re-mcp/import_scanner_results '{
  "json_path": "/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/examples/scanner_batch_example.json"
}'
```

Expected:
```json
{
  "success": true,
  "imported": 3,
  "skipped": 0,
  "errors": [],
  "message": "Imported 3 drivers, skipped 0 duplicates"
}
```

**Test: Duplicate import (run again)**
Expected:
```json
{
  "success": true,
  "imported": 0,
  "skipped": 3,
  "errors": [],
  "message": "Imported 0 drivers, skipped 3 duplicates"
}
```

**Test: Invalid path**
```bash
mcp-cli call driver-re-mcp/import_scanner_results '{
  "json_path": "/nonexistent/file.json"
}'
```

Expected:
```json
{
  "success": false,
  "error": "File not found: /nonexistent/file.json"
}
```

- [ ] Valid batch import works
- [ ] Duplicate detection works
- [ ] Invalid path returns error

---

### 2. get_analysis_queue

**Test: Get all scanner imports**
```bash
mcp-cli call driver-re-mcp/get_analysis_queue '{}'
```

Expected:
```json
{
  "success": true,
  "count": 3,
  "queue": [
    {
      "driver_id": "...",
      "name": "ld9boxdrv.sys",
      "tier": "S",
      "score": 95.0,
      "priority": 1,
      ...
    }
  ]
}
```

**Test: Filter by priority**
```bash
mcp-cli call driver-re-mcp/get_analysis_queue '{
  "min_priority": 1,
  "max_priority": 1
}'
```

Expected: Only Tier S drivers (priority 1)

- [ ] Queue returns all imported drivers
- [ ] Sorting by priority/score works
- [ ] Priority filtering works

---

### 3. import_single_driver_analysis

**Test: Import single driver**
```bash
mcp-cli call driver-re-mcp/import_single_driver_analysis - <<'EOF'
{
  "json_data": "{\"driver\": {\"path\": \"/test/driver.sys\", \"name\": \"test.sys\", \"hash_sha256\": \"test123\", \"hash_md5\": \"md5test\", \"hash_sha1\": \"sha1test\"}, \"scanner_results\": {\"capability_tier\": \"A\", \"final_score\": 85.0, \"classification\": \"Test Driver\", \"driver_family\": \"Test\", \"priority\": 2}, \"capabilities\": {\"module_loading\": true, \"physical_memory\": false, \"msr_access\": false, \"process_control\": false}, \"dangerous_apis\": [\"ZwLoadDriver\"], \"detected_ioctls\": [\"0x222004\"], \"age_analysis\": {\"era\": \"modern\", \"age_score\": 90, \"warning\": \"Test\"}}"
}
EOF
```

Expected:
```json
{
  "success": true,
  "driver_id": "...",
  "tier": "A",
  "score": 85.0,
  "priority": 2
}
```

**Test: Duplicate SHA256**
Run same import again. Expected: Error with existing driver_id.

- [ ] Single driver import works
- [ ] Tags created correctly (tier, family, cap_*)
- [ ] Duplicate detection works

---

### 4. set_driver_tier_info

**Test: Update existing driver**
Get driver_id from previous import, then:
```bash
mcp-cli call driver-re-mcp/set_driver_tier_info '{
  "driver_id": "PASTE_DRIVER_ID_HERE",
  "tier": "S",
  "score": 95.0,
  "classification": "Updated Classification",
  "synergies": "Module loading + Physical memory"
}'
```

Expected:
```json
{
  "success": true,
  "message": "Driver tier info updated: S (95.0)"
}
```

**Test: Invalid driver_id**
```bash
mcp-cli call driver-re-mcp/set_driver_tier_info '{
  "driver_id": "invalid-uuid",
  "tier": "S",
  "score": 95.0,
  "classification": "Test"
}'
```

Expected: Error "Driver not found"

- [ ] Tier update works
- [ ] Tags updated correctly
- [ ] Invalid driver_id returns error

---

### 5. get_drivers_by_capability

**Test: Find module loading drivers**
```bash
mcp-cli call driver-re-mcp/get_drivers_by_capability '{
  "capability": "module_loading"
}'
```

Expected: Returns drivers with `cap_module_loading` tag

**Test: Find physical memory drivers**
```bash
mcp-cli call driver-re-mcp/get_drivers_by_capability '{
  "capability": "physical_memory"
}'
```

Expected: Returns drivers with `cap_physical_memory` tag

- [ ] Capability filtering works
- [ ] Results sorted by score
- [ ] Returns empty list if no matches

---

## Integration Testing

### Workflow: Batch Import → Queue → Analyze → Update

1. Import batch:
```bash
mcp-cli call driver-re-mcp/import_scanner_results '{
  "json_path": "/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/examples/scanner_batch_example.json"
}'
```

2. Get queue:
```bash
mcp-cli call driver-re-mcp/get_analysis_queue '{"min_priority": 1, "max_priority": 1}'
```

3. Get first driver_id from queue

4. Update status (using existing tool):
```bash
mcp-cli call driver-re-mcp/update_driver_status '{
  "driver_id": "DRIVER_ID",
  "status": "in_progress"
}'
```

5. Update tier after "analysis":
```bash
mcp-cli call driver-re-mcp/set_driver_tier_info '{
  "driver_id": "DRIVER_ID",
  "tier": "S",
  "score": 98.0,
  "classification": "Confirmed BYOVD"
}'
```

6. Verify with get_driver:
```bash
mcp-cli call driver-re-mcp/get_driver '{
  "driver_id": "DRIVER_ID"
}'
```

- [ ] Full workflow executes successfully
- [ ] Status transitions work
- [ ] Tier updates persist

---

## Database Verification

After running tests, verify database contents:

```bash
sqlite3 /path/to/driver_re.db

-- Check imported drivers
SELECT id, original_name, analysis_status, tags FROM drivers WHERE analysis_status = 'scanner_imported';

-- Check tags
SELECT original_name, tags FROM drivers WHERE tags LIKE '%tier_S%';

-- Check notes
SELECT original_name, notes FROM drivers WHERE notes LIKE '%Tier:%';
```

- [ ] Drivers inserted correctly
- [ ] Tags stored as JSON arrays
- [ ] Notes structured properly
- [ ] No duplicate SHA256 entries

---

## Error Handling

Test error conditions:

1. Invalid JSON path
2. Malformed JSON
3. Missing required fields in JSON
4. Duplicate SHA256
5. Invalid driver_id
6. Invalid tier value
7. Invalid capability name

- [ ] All error cases return proper error messages
- [ ] No database corruption on errors
- [ ] Transactions rolled back on failure

---

## Performance Testing

### Batch Import

Test with larger batch (100+ drivers):
```bash
# Generate test batch with 100 drivers
# Run import_scanner_results
# Measure time
```

Expected: <1 second for 100 drivers

- [ ] Batch import completes in reasonable time
- [ ] No memory leaks
- [ ] Database handles large imports

### Queue Retrieval

Test with 1000+ imported drivers:
```bash
# Import many drivers
# Run get_analysis_queue
# Measure time
```

Expected: <100ms

- [ ] Queue retrieval fast (<100ms for 1000 drivers)
- [ ] Sorting works correctly at scale

---

## Documentation Verification

- [ ] scanner_integration.md examples work
- [ ] SCANNER_TOOLS_REFERENCE.md accurate
- [ ] README.md quick start works
- [ ] Example JSON files valid

---

## Cleanup

After testing:
```bash
# Remove test database
rm /path/to/driver_re.db

# Re-initialize for production
python scripts/init_db.py
```

- [ ] Test database cleaned up
- [ ] Fresh database ready for production

---

## Sign-Off

- [ ] All 5 tools tested and working
- [ ] Integration workflow verified
- [ ] Error handling tested
- [ ] Performance acceptable
- [ ] Documentation accurate

**Tester:**
**Date:**
**Status:** [ ] PASS / [ ] FAIL
**Notes:**
