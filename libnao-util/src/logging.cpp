/*  This file is part of libnao-util.

    libnao-util is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libnao-util is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libnao-util.  If not, see <https://www.gnu.org/licenses/>.   */
#include "logging.h"

#include "windows_min.h"
#include "strings.h"

#include <atomic>
#include <deque>
#include <mutex>

namespace {
    void print(const std::string& str) {
        OutputDebugStringW(strings::to_utf16(str).c_str());
    }

    class log_helper {
        // Contains all strings to be printed
        std::deque<std::string> _queue;
        // Guards the queue
        std::mutex _queue_mutex;

        // Notify print thread of updates
        std::mutex _condition_mutex;
        std::condition_variable _condition;

        std::atomic<bool> _stop { false };

        // Actual logging thread
        std::thread _log_thread { &log_helper::_log_func, this };

        public:
        ~log_helper() {
            _stop = true;
            {
                std::unique_lock lock { _condition_mutex };
                _condition.notify_all();
            }

            if (_log_thread.joinable()) {
                _log_thread.join();
            }
        }

        void push(const std::string& str) {
            std::unique_lock lock { _queue_mutex };
            _queue.push_back(str);
        }

        void notify() {
            std::unique_lock lock { _condition_mutex };
            _condition.notify_one();
        }

        private:
        void _log_func() {
            // Fallthrough to wait immediately
            bool is_empty = true;
            while (true) {
                // Print elements if there are any
                while (!is_empty) {
                    std::unique_lock lock { _queue_mutex };
                    print(_queue.front());
                    _queue.pop_front();

                    if (_stop) {
                        return;
                    }

                    is_empty = _queue.empty();
                }

                // Else wait for a notification
                std::unique_lock lock { _condition_mutex };
                _condition.wait(lock, [this, &is_empty] {
                    is_empty = _queue.empty();
                    return !is_empty || _stop;
                    });

                if (_stop) {
                    return;
                }
            }
        }
    };

    log_helper logger;
}

namespace logging {
    void cout(const std::string& str) {
        logger.push(str);
        logger.notify();
    }

}