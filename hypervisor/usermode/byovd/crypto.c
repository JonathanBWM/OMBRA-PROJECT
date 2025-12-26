/**
 * @file crypto.c
 * @brief XOR encryption implementation (C port)
 */

#include "crypto.h"
#include <string.h>

//=============================================================================
// Core Encryption/Decryption
//=============================================================================

void Crypto_EncryptInPlace(UINT8* pData, size_t cbData, UINT32 dwKey) {
    if (!pData || cbData == 0) return;

    UINT32 key = dwKey;

    for (size_t i = 0; i < cbData; i++) {
        // Extract key byte based on position
        UINT8 keyByte = (UINT8)((key >> ((i % 4) * 8)) & 0xFF);

        // XOR with key byte
        UINT8 encrypted = pData[i] ^ keyByte;
        pData[i] = encrypted;

        // Evolve key based on ciphertext
        key = Crypto_RotateLeft(key, 7) ^ encrypted;
    }
}

void Crypto_DecryptInPlace(UINT8* pData, size_t cbData, UINT32 dwKey) {
    if (!pData || cbData == 0) return;

    UINT32 key = dwKey;

    for (size_t i = 0; i < cbData; i++) {
        // Extract key byte based on position
        UINT8 keyByte = (UINT8)((key >> ((i % 4) * 8)) & 0xFF);

        // Save ciphertext byte for key evolution
        UINT8 cipherByte = pData[i];

        // XOR with key byte
        pData[i] = cipherByte ^ keyByte;

        // Evolve key based on ciphertext (same as encryption!)
        key = Crypto_RotateLeft(key, 7) ^ cipherByte;
    }
}

bool Crypto_Encrypt(const UINT8* pData, size_t cbData, UINT32 dwKey,
                    UINT8** ppEncrypted, size_t* pcbEncrypted) {
    if (!pData || cbData == 0 || !ppEncrypted || !pcbEncrypted) {
        return false;
    }

    // Allocate output buffer
    UINT8* pEncrypted = (UINT8*)HeapAlloc(GetProcessHeap(), 0, cbData);
    if (!pEncrypted) {
        return false;
    }

    // Copy data
    memcpy(pEncrypted, pData, cbData);

    // Encrypt in place
    Crypto_EncryptInPlace(pEncrypted, cbData, dwKey);

    *ppEncrypted = pEncrypted;
    *pcbEncrypted = cbData;
    return true;
}

bool Crypto_Decrypt(const UINT8* pEncrypted, size_t cbData, UINT32 dwKey,
                    UINT8** ppDecrypted, size_t* pcbDecrypted) {
    if (!pEncrypted || cbData == 0 || !ppDecrypted || !pcbDecrypted) {
        return false;
    }

    // Allocate output buffer
    UINT8* pDecrypted = (UINT8*)HeapAlloc(GetProcessHeap(), 0, cbData);
    if (!pDecrypted) {
        return false;
    }

    // Copy data
    memcpy(pDecrypted, pEncrypted, cbData);

    // Decrypt in place
    Crypto_DecryptInPlace(pDecrypted, cbData, dwKey);

    *ppDecrypted = pDecrypted;
    *pcbDecrypted = cbData;
    return true;
}

//=============================================================================
// Header-Based Encryption
//=============================================================================

bool Crypto_EncryptWithHeader(const UINT8* pData, size_t cbData, UINT32 dwKey,
                               UINT8** ppBlob, size_t* pcbBlob) {
    if (!pData || cbData == 0 || !ppBlob || !pcbBlob) {
        return false;
    }

    size_t blobSize = CRYPTO_HEADER_SIZE + cbData;

    // Allocate blob
    UINT8* pBlob = (UINT8*)HeapAlloc(GetProcessHeap(), 0, blobSize);
    if (!pBlob) {
        return false;
    }

    // Write magic marker
    UINT32 magic = CRYPTO_MARKER_MAGIC;
    memcpy(&pBlob[0], &magic, sizeof(magic));

    // Write XOR'd key
    UINT32 xoredKey = dwKey ^ CRYPTO_KEY_XOR_MASK;
    memcpy(&pBlob[4], &xoredKey, sizeof(xoredKey));

    // Copy plaintext data after header
    memcpy(&pBlob[CRYPTO_HEADER_SIZE], pData, cbData);

    // Encrypt the data portion in place
    Crypto_EncryptInPlace(&pBlob[CRYPTO_HEADER_SIZE], cbData, dwKey);

    *ppBlob = pBlob;
    *pcbBlob = blobSize;
    return true;
}

bool Crypto_DecryptWithHeader(const UINT8* pBlob, size_t cbBlob,
                               UINT8** ppDecrypted, size_t* pcbDecrypted,
                               UINT32* pdwKeyOut) {
    if (!pBlob || !ppDecrypted || !pcbDecrypted) {
        return false;
    }

    // Validate minimum size
    if (cbBlob < CRYPTO_HEADER_SIZE) {
        DbgLog("Crypto_DecryptWithHeader: Blob too small (%zu < %d)",
               cbBlob, CRYPTO_HEADER_SIZE);
        return false;
    }

    // Validate magic marker
    UINT32 magic;
    memcpy(&magic, &pBlob[0], sizeof(magic));
    if (magic != CRYPTO_MARKER_MAGIC) {
        DbgLog("Crypto_DecryptWithHeader: Invalid magic 0x%08X (expected 0x%08X)",
               magic, CRYPTO_MARKER_MAGIC);
        return false;
    }

    // Extract and de-XOR key
    UINT32 xoredKey;
    memcpy(&xoredKey, &pBlob[4], sizeof(xoredKey));
    UINT32 key = xoredKey ^ CRYPTO_KEY_XOR_MASK;

    if (pdwKeyOut) {
        *pdwKeyOut = key;
    }

    // Calculate data size
    size_t cbData = cbBlob - CRYPTO_HEADER_SIZE;

    // Allocate output buffer
    UINT8* pDecrypted = (UINT8*)HeapAlloc(GetProcessHeap(), 0, cbData);
    if (!pDecrypted) {
        return false;
    }

    // Copy encrypted data
    memcpy(pDecrypted, &pBlob[CRYPTO_HEADER_SIZE], cbData);

    // Decrypt in place
    Crypto_DecryptInPlace(pDecrypted, cbData, key);

    *ppDecrypted = pDecrypted;
    *pcbDecrypted = cbData;
    return true;
}

//=============================================================================
// Key Generation
//=============================================================================

UINT32 Crypto_GenerateKey(void) {
    // Simple entropy mixing using available system values
    UINT32 key = 0;

    // Mix in tick count
    key ^= GetTickCount();

    // Mix in performance counter
    LARGE_INTEGER perfCount;
    if (QueryPerformanceCounter(&perfCount)) {
        key ^= (UINT32)perfCount.LowPart;
        key ^= (UINT32)perfCount.HighPart << 16;
    }

    // Mix in process/thread IDs
    key ^= GetCurrentProcessId();
    key ^= GetCurrentThreadId() << 16;

    // Additional mixing
    key = Crypto_RotateLeft(key, 13);
    key ^= 0xDEADBEEF;
    key = Crypto_RotateLeft(key, 7);

    return key;
}
