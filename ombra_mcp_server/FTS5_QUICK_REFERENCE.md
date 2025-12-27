# FTS5 Quick Reference for OmbraMCP

## Table Reference

| Base Table | FTS5 Table | Rows | Search Fields |
|------------|------------|------|---------------|
| `vmcs_fields` | `vmcs_fields_fts` | 167 | name, description, category, subcategory |
| `exit_reasons` | `exit_reasons_fts` | 66 | name, description, handling_notes, qualification_format |
| `msrs` | `msrs_fts` | 35 | name, description, category, bit_fields |

## Common Query Patterns

### 1. Simple Search
```sql
-- Find VMCS fields related to "guest rip"
SELECT name, encoding, description
FROM vmcs_fields_fts
WHERE vmcs_fields_fts MATCH 'guest rip';
```

### 2. Multi-Term Search (AND)
```sql
-- Both terms must match
SELECT name FROM vmcs_fields_fts
WHERE vmcs_fields_fts MATCH 'processor control';
```

### 3. OR Search
```sql
-- Either term can match
SELECT name FROM exit_reasons_fts
WHERE exit_reasons_fts MATCH 'ept OR violation';
```

### 4. Exclude Terms (NOT)
```sql
-- Match first term but not second
SELECT name FROM vmcs_fields_fts
WHERE vmcs_fields_fts MATCH 'guest NOT selector';
```

### 5. Exact Phrase
```sql
-- Match exact phrase
SELECT name FROM vmcs_fields_fts
WHERE vmcs_fields_fts MATCH '"VM exit"';
```

### 6. Prefix Wildcard
```sql
-- Match any field starting with GUEST_CR
SELECT name FROM vmcs_fields_fts
WHERE vmcs_fields_fts MATCH 'GUEST_CR*';
```

### 7. Column-Specific Search
```sql
-- Search only in category field
SELECT name FROM vmcs_fields_fts
WHERE category MATCH 'host_state';
```

### 8. Ranked Results
```sql
-- Get best matches first
SELECT name, rank FROM vmcs_fields_fts
WHERE vmcs_fields_fts MATCH 'control'
ORDER BY rank
LIMIT 10;
```

### 9. Complex Boolean
```sql
-- Combine operators
SELECT name FROM exit_reasons_fts
WHERE exit_reasons_fts MATCH '(ept OR memory) AND violation';
```

### 10. Join with Base Table
```sql
-- Get full details from base table
SELECT vf.*
FROM vmcs_fields vf
JOIN vmcs_fields_fts vft ON vf.id = vft.rowid
WHERE vft.vmcs_fields_fts MATCH 'guest state'
LIMIT 10;
```

## Python Examples

### Basic Search
```python
def search_vmcs_fields(query: str) -> list:
    cursor.execute("""
        SELECT name, encoding, description
        FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH ?
    """, (query,))
    return cursor.fetchall()
```

### Ranked Search
```python
def search_ranked(query: str, limit: int = 10) -> list:
    cursor.execute("""
        SELECT name, description, rank
        FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH ?
        ORDER BY rank
        LIMIT ?
    """, (query, limit))
    return cursor.fetchall()
```

### Smart Query Builder
```python
def build_fts_query(user_query: str) -> str:
    """Convert natural language to FTS5 query."""
    terms = user_query.lower().split()

    # If it looks like a phrase, use exact match
    if len(terms) > 2 and user_query.count('"') == 0:
        return f'"{user_query}"'

    # If it has "or", convert to OR
    if 'or' in terms:
        return user_query.replace('or', 'OR')

    # If it has "not", convert to NOT
    if 'not' in terms:
        idx = terms.index('not')
        return f"{' '.join(terms[:idx])} NOT {' '.join(terms[idx+1:])}"

    # Default: AND query
    return user_query
```

## Performance Tips

1. **Use FTS5 for text search, not exact matches**
   ```sql
   -- Bad: FTS5 for exact ID lookup
   WHERE vmcs_fields_fts MATCH 'encoding:0x681E'

   -- Good: Use base table
   WHERE encoding = 0x681E
   ```

2. **Combine FTS5 with filters**
   ```sql
   -- Search within a category
   SELECT * FROM vmcs_fields_fts
   WHERE vmcs_fields_fts MATCH 'control'
   AND category = 'guest_state'
   ```

3. **Use LIMIT for large result sets**
   ```sql
   -- Always limit when exploring
   WHERE vmcs_fields_fts MATCH 'guest*' LIMIT 20
   ```

4. **Prefix over suffix**
   ```sql
   -- Good: prefix wildcard
   MATCH 'GUEST*'

   -- Avoid: suffix search (not supported)
   -- MATCH '*GUEST'  -- This won't work
   ```

## Syntax Quick Ref

| Operator | Syntax | Example |
|----------|--------|---------|
| AND | `space` or `AND` | `guest state` or `guest AND state` |
| OR | `OR` | `guest OR host` |
| NOT | `NOT` | `guest NOT selector` |
| Phrase | `"..."` | `"VM exit"` |
| Prefix | `*` | `GUEST*` |
| Column | `:` | `category:control` |
| Group | `()` | `(ept OR memory) violation` |

## Common Pitfalls

### ❌ Don't Do This
```sql
-- Using LIKE on FTS table (defeats purpose)
WHERE name LIKE '%guest%'

-- Suffix wildcards (not supported)
WHERE vmcs_fields_fts MATCH '*_SELECTOR'

-- Over-complex queries (slow)
WHERE vmcs_fields_fts MATCH '(a OR b) AND (c OR d) AND (e OR f)'
```

### ✅ Do This Instead
```sql
-- Use FTS5 MATCH
WHERE vmcs_fields_fts MATCH 'guest'

-- Use prefix wildcards
WHERE vmcs_fields_fts MATCH 'GUEST_*'

-- Simplify or break into multiple queries
WHERE vmcs_fields_fts MATCH 'a OR b'
AND category IN ('cat1', 'cat2')
```

## Testing Queries

```bash
# Interactive SQL shell
cd ombra_mcp_server
sqlite3 src/ombra_mcp/data/intel_sdm.db

# Try a query
sqlite> SELECT name FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'guest rip';
```

## Migration Status

- ✅ FTS5 tables created
- ✅ Triggers installed (auto-sync)
- ✅ Data populated (167 + 66 + 35 rows)
- ✅ Tests passing
- ⏳ MCP tools migration pending

## Resources

- Full documentation: `/ombra_mcp_server/FTS5_MIGRATION.md`
- Examples script: `python3 scripts/fts5_examples.py`
- Test suite: `python3 scripts/test_fts5.py`
- SQLite FTS5 docs: https://www.sqlite.org/fts5.html
