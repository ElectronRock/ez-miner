
#include <iostream>
#include <array>
#include <functional>
#include <random>
#include <cassert>
#include <cstring>

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


void test_function1(int complexity) {
    data data_obj;

    std::random_device r;
    std::default_random_engine e1(r());
    std::uniform_int_distribution<int> uniform_dist(0, 256);

    for(unsigned i{0}; i < 75; ++i) {
        data_obj.header[i] = uniform_dist(e1);
    }

    crypto::sha256_openssl hasher;
    hasher.transform(data_obj.header.data());

    miner miner_obj(
            data_obj,
            [&](const data& d, unsigned id){
                header_t to_compute = d.header;
                auto* header_ptr = (uint32_t*)to_compute.data();
                header_ptr[19] = id;

                std::array<unsigned char, 64> block;
                block.fill(0);

                std::memcpy(block.data(), header_ptr + 16, 16);

                auto hasher_copy = hasher;
                hasher_copy.update(to_compute.data() + 64, 16);
                hasher_copy.update(to_compute.data(), 80);
                return hasher_copy.finalize();
            },
            [&] (const crypto::sha256::hash_t& hash ){
                uint8_t* presult = (uint8_t * )hash.data();
                for(unsigned i = 31; i >= 32 - complexity; --i){
                    printf("%02X ", presult[i]);
                    if(presult[i] > 0) return false;
                }
                std::cout << std::endl;
                return true;
            });

    auto res = miner_obj.do_work();
    auto* header = (uint32_t*)data_obj.header.data();
    header[19] = res;
    auto hash = compute_hash(compute_hash(data_obj.header));
    for(unsigned i = 31; i >= 32 - complexity; --i){
        assert(hash[i] == 0);
    }
    std::cout << res << std::endl;
}
int main() {
    for (unsigned _{0}; _ < 10; ++_)
        test_function1(1);

    for (unsigned _{0}; _ < 10; ++_)
        test_function1(2);

    for (unsigned _{0}; _ < 10; ++_)
        test_function1(3);
}
