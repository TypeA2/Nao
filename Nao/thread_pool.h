#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <atomic>
#include <future>

#include "utils.h"

class thread_pool {
    public:
    static size_t pool_size();

    explicit thread_pool();
    explicit thread_pool(size_t n_threads);
    explicit thread_pool(size_t n_threads, const std::function<void()>& before, const std::function<void()>& after = {});
    ~thread_pool();

    // Push a function with arguments, return the packaged task
    template <typename Func, typename... Args>
    [[maybe_unused]] auto push(Func&& f, Args&&... args) {

        auto packed =
            std::make_shared<std::packaged_task<
                decltype(std::invoke(std::forward<Func>(f), std::forward<Args>(args)...))()>>(
                    std::bind(std::forward<Func>(f), std::forward<Args>(args)...));

        {
            std::unique_lock lock(_m_mutex);
            
            _m_queue.push(new std::function<void()>([packed] {
                (*packed)();

                auto future = packed->get_future();

                try {
                    future.get();
                } catch (const std::exception& e) {
                    utils::coutln("exception in thread", std::this_thread::get_id(), ":", e.what());
                    throw;
                }
            }));
        }

        std::unique_lock lock(_m_mutex);
        _m_condition.notify_one();

        return packed;
    }

    private:
    std::vector<std::unique_ptr<std::thread>> _m_threads;

    std::queue<std::function<void()>*> _m_queue;
    std::mutex _m_queue_mutex;
    std::mutex _m_mutex;
    std::condition_variable _m_condition;
    std::atomic<bool> _m_stop;

    std::function<void()> _m_before;
    std::function<void()> _m_after;
};

