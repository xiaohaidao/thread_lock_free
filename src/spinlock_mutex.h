// Copyright (C) 2021 All rights reserved.
// Email: oxox0@qq.com. Created in 202104

#ifndef THREAD_SPINLOCK_MUTEX_H
#define THREAD_SPINLOCK_MUTEX_H

#include <thread>

namespace thread {

class spinlock_mutex {
    std::atomic_flag flag;

public:
    spinlock_mutex() : flag(ATOMIC_FLAG_INIT) {}

    void lock() {
        while(flag.test_and_set(std::memory_order_acquire));
    }
    void unlock() {
        flag.clear(std::memory_order_release);
    }
};

} // namespace thread

#endif // THREAD_SPINLOCK_MUTEX_H

