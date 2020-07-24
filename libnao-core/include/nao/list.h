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

#include "internal.h"

#include <cstddef>
#include <list>

//
// STL-like linked list
//

namespace nao {
    template <typename T>
    struct list_node {
        list_node* next = nullptr;
        list_node* prev = nullptr;
        T value;
    };
    template <typename T>
    class list {
        public:
        template <typename T>
        class list_iterator {
            list_node<T>* _ptr = nullptr;
            list* _owner = nullptr;

            public:
            list_iterator() noexcept = delete;
            list_iterator(list_node<T>* ptr, list* owner) noexcept
                : _ptr { ptr }, _owner { owner } { }

            T& operator*() { return _ptr->value; }
            const T& operator*() const { return _ptr->value; }

            T* operator->() const { return &_ptr->value; }

            list_iterator& operator++() {
                _ptr = _ptr->next;
                return *this;
            }

            list_iterator& operator--() {
                if (_ptr) {
                    _ptr = _ptr->prev;
                } else {
                    // This was past-the-end
                    _ptr = _owner->_last;
                }

                return *this;
            }

            list_iterator operator++(int) {
                _ptr = _ptr->next;
                return *this;
            }

            list_iterator operator--(int) {
                if (_ptr) {
                    _ptr = _ptr->prev;
                } else {
                    // This was past-the-end
                    _ptr = _owner->_last;
                }

                return *this;
            }

            bool operator==(const list_iterator& other) const {
                return _ptr == other._ptr;
            }

            bool operator!=(const list_iterator& other) const {
                return _ptr != other._ptr;
            }
        };

        using value_type = T;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = list_iterator<T>;
        using const_iterator = const list_iterator<T>;

        private:
        friend class list_iterator<T>;
        list_node<T>* _first = nullptr;
        list_node<T>* _last = nullptr;
        size_type _size = 0;

        public:
        ~list() { clear(); }

        list() noexcept = default;

        list(const list& other) {
            for (const auto& v : other) {
                push_back(v);
            }
        }

        list& operator=(const list& other) = delete;

        list(list&& other) noexcept 
            : _first { std::exchange(other._first, nullptr) }
            , _last { std::exchange(other._last, nullptr) }
            , _size { std::exchange(other._size, 0) } { }

        list& operator=(list&& other) noexcept {
            clear();
            _first = std::exchange(other._first, nullptr);
            _last = std::exchange(other._last, nullptr);
            _size = std::exchange(other._size, nullptr);
            return *this;
        }

        void clear() noexcept {
            auto node = _first;
            while (node) {
                node->value->~T();
                auto tmp = node;
                node = node->next;
                _nc_free_aligned(tmp);
            }

            _first = nullptr;
            _last = nullptr;
            _size = 0;
        }

        void push_back(const T& v) {
            T copy = v;
            push_back(std::move(v));
        }

        void push_back(T&& v) {
            list_node<T>* node = _nc_alloc_aligned<list_node<T>>(1);
            node->value = std::move(v);
            node->next = nullptr;
            node->prev = _last;

            if (!_first) {
                // Empty list
                _first = node;
            } else {
                _last->next = node;
            }

            _last = node;
        }

        iterator begin() { return { _first, this }; }
        const_iterator begin() const { return { _first, this }; }
        const_iterator cbegin() const { return { _first, this }; }

        iterator end() { return { nullptr, this }; }
        const_iterator end() const { return { nullptr, this }; }
        const_iterator cend() const { return { nullptr, this }; }
   };
}