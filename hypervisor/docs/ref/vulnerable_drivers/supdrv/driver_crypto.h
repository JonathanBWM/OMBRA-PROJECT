/**
 * @file driver_crypto.h
 * @brief XOR encryption for embedded driver blobs
 *
 * Simple rolling-key XOR encryption to avoid static AV detection
 * of embedded driver signatures. Not cryptographically secure,
 * but effective against signature-based scanning.
 */

#pragma once
#include <Windows.h>
#include <vector>
#include <cstdint>

namespace supdrv {

/**
 * DriverCrypto - Rolling XOR encryption/decryption
 *
 * The key evolves with each byte, making pattern matching difficult.
 * Algorithm:
 *   encrypted[i] = data[i] XOR key_byte
 *   key = rotl(key, 7) XOR encrypted[i]
 *
 * This ensures the same plaintext at different offsets produces
 * different ciphertext, defeating simple XOR pattern detection.
 */
class DriverCrypto {
public:
    /**
     * Encrypt data with rolling XOR
     *
     * @param pData Pointer to plaintext data
     * @param cbData Size of data
     * @param dwKey Initial 32-bit key
     * @return Encrypted data as vector
     */
    static std::vector<uint8_t> Encrypt(const uint8_t* pData, size_t cbData,
                                         uint32_t dwKey);

    /**
     * Decrypt data with rolling XOR
     *
     * @param pEncrypted Pointer to encrypted data
     * @param cbData Size of data
     * @param dwKey Initial 32-bit key (must match encryption key)
     * @return Decrypted data as vector
     */
    static std::vector<uint8_t> Decrypt(const uint8_t* pEncrypted, size_t cbData,
                                         uint32_t dwKey);

    /**
     * Encrypt data in-place
     *
     * @param pData Pointer to data (modified in place)
     * @param cbData Size of data
     * @param dwKey Initial 32-bit key
     */
    static void EncryptInPlace(uint8_t* pData, size_t cbData, uint32_t dwKey);

    /**
     * Decrypt data in-place
     *
     * @param pData Pointer to encrypted data (modified in place)
     * @param cbData Size of data
     * @param dwKey Initial 32-bit key
     */
    static void DecryptInPlace(uint8_t* pData, size_t cbData, uint32_t dwKey);

    /**
     * Generate a random key based on system entropy
     * @return Random 32-bit key
     */
    static uint32_t GenerateKey();

    /**
     * Default encryption key
     * Should be randomized per-build for production
     */
    static constexpr uint32_t DEFAULT_KEY = 0xDEADBEEF;

    /**
     * Marker bytes prepended to encrypted data for validation
     * Format: [MARKER_MAGIC][KEY_XOR_MASK][encrypted_data...]
     */
    static constexpr uint32_t MARKER_MAGIC = 0x4F4D4252;  // "OMBR"
    static constexpr uint32_t KEY_XOR_MASK = 0xBABAB00E;

    /**
     * Encrypt with header (includes magic and XOR'd key)
     *
     * @param pData Pointer to plaintext
     * @param cbData Size of plaintext
     * @param dwKey Encryption key
     * @return Encrypted blob with 8-byte header
     */
    static std::vector<uint8_t> EncryptWithHeader(const uint8_t* pData,
                                                   size_t cbData,
                                                   uint32_t dwKey);

    /**
     * Decrypt blob with header validation
     *
     * @param pBlob Pointer to encrypted blob
     * @param cbBlob Total size including header
     * @param pdwKeyOut Optional: receives the extracted key
     * @return Decrypted data, or empty vector on validation failure
     */
    static std::vector<uint8_t> DecryptWithHeader(const uint8_t* pBlob,
                                                   size_t cbBlob,
                                                   uint32_t* pdwKeyOut = nullptr);

private:
    // Rotate left helper
    static inline uint32_t RotateLeft(uint32_t value, int shift)
    {
        return (value << shift) | (value >> (32 - shift));
    }
};

} // namespace supdrv
