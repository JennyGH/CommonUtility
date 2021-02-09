#include "thread_local_storage.hpp"
#include "condition_variable.h"
#include "event.h"
#include <queue>
#include <ctime>
#include <string>
#include <iostream>

// ========== NEED C++11 ==========
#include <thread>
// ================================

typedef std::queue<time_t>     queue_t;
static std::condition_variable g_cond;
static std::mutex              g_mutex;
static queue_t                 g_queue;
static event                   g_event;
static __thread_local(int) g_tls;

static void _PrintQueue(queue_t& queue)
{
    while (!queue.empty())
    {
        std::cout << queue.front() << std::endl;
        queue.pop();
    }
}

static bool _ConditionCallback()
{
    return !g_queue.empty();
}

static void _TestMutex()
{
    std::scoped_lock<std::mutex> lock(g_mutex);
    _TestMutex();
}

int main(int argc, char* argv[])
{
    //_TestMutex();
    std::thread([]() {
        g_tls = 1;
        while (true)
        {
            queue_t tmpQueue;
            {
                std::scoped_lock<std::mutex> lock(g_mutex);
                g_cond.wait(lock, _ConditionCallback);
                // g_cond.wait_for(lock, std::chrono::seconds(1));
                // xtime xt;
                // xt.sec  = 1;
                // xt.nsec = 0;
                // g_cond.wait_until(lock, &xt);
                tmpQueue.swap(g_queue);
            }
            _PrintQueue(tmpQueue);
            std::cout << "g_tls: " << g_tls << std::endl;
        }
    }).detach();

    std::thread([]() {
        g_tls = 2;
        while (true)
        {
            int errcode = g_event.wait(3 * 1000);
            std::cout << "g_tls: " << g_tls << ", errcode: " << errcode << std::endl;
        }
    }).detach();

    g_tls = 0;
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        {
            std::scoped_lock<std::mutex> lock(g_mutex);
            g_queue.push(::time(NULL));
        }
        std::cout << "g_tls: " << g_tls << std::endl;
        g_cond.notify_one();
        g_event.notify();
    }

    return 0;
}