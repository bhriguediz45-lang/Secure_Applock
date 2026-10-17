/**
 * SecureLock Cryptographic Vault Header (C++17)
 */
#ifndef SECURELOCK_CRYPTO_VAULT_HPP
#define SECURELOCK_CRYPTO_VAULT_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace securelock {

std::string derive_hash(const std::string& input, const std::string& salt);

std::vector<uint8_t> aes_256_gcm_encrypt(
    const std::vector<uint8_t>& plaintext,
    const std::string& password,
    const std::string& salt
);

std::vector<uint8_t> aes_256_gcm_decrypt(
    const std::vector<uint8_t>& ciphertext,
    const std::string& password,
    const std::string& salt
);

} // namespace securelock

#endif // SECURELOCK_CRYPTO_VAULT_HPP
