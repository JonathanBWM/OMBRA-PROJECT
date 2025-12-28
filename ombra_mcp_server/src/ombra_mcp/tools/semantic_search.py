"""
Unified Semantic Search for OmbraMCP

Provides semantic (meaning-based) search across all MCP databases using
ChromaDB for vector storage.

For embeddings, we use ChromaDB's built-in default embedding function which
uses onnxruntime (no PyTorch dependency - works on Python 3.13).

Usage:
    semantic_search("how do I hide memory from EAC scans")
    -> Returns relevant results from anticheat_intel, evasion_techniques, etc.
"""

import sqlite3
from pathlib import Path
from typing import List, Dict, Any, Optional
import json
import hashlib

# Lazy imports for optional dependencies
_chromadb = None
_embedding_fn = None

DATA_DIR = Path(__file__).parent.parent / "data"
CHROMA_DIR = DATA_DIR / "chroma"

# Collection names
COLLECTIONS = {
    "anticheat": "anticheat_intel",
    "evasion": "evasion_techniques",
    "byovd": "byovd_drivers",
    "sdm": "intel_sdm",
    "vergilius": "vergilius",
    "mslearn": "mslearn_reference",
    "brain": "project_brain",
}


def _get_chromadb():
    """Lazy load chromadb."""
    global _chromadb
    if _chromadb is None:
        try:
            import chromadb
            _chromadb = chromadb
        except ImportError:
            raise ImportError(
                "chromadb not installed. Run: pip install chromadb"
            )
    return _chromadb


def _get_embedding_fn():
    """Get ChromaDB's default embedding function (uses onnxruntime, no PyTorch needed)."""
    global _embedding_fn
    if _embedding_fn is None:
        try:
            from chromadb.utils.embedding_functions import DefaultEmbeddingFunction
            _embedding_fn = DefaultEmbeddingFunction()
        except ImportError:
            raise ImportError(
                "chromadb embedding function not available. Run: pip install chromadb"
            )
    return _embedding_fn


def _get_client():
    """Get or create ChromaDB client."""
    chromadb = _get_chromadb()
    CHROMA_DIR.mkdir(parents=True, exist_ok=True)
    return chromadb.PersistentClient(path=str(CHROMA_DIR))


def _content_hash(text: str) -> str:
    """Generate hash for content deduplication."""
    return hashlib.md5(text.encode()).hexdigest()[:12]


class SemanticIndex:
    """Manages semantic search indexes across all databases."""

    def __init__(self):
        self.client = _get_client()
        self.embedding_fn = _get_embedding_fn()
        self._collections = {}

    def _get_collection(self, name: str):
        """Get or create a collection with embedding function."""
        if name not in self._collections:
            self._collections[name] = self.client.get_or_create_collection(
                name=name,
                embedding_function=self.embedding_fn,
                metadata={"hnsw:space": "cosine"}
            )
        return self._collections[name]

    def index_anticheat_intel(self):
        """Index anticheat_intel.db content."""
        db_path = DATA_DIR / "anticheat_intel.db"
        if not db_path.exists():
            return {"indexed": 0, "error": "Database not found"}

        conn = sqlite3.connect(db_path)
        conn.row_factory = sqlite3.Row
        c = conn.cursor()

        collection = self._get_collection("anticheat_intel")
        indexed = 0

        # Index detection methods
        c.execute("""
            SELECT dm.id, dm.method_id, dm.name, dm.description, dm.technique,
                   dm.category, dm.severity, a.name as anticheat
            FROM detection_methods dm
            JOIN anticheats a ON dm.anticheat_id = a.id
        """)

        for row in c.fetchall():
            text = f"{row['anticheat']} {row['name']}: {row['description']} {row['technique'] or ''}"
            doc_id = f"detection_{row['method_id']}"

            # Let ChromaDB handle embedding via the collection's embedding function
            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "detection_method",
                    "anticheat": row['anticheat'],
                    "method_id": row['method_id'],
                    "name": row['name'],
                    "category": row['category'],
                    "severity": row['severity'],
                    "db_id": row['id'],
                }]
            )
            indexed += 1

        # Index bypasses
        c.execute("""
            SELECT b.id, b.technique, b.description, b.implementation,
                   dm.method_id, a.name as anticheat
            FROM bypasses b
            JOIN detection_methods dm ON b.detection_method_id = dm.id
            JOIN anticheats a ON dm.anticheat_id = a.id
        """)

        for row in c.fetchall():
            text = f"Bypass for {row['anticheat']}: {row['technique']} - {row['description']} {row['implementation'] or ''}"
            doc_id = f"bypass_{row['id']}"

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "bypass",
                    "anticheat": row['anticheat'],
                    "technique": row['technique'],
                    "for_method": row['method_id'],
                    "db_id": row['id'],
                }]
            )
            indexed += 1

        conn.close()
        return {"indexed": indexed, "collection": "anticheat_intel"}

    def index_evasion_techniques(self):
        """Index evasion_techniques.db content."""
        db_path = DATA_DIR / "evasion_techniques.db"
        if not db_path.exists():
            return {"indexed": 0, "error": "Database not found"}

        conn = sqlite3.connect(db_path)
        conn.row_factory = sqlite3.Row
        c = conn.cursor()

        collection = self._get_collection("evasion_techniques")
        indexed = 0

        # Index techniques
        c.execute("""
            SELECT t.id, t.name, t.short_name, t.description, t.use_case,
                   t.requirements, t.limitations, c.name as category
            FROM techniques t
            JOIN categories c ON t.category_id = c.id
        """)

        for row in c.fetchall():
            text = f"{row['category']}: {row['name']} - {row['description']} Use case: {row['use_case'] or ''} Limitations: {row['limitations'] or ''}"
            doc_id = f"technique_{row['short_name'] or row['id']}"

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "technique",
                    "name": row['name'],
                    "short_name": row['short_name'],
                    "category": row['category'],
                    "db_id": row['id'],
                }]
            )
            indexed += 1

        # Index bypass chains
        c.execute("SELECT id, name, description, goal FROM bypass_chains")

        for row in c.fetchall():
            text = f"Bypass chain: {row['name']} - {row['description']} Goal: {row['goal']}"
            doc_id = f"chain_{row['id']}"

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "bypass_chain",
                    "name": row['name'],
                    "db_id": row['id'],
                }]
            )
            indexed += 1

        conn.close()
        return {"indexed": indexed, "collection": "evasion_techniques"}

    def index_byovd_drivers(self):
        """Index byovd_drivers.db content."""
        db_path = DATA_DIR / "byovd_drivers.db"
        if not db_path.exists():
            return {"indexed": 0, "error": "Database not found"}

        conn = sqlite3.connect(db_path)
        conn.row_factory = sqlite3.Row
        c = conn.cursor()

        collection = self._get_collection("byovd_drivers")
        indexed = 0

        # Index drivers
        c.execute("""
            SELECT id, name, original_name, vendor, device_path,
                   legitimate_use, notes
            FROM drivers
        """)

        for row in c.fetchall():
            text = f"Driver: {row['name']} (originally {row['original_name'] or 'unknown'}) by {row['vendor']} - {row['legitimate_use'] or ''} {row['notes'] or ''}"
            doc_id = f"driver_{row['name']}"

            # Let ChromaDB handle embedding via the collection's embedding function
            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "driver",
                    "name": row['name'],
                    "vendor": row['vendor'],
                    "device_path": row['device_path'],
                    "db_id": row['id'],
                }]
            )
            indexed += 1

        # Index IOCTLs
        c.execute("""
            SELECT i.id, i.name, i.code, i.description, i.notes, d.name as driver
            FROM ioctls i
            JOIN drivers d ON i.driver_id = d.id
        """)

        for row in c.fetchall():
            text = f"IOCTL {row['name']} ({row['code']}) for {row['driver']}: {row['description'] or ''} {row['notes'] or ''}"
            doc_id = f"ioctl_{row['driver']}_{row['name']}"

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "ioctl",
                    "name": row['name'],
                    "code": row['code'],
                    "driver": row['driver'],
                    "db_id": row['id'],
                }]
            )
            indexed += 1

        # Index gotchas
        c.execute("""
            SELECT g.id, g.symptom, g.cause, g.fix, d.name as driver
            FROM driver_gotchas g
            JOIN drivers d ON g.driver_id = d.id
        """)

        for row in c.fetchall():
            text = f"Gotcha for {row['driver']}: {row['symptom']} Cause: {row['cause']} Fix: {row['fix'] or 'unknown'}"
            doc_id = f"gotcha_{row['driver']}_{row['id']}"

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "gotcha",
                    "driver": row['driver'],
                    "symptom": row['symptom'],
                    "db_id": row['id'],
                }]
            )
            indexed += 1

        conn.close()
        return {"indexed": indexed, "collection": "byovd_drivers"}

    def index_intel_sdm(self):
        """Index intel_sdm.db content (VMCS fields, exit reasons, MSRs)."""
        db_path = DATA_DIR / "intel_sdm.db"
        if not db_path.exists():
            return {"indexed": 0, "error": "Database not found"}

        conn = sqlite3.connect(db_path)
        conn.row_factory = sqlite3.Row
        c = conn.cursor()

        collection = self._get_collection("intel_sdm")
        indexed = 0

        # Index VMCS fields
        c.execute("SELECT * FROM vmcs_fields")
        for row in c.fetchall():
            text = f"VMCS field {row['name']}: {row['description'] or ''} Category: {row['category']} Encoding: {row['encoding']}"
            doc_id = f"vmcs_{row['encoding']}"

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "vmcs_field",
                    "name": row['name'],
                    "encoding": row['encoding'],
                    "category": row['category'],
                }]
            )
            indexed += 1

        # Index exit reasons
        c.execute("SELECT * FROM exit_reasons")
        for row in c.fetchall():
            text = f"VM-exit reason {row['reason_number']}: {row['name']} - {row['description'] or ''}"
            doc_id = f"exit_{row['reason_number']}"

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "exit_reason",
                    "name": row['name'],
                    "reason": row['reason_number'],
                }]
            )
            indexed += 1

        # Index MSRs
        c.execute("SELECT * FROM msrs")
        for row in c.fetchall():
            text = f"MSR {row['name']} (0x{row['address']:X}): {row['description'] or ''}"
            doc_id = f"msr_{row['address']}"

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "msr",
                    "name": row['name'],
                    "address": row['address'],
                }]
            )
            indexed += 1

        conn.close()
        return {"indexed": indexed, "collection": "intel_sdm"}

    def index_mslearn_reference(self):
        """Index mslearn_reference.db content (concepts, APIs, examples)."""
        db_path = DATA_DIR / "mslearn_reference.db"
        if not db_path.exists():
            return {"indexed": 0, "error": "Database not found"}

        conn = sqlite3.connect(db_path)
        conn.row_factory = sqlite3.Row
        c = conn.cursor()

        collection = self._get_collection("mslearn_reference")
        indexed = 0

        # Index concepts
        c.execute("""
            SELECT c.id, c.heading, c.content, c.concept_type, c.keywords,
                   p.title as page_title, p.url
            FROM concepts c
            JOIN pages p ON c.page_id = p.id
        """)
        for row in c.fetchall():
            text = f"{row['heading']}: {row['content']} Keywords: {row['keywords'] or ''} Page: {row['page_title']}"
            doc_id = f"concept_{row['id']}"

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "concept",
                    "heading": row['heading'],
                    "concept_type": row['concept_type'],
                    "page_title": row['page_title'],
                    "url": row['url'],
                    "db_id": row['id'],
                }]
            )
            indexed += 1

        # Index API references
        c.execute("SELECT * FROM api_references")
        for row in c.fetchall():
            text = f"API {row['name']} ({row['api_type']}): {row['short_description'] or ''} {row['remarks'] or ''}"
            doc_id = f"api_{row['name']}"

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "api",
                    "name": row['name'],
                    "api_type": row['api_type'],
                    "header": row['header'],
                    "db_id": row['id'],
                }]
            )
            indexed += 1

        # Index code examples
        c.execute("""
            SELECT e.id, e.title, e.code, e.language, c.heading as concept_heading
            FROM code_examples e
            LEFT JOIN concepts c ON e.concept_id = c.id
        """)
        for row in c.fetchall():
            text = f"Code example: {row['title'] or 'Untitled'} ({row['language']}): {row['code'][:500]}"
            doc_id = f"example_{row['id']}"

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "code_example",
                    "title": row['title'],
                    "language": row['language'],
                    "concept": row['concept_heading'],
                    "db_id": row['id'],
                }]
            )
            indexed += 1

        conn.close()
        return {"indexed": indexed, "collection": "mslearn_reference"}

    def index_project_brain(self):
        """Index project_brain.db content (gotchas, decisions)."""
        db_path = DATA_DIR / "project_brain.db"
        if not db_path.exists():
            return {"indexed": 0, "error": "Database not found"}

        conn = sqlite3.connect(db_path)
        conn.row_factory = sqlite3.Row
        c = conn.cursor()

        collection = self._get_collection("project_brain")
        indexed = 0

        # Index gotchas
        try:
            c.execute("SELECT * FROM gotchas")
            for row in c.fetchall():
                text = f"Gotcha: {row['symptom']} Cause: {row['cause']} Fix: {row['fix'] or 'unknown'}"
                doc_id = f"gotcha_{row['id']}"

                collection.upsert(
                    ids=[doc_id],
                    documents=[text],
                    metadatas=[{
                        "type": "gotcha",
                        "symptom": row['symptom'],
                        "db_id": row['id'],
                    }]
                )
                indexed += 1
        except Exception:
            pass

        # Index decisions
        try:
            c.execute("SELECT * FROM decisions")
            for row in c.fetchall():
                text = f"Decision: {row['title']} Context: {row['context']} Choice: {row['decision']} Rationale: {row['rationale'] or ''}"
                doc_id = f"decision_{row['id']}"

                collection.upsert(
                    ids=[doc_id],
                    documents=[text],
                    metadatas=[{
                        "type": "decision",
                        "title": row['title'],
                        "db_id": row['id'],
                    }]
                )
                indexed += 1
        except Exception:
            pass

        conn.close()
        return {"indexed": indexed, "collection": "project_brain"}

    def index_vergilius(self):
        """Index vergilius.db content (Windows kernel structures and offsets)."""
        db_path = DATA_DIR / "vergilius.db"
        if not db_path.exists():
            return {"indexed": 0, "error": "Database not found"}

        conn = sqlite3.connect(db_path)
        conn.row_factory = sqlite3.Row
        c = conn.cursor()

        collection = self._get_collection("vergilius")
        indexed = 0

        # Index type definitions (structures, enums, etc.)
        c.execute("""
            SELECT t.*, v.short_name as version_name
            FROM type_definitions t
            JOIN os_versions v ON t.version_id = v.id
        """)
        for row in c.fetchall():
            text = f"Windows kernel structure {row['name']} ({row['type_kind']}): Size {row['size_bytes'] or 'unknown'} bytes. Version: {row['version_name']}"
            doc_id = f"type_{row['id']}"

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "kernel_structure",
                    "name": row['name'],
                    "kind": row['type_kind'],
                    "size": row['size_bytes'],
                    "version": row['version_name'],
                }]
            )
            indexed += 1

        # Index critical offsets (the really useful stuff for hypervisor work)
        c.execute("""
            SELECT o.*, v.short_name as version_name
            FROM critical_offsets o
            JOIN os_versions v ON o.version_id = v.id
        """)
        for row in c.fetchall():
            text = f"Offset {row['struct_name']}.{row['field_name']} = {row['offset_hex']} ({row['offset_dec']}). Use case: {row['use_case'] or 'general'}. Version: {row['version_name']}"
            doc_id = f"offset_{row['id']}"

            collection.upsert(
                ids=[doc_id],
                documents=[text],
                metadatas=[{
                    "type": "critical_offset",
                    "struct": row['struct_name'],
                    "field": row['field_name'],
                    "offset_hex": row['offset_hex'],
                    "offset_dec": row['offset_dec'],
                    "use_case": row['use_case'],
                    "version": row['version_name'],
                }]
            )
            indexed += 1

        conn.close()
        return {"indexed": indexed, "collection": "vergilius"}

    def index_all(self) -> Dict[str, Any]:
        """Index all databases."""
        results = {}
        results["anticheat_intel"] = self.index_anticheat_intel()
        results["evasion_techniques"] = self.index_evasion_techniques()
        results["byovd_drivers"] = self.index_byovd_drivers()
        results["intel_sdm"] = self.index_intel_sdm()
        results["mslearn_reference"] = self.index_mslearn_reference()
        results["project_brain"] = self.index_project_brain()
        results["vergilius"] = self.index_vergilius()

        total = sum(r.get("indexed", 0) for r in results.values())
        results["total_indexed"] = total

        return results

    def search(
        self,
        query: str,
        collections: Optional[List[str]] = None,
        n_results: int = 10,
        filter_type: Optional[str] = None,
    ) -> List[Dict[str, Any]]:
        """
        Search across collections with semantic matching.

        Args:
            query: Natural language search query
            collections: List of collection names to search (None = all)
            n_results: Max results per collection
            filter_type: Filter by document type (e.g., "detection_method", "technique")

        Returns:
            List of results with scores and metadata
        """
        if collections is None:
            collections = list(COLLECTIONS.values())

        all_results = []

        for coll_name in collections:
            try:
                collection = self._get_collection(coll_name)

                where_filter = None
                if filter_type:
                    where_filter = {"type": filter_type}

                # Let ChromaDB handle embedding via query_texts
                results = collection.query(
                    query_texts=[query],
                    n_results=n_results,
                    where=where_filter,
                    include=["documents", "metadatas", "distances"]
                )

                if results and results["ids"] and results["ids"][0]:
                    for i, doc_id in enumerate(results["ids"][0]):
                        all_results.append({
                            "id": doc_id,
                            "collection": coll_name,
                            "document": results["documents"][0][i] if results["documents"] else None,
                            "metadata": results["metadatas"][0][i] if results["metadatas"] else {},
                            "distance": results["distances"][0][i] if results["distances"] else 1.0,
                            "score": 1.0 - results["distances"][0][i] if results["distances"] else 0.0,
                        })
            except Exception as e:
                # Collection may not exist yet
                continue

        # Sort by score (highest first)
        all_results.sort(key=lambda x: x["score"], reverse=True)

        return all_results[:n_results]


# Singleton instance
_index: Optional[SemanticIndex] = None


def get_index() -> SemanticIndex:
    """Get or create the semantic index singleton."""
    global _index
    if _index is None:
        _index = SemanticIndex()
    return _index


# MCP Tool Functions

def semantic_search(
    query: str,
    collections: Optional[List[str]] = None,
    n_results: int = 10,
    filter_type: Optional[str] = None,
) -> Dict[str, Any]:
    """
    Search all MCP databases using semantic matching.

    Args:
        query: Natural language query (e.g., "how to hide memory from EAC")
        collections: Optional list of collections to search
        n_results: Maximum number of results
        filter_type: Optional filter by type (detection_method, technique, driver, etc.)

    Returns:
        Dict with results and metadata
    """
    try:
        index = get_index()
        results = index.search(query, collections, n_results, filter_type)

        return {
            "query": query,
            "results": results,
            "count": len(results),
            "collections_searched": collections or list(COLLECTIONS.values()),
        }
    except ImportError as e:
        return {
            "error": str(e),
            "query": query,
            "results": [],
        }


def rebuild_semantic_index() -> Dict[str, Any]:
    """
    Rebuild the semantic search index from all databases.

    Call this after adding new data to databases.
    """
    try:
        index = get_index()
        return index.index_all()
    except ImportError as e:
        return {"error": str(e)}


def get_semantic_index_stats() -> Dict[str, Any]:
    """Get statistics about the semantic index."""
    try:
        client = _get_client()
        collections = client.list_collections()

        stats = {
            "collections": [],
            "total_documents": 0,
        }

        for coll in collections:
            count = coll.count()
            stats["collections"].append({
                "name": coll.name,
                "count": count,
            })
            stats["total_documents"] += count

        return stats
    except ImportError as e:
        return {"error": str(e)}
    except Exception as e:
        return {"error": str(e), "collections": []}
