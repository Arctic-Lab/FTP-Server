/*
 * Thread-safe queue based on std::queue, std::mutex, std::condition_variable
 */
#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

template<class T> class ThreadsafeQueue
{
public:
    ThreadsafeQueue() {};
    ThreadsafeQueue(const ThreadsafeQueue& ) = delete;
    ThreadsafeQueue& operator=(const ThreadsafeQueue& ) = delete;

    /* clear all items in queue */
    void clear();

    /* push an item append to queue */
    void push(T item);

    /* try to pop an item from queue */
    bool try_pop(T& item);

    /* try to pop an item from queue, but shared_ptr style */
    std::shared_ptr<T> try_pop();

    /* try to pop an item from queue, and will wait until it can */
    void wait_and_pop(T& item);

    /* try to pop an item from queue, and will wait until it can, but shared_ptr style */
    std::shared_ptr<T> wait_and_pop();

    /* return if queue is empty */
    bool empty();

private:
    std::mutex mutex_;
    std::queue<T> queue_;
    std::condition_variable cond_;
};
