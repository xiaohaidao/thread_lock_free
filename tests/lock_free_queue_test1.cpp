#include <set>
#include <thread>

#include "catch.hpp"
#include "lock_free_queue.h"

void thread_func(lock_free_queue<int> &queue, size_t id) {
    static const int LOOP_SIZE = 1000;
    for (size_t i = 0; i < LOOP_SIZE; ++i) {
        int v = (int)((id + 1) * LOOP_SIZE + i);
        queue.push(v);
    }
}

TEST_CASE("Lock Free Queue Function", "[LockFreeQueueFunction]") {
    lock_free_queue<int> queue;

    size_t thread_size = 10;
    std::vector<std::thread> threads;
    for (size_t i = 0; i < thread_size; ++i) {
        threads.push_back(std::thread(thread_func, std::ref(queue), i));
    }

    for (size_t i = 0; i < thread_size; ++i) {
        threads[i].join();
    }

    std::set<int> result;
    while (auto v = queue.pop()) {
      REQUIRE(result.find(*v) == result.end());
      result.insert(*v);
    }
}
