#include "thread_pool.h"

size_t thread_pool::pool_size() {
    return std::thread::hardware_concurrency();
}

thread_pool::thread_pool() : thread_pool(pool_size()) {

}

thread_pool::~thread_pool() {
    _m_stop = true;

    {
        std::unique_lock lock(_m_mutex);
        _m_condition.notify_all();
    }

    for (const auto& thread : _m_threads) {
        if (thread->joinable()) {
            thread->join();
        }
    }

    while (!_m_queue.empty()) {
        delete _m_queue.front();
        _m_queue.pop();
    }
}

thread_pool::thread_pool(size_t n_threads) : thread_pool(n_threads, {}, {}) {
    
}


thread_pool::thread_pool(size_t n_threads, const std::function<void()>& before,
    const std::function<void()>& after)
    : _m_threads(n_threads)
    , _m_stop(false)
    , _m_before(before), _m_after(after) {

    auto loop_fun = [this] {
        if (_m_before) {
            _m_before();
        }

        bool is_empty;

        {
            std::unique_lock lock(_m_queue_mutex);
            is_empty = _m_queue.empty();
        }

        while (true) {
            while (!is_empty) {
                std::unique_ptr<std::function<void()>> func;

                {
                    std::unique_lock lock(_m_queue_mutex);
                    func.reset(_m_queue.front());
                    _m_queue.pop();
                }

                (*func)();

                // Stop if a kill is requested
                if (_m_stop) {
                    if (_m_after) {
                        _m_after();
                    }
                    return;
                }

                std::unique_lock lock(_m_queue_mutex);
                is_empty = _m_queue.empty();
            }

            // Queue is empty, wait using condition_variable
            std::unique_lock lock(_m_mutex);
            _m_condition.wait(lock, [this, &is_empty]() -> bool {
                is_empty = _m_queue.empty();
                return !is_empty || _m_stop;
            });

            if (_m_stop) {
                if (_m_after) {
                    _m_after();
                }
                return;
            }
        }
    };

    for (size_t i = 0; i < n_threads; ++i) {
        _m_threads[i].reset(new std::thread(loop_fun));
    }
}

size_t thread_pool::queue_size() const {
    std::unique_lock lock(_m_queue_mutex);

    return _m_queue.size();
}

