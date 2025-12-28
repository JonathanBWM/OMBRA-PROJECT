"""
File hashing utilities.

Computes standard hashes for driver files:
- MD5
- SHA1
- SHA256
- ImpHash (import hash)
- SSDEEP (fuzzy hash for similarity)
"""

import hashlib
import pefile
from typing import Dict, Optional


def compute_hashes(file_path: str) -> Dict[str, str]:
    """
    Compute all standard hashes for a file.

    Args:
        file_path: Path to file

    Returns:
        Dict with md5, sha1, sha256, imphash, ssdeep
    """
    hashes = {}

    # Read file
    with open(file_path, 'rb') as f:
        data = f.read()

    # Standard cryptographic hashes
    hashes['md5'] = hashlib.md5(data).hexdigest()
    hashes['sha1'] = hashlib.sha1(data).hexdigest()
    hashes['sha256'] = hashlib.sha256(data).hexdigest()

    # Import hash (PE-specific)
    try:
        hashes['imphash'] = compute_imphash(file_path)
    except:
        hashes['imphash'] = None

    # SSDEEP fuzzy hash (if available)
    try:
        hashes['ssdeep'] = compute_ssdeep(file_path)
    except:
        hashes['ssdeep'] = None

    return hashes


def compute_md5(file_path: str) -> str:
    """Compute MD5 hash."""
    with open(file_path, 'rb') as f:
        return hashlib.md5(f.read()).hexdigest()


def compute_sha1(file_path: str) -> str:
    """Compute SHA1 hash."""
    with open(file_path, 'rb') as f:
        return hashlib.sha1(f.read()).hexdigest()


def compute_sha256(file_path: str) -> str:
    """Compute SHA256 hash."""
    with open(file_path, 'rb') as f:
        return hashlib.sha256(f.read()).hexdigest()


def compute_imphash(file_path: str) -> Optional[str]:
    """
    Compute import hash (imphash) for PE file.

    Imphash is a hash of the import table, useful for identifying
    malware families that share similar import patterns.

    Args:
        file_path: Path to PE file

    Returns:
        Imphash as hex string, or None if failed
    """
    try:
        pe = pefile.PE(file_path)
        return pe.get_imphash()
    except:
        return None


def compute_ssdeep(file_path: str) -> Optional[str]:
    """
    Compute SSDEEP fuzzy hash.

    SSDEEP allows finding similar (but not identical) files.
    Useful for detecting driver variants.

    Args:
        file_path: Path to file

    Returns:
        SSDEEP hash string, or None if ssdeep not available
    """
    try:
        import ssdeep
        with open(file_path, 'rb') as f:
            return ssdeep.hash(f.read())
    except ImportError:
        # ssdeep not installed
        return None
    except Exception:
        return None


def compare_ssdeep(hash1: str, hash2: str) -> int:
    """
    Compare two SSDEEP hashes.

    Args:
        hash1: First SSDEEP hash
        hash2: Second SSDEEP hash

    Returns:
        Similarity score (0-100), or 0 if ssdeep not available
    """
    try:
        import ssdeep
        return ssdeep.compare(hash1, hash2)
    except ImportError:
        return 0
    except Exception:
        return 0


def hash_data(data: bytes, algorithm: str = 'sha256') -> str:
    """
    Hash arbitrary data.

    Args:
        data: Bytes to hash
        algorithm: 'md5', 'sha1', or 'sha256'

    Returns:
        Hash as hex string
    """
    if algorithm == 'md5':
        return hashlib.md5(data).hexdigest()
    elif algorithm == 'sha1':
        return hashlib.sha1(data).hexdigest()
    elif algorithm == 'sha256':
        return hashlib.sha256(data).hexdigest()
    else:
        raise ValueError(f"Unknown algorithm: {algorithm}")


def verify_hash(file_path: str, expected_hash: str, algorithm: str = 'sha256') -> bool:
    """
    Verify file hash matches expected value.

    Args:
        file_path: Path to file
        expected_hash: Expected hash value
        algorithm: Hash algorithm

    Returns:
        True if hashes match
    """
    if algorithm == 'md5':
        actual = compute_md5(file_path)
    elif algorithm == 'sha1':
        actual = compute_sha1(file_path)
    elif algorithm == 'sha256':
        actual = compute_sha256(file_path)
    else:
        raise ValueError(f"Unknown algorithm: {algorithm}")

    return actual.lower() == expected_hash.lower()
