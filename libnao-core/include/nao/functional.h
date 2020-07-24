/*  This file is part of libnao-core.

    libnao-core is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libnao-core is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libnao-core.  If not, see <https://www.gnu.org/licenses/>.   */
#pragma once

#include "nao/memory.h"

#include <functional>

namespace nao {
    std::function<int(char)> x;

    template <typename T>
    class callable_base;

    template <typename R, typename... Args>
    class callable_base<R(Args...)> {
        public:
        virtual R operator()(Args... args) const = 0;
        virtual ~callable_base() {}
    };

    template <typename Obj, typename R, typename... Args>
    class ptm_callable final : callable_base<R(Args...)> {
        Obj* _this;
        R(Obj::* _member)(Args...);

        public:
        ptm_callable(Obj* ptr, R(Obj::* member)(Args...)) : _this { ptr }, _member { member } { }
        R operator()(Args&&... args) {
            return (_this->*(_member))(std::forward<Args>(args)...);
        }
    };

    template <typename T>
    class function;

    template <typename R, typename... Args>
    class function<R(Args...)> {
        template class NAOCORE_API unique_ptr<callable_base<R(Args...)>>::pair_type;
        template class NAOCORE_API nao::unique_ptr<callable_base<R(Args...)>>;

        unique_ptr<callable_base<R(Args...)>> _ptr = nullptr;

        public:
        template <typename Obj>
        function(R(Obj::* member)(Args...), Obj* obj)
            : _ptr { make_unique<ptm_callable<Obj, R, Args...>>(member, obj) } {  }
    };
}