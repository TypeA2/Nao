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

#include <Windows.h>

namespace nao {
    enum class event_result {
        ok
    };

    struct pos {
        int x, y;
    };

    struct size {
        int w, h;
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
        event(const native_event& native);
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

        size new_size() const;
    };
}
