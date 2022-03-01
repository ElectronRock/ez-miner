/* 
 *       Copyright (C) 2022 ElectronRock - All Rights Reserved
 */

#pragma once
#include <array>

namespace crypto {

        class sha256 {
        public:
                using hash_t = std::array<unsigned char, 32>;

                virtual ~sha256() = default;
                
                virtual void update(const unsigned char* data, std::size_t length) {};

                virtual void transform(const unsigned char* data) {};

                virtual const hash_t& finalize() {};

                virtual const hash_t& hash() {};
        };
}