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

#include "nao/string.h"
#include "nao/concepts.h"

#include <stdexcept>

//
// Expected<T> for error handling
// Throws an exception when retrieving when in error state,
// inline definition as to not cross DLL boundaries
//

namespace nao {
    enum class exception_type {
        logic_error,
        invalid_argument,
        domain_error,
        length_error,
        out_of_range,
        runtime_error,
        range_error,
        overflow_error,
        underflow_error
    };

    using except = exception_type;

    [[noreturn]] inline void _nc_throw(exception_type type, const string& msg) {
        using enum exception_type;
        switch (type) {
            case logic_error:      throw std::logic_error(msg.c_str());
            case invalid_argument: throw std::invalid_argument(msg.c_str());
            case domain_error:     throw std::domain_error(msg.c_str());
            case length_error:     throw std::length_error(msg.c_str());
            case out_of_range:     throw std::out_of_range(msg.c_str());
            case runtime_error:    throw std::runtime_error(msg.c_str());
            case range_error:      throw std::range_error(msg.c_str());
            case overflow_error:   throw std::overflow_error(msg.c_str());
            case underflow_error:  throw std::underflow_error(msg.c_str());
            default: std::terminate();
        }
    }

    template <typename T>
    class expected {
        struct error_wrapper {
            string msg;
            exception_type type;
        };

        union {
            error_wrapper _error;
            T _result;
        };
        bool _good = false;

        public:
        expected() = delete;

        template <std::constructible_from<T> Arg>
        expected(const Arg& v) : _result { v }, _good { true } { }

        template <safe_forwarding_reference<expected<T>> A>
        expected(A&& v) : _result { std::forward<A>(v) }, _good { true } { }

        expected(exception_type type, string msg) : _error { std::move(msg), type } { }

        expected(const expected& other) : _good { other._good } {
            if (_good) {
                _result = other._result;
            } else {
                _error = other._error;
            }
        }

        expected(expected&& other) noexcept : _good { other._good } {
            if (_good) {
                _result = std::move(other._result);
            } else {
                _error = std::move(other._error);
            }
        }

        ~expected() {
            if (_good) {
                _result.~T();
            } else {
                _error.msg.~string();
            }
        }

        bool good() const { return _good; }
        T& get() {
            _throw();

            return _result;
        }

        const T& get() const {
            _throw();
            return _result;
        }

        operator const T& () const { _throw(); return _result; }

        private:
        void _throw() const {
            if (!_good) {
                _nc_throw(_error.type, _error.msg);
            }
        }
    };

    template <>
    class expected<void> {
        exception_type _type = exception_type::runtime_error;
        string _msg;
        bool _good = true;

        public:
        expected() = default;
        expected(const expected& other) : _good { other._good } {
            if (!_good) {
                _type = other._type;
                _msg = other._msg;
            }
        }

        expected(expected&& other) noexcept : _good { other._good } {
            if (!_good) {
                _type = other._type;
                _msg = std::move(other._msg);
            }
        }

        ~expected() = default;

        bool good() const { return _good; }
        void get() const { if (!_good) { _nc_throw(_type, _msg); } }
    };

    template <typename Left, addable<Left> Right>
    auto operator+(const expected<Left>& lhs, const Right& rhs) {
        return lhs.get() + rhs;
    }
}
