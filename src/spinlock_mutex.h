#ifndef THREAD_NO_BLOCK_SPINLOCK_MUTEX_H
#define THREAD_NO_BLOCK_SPINLOCK_MUTEX_H

#include <thread>

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

#endif // THREAD_NO_BLOCK_SPINLOCK_MUTEX_H

