// Copyright (C) 2021 All rights reserved.
// Email: oxox0@qq.com. Created in 202105

#ifndef THREAD_THREAD_POOL_H
#define THREAD_THREAD_POOL_H

#include <thread>

namespace thread {

class thread_pool {
    class function_wrapper {
        struct impl_base {
            virtual void call() = 0;
            virtual ~impl_base() {}
        };

        std::unique_ptr<impl_base> impl;
        template<typename F>
        struct impl_type: impl_base {
            F f_;
            impl_type(F &&f): f_(std::move(f)) {}
            void call() { f_(); }
        };

    public:
        template<typename F>
        function_wrapper(F &&f): impl(new impl_type<F>(std::move(f))) {}
        void operator()() { impl->call(); }
        function_wrapper() = default;
        function_wrapper(function_wrapper &&other): impl(std::move(other.impl)) {}
        function_wrapper &operator=(function_wrapper &&other)   {
            impl = std::move(other.impl);
            return *this;
        }

        function_wrapper(const function_wrapper &) = delete;
        function_wrapper(function_wrapper &) = delete;
        function_wrapper &operator=(const function_wrapper &) = delete;
    };

    std::atomic_bool done;
    lock_free_queue<function_wrapper> work_queue;
    typedef std::queue<function_wrapper> local_queue_type;
    static thread_local std::unique_ptr<local_queue_type> local_work_queue;
    std::vector<std::thread> threads;
    //join_threads joiner;

    void work_thread() {
        local_work_queue.reset(new local_queue_type);
        while (!done) {
            run_pending_task();
        }
    }

    void run_pending_task() {
        function_wrapper task;
        if (local_work_queue && !local_work_queue->empty()) {
            task = std::move(local_work_queue->front());
            local_work_queue->pop();
            task();
        } else if (task = work_queue.pop()) {
            task();
        } else {
            std::this_thread::yield();
        }
    }

public:
    thread_pool() {
        uint32_t const thread_count = std::thread::hardware_concurrency();
        try {
            for (uint32_t i = 0; i < thread_count; ++i) {
                threads.push_back(std::thread(&thrad_pool::worker_thread, this));
            }
        } catch (...) {
            done = true;
            throw;
        }
    }
    ~thread_pool() {
        done = true;
    }

    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type>
    submit(FunctionType f) {
        typedef typename std::result_of<FunctionType()>::type result_type;

        std::package_task<result_type()> task(std::move(f));
        std::future<result_type> res(task.get_future());
        if (local_work_queue) {
            local_work_queue->push(std::move(task));
        } else {
            work_queue.push(std::move(task));
        }
        return res;
    }
};

} // namespace thread

#endif // THREAD_THREAD_POOL_H

