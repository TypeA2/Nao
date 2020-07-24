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

#include "naocore_defs.h"

#include <algorithm>

//
// STL-like vector
//

namespace nao {
    template <typename T>
    class vector {
        public:
        // Types
        using value_type = T;
        using size_type = size_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = pointer;
        using const_iterator = const_pointer;

        private:
        pointer _data = nullptr;
        size_type _size = 0;
        size_type _capacity = 0;

        public:

        ~vector() {
            for (size_type i = 0; i < _size; ++i) {
                _data[i].~value_type();
            }
            _nc_free_aligned(_data);
        }

        vector() = default;

        vector(size_type count, const value_type& val = value_type()) { assign(count, val); }

        // Copy constructor
        vector(const vector& other) { assign(other.begin(), other.end()); }
        vector& operator=(const vector& other) {
            assign(other.begin(), other.end());
            return *this;
        }

        // Move
        vector(vector&& other) noexcept { assign(std::forward<vector<value_type>>(other)); }
        vector& operator=(vector&& other) noexcept { return assign(std::forward<vector<value_type>>(other)); }

        vector& assign(size_type count, const value_type& val) {
            _resize(count);

            pointer data = _data;
            for (size_type i = 0; i < count; ++i) {
                new (data++) value_type(val);
            }
            _size = count;
            return *this;
        }

        template <typename InputIt>
        vector& assign(InputIt first, InputIt last) {
            size_type count = std::distance(first, last);
            _resize(count);
            std::copy(first, last, _data);
            _size = count;
            return *this;
        }

        vector& assign(vector&& other) noexcept {
            _data = other._data;
            _size = other._size;
            _capacity = other._capacity;

            other._data = nullptr;
            other._size = 0;
            other._capacity = 0;

            return *this;
        }

        iterator begin() { return _data; }
        const_iterator begin() const { return _data; }
        const_iterator cbegin() const { return _data; }

        iterator end() { return _data + _size; }
        const_iterator end() const { return _data + _size; }
        const_iterator cend() const { return _data + _size; }

        size_type size() const { return _size; }
        size_type capacity() const { return _capacity; }

        reference operator[](size_t index) { return _data[index]; }
        const_reference operator[](size_t index) const { return _data[index]; }

        template <typename... Args>
        reference emplace_back(Args&&... args) {
            _resize(_size + 1);
            new (_data + _size) value_type(std::forward<Args>(args)...);
            _size += 1;
            return _data[_size - 1];
        }

        void push_back(const_reference value) {
            _resize(_size + 1);
            new (_data + _size) value_type(value);
            _size += 1;
        }

        void push_back(value_type&& value) {
            _resize(_size + 1);
            new (_data + _size) value_type(value);
            _size += 1;
        }

        private:
        void _resize(size_type new_max) {
            if (new_max > _capacity) {
                size_type target = std::max<size_type>(16, new_max);
                while (target < new_max) {
                    target = static_cast<size_type>(target * ((_size > 1024) ? 1.25 : 2.));
                }

                pointer temp = _nc_alloc_aligned<value_type>(target);

                if (_data) {
                    std::move(_data, _data + _size, temp);
                    _nc_free_aligned(_data);
                }

                _data = temp;
                _capacity = target;
            }
        }
    };
}