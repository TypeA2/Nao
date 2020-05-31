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
#include "nao/string_view.h"
#include "nao/concepts.h"

#include <cstddef>
#include <algorithm>

//
// STL-like string implementation
//

namespace nao {
    template <typename CharT>
    class basic_string {
        // For some reason MSVC isn't happy with an equivalent requires clause
        static_assert(std::same_as<CharT, char> || std::same_as<CharT, wchar_t>, "unsupported character type");

        public:
        // Types
        using value_type = CharT;
        using size_type = size_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = pointer;
        using const_iterator = const_pointer;

        static constexpr size_type npos = size_type(-1);

        private:
        // Pointer to the dynamically allocated buffer
        pointer _data = nullptr;

        // Buffer dimensions. Does not include null terminator
        size_type _size = 0;
        size_type _capacity = 0;

        public:

        ~basic_string() {
            _nc_free_aligned(_data);
        }

        // Empty
        constexpr basic_string() noexcept {
            // Empty
            _resize(16);
            _data[0] = 0;
        }

        // Copying
        constexpr basic_string(const basic_string& other) { assign(other); }
        constexpr basic_string& operator=(const basic_string& other) { return assign(other); }

        // Moving
        constexpr basic_string(basic_string&& other) noexcept { assign(other); }
        constexpr basic_string& operator=(basic_string&& other) noexcept { return assign(other); }

        constexpr basic_string(size_type count, value_type ch) { assign(count, ch); }

        constexpr basic_string(const_pointer s) { assign(s); }
        constexpr basic_string& operator=(const_pointer s) { return assign(s); }

        // Other string implementations
        template <string_like<value_type> T> requires !std::same_as<std::decay_t<T>, basic_string<CharT>>
        constexpr basic_string(const T& s) { assign(s); }

        template <string_like<value_type> T> requires !std::same_as<std::decay_t<T>, basic_string<CharT>>
        constexpr basic_string& operator=(const T& s) { return assign(s); }

        // String view
        constexpr basic_string(basic_string_view<CharT> sv) { assign(sv.data(), sv.size()); }
        constexpr basic_string& operator=(basic_string_view<CharT> sv) { return assign(sv.data(), sv.size()); }

        // Assignment implementation
        constexpr basic_string& assign(const basic_string& other) {
            _size = other._size;
            _capacity = 0;
            _resize(_size + 1);
            std::copy_n(other._data, _size + 1, _data);
            return *this;
        }

        constexpr basic_string& assign(basic_string&& other) noexcept {
            _size = other._size;
            _capacity = other._capacity;
            _data = other._data;

            other._size = 0;
            other._capacity = 0;
            other._data = nullptr;

            return *this;
        }

        constexpr basic_string& assign(size_type count, value_type ch) {
            _resize(count + 1);
            std::fill_n(_data, count, ch);
            _data[count] = 0;
            _size = count;
            return *this;
        }

        constexpr basic_string& assign(const_pointer s) {
            _size = 0;
            const_pointer p = s;
            while (*p++) { ++_size; }
            _resize(_size + 1);

            std::copy_n(s, _size + 1, _data);
            return *this;
        }

        constexpr basic_string& assign(const_pointer s, size_type size) {
            _resize(size + 1);
            std::copy_n(s, size, _data);
            _data[size] = 0;
            _size = size;
            return *this;
        }

        template <string_like<value_type> T> requires !std::same_as<std::decay_t<T>, basic_string<CharT>>
        constexpr basic_string& assign(const T& s) { return assign(s.c_str(), s.size()); }

        constexpr basic_string& append(const_pointer s) {
            size_type appended_length = 0;
            const auto* p = s;
            while (*p++) { ++appended_length; }
            _resize(_size + appended_length + 1);

            std::copy_n(s, appended_length, _data + _size);
            _size += appended_length;
            _data[_size] = 0;
            return *this;
        }

        constexpr basic_string& append(const_pointer s, size_type size) {
            _resize(_size + size + 1);
            std::copy_n(s, size, _data + _size);
            _size += size;
            _data[_size] = 0;
            return *this;
        }

        constexpr basic_string& append(basic_string_view<CharT> sv) {
            _resize(_size + sv.size() + 1);
            std::copy_n(sv.data(), sv.size(), _data + _size);
            _size += sv.size();
            _data[_size] = 0;
            return *this;
        }

        constexpr pointer data() { return _data; }
        constexpr const_pointer data() const { return _data; }
        constexpr const_pointer c_str() const { return _data; }
        constexpr size_type size() const { return _size; }

        constexpr reference front() { return *_data; }
        constexpr const_reference front() const { return *_data; }

        constexpr operator basic_string_view<CharT>() const noexcept { return { _data, _size }; }

        constexpr iterator begin() { return _data; }
        constexpr const_iterator begin() const { return _data; }
        constexpr const_iterator cbegin() const { return _data; }

        constexpr iterator end() { return _data + _size; }
        constexpr const_iterator end() const { return _data + _size; }
        constexpr const_iterator cend() const { return _data + _size; }

        constexpr basic_string<wchar_t> wide() const {
            if constexpr (std::is_same_v<CharT, wchar_t>) {
                // Copy
                return *this;
            } else {
                // UTF-8 -> UTF-16
                size_t size = _nc_narrow_to_wide(_data, _size, nullptr);
                if (size == 0) { return {}; }

                basic_string<wchar_t> conv(size, 0);
                if (_nc_narrow_to_wide(_data, _size, conv.data()) != conv.size()) {
                    return {};
                }

                return conv;
            }
        }

        constexpr basic_string<char> narrow() const {
            if constexpr (std::is_same_v<CharT, char>) {
                return *this;
            } else {
                size_t size = _nc_wide_to_narrow(_data, _size, nullptr);
                if (size == 0) { return {}; }

                basic_string<char> conv(size, 0);
                if (_nc_wide_to_narrow(_data, _size, conv.data()) != conv.size()) {
                    return {};
                }

                return conv;
            }
        }

        private:
        constexpr void _resize(size_type new_max) {
            if (new_max > _capacity) {
                size_type target = std::max<size_type>(16, new_max);
                while (target < new_max) {
                    target = static_cast<size_type>(target * ((_size > 1024) ? 1.25 : 2.));
                }

                // Don't need to initialise, zero-terminator is copied
                // also throw on error, just exit everything
                //pointer temp = new value_type[target];
                pointer temp =  _nc_alloc_aligned<value_type>(target);

                if (_data) {
                    std::copy_n(_data, _size + 1, temp);
                    _nc_free_aligned(_data);
                }
                
                _data = temp;
                _capacity = target;
            }
        }
    };

    template <typename CharT>
    constexpr basic_string<CharT> operator+(const basic_string<CharT>& lhs, const basic_string<CharT>& rhs) {
        return basic_string { lhs }.append(rhs);
    }

    template <typename CharT>
    constexpr basic_string<CharT> operator+(const basic_string<CharT>& lhs, const CharT* rhs) {
        return basic_string { lhs }.append(rhs);
    }

    template <typename CharT>
    constexpr basic_string<CharT> operator+(const CharT* lhs, const basic_string<CharT>& rhs) {
        return basic_string { lhs }.append(rhs);
    }

    template <typename CharT>
    constexpr basic_string<CharT> operator+(const basic_string<CharT>& lhs, basic_string_view<CharT> rhs) {
        return basic_string { lhs }.append(rhs);
    }

    template <typename CharT>
    constexpr basic_string<CharT> operator+(basic_string_view<CharT> lhs, const basic_string<CharT>& rhs) {
        return basic_string { lhs }.append(rhs);
    }

    // Common strings
    using string = basic_string<char>;
    using wstring = basic_string<wchar_t>;
}
