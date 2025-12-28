#!/usr/bin/env python3
"""
Initialize the MS Learn Reference Database.

Stores curated Windows kernel documentation with intelligent chunking,
API references, code examples, and our project-specific annotations.

Usage:
    python init_mslearn_db.py
"""

import sqlite3
from pathlib import Path

DATA_DIR = Path(__file__).parent.parent / "src" / "ombra_mcp" / "data"
DB_PATH = DATA_DIR / "mslearn_reference.db"


def init_database():
    """Create the MS Learn reference database schema."""
    DATA_DIR.mkdir(parents=True, exist_ok=True)

    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()

    # Topics - high-level categories we care about
    c.execute("""
        CREATE TABLE IF NOT EXISTS topics (
            id INTEGER PRIMARY KEY,
            name TEXT UNIQUE NOT NULL,
            description TEXT,
            relevance TEXT,              -- Why this matters for our project
            priority INTEGER DEFAULT 0   -- Higher = more important for our work
        )
    """)

    # Pages - full MS Learn pages we've scraped
    c.execute("""
        CREATE TABLE IF NOT EXISTS pages (
            id INTEGER PRIMARY KEY,
            url TEXT UNIQUE NOT NULL,
            title TEXT,
            toc_path TEXT,               -- Breadcrumb navigation path
            full_content TEXT,           -- Complete page text (for full context retrieval)
            word_count INTEGER,
            last_fetched TEXT,
            last_updated TEXT,           -- MS's last update date
            our_notes TEXT               -- Our project-specific annotations
        )
    """)

    # Page-Topic mapping (many-to-many)
    c.execute("""
        CREATE TABLE IF NOT EXISTS page_topics (
            page_id INTEGER REFERENCES pages(id),
            topic_id INTEGER REFERENCES topics(id),
            PRIMARY KEY (page_id, topic_id)
        )
    """)

    # Concepts - semantic chunks of ~200-400 words
    # This is the main queryable unit for quick lookups
    c.execute("""
        CREATE TABLE IF NOT EXISTS concepts (
            id INTEGER PRIMARY KEY,
            page_id INTEGER REFERENCES pages(id),
            heading TEXT,                -- Section heading this came from
            content TEXT NOT NULL,       -- The chunk content
            word_count INTEGER,
            chunk_index INTEGER,         -- Order within the page
            concept_type TEXT,           -- 'overview', 'usage', 'warning', 'example', 'api_list'
            keywords TEXT,               -- Extracted keywords for search boost
            our_notes TEXT               -- Our annotations
        )
    """)

    # API References - individual functions, macros, structures
    c.execute("""
        CREATE TABLE IF NOT EXISTS api_references (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL,
            api_type TEXT,               -- 'function', 'macro', 'structure', 'enum', 'callback', 'constant'
            header TEXT,                 -- e.g., 'wdm.h', 'ntifs.h'
            url TEXT,                    -- Link to full DDI docs
            short_description TEXT,      -- One-liner
            parameters TEXT,             -- JSON: [{name, type, description}]
            return_value TEXT,
            remarks TEXT,                -- Important usage notes from MS
            our_notes TEXT,              -- Our experience, gotchas, tips
            related_apis TEXT,           -- JSON array of related API names
            UNIQUE(name, api_type)
        )
    """)

    # API mentioned in which concepts
    c.execute("""
        CREATE TABLE IF NOT EXISTS concept_apis (
            concept_id INTEGER REFERENCES concepts(id),
            api_id INTEGER REFERENCES api_references(id),
            PRIMARY KEY (concept_id, api_id)
        )
    """)

    # Code Examples - extracted and annotated
    c.execute("""
        CREATE TABLE IF NOT EXISTS code_examples (
            id INTEGER PRIMARY KEY,
            page_id INTEGER REFERENCES pages(id),
            concept_id INTEGER REFERENCES concepts(id),  -- Which concept this belongs to
            language TEXT DEFAULT 'c',
            title TEXT,                  -- What this example demonstrates
            code TEXT NOT NULL,
            line_count INTEGER,
            our_notes TEXT,              -- Our annotations, modifications
            use_case TEXT,               -- How this applies to our work
            verified INTEGER DEFAULT 0   -- Have we tested/verified this?
        )
    """)

    # Cross-references to our other databases
    c.execute("""
        CREATE TABLE IF NOT EXISTS cross_references (
            id INTEGER PRIMARY KEY,
            source_type TEXT,            -- 'api', 'concept', 'example'
            source_id INTEGER,
            target_db TEXT,              -- 'project_brain', 'evasion_techniques', 'anticheat_intel', 'byovd_drivers'
            target_type TEXT,            -- 'gotcha', 'technique', 'detection', 'driver'
            target_id TEXT,              -- ID in that database
            relationship TEXT,           -- 'relates_to', 'warns_about', 'enables', 'detected_by'
            notes TEXT
        )
    """)

    # Search optimization - FTS5 virtual table
    c.execute("""
        CREATE VIRTUAL TABLE IF NOT EXISTS concepts_fts USING fts5(
            heading,
            content,
            keywords,
            our_notes,
            content='concepts',
            content_rowid='id'
        )
    """)

    c.execute("""
        CREATE VIRTUAL TABLE IF NOT EXISTS api_fts USING fts5(
            name,
            short_description,
            remarks,
            our_notes,
            content='api_references',
            content_rowid='id'
        )
    """)

    # Triggers to keep FTS in sync
    c.execute("""
        CREATE TRIGGER IF NOT EXISTS concepts_ai AFTER INSERT ON concepts BEGIN
            INSERT INTO concepts_fts(rowid, heading, content, keywords, our_notes)
            VALUES (new.id, new.heading, new.content, new.keywords, new.our_notes);
        END
    """)

    c.execute("""
        CREATE TRIGGER IF NOT EXISTS concepts_ad AFTER DELETE ON concepts BEGIN
            INSERT INTO concepts_fts(concepts_fts, rowid, heading, content, keywords, our_notes)
            VALUES ('delete', old.id, old.heading, old.content, old.keywords, old.our_notes);
        END
    """)

    c.execute("""
        CREATE TRIGGER IF NOT EXISTS concepts_au AFTER UPDATE ON concepts BEGIN
            INSERT INTO concepts_fts(concepts_fts, rowid, heading, content, keywords, our_notes)
            VALUES ('delete', old.id, old.heading, old.content, old.keywords, old.our_notes);
            INSERT INTO concepts_fts(rowid, heading, content, keywords, our_notes)
            VALUES (new.id, new.heading, new.content, new.keywords, new.our_notes);
        END
    """)

    c.execute("""
        CREATE TRIGGER IF NOT EXISTS api_ai AFTER INSERT ON api_references BEGIN
            INSERT INTO api_fts(rowid, name, short_description, remarks, our_notes)
            VALUES (new.id, new.name, new.short_description, new.remarks, new.our_notes);
        END
    """)

    c.execute("""
        CREATE TRIGGER IF NOT EXISTS api_ad AFTER DELETE ON api_references BEGIN
            INSERT INTO api_fts(api_fts, rowid, name, short_description, remarks, our_notes)
            VALUES ('delete', old.id, old.name, old.short_description, old.remarks, old.our_notes);
        END
    """)

    c.execute("""
        CREATE TRIGGER IF NOT EXISTS api_au AFTER UPDATE ON api_references BEGIN
            INSERT INTO api_fts(api_fts, rowid, name, short_description, remarks, our_notes)
            VALUES ('delete', old.id, old.name, old.short_description, old.remarks, old.our_notes);
            INSERT INTO api_fts(rowid, name, short_description, remarks, our_notes)
            VALUES (new.id, new.name, new.short_description, new.remarks, new.our_notes);
        END
    """)

    # Indexes for common queries
    c.execute("CREATE INDEX IF NOT EXISTS idx_concepts_page ON concepts(page_id)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_concepts_type ON concepts(concept_type)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_api_type ON api_references(api_type)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_api_header ON api_references(header)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_examples_page ON code_examples(page_id)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_crossref_source ON cross_references(source_type, source_id)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_crossref_target ON cross_references(target_db, target_id)")

    conn.commit()

    # Seed initial topics relevant to our project
    topics = [
        ("MDL", "Memory Descriptor Lists - describe physical page layout for virtual buffers",
         "Core to understanding how drivers manage physical memory. Used in our physical read/write primitives.", 10),
        ("Physical Memory", "Direct physical memory access and mapping",
         "Essential for hypervisor EPT manipulation and BYOVD exploitation.", 10),
        ("Pool Allocation", "Kernel pool memory allocation (paged, nonpaged, session)",
         "Understanding pool allocation helps us hide allocations from BigPoolTable scans.", 9),
        ("Driver Communication", "IRP handling, IOCTL, device objects",
         "Required for exploiting ld9boxsup.sys and building OmbraDriver communication.", 9),
        ("Memory Manager", "Windows Memory Manager internals",
         "Core knowledge for EPT manipulation, PFN database understanding.", 9),
        ("PFN Database", "Page Frame Number database operations",
         "Critical for understanding how anti-cheats detect hidden memory.", 8),
        ("Driver Objects", "DRIVER_OBJECT, DEVICE_OBJECT creation and management",
         "Understanding these helps us avoid detection via driver enumeration.", 8),
        ("Interrupts", "ISR, DPC, interrupt handling",
         "Relevant for NMI handling and interrupt-based detection evasion.", 7),
        ("Synchronization", "Spinlocks, mutexes, IRQL",
         "Required for safe hypervisor code that doesn't deadlock.", 7),
        ("Virtual Memory", "Virtual address translation, page tables",
         "Foundation for manual PT walk implementation.", 8),
        ("System Routines", "Zw/Nt routines, kernel exports",
         "Understanding which APIs are safe to call from different contexts.", 6),
        ("Driver Loading", "Driver entry, initialization, unloading",
         "Helps understand driver loading detection and evasion.", 7),
    ]

    for name, desc, relevance, priority in topics:
        try:
            c.execute("""
                INSERT INTO topics (name, description, relevance, priority)
                VALUES (?, ?, ?, ?)
            """, (name, desc, relevance, priority))
        except sqlite3.IntegrityError:
            pass  # Already exists

    conn.commit()
    conn.close()

    print(f"Database initialized at: {DB_PATH}")
    print(f"Seeded {len(topics)} topics")

    return DB_PATH


if __name__ == "__main__":
    init_database()
