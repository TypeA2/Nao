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

#include "nao/internal.h"

#include <utility>
#include <new>

namespace nao {
    struct default_init_first {};
    struct value_init_first {};
    template <typename Base, typename Value> requires std::is_empty_v<Base> && !std::is_final_v<Base>
        class compressed_pair final : Base {
            Value _val2;
            public:
            using base = Base;

            static constexpr bool is_nothrow_constructible = std::is_nothrow_constructible_v<Base> && std::is_nothrow_constructible_v<Value>;

            template <typename... Args>
            constexpr explicit compressed_pair(default_init_first, Args&&... args) noexcept(is_nothrow_constructible)
                : Base {}, _val2 { std::forward<Args>(args)... } { }

            template <typename A, typename... B>
            constexpr explicit compressed_pair(value_init_first, A&& a, B&&... b) noexcept(is_nothrow_constructible)
                : Base { std::forward<A>(a) }, _val2 { std::forward<B>(b)... } { }

            constexpr Base& first() noexcept { return *this; }
            constexpr const Base& first() const noexcept { return *this; }

            constexpr Value& second() noexcept { return _val2; }
            constexpr const Value& second() const noexcept { return _val2; }
        };

        template <typename T>
        struct naocore_delete {
            constexpr naocore_delete() noexcept = default;

            template <typename U>
            naocore_delete(const naocore_delete<U>& /*d*/) noexcept {}
            void operator()(T* ptr) const {
                ptr->~T();
                _nc_free_aligned(ptr);
            }
        };

        template <typename T>
        concept default_constructible_pointer = !std::is_pointer_v<T> && std::default_initializable<T>;

        template <typename T, typename Deleter = naocore_delete<T>> requires !std::is_array_v<T>
        class unique_ptr {
            public:
            using element_type = T;
            using deleter_type = Deleter;
            using pointer = element_type*;
            using const_pointer = element_type const*;

            using pair_type = compressed_pair<deleter_type, pointer>;

            private:
            pair_type _pair;

            public:

            ~unique_ptr() noexcept {
                if (_pair.second()) {
                    _pair.first()(_pair.second());
                }
            }
            template <default_constructible_pointer Deleter2 = deleter_type>
            constexpr unique_ptr() noexcept : _pair { default_init_first{} } {}

            template <default_constructible_pointer Deleter2 = deleter_type>
            constexpr unique_ptr(nullptr_t) noexcept : _pair { default_init_first{} } {}
            unique_ptr& operator=(nullptr_t) noexcept {
                reset();
                return *this;
            }

            template <default_constructible_pointer Deleter2 = deleter_type>
            explicit unique_ptr(pointer ptr) noexcept : _pair { default_init_first{}, ptr } {}

            unique_ptr(unique_ptr&& other) noexcept
                : _pair { value_init_first{}, std::forward<deleter_type>(other.get_deleter()), other.release() } {}

            unique_ptr& operator=(unique_ptr&& other) noexcept {
                if (this != std::addressof(other)) {
                    reset(other.release());
                    _pair.first() = std::forward<deleter_type>(other._pair.first());
                }

                return *this;
            }

            pointer operator->() const noexcept {
                return _pair.second();
            }

            operator bool() const noexcept {
                return _pair.second();
            }

            std::add_lvalue_reference_t<T> operator*() const {
                return *_pair.second();
            }

            pointer release() noexcept {
                return std::exchange(_pair.second(), pointer {});
            }

            void reset(pointer ptr = pointer {}) noexcept {
                pointer old = std::exchange(_pair.second(), ptr);
                if (old) {
                    _pair.first()(old);
                }
            }

            unique_ptr(const unique_ptr&) = delete;
            unique_ptr& operator=(const unique_ptr&) = delete;

            deleter_type& get_deleter() noexcept { return _pair.first(); }
            const deleter_type& get_deleter() const noexcept { return _pair.first(); }

            pointer get() const noexcept { return _pair.second(); }

            
        };

        // Assures the element is created on the libnao-core heap
        template <typename T, typename... Args>
        auto make_unique(Args&&... args) {
            return unique_ptr<T>(new(_nc_alloc_aligned<T>(1)) T(std::forward<Args>(args)...));
        }
}

#define NAO_EXPORT_UNIQUE_PTR(spec, type) \
    template class spec nao::unique_ptr<type>::pair_type; \
    template class spec nao::unique_ptr<type>

#define NAO_DEFINE_PTR(spec, type, name) \
    class type; \
    NAO_EXPORT_UNIQUE_PTR(spec, type); \
    ::nao::unique_ptr<type> name;
