// Copyright (C) 2021 All rights reserved.
// Email: oxox0@qq.com. Created in 202105

#ifndef THREAD_INTERRUPTIBLE_THREAD_H
#define THREAD_INTERRUPTIBLE_THREAD_H

#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <stdexcept>
#include <thread>

namespace thread {
void interruption_point();

namespace impl {

class interrupt_flag {
    std::atomic<bool> flag;
    std::condition_variable *thread_cond;
    std::condition_variable_any *thread_cond_any;
    std::mutex set_clear_mutex;
public:
    interrupt_flag(): flag(false), thread_cond(0), thread_cond_any(0) {}
    void set() {
        flag.store(true, std::memory_order_relaxed);
        std::lock_guard<std::mutex> lk(set_clear_mutex);
        if (thread_cond) {
            thread_cond->notify_all();
        } else if (thread_cond_any) {
            thread_cond_any->notify_all();
        }
    }

    bool is_set()const {
        return flag.load(std::memory_order_relaxed);
    }

    void set_condition_variable(std::condition_variable &cv) {
        std::lock_guard<std::mutex> lk(set_clear_mutex);
        thread_cond = &cv;
    }

    void clear_condition_variable() {
        std::lock_guard<std::mutex> lk(set_clear_mutex);
        thread_cond = 0;
    }

    template<typename Lockable>
    void wait(std::condition_variable_any &cv, Lockable &lk) {
        struct custom_lock {
            interrupt_flag *self_;
            Lockable &lk_;
            custom_lock(interrupt_flag *self,
                    std::condition_variable_any &cond,
                    Lockable &lk): self_(self), lk_(lk) {

                self_->set_clear_mutex.lock();
                self_->thread_cond_any = &cond;
            }

            void unlock() {
                lk_.unlock();
                self_->set_clear_mutex.unlock();
            }

            void lock() {
                std::lock(self_->set_clear_mutex, lk_);
            }

            ~custom_lock() {
                self_->thread_cond_any = 0;
                self_->set_clear_mutex.unlock();
            }
        };
        custom_lock cl(this, cv, lk);
        interruption_point();
        cv.wait(cl);
        interruption_point();
    }
};

thread_local interrupt_flag this_thread_interrupt_flag;

} // namespace impl

void interruption_point() {
    if (impl::this_thread_interrupt_flag.is_set()) {
        // throw thread_intterrupted();
        throw std::runtime_error("thread_intterrupted");
    }
}

template<typename Lockable>
void interruptible_wait(std::condition_variable_any& cv,
        Lockable& lk) {

    impl::this_thread_interrupt_flag.wait(cv,lk);
}

template<typename T>
void interruptible_wait(std::future<T> &uf) {
    while (!impl::this_thread_interrupt_flag.is_set()) {
        if (std::future_status::ready == 
            uf.wait_for(std::chrono::milliseconds(1))) {

            break;
        }
    }
    interruption_point();
}

class interruptible_thread {
    std::thread internal_thread;
    impl::interrupt_flag *flag;
public:
    template<typename FunctionType>
    explicit interruptible_thread(FunctionType f) {
        std::promise<impl::interrupt_flag *> p;
        internal_thread = std::thread([f, &p] {
                p.set_value(&impl::this_thread_interrupt_flag);
                try {
                    f();
                } catch (std::exception const &) {

                }
                });
        flag = p.get_future().get();
    }

    void join() { internal_thread.join(); }
    void detach() { internal_thread.detach(); }
    bool joinable() const { return internal_thread.joinable(); }
    void interrupt() {
        if (flag) {
            flag->set();
        }
    }

};


} // namespace thread

#endif // THREAD_INTERRUPTIBLE_THREAD_H

