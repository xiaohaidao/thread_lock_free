#include <set>
#include <thread>

#include "gtest/gtest.h"
#include "lock_free_stack.h"

void thread_func(thread::lock_free_stack<int> &stack, size_t id) {
    static const int LOOP_SIZE = 1000;
    for (size_t i = 0; i < LOOP_SIZE; ++i) {
        int v = (int)((id + 1) * LOOP_SIZE + i);
        stack.push(v);
    }
}

TEST(LockFreeStack, LockFreeStackFunction1) {

    thread::lock_free_stack<int> stack;

    size_t thread_size = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    for (size_t i = 0; i < thread_size; ++i) {
        threads.push_back(std::thread(thread_func, std::ref(stack), i));
    }

    for (size_t i = 0; i < thread_size; ++i) {
        threads[i].join();
    }

    std::set<int> result;
    while (auto v = stack.pop()) {
      ASSERT_TRUE(result.find(*v) == result.end());
      result.insert(*v);
    }
}
