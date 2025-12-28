"""
Embedding providers for semantic search.

Supports multiple embedding backends:
- ChromaDB (default all-MiniLM-L6-v2)
- OpenAI
- Local sentence-transformers
"""

from .provider import EmbeddingProvider
from .chromadb_provider import ChromaDBProvider, DriverREChroma

__all__ = [
    "EmbeddingProvider",
    "ChromaDBProvider",
    "DriverREChroma"
]
