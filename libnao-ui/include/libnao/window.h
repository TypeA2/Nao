#pragma once

#include <Windows.h>

#include <libnao/logging.h>

namespace nao {
    enum class event_result {
        ok
    };

    class event {
        public:
        struct native_event {
            HWND hwnd;
            UINT msg;
            WPARAM wparam;
            LPARAM lparam;
        };

        private:
        native_event _native;

        public:

        event(const native_event& native);
        ~event() = default;

        event(const event&) = delete;
        event& operator=(const event&) = delete;

        event(event&& other) noexcept;
        event& operator=(event&& other) noexcept;

        [[nodiscard]] const native_event& native() const;
    };


    struct pos {
        int x, y;
    };

    struct dims {
        int w, h;
    };


    class window {
        static spdlog::logger& logger();

        protected:
        HWND _handle;

        struct window_descriptor {
            // Builtin classes aren't registered
            bool builtin = false;

            // UTF-8 classname
            std::string_view cls;

            // All styles applied on creation
            DWORD style;

            // UTF-8 window name
            std::string_view name;

            // Starting position
            pos pos = { .x = CW_USEDEFAULT, .y = CW_USEDEFAULT };

            // Starting dimensions
            dims dims = { .w = CW_USEDEFAULT, .h = CW_USEDEFAULT };

            // Parent window
            window* parent = nullptr;
        };

        // For built-in classess
        window(const window_descriptor& w);

        public:
        window() = delete;
        virtual ~window();

        window(const window&) = delete;
        window& operator=(const window&) = delete;

        window(window&& other) noexcept;
        window& operator=(window&& other) noexcept;

        [[nodiscard]] virtual event_result on_event(event& e);

        HWND handle() const;

        private:
        static LRESULT wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    };
}
