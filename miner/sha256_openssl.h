/* 
 *       Copyright (C) 2022 ElectronRock - All Rights Reserved
 */

#pragma once

#include "sha256.h"

struct SHA256state_st;

namespace crypto {

    class sha256_openssl final : public sha256 {
    public:
        sha256_openssl();
        virtual ~sha256_openssl();

        virtual void update(const unsigned char* data, std::size_t length) override; //update

        virtual void transform(const unsigned char* data) override; //transform

        virtual const hash_t& finalize() override; //finalize

        virtual const hash_t& hash() override; //getter
    private:
        SHA256state_st* m_context { nullptr };
        hash_t m_hash = {0};
};

}