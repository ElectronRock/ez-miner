#include "sha256_openssl.h"
#include <openssl/sha.h>

namespace crypto {

    sha256_openssl::sha256_openssl(){
        m_context = new SHA256state_st;
        SHA256_Init(m_context);
    }

    sha256_openssl::sha256_openssl(const sha256_openssl& rhs){
        m_context = new SHA256state_st;
        *m_context = *rhs.m_context;
    }

    sha256_openssl::~sha256_openssl() noexcept {
        delete m_context;
    }

    void sha256_openssl::update(const unsigned char* data, std::size_t length) {
        if(m_context != nullptr) {
            SHA256_Update(m_context, data, length);
        }
    }

    void sha256_openssl::transform(const unsigned char* data) {
        if(m_context != nullptr) {
            SHA256_Transform(m_context, data);
        }
    }

    const sha256::hash_t& sha256_openssl::finalize() {
        if(m_context != nullptr) {
            SHA256_Final(m_hash.data(), m_context);
            m_context = nullptr;
            return m_hash;
        }
    }

    const sha256::hash_t& sha256_openssl::hash() {
        return m_hash;
    }
}