
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

        miner miner_obj(
                data_obj, 
                [](const data& d, unsigned id){
                    header_t to_compute = d.header;
                    auto* header_ptr = (uint32_t*)to_compute.data();
                    header_ptr[19] = id;

                    return compute_hash(compute_hash(to_compute));
                },
                [] (const crypto::sha256::hash_t& hash ){
                        return true;
                }
        );

        miner_obj.do_work();
        std::cout << "kekv" << std::endl;
}