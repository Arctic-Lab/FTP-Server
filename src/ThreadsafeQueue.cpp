#pragma once

#include "ThreadsafeQueue.h"
using namespace std;

void ThreadsafeQueue::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    while(!queue_.empty()) {
        queue_.pop();
    }
}

void ThreadsafeQueue::push(T item)
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(item);
    cond_.notify_one();     
}

bool ThreadsafeQueue::try_pop(T& item)
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

std::shared_ptr<T> ThreadsafeQueue::try_pop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if(queue_.empty()) {
        return std::make_shared<T>();
    } else {
        std::shared_ptr<T> ptr = std::make_shared<T>(queue_.front());
        queue_.pop();
        return ptr;
    }
}

void ThreadsafeQueue::wait_and_pop(T& item)
{
    std::unique_lock<std::mutex> lock<mutex_>;
    cond_.wait(lock, [] { return !queue.empty(); } );
    item = queue_.front();
    queue_.pop();
}

std::shared_ptr<T> ThreadsafeQueue::wait_and_pop()
{
    std::unique_lock<std::mutex> lock<mutex_>;
    cond_.wait(lock, [] { return !queue.empty(); } );
    std::shared_ptr<T> ptr = std::make_shared<T>(queue_.fornt());
    queue_.pop();
    return ptr;
}

bool ThreadsafeQueue::empty()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}
