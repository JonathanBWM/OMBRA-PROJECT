"""
ChromaDB embedding provider.

Uses ChromaDB's default embedding function (all-MiniLM-L6-v2 via ONNX).
No PyTorch dependency, lightweight, fast.
"""

import chromadb
from chromadb.config import Settings
from chromadb.utils import embedding_functions
from typing import Dict, List, Optional
import os


class ChromaDBProvider:
    """
    ChromaDB-backed embedding and vector storage provider.

    Uses default embedding function: all-MiniLM-L6-v2 (384 dimensions)
    Stored in chroma.sqlite3 with HNSW index for fast similarity search.
    """

    def __init__(self, persist_directory: str):
        """
        Initialize ChromaDB client.

        Args:
            persist_directory: Path to persist ChromaDB data
        """
        self.persist_directory = persist_directory

        # Create directory if it doesn't exist
        os.makedirs(persist_directory, exist_ok=True)

        # Initialize ChromaDB client with persistence
        self.client = chromadb.PersistentClient(
            path=persist_directory,
            settings=Settings(
                anonymized_telemetry=False,
                allow_reset=True
            )
        )

        # Use default embedding function (all-MiniLM-L6-v2)
        self.embedding_function = embedding_functions.DefaultEmbeddingFunction()

        # Cache for collections
        self._collections: Dict[str, chromadb.Collection] = {}

    def get_or_create_collection(self, name: str) -> chromadb.Collection:
        """
        Get or create a ChromaDB collection.

        Args:
            name: Collection name

        Returns:
            ChromaDB collection
        """
        if name in self._collections:
            return self._collections[name]

        collection = self.client.get_or_create_collection(
            name=name,
            embedding_function=self.embedding_function,
            metadata={"hnsw:space": "cosine"}  # Use cosine similarity
        )

        self._collections[name] = collection
        return collection

    async def add_document(
        self,
        collection: str,
        doc_id: str,
        text: str,
        metadata: Dict
    ):
        """
        Add a document to a collection.

        Args:
            collection: Collection name
            doc_id: Unique document ID
            text: Document text to embed
            metadata: Document metadata
        """
        coll = self.get_or_create_collection(collection)

        coll.upsert(
            ids=[doc_id],
            documents=[text],
            metadatas=[metadata]
        )

    async def add_documents_batch(
        self,
        collection: str,
        doc_ids: List[str],
        texts: List[str],
        metadatas: List[Dict]
    ):
        """
        Add multiple documents in batch.

        Args:
            collection: Collection name
            doc_ids: List of unique document IDs
            texts: List of document texts
            metadatas: List of metadata dicts
        """
        if len(doc_ids) != len(texts) or len(doc_ids) != len(metadatas):
            raise ValueError("doc_ids, texts, and metadatas must have same length")

        coll = self.get_or_create_collection(collection)

        # ChromaDB handles batching internally
        coll.upsert(
            ids=doc_ids,
            documents=texts,
            metadatas=metadatas
        )

    async def search(
        self,
        collection: str,
        query: str,
        n_results: int = 10,
        where: Optional[Dict] = None
    ) -> List[Dict]:
        """
        Search for similar documents.

        Args:
            collection: Collection name
            query: Query text
            n_results: Number of results to return
            where: Optional metadata filter (e.g., {"driver_id": "..."})

        Returns:
            List of results: [{id, document, metadata, distance}, ...]
        """
        coll = self.get_or_create_collection(collection)

        results = coll.query(
            query_texts=[query],
            n_results=n_results,
            where=where
        )

        # Format results
        formatted = []
        if results["ids"] and len(results["ids"][0]) > 0:
            for i in range(len(results["ids"][0])):
                formatted.append({
                    "id": results["ids"][0][i],
                    "document": results["documents"][0][i],
                    "metadata": results["metadatas"][0][i],
                    "distance": results["distances"][0][i] if "distances" in results else None
                })

        return formatted

    async def delete(self, collection: str, doc_id: str):
        """
        Delete a document from a collection.

        Args:
            collection: Collection name
            doc_id: Document ID to delete
        """
        coll = self.get_or_create_collection(collection)
        coll.delete(ids=[doc_id])

    async def delete_batch(self, collection: str, doc_ids: List[str]):
        """
        Delete multiple documents.

        Args:
            collection: Collection name
            doc_ids: List of document IDs to delete
        """
        coll = self.get_or_create_collection(collection)
        coll.delete(ids=doc_ids)

    async def delete_collection(self, collection: str):
        """
        Delete an entire collection.

        Args:
            collection: Collection name
        """
        try:
            self.client.delete_collection(name=collection)
            if collection in self._collections:
                del self._collections[collection]
        except Exception as e:
            print(f"Failed to delete collection {collection}: {e}")

    def get_collection_count(self, collection: str) -> int:
        """
        Get number of documents in a collection.

        Args:
            collection: Collection name

        Returns:
            Document count
        """
        coll = self.get_or_create_collection(collection)
        return coll.count()

    def list_collections(self) -> List[str]:
        """
        List all collections.

        Returns:
            List of collection names
        """
        return [coll.name for coll in self.client.list_collections()]

    async def rebuild_index(self, collection: str):
        """
        Rebuild the HNSW index for a collection (useful after bulk updates).

        Args:
            collection: Collection name
        """
        # ChromaDB rebuilds index automatically, but we can force a persist
        coll = self.get_or_create_collection(collection)
        # Trigger a query to ensure index is built
        coll.query(query_texts=["_rebuild_"], n_results=1)

    def get_stats(self) -> Dict:
        """
        Get ChromaDB statistics.

        Returns:
            Stats dict with collection counts
        """
        collections = self.list_collections()
        stats = {
            "total_collections": len(collections),
            "collections": {}
        }

        for coll_name in collections:
            stats["collections"][coll_name] = self.get_collection_count(coll_name)

        return stats


# Convenience functions for common collections
class DriverREChroma:
    """
    High-level interface for driver reverse engineering collections.
    """

    COLLECTIONS = {
        "drivers": "driver_metadata",
        "ioctls": "driver_ioctls",
        "functions": "driver_functions",
        "imports": "driver_imports",
        "exports": "driver_exports",
        "vulnerabilities": "driver_vulnerabilities",
        "strings": "driver_strings",
        "notes": "analysis_notes"
    }

    def __init__(self, persist_directory: str):
        self.provider = ChromaDBProvider(persist_directory)

    async def add_ioctl(self, ioctl_id: str, ioctl_data: Dict):
        """Add IOCTL to semantic search."""
        text = f"{ioctl_data['name']}: {ioctl_data.get('description', '')}. {ioctl_data.get('vulnerability_description', '')}"
        await self.provider.add_document(
            collection=self.COLLECTIONS["ioctls"],
            doc_id=ioctl_id,
            text=text,
            metadata={
                "driver_id": ioctl_data["driver_id"],
                "is_vulnerable": ioctl_data.get("is_vulnerable", False)
            }
        )

    async def search_ioctls(self, query: str, driver_id: Optional[str] = None, limit: int = 10) -> List[Dict]:
        """Search IOCTLs by semantic similarity."""
        where = {"driver_id": driver_id} if driver_id else None
        return await self.provider.search(
            collection=self.COLLECTIONS["ioctls"],
            query=query,
            n_results=limit,
            where=where
        )

    async def add_vulnerability(self, vuln_id: str, vuln_data: Dict):
        """Add vulnerability to semantic search."""
        text = f"{vuln_data['title']}: {vuln_data['description']}. {vuln_data.get('technical_details', '')}"
        await self.provider.add_document(
            collection=self.COLLECTIONS["vulnerabilities"],
            doc_id=vuln_id,
            text=text,
            metadata={
                "driver_id": vuln_data["driver_id"],
                "severity": vuln_data["severity"],
                "vulnerability_class": vuln_data["vulnerability_class"]
            }
        )

    async def search_vulnerabilities(self, query: str, driver_id: Optional[str] = None, limit: int = 10) -> List[Dict]:
        """Search vulnerabilities by semantic similarity."""
        where = {"driver_id": driver_id} if driver_id else None
        return await self.provider.search(
            collection=self.COLLECTIONS["vulnerabilities"],
            query=query,
            n_results=limit,
            where=where
        )

    def get_stats(self) -> Dict:
        """Get stats for all driver RE collections."""
        return self.provider.get_stats()
