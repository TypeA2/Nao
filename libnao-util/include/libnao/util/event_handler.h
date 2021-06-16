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

#include <functional>
#include <vector>

namespace nao {
    template <typename... Args>
    class event_handler {
        public:
        using FuncType = std::function<void(Args...)>;

        private:
        std::vector<FuncType> _callbacks{};

        public:
        event_handler() = default;
        ~event_handler() = default;

        // Don't touch my class
        event_handler(const event_handler&) = delete;
        event_handler& operator=(const event_handler&) = delete;

        event_handler(event_handler&&) noexcept = delete;
        event_handler& operator=(event_handler&&) noexcept = delete;

        template <std::invocable<Args...> Func>
        void add(Func fptr) {
            _callbacks.emplace_back([fptr](Args&&... args) {
                fptr(std::forward<Args>(args)...);
            });
        }

        template <typename Func, typename Inst>
        void add(Func fptr, Inst& instance)
                requires std::invocable<Func, Inst&, Args...> {

            _callbacks.emplace_back([fptr, &instance](Args&&... args) {
                std::invoke(fptr, instance, std::forward<Args>(args)...);
            });
        }

        void call(Args&&... args) {
            for (auto& cb : _callbacks) {
                cb(std::forward<Args>(args)...);
            }
        }
    };
}    
