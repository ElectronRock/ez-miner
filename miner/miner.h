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

    auto do_work() {
        auto thread_count = std::jthread::hardware_concurrency();
        m_pool.resize(thread_count);
        m_result.resize(thread_count);
        while (true) {
            for (int thread_id = 0; thread_id < m_result.size(); thread_id++) {
                auto cur_status = m_result[thread_id].status;
                if (cur_status == workerStatus::success)
                    return m_result[thread_id].data;
                if (cur_status == workerStatus::failure || cur_status == workerStatus::initial)
                    add_task(thread_id);
            }
        }
    }

private:
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


    void add_task(int thread_id) {
        m_result[thread_id].status = workerStatus::inProgress;
        m_pool[thread_id] = std::jthread([=, this] { payload(thread_id, m_last_task_id); });
        m_last_task_id++;
    }

    void payload(int thread_id, unsigned last_task_id) {
        auto result = m_workFunction(m_data, last_task_id);
        bool is_correct = m_checkFunction(result);
        if (is_correct) {
            m_result[thread_id].data = last_task_id;
            m_result[thread_id].status = workerStatus::success;
        } else {
            m_result[thread_id].status = workerStatus::failure;
        }
    }

    unsigned m_last_task_id = 0;
    Data m_data;
    WorkFunction m_workFunction;
    CheckFunction m_checkFunction;
    std::vector<result> m_result;
    std::vector<std::jthread> m_pool;
};

template<typename Data, typename WorkFunction, typename CheckFunction>
miner(Data, WorkFunction, CheckFunction)->miner<Data, WorkFunction, CheckFunction>;