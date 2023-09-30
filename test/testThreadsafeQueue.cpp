#include <iostream>
#include <thread>
#include <atomic>
#include <assert.h>
#include <functional>
#include "../src/ThreadsafeQueue.h"
using namespace std;

class TestThreadsafeQueue
{
public:
    TestThreadsafeQueue(int n, int m, int times): m_product(n), m_comsum(m), m_max_times(times), m_arr(std::vector<std::atomic<int>>(n)) {}
    ~TestThreadsafeQueue() {}

public:
    void test()
    {
        thread* pArr[m_product];
        for(int i = 0; i < m_product; i++)
            pArr[i] = new thread([this,i](){ product(i); });

        thread* pBrr[m_comsum];
        for(int i = 0; i < m_comsum; i++)
            pBrr[i] = new thread([this](){ comsume(); });

        for(int i = 0; i < m_product; i++)
            pArr[i]->join();
        
        for(int i = 0; i < m_comsum; i++)
        {
            pBrr[i]->join();
        }

        for(int i = 0; i < m_product; i++)
        {
            assert(m_arr[i] == m_max_times);
        }
    }

private:
    void product(int x)
    {
        for(int i = 0; i < m_max_times; ++i)
        {
            m_que.push(x);
        }
    }

    void comsume()
    {
        while(m_run.load() < m_product * m_max_times)
        {
            int x = -1;
            bool f = m_que.wait_for_and_pop(x, 1000ms);
            if(!f) continue;
            m_arr[x].fetch_add(1);
            m_run.fetch_add(1);
        }
    }

private:
    int m_product;
    int m_comsum;
    int m_max_times;
    std::vector<std::atomic<int>> m_arr;
    ThreadsafeQueue<int> m_que;
    std::atomic<int> m_run{0};
};

class RunTimeTest
{
public:
    static void getRunTime(string unit_name, std::function<void()> f)
    {
        auto start = chrono::steady_clock::now();

        f();

        auto end = chrono::steady_clock::now();
    
        cout << unit_name
            << ": Elapsed time in milliseconds: "
            << chrono::duration_cast<chrono::milliseconds>(end - start - m_offset).count()
            << " ms" << endl;
    }
private:
    // queue has 1000ms time out
    static constexpr const chrono::milliseconds m_offset = 1000ms;
};

int main()
{
    RunTimeTest::getRunTime("UNIT1", [](){ TestThreadsafeQueue{3, 3, 100000}.test(); });
    RunTimeTest::getRunTime("UNIT2", [](){ TestThreadsafeQueue{10, 10, 100000}.test(); });
    RunTimeTest::getRunTime("UNIT3", [](){ TestThreadsafeQueue{10, 10, 1000000}.test(); });
    RunTimeTest::getRunTime("UNIT4", [](){ TestThreadsafeQueue{10, 5, 1000000}.test(); });
    RunTimeTest::getRunTime("UNIT5", [](){ TestThreadsafeQueue{10, 1, 1000000}.test(); });
}