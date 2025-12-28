/**
 * @file driver_crypto.cpp
 * @brief XOR encryption implementation
 */

#include "driver_crypto.h"
#include <random>

namespace supdrv {

//-----------------------------------------------------------------------------
// Core encryption/decryption
//-----------------------------------------------------------------------------

std::vector<uint8_t> DriverCrypto::Encrypt(const uint8_t* pData, size_t cbData,
                                            uint32_t dwKey)
{
    std::vector<uint8_t> encrypted(cbData);
    uint32_t key = dwKey;

    for (size_t i = 0; i < cbData; i++) {
        // Extract key byte based on position
        uint8_t keyByte = static_cast<uint8_t>((key >> ((i % 4) * 8)) & 0xFF);

        // XOR with key byte
        encrypted[i] = pData[i] ^ keyByte;

        // Evolve key based on ciphertext
        key = RotateLeft(key, 7) ^ encrypted[i];
    }

    return encrypted;
}

std::vector<uint8_t> DriverCrypto::Decrypt(const uint8_t* pEncrypted, size_t cbData,
                                            uint32_t dwKey)
{
    std::vector<uint8_t> decrypted(cbData);
    uint32_t key = dwKey;

    for (size_t i = 0; i < cbData; i++) {
        // Extract key byte based on position
        uint8_t keyByte = static_cast<uint8_t>((key >> ((i % 4) * 8)) & 0xFF);

        // XOR with key byte
        decrypted[i] = pEncrypted[i] ^ keyByte;

        // Evolve key based on ciphertext (same as encryption!)
        key = RotateLeft(key, 7) ^ pEncrypted[i];
    }

    return decrypted;
}

void DriverCrypto::EncryptInPlace(uint8_t* pData, size_t cbData, uint32_t dwKey)
{
    uint32_t key = dwKey;

    for (size_t i = 0; i < cbData; i++) {
        uint8_t keyByte = static_cast<uint8_t>((key >> ((i % 4) * 8)) & 0xFF);
        uint8_t encrypted = pData[i] ^ keyByte;
        pData[i] = encrypted;
        key = RotateLeft(key, 7) ^ encrypted;
    }
}

void DriverCrypto::DecryptInPlace(uint8_t* pData, size_t cbData, uint32_t dwKey)
{
    uint32_t key = dwKey;

    for (size_t i = 0; i < cbData; i++) {
        uint8_t keyByte = static_cast<uint8_t>((key >> ((i % 4) * 8)) & 0xFF);
        uint8_t cipherByte = pData[i];
        pData[i] = cipherByte ^ keyByte;
        key = RotateLeft(key, 7) ^ cipherByte;
    }
}

//-----------------------------------------------------------------------------
// Key generation
//-----------------------------------------------------------------------------

uint32_t DriverCrypto::GenerateKey()
{
    // Use hardware RNG if available, fallback to Mersenne Twister
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);

    // XOR multiple samples for extra entropy
    return dist(gen) ^ (dist(gen) << 16) ^ GetTickCount();
}

//-----------------------------------------------------------------------------
// Header-based encryption (self-describing blobs)
//-----------------------------------------------------------------------------

std::vector<uint8_t> DriverCrypto::EncryptWithHeader(const uint8_t* pData,
                                                      size_t cbData,
                                                      uint32_t dwKey)
{
    // Header format:
    // [0-3]  MARKER_MAGIC (0x4F4D4252 = "OMBR")
    // [4-7]  XOR'd key (key ^ KEY_XOR_MASK)
    // [8...]  Encrypted data

    constexpr size_t HEADER_SIZE = 8;
    std::vector<uint8_t> blob(HEADER_SIZE + cbData);

    // Write magic marker
    uint32_t magic = MARKER_MAGIC;
    memcpy(&blob[0], &magic, sizeof(magic));

    // Write XOR'd key
    uint32_t xoredKey = dwKey ^ KEY_XOR_MASK;
    memcpy(&blob[4], &xoredKey, sizeof(xoredKey));

    // Encrypt data
    auto encrypted = Encrypt(pData, cbData, dwKey);
    memcpy(&blob[HEADER_SIZE], encrypted.data(), cbData);

    return blob;
}

std::vector<uint8_t> DriverCrypto::DecryptWithHeader(const uint8_t* pBlob,
                                                      size_t cbBlob,
                                                      uint32_t* pdwKeyOut)
{
    constexpr size_t HEADER_SIZE = 8;

    // Validate minimum size
    if (cbBlob < HEADER_SIZE) {
        return {};
    }

    // Validate magic marker
    uint32_t magic;
    memcpy(&magic, &pBlob[0], sizeof(magic));
    if (magic != MARKER_MAGIC) {
        return {};
    }

    // Extract and de-XOR key
    uint32_t xoredKey;
    memcpy(&xoredKey, &pBlob[4], sizeof(xoredKey));
    uint32_t key = xoredKey ^ KEY_XOR_MASK;

    if (pdwKeyOut) {
        *pdwKeyOut = key;
    }

    // Decrypt data
    size_t cbData = cbBlob - HEADER_SIZE;
    return Decrypt(&pBlob[HEADER_SIZE], cbData, key);
}

} // namespace supdrv
