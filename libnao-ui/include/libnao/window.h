#pragma once

#include <Windows.h>

#include <libnao/logging.h>

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

            LRESULT call_default() const;
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


    class window {
        NAO_LOGGER(window);

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
