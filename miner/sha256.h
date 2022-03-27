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
            
        virtual void update(const unsigned char* data, std::size_t length) = 0;    
        virtual void transform(const unsigned char* data) = 0;     
        virtual const hash_t& finalize() = 0;      
        virtual const hash_t& hash() = 0;
    };

}