/*
 * Thread-safe queue based on std::queue, std::mutex, std::condition_variable
 */
#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <chrono>
using namespace std::chrono_literals;

template<class T> class ThreadsafeQueue
{
public:
    ThreadsafeQueue() {};
    ThreadsafeQueue(const ThreadsafeQueue& ) = delete;
    ThreadsafeQueue& operator=(const ThreadsafeQueue& ) = delete;

    /* clear all items in queue */
    void clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        while(!queue_.empty()) {
            queue_.pop();
        }
    }

    /* push an item append to queue */
    void push(T item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
        cond_.notify_one();     
    }

    /* try to pop an item from queue */
    bool try_pop(T& item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(queue_.empty()) {
            return false;
        } else {
            item = queue_.front();
            queue_.pop();
            return true;
        }
    }

    /* try to pop an item from queue, and will wait until it can */
    bool wait_and_pop(T& item)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return !queue_.empty(); } );
        if(queue_.empty()) return false;
        item = queue_.front();
        queue_.pop();
        return true;
    }

    /* try to pop an item from queue, and will wait until it can or time out */
    bool wait_for_and_pop(T& item, const std::chrono::milliseconds& wait_time)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if ( !cond_.wait_for(lock, wait_time, [this] { return !queue_.empty(); }) )
            return false;
        item = queue_.front();
        queue_.pop();
        return true;
    }

    /* return if queue is empty */
    bool empty()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    std::mutex mutex_;
    std::queue<T> queue_;
    std::condition_variable cond_;
};
