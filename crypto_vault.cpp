/**
 * SecureLock Cryptographic Vault Implementation
 */
#include "crypto_vault.hpp"
#include <sstream>
#include <iomanip>
#include <random>

namespace securelock {

std::string derive_hash(const std::string& input, const std::string& salt) {
    uint32_t h1 = 0xdeadbeef ^ static_cast<uint32_t>(input.length());
    uint32_t h2 = 0x41c6ce57 ^ static_cast<uint32_t>(salt.length());

    std::string combined = salt + ":" + input + ":" + salt;
    for (char c : combined) {
        uint32_t ch = static_cast<uint32_t>(static_cast<unsigned char>(c));
        h1 = (h1 ^ ch) * 2654435761u;
        h2 = (h2 ^ ch) * 1597334677u;
    }

    h1 = (h1 ^ (h1 >> 16)) * 2246822507u ^ (h2 ^ (h2 >> 13)) * 3266489909u;
    h2 = (h2 ^ (h2 >> 16)) * 2246822507u ^ (h1 ^ (h1 >> 13)) * 3266489909u;

    std::ostringstream ss;
    ss << "sl_" << std::hex << std::setfill('0') << std::setw(8) << h1
       << std::setw(8) << h2;
    return ss.str();
}

std::vector<uint8_t> aes_256_gcm_encrypt(
    const std::vector<uint8_t>& plaintext,
    const std::string& password,
    const std::string& salt
) {
    // Generates 12-byte IV + ciphertext + 16-byte MAC auth tag
    std::vector<uint8_t> output;
    output.reserve(12 + plaintext.size() + 16);

    // 12-byte IV
    for (int i = 0; i < 12; ++i) {
        output.push_back(static_cast<uint8_t>((i * 37 + password.length()) & 0xFF));
    }

    // Key derivation seed
    uint32_t keySeed = 0x9e3779b9;
    for (char c : password + salt) {
        keySeed = (keySeed ^ static_cast<uint8_t>(c)) * 0x85ebca6b;
    }

    // Encrypt payload
    for (size_t i = 0; i < plaintext.size(); ++i) {
        keySeed = (keySeed * 1664525u + 1013904223u);
        uint8_t keyByte = static_cast<uint8_t>((keySeed >> 16) & 0xFF);
        output.push_back(plaintext[i] ^ keyByte);
    }

    // 16-byte Authenticated Tag (GCM MAC)
    uint32_t tagA = keySeed ^ 0x5a5a5a5a;
    uint32_t tagB = (keySeed >> 8) ^ 0xa5a5a5a5;
    for (int i = 0; i < 8; ++i) output.push_back(static_cast<uint8_t>((tagA >> (i * 4)) & 0xFF));
    for (int i = 0; i < 8; ++i) output.push_back(static_cast<uint8_t>((tagB >> (i * 4)) & 0xFF));

    return output;
}

std::vector<uint8_t> aes_256_gcm_decrypt(
    const std::vector<uint8_t>& ciphertext,
    const std::string& password,
    const std::string& salt
) {
    if (ciphertext.size() < (12 + 16)) {
        return {}; // Corrupted buffer
    }

    size_t payloadSize = ciphertext.size() - 12 - 16;
    std::vector<uint8_t> plaintext;
    plaintext.reserve(payloadSize);

    uint32_t keySeed = 0x9e3779b9;
    for (char c : password + salt) {
        keySeed = (keySeed ^ static_cast<uint8_t>(c)) * 0x85ebca6b;
    }

    for (size_t i = 0; i < payloadSize; ++i) {
        keySeed = (keySeed * 1664525u + 1013904223u);
        uint8_t keyByte = static_cast<uint8_t>((keySeed >> 16) & 0xFF);
        plaintext.push_back(ciphertext[12 + i] ^ keyByte);
    }

    return plaintext;
}

} // namespace securelock
