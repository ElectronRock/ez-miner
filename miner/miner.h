/* 
 *       Copyright (C) 2022 ElectronRock - All Rights Reserved
 */

#pragma once

#include <array>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

template<typename Data, typename WorkFunction, typename CheckFunction>
class miner final {
public:

    /*
    * <Constructor>
    *
    * @param data DTO object, contains previous block-chain header or what ever else;
    *
    * @param workFunction the function to be used to compute hash, signature:
    *    hash_t WorkFunction(const Data& data, unsigned task_number);
    *
    * @param checkFunction the function to be used to check weather computed hash suitable to the specified conditions
    * or not (mean hash complexity), signature:
    * bool CheckFunction(const hash_t& hash);
    *
    *  @usage
    * miner miner_obj(data,
    *   [](const data_t&, unsigned task_id){ /// return hash_t{};},
    *   [](const hash_t){ return true; }
    * );
    */
    miner(const Data& data, WorkFunction&& workFunction, CheckFunction&& checkFunction)
            : m_data(data)
              , m_workFunction(std::move(workFunction))
              , m_checkFunction(std::move(checkFunction)) {
    };

    ~miner(){
        for(auto&& thread : m_pool)
            if(thread.joinable()) thread.join();
    }

    auto do_work() {
        auto thread_count = std::thread::hardware_concurrency();
        m_pool.reserve(thread_count);
        for(auto&& active : m_active)
            active.store(true);
        auto step = std::numeric_limits<unsigned>::max() / thread_count;
        m_found = false;
        for (unsigned thread_id = 0; thread_id < thread_count; ++thread_id) {
            m_pool.emplace_back([=, this]{ 
                auto first_nonce = thread_id * step;
                find(thread_id, first_nonce, first_nonce + step);
            });
        }

        m_found.wait(false);

        for(auto&& active : m_active)
            active.store(false, std::memory_order::release);
        
        return m_result.load(std::memory_order::acquire);
    }

private:

    void find(unsigned thread, unsigned first_nonce, unsigned last_nonce){
        for(unsigned nonce { first_nonce }; nonce < last_nonce && m_active[thread].load(); ++nonce) {
            auto result = m_workFunction(m_data, nonce);
            bool is_correct = m_checkFunction(result);
            if (is_correct) {
                m_result.store(nonce);
                m_found = true;
                m_found.notify_one();
                break;
            }
        }
    }

    Data m_data;
    WorkFunction m_workFunction;
    CheckFunction m_checkFunction;

    constexpr static unsigned MaxThreadCount = 64;
    std::atomic<unsigned> m_result;
    std::vector<std::thread> m_pool;
    std::array<std::atomic_bool, MaxThreadCount> m_active;
    std::atomic_bool m_found;
};

template<typename Data, typename WorkFunction, typename CheckFunction>
miner(Data, WorkFunction, CheckFunction)->miner<Data, WorkFunction, CheckFunction>;