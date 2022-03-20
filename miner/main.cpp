
#include <iostream>
#include <array>
#include <functional>

#include "sha256_openssl.h"
#include  "miner.h"

using header_t = std::array<unsigned char, 80>;

struct data  {
    header_t header;
};

template<typename T, std::size_t Size>
auto compute_hash(const std::array<T, Size>& to_compute) {
    crypto::sha256_openssl context;
    context.update((unsigned char*)to_compute.data(), 80);
    return context.finalize();
};

int main() {
        data data_obj;

    constexpr unsigned complexity = 1;
    miner miner_obj(
        data_obj, 
        [](const data& d, unsigned id){
            header_t to_compute = d.header;
            auto* header_ptr = (uint32_t*)to_compute.data();
            header_ptr[19] = id;
            return compute_hash(compute_hash(to_compute));
        },
        [] (const crypto::sha256::hash_t& hash ){
            uint8_t* presult = (uint8_t * )hash.data();
            for(unsigned i = 31; i >= 32 - complexity; --i){
                printf("%02X ", presult[i]);
                if(presult[i] > 0) return false;
            }
            std::cout << std::endl;
            return true;
        }
    );
    const auto nonce = miner_obj.do_work();
    std::cout << "Result: " <<  nonce << std::endl;
}