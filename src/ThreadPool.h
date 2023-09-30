/*************************************************************
 * ThreadPool based on C++17
 **************************************************************/

#pragma once
#include <iostream>
#include <atomic>
#include <memory>
#include <functional>
#include <future>
#include "./ThreadsafeQueue.h"
using namespace std;

class ThreadPool {
public:
    ThreadPool(int n) : m_threads(n), m_is_shut_down{ false } {
        for (auto& t : m_threads)
            t = thread{ [this]() {worker();} };
    }

    ~ThreadPool() {
        auto f = submit([]() {});
        f.get();
        m_is_shut_down = true;
        for (auto& t : m_threads) {
            if (t.joinable()) t.join();
        }
    }
    
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
    

    template <typename F, typename... Args>
    auto submit(F&& f, Args &&...args) -> std::future<decltype(f(args...))>;

    void worker()
    {
        while (!is_shut_down()) {
            function<void()> func;

            bool flag = m_que.wait_for_and_pop(func, 1000ms);

            if (flag) {
                func();
            }
        }
    }

    bool is_shut_down() const {
        return m_is_shut_down;
    }

    void shut_down() {
        m_is_shut_down = true;
    }

private:
    atomic<bool> m_is_shut_down;
    ThreadsafeQueue<std::function<void()>> m_que;
    vector<std::thread> m_threads;
};

/* should check return value with valid function */
template <typename F, typename... Args>
auto ThreadPool::submit(F&& f, Args &&...args) -> std::future<decltype(f(args...))> 
{
    function<decltype(f(args...))()> func = [&f, args...]() {return f(args...); };
    auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
    
    std::function<void()> warpper_func = [task_ptr]() {
        (*task_ptr)();
    };
    
    /* already shutdown, return future of empty */
    if(is_shut_down()) 
        return std::future<decltype(f(args...))>();

    m_que.push(warpper_func);
    return task_ptr->get_future();
}