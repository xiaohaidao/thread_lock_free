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

    class work_stealing_queue {
        typedef function_wrapper data_type;
        std::deque<data_type> the_queue;
        mutable std::mutex the_mutex;
    public:
        work_stealing_queue() {}
        work_stealing_queue(const work_stealing_queue& other) = delete;
        work_stealing_queue& operator=(
                const work_stealing_queue& other) = delete;

        void push(data_type data) {
            std::lock_guard<std::mutex> lock(the_mutex);
            the_queue.push_front(std::move(data));
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(the_mutex);
            return the_queue.empty();
        }

        bool try_pop(data_type& res) {
            std::lock_guard<std::mutex> lock(the_mutex);
            if (the_queue.empty()) {
                return false;
            }
            res = std::move(the_queue.front());
            the_queue.pop_front();
            return true;
        }
        bool try_steal(data_type& res) {
            std::lock_guard<std::mutex> lock(the_mutex);
            if (the_queue.empty()) {
                return false;
            }
            res = std::move(the_queue.back());
            the_queue.pop_back();
            return true;
        }
    };

    typedef function_wrapper task_type;

    std::atomic_bool done;
    lock_free_queue<task_type> pool_work_queue;
    std::vector<std::unique_ptr<work_stealing_queue> > queues;
    std::vector<std::thread> threads;
    //join_threads joiner;

    static thread_local work_stealing_queue *local_work_queue;
    static thread_local uint32_t my_index;

    void work_thread(uint32_t index) {
        my_index = index;
        local_work_queue = queues[my_index].get();
        while (!done) {
            run_pending_task();
        }
    }

    bool pop_task_from_local_queue(task_type &task) {
        return local_work_queue && local_work_queue->try_pop(task);
    }
    bool pop_task_from_pool_queue(task_type &task) {
        auto t = pool_work_queue.pop();
        if (t) {
            task = *t;
        }
        return t;
    }
    bool pop_task_from_other_thread_queue(task_type &task) {
        size_t queues_size = queues.size();
        for (size_t i = 0; i < queues_size; ++i) {
            size_t index = (my_index + i + 1) % queues_size;
            if (queues[index]->try_streal(task)) {
                return true;
            }
        }
        return false;
    }

    void run_pending_task() {
        task_type task;
        if (pop_task_from_local_queue(task) ||
                pop_task_from_pool_queue(task) ||
                pop_task_from_other_thread_queue(task)) {

            task();
        } else {
            std::this_thread::yield();
        }
    }

public:
    thread_pool(): done(false) {
        uint32_t const thread_count = std::thread::hardware_concurrency();
        try {
            for (uint32_t i = 0; i < thread_count; ++i) {
                queues.push_bask(std::unique_ptr<work_stealing_queue>(
                            new work_stealing_queue));

                threads.push_back(std::thread(&thrad_pool::work_thread,
                            this, i));
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
            pool_work_queue.push(std::move(task));
        }
        return res;
    }
};

} // namespace thread

#endif // THREAD_THREAD_POOL_H

