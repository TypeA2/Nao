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
#pragma once

#include <thread>
#include <vector>
#include <functional>
#include <queue>
#include <mutex>

namespace nao {
    class thread_pool {
        std::vector<std::thread> _threads;

        /* Whether all threads should stop */
        std::atomic<bool> _cancelled = false;

        std::queue<std::function<void()>> _queue;
        std::mutex _queue_mutex;

        std::mutex _condition_mutex;
        std::condition_variable _condition;

        public:
        thread_pool();
        explicit thread_pool(unsigned n_threads);

        ~thread_pool();

        thread_pool(const thread_pool&) = delete;
        thread_pool& operator=(const thread_pool&) = delete;

        thread_pool(thread_pool&&) noexcept = delete;
        thread_pool& operator=(thread_pool&&) noexcept = delete;

        template <typename Func, typename... Args>
        void push(Func f, Args&&... args) requires std::invocable<Func, Args...>  {
            {
                std::unique_lock lock{ _queue_mutex };
                _queue.push(std::bind(f, std::forward<Args>(args)...));
            }

            std::unique_lock lock{ _condition_mutex };
            _condition.notify_one();
        }
    };
}