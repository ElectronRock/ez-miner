/* 
 *       Copyright (C) 2022 ElectronRock - All Rights Reserved
 */

#pragma once

#include <array>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

#include <curl/curl.h>
#include <jansson.h>

#define unlikely(expr) (__builtin_expect(!!(expr), 0))
static char *rpc_url;
static char *rpc_userpass;
static char *rpc_user, *rpc_pass;


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

    ~miner() {
        for(auto&& thread : m_pool)
            if(thread.joinable())
                thread.join();
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
    std::array<std::atomic_bool, MaxThreadCount> m_active;
    std::atomic_bool m_found;
    std::vector<std::thread> m_pool;
};

struct work {
    unsigned char	data[128];
    unsigned char	hash1[64];
    unsigned char	midstate[32];
    unsigned char	target[32];

    unsigned char	hash[32];
};

struct thr_info {
    int		id;
    pthread_t	pth;
    struct thread_q	*q;
};

extern char *bin2hex(const unsigned char *p, size_t len);
extern bool hex2bin(unsigned char *p, const char *hexstr, size_t len);
extern json_t *json_rpc_call(CURL *curl, const char *url, const char *userpass,
                             const char *rpc_req, bool, bool);

static bool submit_upstream_work(CURL *curl, const struct work *work)
{
    char *hexstr = NULL;
    json_t *val, *res;
    char s[345];
    bool rc = false;

    /* build hex string */
    hexstr = bin2hex(work->data, sizeof(work->data));
    if (unlikely(!hexstr)) {
        goto out;
    }

    /* build JSON-RPC request */
    sprintf(s,
            "{\"method\": \"getwork\", \"params\": [ \"%s\" ], \"id\":1}\r\n",
            hexstr);


    /* issue JSON-RPC request */
    val = json_rpc_call(curl, rpc_url, rpc_userpass, s, false, false);
    if (unlikely(!val)) {
        goto out;
    }

    res = json_object_get(val, "result");


    json_decref(val);

    rc = true;

    out:
    free(hexstr);
    return rc;
}
static const char *rpc_req =
        "{\"method\": \"getwork\", \"params\": [], \"id\":0}\r\n";

static bool jobj_binary(const json_t *obj, const char *key,
                        void *buf, size_t buflen)
{
    const char *hexstr;
    json_t *tmp;

    tmp = json_object_get(obj, key);
    if (unlikely(!tmp)) {
        return false;
    }
    hexstr = json_string_value(tmp);
    if (unlikely(!hexstr) || !hex2bin(buf, hexstr, buflen))) {
        return false;
    }
    return true;
}

static bool work_decode(const json_t *val, struct work *work)
{

    if (unlikely(!jobj_binary(val, "midstate", work->midstate, sizeof(work->midstate))) ||
            unlikely(!jobj_binary(val, "data", work->data, sizeof(work->data))) ||
            unlikely(!jobj_binary(val, "hash1", work->hash1, sizeof(work->hash1))) ||
            unlikely(!jobj_binary(val, "target", work->target, sizeof(work->target)))) {
        return false;
    }

    memset(work->hash, 0, sizeof(work->hash));

    return true;
}

class bitcoin_client final {
    static bool workio_get_work(work *wc, CURL *curl)
    {
        struct work *ret_work;
        int failures = 0;

        ret_work = new work;
        if (!ret_work)
            return false;

        /* obtain new work from bitcoin via JSON-RPC */
        while (!get_upstream_work(curl, ret_work)) {
            if (unlikely((10 >= 0) && (++failures > 10))) {
                free(ret_work);
                return false;
            }

            /* pause, then restart work-request loop */
            sleep(30);
        }

        return true;
    }

    static bool workio_submit_work(work	*wc, CURL *curl)
    {
        int failures = 0;

        /* submit solution to bitcoin via JSON-RPC */
        while (!submit_upstream_work(curl, wc)) {
            if (unlikely((10 >= 0) && (++failures > 10))) {
                return false;
            }

            sleep(30);
        }

        return true;
    }
private:
    static bool get_upstream_work(CURL *curl, struct work *work)
    {
        json_t *val;
        bool rc;

        val = json_rpc_call(curl, rpc_url, rpc_userpass, rpc_req,
                            true, false);
        if (!val)
            return false;

        rc = work_decode(json_object_get(val, "result"), work);

        json_decref(val);

        return rc;
    }
};

template<typename Data, typename WorkFunction, typename CheckFunction>
miner(Data, WorkFunction, CheckFunction)->miner<Data, WorkFunction, CheckFunction>;