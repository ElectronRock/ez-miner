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

    ~miner() = default;

    enum class workerStatus {
        initial,
        inProgress,
        failure,
        success
    };

    struct result {
        unsigned data = 0;
        workerStatus status = workerStatus::initial;
    };

    auto compute_duration() {
        auto t1 = std::chrono::high_resolution_clock::now();
        payload(0);
        auto t2 = std::chrono::high_resolution_clock::now();
        return duration_cast<std::chrono::milliseconds>(t2 - t1);
    }

    auto do_work() {
        auto ms_int = compute_duration();
        auto thread_count = std::thread::hardware_concurrency();
        m_pool.resize(thread_count);
        m_result.resize(thread_count);
        while(true) {
            for(int index = 0; index <= m_result.size(); index++) {
                auto cur_status = m_result[index].status;
                if(cur_status == workerStatus::success)
                    return m_result[index].data;
                if(cur_status == workerStatus::failure || cur_status == workerStatus::initial)
                    add_task(index);
            }
            std::this_thread::sleep_for((3 * ms_int)/(2 * thread_count));
        }
    }

    void add_task(int index) {
        m_result[index].status = workerStatus::inProgress;
        m_pool[index] = [&, this]{ payload(m_last_task_id); };
        m_last_task_id++;
    }

    void payload(int index) {
        auto result = m_workFunction(m_data, index);
        bool is_correct = m_checkFunction(result);
        if(is_correct) {
            m_result[index].data = result;
            m_result[index].status = workerStatus::success;
        } else {
            m_result[index].status = workerStatus::failure;
        }
    }
    
private:
    int m_last_task_id = 0;
    Data m_data;
    WorkFunction m_workFunction;
    CheckFunction m_checkFunction;
    std::vector<result> m_result;
    std::vector<std::thread> m_pool;
};

template<typename Data, typename WorkFunction, typename CheckFunction>
miner(Data, WorkFunction, CheckFunction)->miner<Data, WorkFunction, CheckFunction>;