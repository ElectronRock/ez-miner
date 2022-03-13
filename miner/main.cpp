
#include <iostream>
#include <array>
#include <functional>

#include "sha256_openssl.h"
#include  "miner.h"

struct data  {

};

int main() {
        data data_obj;

        miner miner_obj(
                data_obj, 
                [](const data& d, unsigned id){
                        return crypto::sha256::hash_t{};
                }, 
                [] (const crypto::sha256::hash_t& hash ){
                        return true;
                }
        );

        miner_obj.do_work();
        std::cout << "kekv" << std::endl;
}