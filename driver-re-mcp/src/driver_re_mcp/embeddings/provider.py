"""
Embedding provider interface.

Defines the contract for all embedding providers.
Allows swapping between ChromaDB default, OpenAI, local models, etc.
"""

from abc import ABC, abstractmethod
from typing import List


class EmbeddingProvider(ABC):
    """Abstract base class for embedding providers."""

    @abstractmethod
    async def embed(self, text: str) -> List[float]:
        """
        Generate embedding for a single text.

        Args:
            text: Text to embed

        Returns:
            Embedding vector as list of floats
        """
        pass

    @abstractmethod
    async def embed_batch(self, texts: List[str]) -> List[List[float]]:
        """
        Generate embeddings for multiple texts (batched for efficiency).

        Args:
            texts: List of texts to embed

        Returns:
            List of embedding vectors
        """
        pass

    @property
    @abstractmethod
    def dimensions(self) -> int:
        """
        Get the dimensionality of embeddings produced by this provider.

        Returns:
            Number of dimensions (e.g., 384 for MiniLM, 1536 for OpenAI)
        """
        pass

    @property
    @abstractmethod
    def model_name(self) -> str:
        """
        Get the name of the embedding model.

        Returns:
            Model name
        """
        pass


# Global embedding provider instance
_embedding_provider: EmbeddingProvider = None


def set_embedding_provider(provider: EmbeddingProvider):
    """Set the global embedding provider."""
    global _embedding_provider
    _embedding_provider = provider


async def generate_embedding(text: str) -> List[float]:
    """
    Generate embedding for text using the global provider.
    If no provider is set, returns a dummy embedding (for testing).

    Args:
        text: Text to embed

    Returns:
        Embedding vector
    """
    if _embedding_provider is None:
        # Return dummy embedding (zeros) if no provider configured
        # In production, this should be initialized properly
        return [0.0] * 384  # Default MiniLM dimension

    return await _embedding_provider.embed(text)
