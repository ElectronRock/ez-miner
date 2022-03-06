/* 
 *       Copyright (C) 2022 ElectronRock - All Rights Reserved
 */

#pragma once

#include <array>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

template<typename Data, typename Result, typename WorkFunction, typename CheckFunction>
class miner final {
public:
    miner(const Data& data, WorkFunction&& workFunction, CheckFunction&& checkFunction): m_data(data),
            m_workFunction(std::move(workFunction)), m_checkFunction(std::move(checkFunction)) {};
    ~miner() = default;

    std::vector<std::thread> m_pool;

    enum class workerStatus {
        initial,
        inProgress,
        failure,
        success
    };
    struct result {
        int data = 0;
        workerStatus status = workerStatus::initial;
    };
    std::vector<result> m_result;

    auto compute_duration() {
        auto t1 = std::chrono::high_resolution_clock::now();
        payload(m_workFunction, m_checkFunction, m_data, 0);
        auto t2 = std::chrono::high_resolution_clock::now();
        return duration_cast<std::chrono::milliseconds>(t2 - t1);
    }

    Result do_work() {
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
        m_pool[index] = [&, this]{payload(m_last_task_id);};
        m_last_task_id++;
    }
    void payload(int index) {
            Result result = m_workFuncion(m_data, index);
            bool is_correct = m_checkFunction(result);
            if(is_correct) {
                m_result[index].data = result;
                m_result[index].status = workerStatus::success;
            }
            else
                m_result[index].status = workerStatus::failure;
        }
private:
    int m_last_task_id = 0;
    Data m_data;
    WorkFunction m_workFunction;
    CheckFunction m_checkFunction;
};
