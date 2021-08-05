/**
 *  This file is part of libnao-util.
 *
 *  libnao-util is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libnao-util is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libnao-util.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "thread_pool.h"

nao::thread_pool::thread_pool() : thread_pool{ std::thread::hardware_concurrency() } {
    
}

nao::thread_pool::thread_pool(unsigned n_threads) {
    _threads.reserve(n_threads);

    auto thread_fun = [this] {
        bool is_empty;

        {
            std::unique_lock lock{ _queue_mutex };
            is_empty = _queue.empty();
        }

        while (!_cancelled) {
            while (!is_empty) {
                std::function<void()> func;

                {
                    std::unique_lock lock{ _queue_mutex };
                    func = std::move(_queue.front());
                    _queue.pop();
                }

                func();

                if (_cancelled) {
                    return;
                }

                std::unique_lock lock{ _queue_mutex };
                is_empty = _queue.empty();
            }

            // Empty queue, wait for condition
            std::unique_lock lock{ _condition_mutex };

            // Wait until the queue is not empty or everything is cancelled
            _condition.wait(lock, [this, &is_empty] {
                is_empty = _queue.empty();
                return !is_empty ||_cancelled;
                });
        }
    };

    for (unsigned i = 0; i < n_threads; ++i) {
        _threads.emplace_back(thread_fun);
    }
}

nao::thread_pool::~thread_pool() {
    _cancelled = true;
    {
        std::unique_lock lock{ _condition_mutex };
        _condition.notify_all();
    }

    for (auto& thread : _threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
