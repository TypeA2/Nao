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

#include <libnao/logging.h>

#include "event.h"

namespace nao {
    class window {
        NAO_LOGGER(window)

        LRESULT _last_msg_result{};

        protected:
        HWND _handle;

        struct window_descriptor {
            // Builtin classes aren't registered
            bool builtin = false;

            // UTF-8 classname
            std::string_view cls;

            // All styles applied on creation
            DWORD style = 0;

            // Extra styles
            DWORD ex_style = 0;

            // UTF-8 window name
            std::string_view name;

            // Starting position
            pos pos = { .x = CW_USEDEFAULT, .y = CW_USEDEFAULT };

            // Starting dimensions
            size size = { .w = CW_USEDEFAULT, .h = CW_USEDEFAULT };

            // Parent window
            window* parent = nullptr;
        };
        
        window(const window_descriptor& w);

        [[nodiscard]] virtual event_result on_event(event& e);
        [[nodiscard]] virtual event_result on_resize(resize_event& e);


        public:
        window() = delete;
        virtual ~window();

        window(const window&) = delete;
        window& operator=(const window&) = delete;

        window(window&& other) noexcept;
        window& operator=(window&& other) noexcept;

        [[nodiscard]] HWND handle() const;

        [[nodiscard]] size size() const;

        private:
        void _create_window(const window_descriptor& w);

        static LRESULT wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    };
}
