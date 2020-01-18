#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <atomic>
#include <future>

class thread_pool {
    public:
    static size_t pool_size();

    thread_pool();
    explicit thread_pool(size_t n_threads);
    ~thread_pool();

    template <typename Func, typename... Args>
    auto push(Func&& f, Args&&... args)
        -> std::future<decltype(f(args...))> {

        auto packed =
            std::make_shared<std::packaged_task<decltype(f(args...))()>>(
                std::bind(std::forward<Func>(f), std::forward<Args>(args)...));

        {
            std::unique_lock lock(_m_mutex);
            _m_queue.push(new std::function<void()>([packed] { (*packed)(); }));
        }

        std::unique_lock lock(_m_mutex);
        _m_condition.notify_one();

        return packed->get_future();
    }

    template <typename Func, typename... Args>
    auto push_detached(Func&& f, Args&&... args) {
        auto packed =
            std::make_shared<std::packaged_task<decltype(f(args...))()>>(
                std::bind(std::forward<Func>(f), std::forward<Args>(args)...));

        {
            std::unique_lock lock(_m_queue_mutex);
            _m_queue.push(new std::function<void()>([packed] { (*packed)(); }));
        }

        std::unique_lock lock(_m_mutex);
        _m_condition.notify_one();
    }

    private:
    std::vector<std::unique_ptr<std::thread>> _m_threads;

    std::queue<std::function<void()>*> _m_queue;
    std::mutex _m_queue_mutex;
    std::mutex _m_mutex;
    std::condition_variable _m_condition;
    std::atomic<bool> _m_stop;
};

