#include "sha256_openssl.h"
#include <openssl/sha.h>

namespace crypto {

        sha256_openssl::sha256_openssl(){

        }

    void sha256_openssl::update(const unsigned char* data, std::size_t length) {
        SHA256_Update(m_context, data, length);
    }

    void sha256_openssl::transform(const unsigned char* data) {
        SHA256_Transform(m_context, data);
    }

    const sha256::hash_t& sha256_openssl::finalize() {
        SHA256_Final(m_hash.data(), m_context);
        return m_hash;
    }

    const sha256::hash_t& sha256_openssl::hash() {
            return m_hash;
    }
}