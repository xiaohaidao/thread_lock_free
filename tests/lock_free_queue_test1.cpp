#include <set>
#include <thread>

#include "gtest/gtest.h"
#include "lock_free_queue.h"
#include "interruptible_thread.h"

void thread_func(thread::lock_free_queue<int> &queue, size_t id) {
    static const int LOOP_SIZE = 1000;
    for (size_t i = 0; i < LOOP_SIZE; ++i) {
        int v = (int)((id + 1) * LOOP_SIZE + i);
        queue.push(v);
    }
}

TEST(LockFreeQueue, LockFreeQueueFunction1) {
    thread::lock_free_queue<int> queue;

    size_t thread_size = std::thread::hardware_concurrency();
    std::vector<thread::interruptible_thread> threads;
    for (size_t i = 0; i < thread_size; ++i) {
        threads.push_back(thread::interruptible_thread(std::bind(thread_func, std::ref(queue), i)));
    }

    for (size_t i = 0; i < thread_size; ++i) {
        threads[i].join();
    }

    std::set<int> result;
    while (auto v = queue.pop()) {
      ASSERT_TRUE(result.find(*v) == result.end());
      result.insert(*v);
    }
}
