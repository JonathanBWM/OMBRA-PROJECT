# Driver RE MCP Database Layer

Complete async SQLite database layer for Windows kernel driver reverse engineering.

## Files Created

### `schema.sql`
Complete database schema with 14 main tables:
- **drivers**: Driver metadata, hashes, PE info, version info
- **sections**: PE sections with characteristics
- **imports**: Imported NT kernel APIs with categorization
- **exports**: Exported functions
- **ioctls**: IOCTL handlers with vulnerability tracking
- **structures**: Data structure definitions
- **structure_members**: Structure field definitions
- **functions**: Analyzed functions with decompiled code
- **xrefs**: Cross-references (calls, jumps, data refs)
- **globals**: Global variables
- **strings**: String constants with FTS5 full-text search
- **vulnerabilities**: Vulnerability findings with CVE tracking
- **attack_chains**: Multi-step attack sequences
- **api_categories**: API categorization reference
- **analysis_sessions**: RE session tracking
- **analysis_notes**: Analysis notes and findings

### `connection.py`
Async database connection management:
- `DatabaseManager` class for connection pooling
- `init_database()` to initialize global DB instance
- `get_db()` to retrieve DB manager
- `get_db_session()` context manager for sessions
- `get_db_transaction()` context manager for transactions
- Helper methods: `execute()`, `fetch_one()`, `fetch_all()`, `insert()`, `update()`, `delete()`, `count()`, `exists()`

### `models.py`
Pydantic models for all entities:
- Type-safe data validation
- UUID generation for IDs
- Optional fields with proper defaults
- JSON field support

### `__init__.py`
Exports all database classes and functions

## Features

### SQLite Optimizations
- **WAL mode** enabled for better concurrency
- **Foreign keys** enabled for referential integrity
- **Proper indexes** on all foreign keys and search columns
- **FTS5 full-text search** for strings table
- **Triggers** for automatic timestamp updates

### Async Operations
All database operations are fully async using `aiosqlite`:
```python
from driver_re_mcp.database import init_database, get_db

# Initialize
db = init_database("driver_re.db")
await db.init_db()

# Use
driver_count = await db.count("drivers")
drivers = await db.fetch_all("SELECT * FROM drivers WHERE analysis_status = 'complete'")
```

### Type Safety
Pydantic models provide type checking and validation:
```python
from driver_re_mcp.database import Driver, IOCTL

driver = Driver(
    original_name="Ld9BoxSup.sys",
    md5="...",
    sha1="...",
    sha256="...",
    file_size=123456,
    image_base=0x140000000,
    entry_point_rva=0x1000,
    size_of_image=0x50000,
    machine=0x8664,
    subsystem=1,
    characteristics=0x2022,
)

# Validate on creation
ioctl = IOCTL(
    driver_id=driver.id,
    name="SUP_IOCTL_COOKIE",
    code=0xC0106900,
    is_vulnerable=False
)
```

### Schema Highlights

#### IOCTL Tracking
- Full CTL_CODE breakdown (device_type, function_code, method, access)
- Links to handler functions
- Input/output structure tracking
- Vulnerability assessment built-in

#### Vulnerability Management
- CVSS scoring
- CVE ID tracking
- Affected component linking (IOCTL, function, export)
- Exploitation difficulty assessment
- PoC code storage

#### Cross-Reference Support
- Function-to-function calls
- Import references
- Export references
- Data references
- Full call graph construction

#### Full-Text Search
Strings table has FTS5 virtual table for fast text search:
```sql
SELECT * FROM strings_fts WHERE value MATCH 'VirtualBox';
```

## Testing

Run the simple test to verify schema:
```bash
python3 test_simple_db.py
```

This creates a test database and verifies:
- Schema execution
- Table creation
- Insert operations
- Query operations

## Usage Example

```python
import asyncio
from driver_re_mcp.database import init_database, Driver, IOCTL

async def main():
    # Initialize database
    db = init_database("my_drivers.db")
    await db.init_db()

    # Insert a driver
    driver = Driver(
        original_name="example.sys",
        md5="abc123",
        sha1="def456",
        sha256="ghi789",
        file_size=50000,
        image_base=0x140000000,
        entry_point_rva=0x1000,
        size_of_image=0x10000,
        machine=0x8664,
        subsystem=1,
        characteristics=0x2022
    )

    driver_id = await db.insert("drivers", driver.model_dump(exclude_none=True))

    # Query
    result = await db.fetch_one(
        "SELECT * FROM drivers WHERE id = ?",
        (driver_id,)
    )
    print(f"Driver: {result['original_name']}")

    # Close
    await db.close()

asyncio.run(main())
```

## Schema Design Philosophy

1. **UUID Primary Keys**: Text UUIDs for better compatibility with MCP tools
2. **Comprehensive Metadata**: Every entity has created_at/updated_at
3. **Flexible JSON Storage**: Complex data in JSON fields (parameters, annotations, etc.)
4. **Strong Typing**: Boolean as INTEGER (0/1), proper foreign keys
5. **Security Focus**: Vulnerability tracking at every level
6. **Analysis-Oriented**: Notes, sessions, findings all first-class citizens

## Foreign Key Relationships

```
drivers
├── sections (driver_id)
├── imports (driver_id)
├── exports (driver_id)
├── ioctls (driver_id)
│   ├── handler_function_id → functions
│   ├── input_struct_id → structures
│   └── output_struct_id → structures
├── structures (driver_id)
│   └── structure_members (structure_id)
├── functions (driver_id)
├── xrefs (driver_id)
│   ├── from_function_id → functions
│   ├── to_function_id → functions
│   ├── to_import_id → imports
│   └── to_export_id → exports
├── globals (driver_id)
│   └── structure_id → structures
├── strings (driver_id)
├── vulnerabilities (driver_id)
│   ├── affected_ioctl_id → ioctls
│   ├── affected_function_id → functions
│   └── affected_export_id → exports
├── attack_chains (driver_id)
├── analysis_sessions (driver_id)
└── analysis_notes (driver_id)
    ├── session_id → analysis_sessions
    ├── related_function_id → functions
    ├── related_ioctl_id → ioctls
    └── related_vuln_id → vulnerabilities
```

## Performance Considerations

- All foreign keys have indexes
- Common query patterns have composite indexes
- FTS5 for string search (much faster than LIKE)
- WAL mode allows concurrent readers
- Transaction support for bulk operations

## Next Steps

This database layer is ready for integration with:
1. MCP tool implementations (add_driver, get_ioctl, etc.)
2. PE parser for automatic driver ingestion
3. Ghidra integration for function sync
4. ChromaDB for semantic search (embeddings can be added later)

The schema is production-ready and tested.
