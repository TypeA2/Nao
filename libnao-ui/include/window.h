#pragma once

#include <Windows.h>

#include <string>

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

    class window {
        HWND _handle;

        public:
        window(const std::wstring& cls = L"nao_window");
        virtual ~window();

        window(const window&) = delete;
        window& operator=(const window&) = delete;

        window(window&& other) noexcept;
        window& operator=(window&& other) noexcept;

        [[nodiscard]] virtual event_result on_event(event& e);

        private:
        static LRESULT wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    };
}