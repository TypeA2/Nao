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


#include "event.h"

#include <variant>

#include <Windows.h>

#include <libnao/util/logging.h>
#include <libnao/util/defs.h>

namespace nao::ui {
    class layout;

    class window {
        NAO_LOGGER(window);

        LRESULT _last_msg_result{};
        size _min_size{};
        size _max_size{};
        margins _padding{};

        /* Optionally, this objec'ts name */
        std::string _name;

        // Parent window, if set
        window* _parent{};

        // Contained layout or widget, whichever applies
        window* _child{};

        protected:
        HWND _handle{};

        struct window_descriptor {
            // Builtin classes aren't registered
            bool builtin = false;

            // UTF-8 classname
            std::string_view cls{};

            // All styles applied on creation
            DWORD style = 0;

            // Extra styles
            DWORD ex_style = 0;

            // UTF-8 window name
            std::string_view name{};

            // Starting position
            position pos = { .x = CW_USEDEFAULT, .y = CW_USEDEFAULT };

            // Starting dimensions
            size size = { .w = CW_USEDEFAULT, .h = CW_USEDEFAULT };

            // Parent window
            window* parent = nullptr;
        };
        
        explicit window(const window_descriptor& w);
        

        [[nodiscard]] virtual event_result on_event(event& e);
        [[nodiscard]] virtual event_result on_resize(resize_event& e);

        /**
         * Send the given event to the requested instance.
         */
        [[nodiscard]] static event_result send_event(window& win, event& e);

        public:
        explicit window(window& w);
        window() = delete;
        virtual ~window();

        window(const window&) = delete;
        window& operator=(const window&) = delete;

        window(window&& other) noexcept;
        window& operator=(window&& other) noexcept;

        [[nodiscard]] HWND handle() const;
        [[nodiscard]] window* parent() const;

        /* Size and position relative to parent */
        [[nodiscard]] size client_size() const;
        [[nodiscard]] position client_pos() const;

        /* Calculate actual size based on target and min/max */
        [[nodiscard]] size constrain_size(size s) const;

        /**
         * Set minimum and maximum dimensions, including padding.
         * Negative values reset axis to default.
         */
        void set_minimum_size(const size& size);
        void set_minimum_size(long w, long h);
        [[nodiscard]] size minimum_size() const;

        void set_maximum_size(const size& size);
        void set_maximum_size(long w, long h);
        [[nodiscard]] size maximum_size() const;

        void set_padding(const margins& padding);
        void set_padding(long top, long right, long bot, long left);
        [[nodiscard]] margins padding() const;

        /* Window name */
        void set_name(std::string_view name);
        [[nodiscard]] std::string_view name() const;

        /* Disabling/enabling */
        void set_enabled(bool enabled);
        [[nodiscard]] bool enabled() const;

        /**
         * Set the contained window.
         * @note Derived classes should call this on the parent in the constructor to ensure
         *          that the vfptr is setup correctly.
         */
        virtual void set_window(window& w);

        /* Set the window's parent */
        void set_parent(window& win);

        private:
        void _create_window(const window_descriptor& w);

        static LRESULT wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    };
}
