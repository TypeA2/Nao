/**
 *  This file is part of libnao-ui.
 *
 *  libnao-ui is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libnao-ui is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libnao-ui.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <libnao/util/defs.h>

#include <Windows.h>

namespace nao::ui {
    enum class event_result {
        /* OK status */
        ok,

        /* Generic error */
        err,
    };

    class event {
        public:
        struct native_event {
            HWND hwnd;
            UINT msg;
            WPARAM wparam;
            LPARAM lparam;

            [[maybe_unused]] LRESULT call_default() const;
        };

        protected:
        native_event _native;

        public:
        explicit event(const native_event& native);
        virtual ~event() = default;

        event(const event&) = delete;
        event& operator=(const event&) = delete;

        event(event&& other) noexcept;
        event& operator=(event&& other) noexcept;

        [[nodiscard]] explicit operator native_event() const;

        [[nodiscard]] const native_event& native() const;
    };

    class resize_event : public event {
        public:
        using event::event;

        [[nodiscard]] size new_size() const;
    };

    enum class key_code {
        invalid      = 0x00,
        left_mouse   = 0x01,
        right_mouse  = 0x02,
        middle_mouse = 0x04,

        backspace = 0x08,
        tab       = 0x09,
        enter     = 0x0D,
        shift     = 0x10,
        ctrl      = 0x11,
        alt       = 0x12,
        pause     = 0x13,
        capslock  = 0x14,
        esc       = 0x1B,
        space     = 0x20,


        key_0 = 0x30, key_1, key_2, key_3, key_4, key_5, key_6, key_7, key_8, key_9,
        A = 0x41, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        play = 0xFA,
    };

    class key_event : public event {
        key_code _code;

        public:
        explicit key_event(const native_event& native, key_code key);

        [[nodiscard]] key_code key() const;
    };
}
