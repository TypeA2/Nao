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

    // Push a function with arguments, return the packaged task
    template <typename Func, typename... Args>
    [[maybe_unused]] auto push(Func&& f, Args&&... args) {

        auto packed =
            std::make_shared<std::packaged_task<decltype(f(args...))()>>(
                std::bind(std::forward<Func>(f), std::forward<Args>(args)...));

        {
            std::unique_lock lock(_m_mutex);
            _m_queue.push(new std::function<void()>([packed] { (*packed)(); }));
        }

        std::unique_lock lock(_m_mutex);
        _m_condition.notify_one();

        return packed;
    }

    // Push a member function with an object pointer, automatically bind
    template <typename Ret, typename T, typename... Args>
    [[maybe_unused]] auto push(Ret(T::* mem)(Args...), T* obj, Args&&... args) {
        return push(std::bind(mem, obj, std::forward<Args>(args)...));
    }

    // Or a reference as object to bind to
    template <typename Ret, typename T, typename... Args>
    [[maybe_unused]] auto push(Ret(T::* mem)(Args...), T& obj, Args&&... args) {
        return push(std::bind(mem, &obj, std::forward<Args>(args)...));
    }

    private:
    std::vector<std::unique_ptr<std::thread>> _m_threads;

    std::queue<std::function<void()>*> _m_queue;
    std::mutex _m_queue_mutex;
    std::mutex _m_mutex;
    std::condition_variable _m_condition;
    std::atomic<bool> _m_stop;
};

