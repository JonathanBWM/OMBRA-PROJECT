"""
Search tools for Driver RE MCP server.

Provides semantic search, full-text search, and specialized search functions
for finding drivers, IOCTLs, functions, vulnerabilities, and strings.

Uses ChromaDB with ONNX-based embeddings (all-MiniLM-L6-v2) for semantic search
and SQLite FTS5 for fast full-text search.
"""

import sqlite3
from typing import Dict, List, Optional, Any
from pathlib import Path

# Lazy imports for optional dependencies
_chromadb = None
_embedding_fn = None


def _get_chromadb():
    """Lazy load chromadb."""
    global _chromadb
    if _chromadb is None:
        try:
            import chromadb
            _chromadb = chromadb
        except ImportError:
            raise ImportError("chromadb not installed. Run: pip install chromadb")
    return _chromadb


def _get_embedding_fn():
    """Get ChromaDB's default embedding function (uses onnxruntime, no PyTorch needed)."""
    global _embedding_fn
    if _embedding_fn is None:
        try:
            from chromadb.utils.embedding_functions import DefaultEmbeddingFunction
            _embedding_fn = DefaultEmbeddingFunction()
        except ImportError:
            raise ImportError("chromadb embedding function not available")
    return _embedding_fn


class SearchTools:
    """Search functionality using ChromaDB for semantic search and SQLite FTS5 for text search."""

    def __init__(self, db_path: str, chroma_path: str):
        """
        Initialize search tools.

        Args:
            db_path: Path to SQLite database
            chroma_path: Path to ChromaDB persistent storage
        """
        self.db_path = db_path
        self.chroma_path = chroma_path
        self._chroma_client = None
        self._collections = {}

    @property
    def chroma_client(self):
        """Lazy-load ChromaDB client."""
        if self._chroma_client is None:
            chromadb = _get_chromadb()
            Path(self.chroma_path).mkdir(parents=True, exist_ok=True)
            self._chroma_client = chromadb.PersistentClient(
                path=self.chroma_path,
                settings=chromadb.config.Settings(anonymized_telemetry=False)
            )
        return self._chroma_client

    def _get_collection(self, name: str):
        """Get or create a ChromaDB collection with embedding function."""
        if name not in self._collections:
            embedding_fn = _get_embedding_fn()
            self._collections[name] = self.chroma_client.get_or_create_collection(
                name=name,
                embedding_function=embedding_fn,
                metadata={"hnsw:space": "cosine"}
            )
        return self._collections[name]

    def _get_db_connection(self):
        """Get SQLite database connection."""
        conn = sqlite3.connect(self.db_path)
        conn.row_factory = sqlite3.Row
        return conn

    def index_drivers(self) -> Dict:
        """Index all drivers into ChromaDB for semantic search."""
        conn = self._get_db_connection()
        cursor = conn.cursor()
        collection = self._get_collection("driver_re_driver")
        indexed = 0

        cursor.execute("""
            SELECT id, original_name, analyzed_name, product_name, company_name,
                   file_version, description, architecture
            FROM drivers
        """)

        for row in cursor.fetchall():
            text = f"Driver {row['original_name']}: {row['description'] or ''} " \
                   f"Product: {row['product_name'] or ''} Company: {row['company_name'] or ''} " \
                   f"Architecture: {row['architecture'] or ''}"
            doc_id = row['id']

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "driver",
                    "name": row['original_name'],
                    "analyzed_name": row['analyzed_name'],
                    "company": row['company_name'],
                    "architecture": row['architecture'],
                }]
            )
            indexed += 1

        conn.close()
        return {"indexed": indexed, "collection": "driver_re_driver"}

    def index_ioctls(self) -> Dict:
        """Index all IOCTLs into ChromaDB for semantic search."""
        conn = self._get_db_connection()
        cursor = conn.cursor()
        collection = self._get_collection("driver_re_ioctl")
        indexed = 0

        cursor.execute("""
            SELECT i.id, i.name, i.code, i.method, i.access,
                   i.description, i.vulnerability_description, i.vulnerability_type,
                   i.is_vulnerable, d.original_name as driver_name
            FROM ioctls i
            JOIN drivers d ON i.driver_id = d.id
        """)

        for row in cursor.fetchall():
            vuln_text = f" VULNERABLE: {row['vulnerability_description']}" if row['is_vulnerable'] else ""
            text = f"IOCTL {row['name']} (0x{row['code']:X}) in {row['driver_name']}: " \
                   f"{row['description'] or ''} Method: {row['method']} Access: {row['access']}{vuln_text}"
            doc_id = row['id']

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "ioctl",
                    "name": row['name'],
                    "code": row['code'],
                    "driver": row['driver_name'],
                    "is_vulnerable": bool(row['is_vulnerable']),
                    "vulnerability_type": row['vulnerability_type'],
                }]
            )
            indexed += 1

        conn.close()
        return {"indexed": indexed, "collection": "driver_re_ioctl"}

    def index_functions(self) -> Dict:
        """Index all functions into ChromaDB for semantic search."""
        conn = self._get_db_connection()
        cursor = conn.cursor()
        collection = self._get_collection("driver_re_function")
        indexed = 0

        cursor.execute("""
            SELECT f.id, f.name, f.rva, f.size, f.calling_convention,
                   f.return_type, f.is_export, f.is_entry_point, f.description,
                   d.original_name as driver_name
            FROM functions f
            JOIN drivers d ON f.driver_id = d.id
        """)

        for row in cursor.fetchall():
            flags = []
            if row['is_export']:
                flags.append("EXPORTED")
            if row['is_entry_point']:
                flags.append("ENTRY_POINT")
            flags_str = f" [{', '.join(flags)}]" if flags else ""

            text = f"Function {row['name']} in {row['driver_name']}: " \
                   f"{row['description'] or ''} Returns: {row['return_type'] or 'unknown'} " \
                   f"Convention: {row['calling_convention'] or 'unknown'}{flags_str}"
            doc_id = row['id']

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "function",
                    "name": row['name'],
                    "driver": row['driver_name'],
                    "rva": row['rva'],
                    "is_export": bool(row['is_export']),
                    "is_entry_point": bool(row['is_entry_point']),
                }]
            )
            indexed += 1

        conn.close()
        return {"indexed": indexed, "collection": "driver_re_function"}

    def index_vulnerabilities(self) -> Dict:
        """Index all vulnerabilities into ChromaDB for semantic search."""
        conn = self._get_db_connection()
        cursor = conn.cursor()
        collection = self._get_collection("driver_re_vulnerability")
        indexed = 0

        cursor.execute("""
            SELECT v.id, v.title, v.vulnerability_class, v.severity, v.cve,
                   v.description, v.technical_details, v.exploitation_notes,
                   d.original_name as driver_name
            FROM vulnerabilities v
            JOIN drivers d ON v.driver_id = d.id
        """)

        for row in cursor.fetchall():
            cve_text = f" CVE: {row['cve']}" if row['cve'] else ""
            text = f"Vulnerability: {row['title']} ({row['vulnerability_class']}) " \
                   f"Severity: {row['severity']}{cve_text} in {row['driver_name']}: " \
                   f"{row['description']} {row['technical_details'] or ''} " \
                   f"Exploitation: {row['exploitation_notes'] or ''}"
            doc_id = row['id']

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "vulnerability",
                    "title": row['title'],
                    "class": row['vulnerability_class'],
                    "severity": row['severity'],
                    "cve": row['cve'],
                    "driver": row['driver_name'],
                }]
            )
            indexed += 1

        conn.close()
        return {"indexed": indexed, "collection": "driver_re_vulnerability"}

    def index_strings(self) -> Dict:
        """Index interesting strings into ChromaDB for semantic search."""
        conn = self._get_db_connection()
        cursor = conn.cursor()
        collection = self._get_collection("driver_re_string")
        indexed = 0

        # Only index categorized strings (skip generic ones)
        cursor.execute("""
            SELECT s.id, s.value, s.rva, s.category, s.encoding,
                   d.original_name as driver_name
            FROM strings s
            JOIN drivers d ON s.driver_id = d.id
            WHERE s.category IS NOT NULL
            LIMIT 5000
        """)

        for row in cursor.fetchall():
            text = f"String in {row['driver_name']} ({row['category']}): {row['value']}"
            doc_id = row['id']

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "string",
                    "value": row['value'][:200],  # Truncate long strings in metadata
                    "category": row['category'],
                    "driver": row['driver_name'],
                    "rva": row['rva'],
                }]
            )
            indexed += 1

        conn.close()
        return {"indexed": indexed, "collection": "driver_re_string"}

    def index_imports(self) -> Dict:
        """Index dangerous imports into ChromaDB for semantic search."""
        conn = self._get_db_connection()
        cursor = conn.cursor()
        collection = self._get_collection("driver_re_import")
        indexed = 0

        # Only index dangerous imports (more useful for search)
        cursor.execute("""
            SELECT i.id, i.dll_name, i.function_name, i.category,
                   i.is_dangerous, i.danger_reason, d.original_name as driver_name
            FROM imports i
            JOIN drivers d ON i.driver_id = d.id
            WHERE i.is_dangerous = 1
        """)

        for row in cursor.fetchall():
            text = f"DANGEROUS import {row['function_name']} from {row['dll_name']} " \
                   f"in {row['driver_name']}: {row['danger_reason'] or 'security risk'} " \
                   f"Category: {row['category'] or 'unknown'}"
            doc_id = row['id']

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "import",
                    "function_name": row['function_name'],
                    "dll_name": row['dll_name'],
                    "driver": row['driver_name'],
                    "is_dangerous": True,
                    "danger_reason": row['danger_reason'],
                }]
            )
            indexed += 1

        conn.close()
        return {"indexed": indexed, "collection": "driver_re_import"}

    def index_exports(self) -> Dict:
        """Index dangerous exports into ChromaDB for semantic search."""
        conn = self._get_db_connection()
        cursor = conn.cursor()
        collection = self._get_collection("driver_re_export")
        indexed = 0

        # Only index dangerous exports
        cursor.execute("""
            SELECT e.id, e.function_name, e.ordinal, e.prefix, e.category,
                   e.is_dangerous, e.danger_reason, d.original_name as driver_name
            FROM exports e
            JOIN drivers d ON e.driver_id = d.id
            WHERE e.is_dangerous = 1
        """)

        for row in cursor.fetchall():
            text = f"DANGEROUS export {row['function_name']} (ordinal {row['ordinal']}) " \
                   f"in {row['driver_name']}: {row['danger_reason'] or 'exploitation primitive'} " \
                   f"Prefix: {row['prefix'] or 'unknown'} Category: {row['category'] or 'unknown'}"
            doc_id = row['id']

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "export",
                    "function_name": row['function_name'],
                    "prefix": row['prefix'],
                    "driver": row['driver_name'],
                    "is_dangerous": True,
                    "danger_reason": row['danger_reason'],
                }]
            )
            indexed += 1

        conn.close()
        return {"indexed": indexed, "collection": "driver_re_export"}

    def rebuild_index(self) -> Dict:
        """Rebuild the entire semantic search index."""
        results = {}
        results["drivers"] = self.index_drivers()
        results["ioctls"] = self.index_ioctls()
        results["functions"] = self.index_functions()
        results["vulnerabilities"] = self.index_vulnerabilities()
        results["strings"] = self.index_strings()
        results["imports"] = self.index_imports()
        results["exports"] = self.index_exports()

        total = sum(r.get("indexed", 0) for r in results.values())
        results["total_indexed"] = total

        return results

    def get_index_stats(self) -> Dict:
        """Get statistics about the semantic index."""
        try:
            collections = self.chroma_client.list_collections()
            stats = {
                "collections": [],
                "total_documents": 0,
            }

            for coll in collections:
                if coll.name.startswith("driver_re_"):
                    count = coll.count()
                    stats["collections"].append({
                        "name": coll.name,
                        "count": count,
                    })
                    stats["total_documents"] += count

            return stats
        except Exception as e:
            return {"error": str(e), "collections": []}


async def semantic_search(
    query: str,
    driver_id: Optional[str] = None,
    entity_types: Optional[List[str]] = None,
    limit: int = 20,
    threshold: float = 0.7,
    db_path: str = "./data/driver_re.db",
    chroma_path: str = "./data/chroma"
) -> List[Dict]:
    """
    Semantic search across all entities using ChromaDB vector similarity.

    Uses all-MiniLM-L6-v2 embeddings (384 dimensions) via ChromaDB default
    embedding function with HNSW approximate nearest neighbor search.

    Args:
        query: Natural language query string
        driver_id: Optional driver UUID to filter results
        entity_types: Optional list of entity types to search
                     (driver, ioctl, function, vulnerability, string)
        limit: Maximum number of results (default 20)
        threshold: Minimum cosine similarity threshold 0-1 (default 0.7)
        db_path: Path to SQLite database
        chroma_path: Path to ChromaDB storage

    Returns:
        List of search results with entity details and similarity scores

    Example queries:
        - "physical memory read write access"
        - "arbitrary kernel memory manipulation"
        - "MSR model specific register access"
        - "privilege escalation code execution"
    """
    tools = SearchTools(db_path, chroma_path)

    # Default to all entity types if not specified
    if entity_types is None:
        entity_types = ["driver", "ioctl", "function", "vulnerability", "string"]

    results = []

    # Search each entity type
    for entity_type in entity_types:
        try:
            collection = tools._get_collection(f"driver_re_{entity_type}")

            # Build metadata filter
            where_filter = {}
            if driver_id:
                where_filter["driver_id"] = driver_id

            # Query ChromaDB
            query_results = collection.query(
                query_texts=[query],
                n_results=limit,
                where=where_filter if where_filter else None,
                include=["metadatas", "documents", "distances"]
            )

            # Process results
            if query_results and query_results["ids"]:
                for i, entity_id in enumerate(query_results["ids"][0]):
                    # Convert distance to similarity (ChromaDB uses cosine distance)
                    distance = query_results["distances"][0][i]
                    similarity = 1.0 - distance

                    if similarity >= threshold:
                        metadata = query_results["metadatas"][0][i]
                        document = query_results["documents"][0][i]

                        results.append({
                            "entity_type": entity_type,
                            "entity_id": entity_id,
                            "similarity": round(similarity, 4),
                            "text": document,
                            "metadata": metadata
                        })
        except Exception as e:
            # Collection might not exist yet - skip silently
            continue

    # Sort by similarity and return top N
    results.sort(key=lambda x: x["similarity"], reverse=True)
    return results[:limit]


async def text_search(
    query: str,
    driver_id: Optional[str] = None,
    entity_types: Optional[List[str]] = None,
    limit: int = 50,
    db_path: str = "./data/driver_re.db"
) -> List[Dict]:
    """
    Full-text search using SQLite FTS5.

    Performs exact and fuzzy matching across all text fields in the database.
    Faster than semantic search but less intelligent - good for exact
    string matching.

    Args:
        query: Text search query (supports FTS5 syntax)
        driver_id: Optional driver UUID to filter results
        entity_types: Optional list of entity types to search
        limit: Maximum number of results (default 50)
        db_path: Path to SQLite database

    Returns:
        List of search results with entity details and relevance scores

    FTS5 Query Syntax:
        - Simple: "MmMapIoSpace"
        - AND: "physical AND memory"
        - OR: "read OR write"
        - Phrase: '"arbitrary read"'
        - Prefix: "Mm*"
        - NOT: "memory NOT pool"
    """
    tools = SearchTools(db_path, None)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    results = []

    # Default to all entity types if not specified
    if entity_types is None:
        entity_types = ["driver", "ioctl", "function", "vulnerability", "string"]

    try:
        # Build entity type filter
        entity_filter = ",".join([f"'{t}'" for t in entity_types])

        # Build driver filter
        driver_filter = ""
        if driver_id:
            driver_filter = f"AND driver_id = '{driver_id}'"

        # Query FTS5 search index
        sql = f"""
            SELECT
                entity_type,
                entity_id,
                driver_id,
                search_text,
                rank
            FROM search_index
            WHERE search_index MATCH ?
                AND entity_type IN ({entity_filter})
                {driver_filter}
            ORDER BY rank
            LIMIT ?
        """

        cursor.execute(sql, (query, limit))
        rows = cursor.fetchall()

        for row in rows:
            results.append({
                "entity_type": row["entity_type"],
                "entity_id": row["entity_id"],
                "driver_id": row["driver_id"],
                "text": row["search_text"],
                "rank": row["rank"]
            })

    except sqlite3.Error as e:
        # FTS5 table might not exist yet
        results = []
    finally:
        conn.close()

    return results


async def search_strings(
    driver_id: str,
    pattern: Optional[str] = None,
    contains: Optional[str] = None,
    category: Optional[str] = None,
    limit: int = 100,
    db_path: str = "./data/driver_re.db"
) -> List[Dict]:
    """
    Search strings in a driver binary.

    Args:
        driver_id: Driver UUID
        pattern: Optional regex pattern for advanced matching
        contains: Optional simple substring match (case-insensitive)
        category: Optional category filter (error_message, debug, path, etc.)
        limit: Maximum results (default 100)
        db_path: Path to SQLite database

    Returns:
        List of string entries with RVA, VA, encoding, and references

    Categories:
        - error_message: Error/warning strings
        - debug: Debug output strings
        - path: File/directory paths
        - registry: Registry key strings
        - device_name: Device object names
        - api_name: API function names
        - format: Format strings
        - url: URLs/web addresses
    """
    tools = SearchTools(db_path, None)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        # Build query
        sql = "SELECT * FROM strings WHERE driver_id = ?"
        params = [driver_id]

        if category:
            sql += " AND category = ?"
            params.append(category)

        if contains:
            sql += " AND value LIKE ?"
            params.append(f"%{contains}%")

        if pattern:
            sql += " AND value REGEXP ?"
            params.append(pattern)

        sql += " ORDER BY rva LIMIT ?"
        params.append(limit)

        cursor.execute(sql, params)
        rows = cursor.fetchall()

        results = []
        for row in rows:
            results.append({
                "id": row["id"],
                "value": row["value"],
                "rva": row["rva"],
                "va": row["va"],
                "length": row["length"],
                "encoding": row["encoding"],
                "section_name": row["section_name"],
                "category": row["category"],
                "reference_count": row["reference_count"]
            })

        return results

    finally:
        conn.close()


async def find_similar_ioctls(
    ioctl_id: str,
    limit: int = 10,
    db_path: str = "./data/driver_re.db",
    chroma_path: str = "./data/chroma"
) -> List[Dict]:
    """
    Find IOCTLs similar to the given one across all drivers.

    Uses semantic similarity on IOCTL descriptions, purposes, and
    vulnerability descriptions to find functionally similar IOCTLs
    in other drivers.

    Args:
        ioctl_id: UUID of the IOCTL to compare against
        limit: Maximum number of similar IOCTLs (default 10)
        db_path: Path to SQLite database
        chroma_path: Path to ChromaDB storage

    Returns:
        List of similar IOCTLs with similarity scores and driver info

    Use cases:
        - Find vulnerable IOCTLs with similar patterns
        - Discover exploitation techniques across drivers
        - Identify code reuse between drivers
    """
    tools = SearchTools(db_path, chroma_path)

    # First get the target IOCTL from database
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        cursor.execute(
            "SELECT id, name, description, vulnerability_description FROM ioctls WHERE id = ?",
            (ioctl_id,)
        )
        row = cursor.fetchone()

        if not row:
            return []

        # Build query text from IOCTL details
        query_parts = [row["name"]]
        if row["description"]:
            query_parts.append(row["description"])
        if row["vulnerability_description"]:
            query_parts.append(row["vulnerability_description"])

        query_text = " ".join(query_parts)

    finally:
        conn.close()

    # Query ChromaDB for similar IOCTLs
    collection = tools._get_collection("driver_re_ioctl")

    results = collection.query(
        query_texts=[query_text],
        n_results=limit + 1,  # +1 because the IOCTL itself will be in results
        include=["metadatas", "documents", "distances"]
    )

    similar_ioctls = []
    if results and results["ids"]:
        for i, entity_id in enumerate(results["ids"][0]):
            # Skip the query IOCTL itself
            if entity_id == ioctl_id:
                continue

            distance = results["distances"][0][i]
            similarity = 1.0 - distance

            metadata = results["metadatas"][0][i]
            document = results["documents"][0][i]

            similar_ioctls.append({
                "ioctl_id": entity_id,
                "similarity": round(similarity, 4),
                "text": document,
                "metadata": metadata
            })

    return similar_ioctls[:limit]


async def find_similar_vulnerabilities(
    vuln_id: str,
    limit: int = 10,
    db_path: str = "./data/driver_re.db",
    chroma_path: str = "./data/chroma"
) -> List[Dict]:
    """
    Find vulnerabilities similar to the given one across all drivers.

    Uses semantic similarity on vulnerability descriptions, technical
    details, and exploitation notes to find similar vulnerability patterns.

    Args:
        vuln_id: UUID of the vulnerability to compare against
        limit: Maximum number of similar vulnerabilities (default 10)
        db_path: Path to SQLite database
        chroma_path: Path to ChromaDB storage

    Returns:
        List of similar vulnerabilities with similarity scores

    Use cases:
        - Find vulnerability patterns across driver families
        - Discover exploitation techniques
        - Identify common coding errors
        - Build exploit primitives database
    """
    tools = SearchTools(db_path, chroma_path)

    # Get the target vulnerability
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        cursor.execute(
            """SELECT id, title, description, technical_details,
                      vulnerability_class, exploitation_notes
               FROM vulnerabilities WHERE id = ?""",
            (vuln_id,)
        )
        row = cursor.fetchone()

        if not row:
            return []

        # Build query text
        query_parts = [row["title"], row["description"]]
        if row["technical_details"]:
            query_parts.append(row["technical_details"])
        if row["exploitation_notes"]:
            query_parts.append(row["exploitation_notes"])

        query_text = " ".join(query_parts)

    finally:
        conn.close()

    # Query ChromaDB
    collection = tools._get_collection("driver_re_vulnerability")

    results = collection.query(
        query_texts=[query_text],
        n_results=limit + 1,
        include=["metadatas", "documents", "distances"]
    )

    similar_vulns = []
    if results and results["ids"]:
        for i, entity_id in enumerate(results["ids"][0]):
            # Skip the query vuln itself
            if entity_id == vuln_id:
                continue

            distance = results["distances"][0][i]
            similarity = 1.0 - distance

            metadata = results["metadatas"][0][i]
            document = results["documents"][0][i]

            similar_vulns.append({
                "vulnerability_id": entity_id,
                "similarity": round(similarity, 4),
                "text": document,
                "metadata": metadata
            })

    return similar_vulns[:limit]


async def search_by_api_usage(
    api_name: str,
    driver_id: Optional[str] = None,
    db_path: str = "./data/driver_re.db"
) -> Dict:
    """
    Find all drivers, functions, and IOCTLs that use a specific Windows kernel API.

    Critical for vulnerability analysis - traces how user input can reach
    dangerous APIs like MmMapIoSpace, ZwSetSystemInformation, etc.

    Args:
        api_name: API function name (e.g., "MmMapIoSpace", "ZwSetSystemInformation")
        driver_id: Optional driver UUID to limit search
        db_path: Path to SQLite database

    Returns:
        Dictionary containing:
            - drivers: List of drivers importing this API
            - functions: List of functions calling this API
            - ioctls: List of IOCTLs whose handlers call this API
            - vulnerabilities: Related vulnerabilities
            - xref_count: Total number of references

    Use cases:
        - Find physical memory mapping capabilities
        - Identify arbitrary read/write primitives
        - Trace MSR/CR register access
        - Discover code execution vectors
    """
    tools = SearchTools(db_path, None)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        # Find the import across all drivers (or specific driver)
        driver_filter = ""
        params = [api_name]

        if driver_id:
            driver_filter = "AND i.driver_id = ?"
            params.append(driver_id)

        # Get drivers using this API
        cursor.execute(f"""
            SELECT DISTINCT d.id, d.original_name, d.analyzed_name, i.id as import_id
            FROM drivers d
            JOIN imports i ON d.id = i.driver_id
            WHERE i.function_name = ?
                {driver_filter}
        """, params)

        drivers = []
        import_ids = []
        for row in cursor.fetchall():
            drivers.append({
                "driver_id": row["id"],
                "original_name": row["original_name"],
                "analyzed_name": row["analyzed_name"]
            })
            import_ids.append(row["import_id"])

        if not import_ids:
            return {
                "api_name": api_name,
                "drivers": [],
                "functions": [],
                "ioctls": [],
                "vulnerabilities": [],
                "xref_count": 0
            }

        # Get all functions calling this API via xrefs
        import_id_filter = ",".join([f"'{id}'" for id in import_ids])

        cursor.execute(f"""
            SELECT DISTINCT
                f.id,
                f.driver_id,
                f.name,
                f.rva,
                f.va,
                x.from_rva,
                x.instruction
            FROM xrefs x
            JOIN functions f ON x.from_function_id = f.id
            WHERE x.to_import_id IN ({import_id_filter})
            ORDER BY f.driver_id, f.rva
        """)

        functions = []
        function_ids = []
        for row in cursor.fetchall():
            functions.append({
                "function_id": row["id"],
                "driver_id": row["driver_id"],
                "name": row["name"],
                "rva": row["rva"],
                "va": row["va"],
                "call_site_rva": row["from_rva"],
                "instruction": row["instruction"]
            })
            function_ids.append(row["id"])

        # Find IOCTLs whose handlers are in the call chain
        ioctls = []
        if function_ids:
            function_id_filter = ",".join([f"'{id}'" for id in function_ids])

            cursor.execute(f"""
                SELECT DISTINCT
                    io.id,
                    io.driver_id,
                    io.name,
                    io.code,
                    io.handler_rva,
                    io.is_vulnerable,
                    io.vulnerability_type
                FROM ioctls io
                WHERE io.handler_function_id IN ({function_id_filter})
            """)

            for row in cursor.fetchall():
                ioctls.append({
                    "ioctl_id": row["id"],
                    "driver_id": row["driver_id"],
                    "name": row["name"],
                    "code": row["code"],
                    "handler_rva": row["handler_rva"],
                    "is_vulnerable": bool(row["is_vulnerable"]),
                    "vulnerability_type": row["vulnerability_type"]
                })

        # Find related vulnerabilities
        cursor.execute(f"""
            SELECT DISTINCT
                v.id,
                v.driver_id,
                v.title,
                v.vulnerability_class,
                v.severity,
                v.description
            FROM vulnerabilities v
            WHERE v.affected_function_id IN ({function_id_filter})
        """)

        vulnerabilities = []
        for row in cursor.fetchall():
            vulnerabilities.append({
                "vulnerability_id": row["id"],
                "driver_id": row["driver_id"],
                "title": row["title"],
                "class": row["vulnerability_class"],
                "severity": row["severity"],
                "description": row["description"]
            })

        # Count total xrefs
        cursor.execute(f"""
            SELECT COUNT(*) as count
            FROM xrefs
            WHERE to_import_id IN ({import_id_filter})
        """)
        xref_count = cursor.fetchone()["count"]

        return {
            "api_name": api_name,
            "drivers": drivers,
            "functions": functions,
            "ioctls": ioctls,
            "vulnerabilities": vulnerabilities,
            "xref_count": xref_count
        }

    finally:
        conn.close()


# MCP Tool Functions for Index Management

async def rebuild_semantic_index(
    db_path: str = "./data/driver_re.db",
    chroma_path: str = "./data/chroma"
) -> Dict:
    """
    Rebuild the entire semantic search index from the database.

    Call this after bulk-loading drivers or making significant changes
    to the database. Indexes all drivers, IOCTLs, functions, vulnerabilities,
    strings, imports, and exports into ChromaDB for semantic search.

    Args:
        db_path: Path to SQLite database
        chroma_path: Path to ChromaDB storage

    Returns:
        Dict with indexing statistics per entity type and total count
    """
    try:
        tools = SearchTools(db_path, chroma_path)
        return tools.rebuild_index()
    except ImportError as e:
        return {"error": str(e), "total_indexed": 0}
    except Exception as e:
        return {"error": str(e), "total_indexed": 0}


async def get_semantic_index_stats(
    chroma_path: str = "./data/chroma"
) -> Dict:
    """
    Get statistics about the semantic search index.

    Returns collection names and document counts for the driver RE
    semantic search index.

    Args:
        chroma_path: Path to ChromaDB storage

    Returns:
        Dict with collection statistics and total document count
    """
    try:
        tools = SearchTools("", chroma_path)  # db_path not needed for stats
        return tools.get_index_stats()
    except ImportError as e:
        return {"error": str(e), "collections": []}
    except Exception as e:
        return {"error": str(e), "collections": []}
