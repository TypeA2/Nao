#include "window.h"

#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace nao {
    event::event(const native_event& native) : _native{ native } { }


    event::event(event&& other) noexcept {
        *this = std::forward<event>(other);
    }


    event& event::operator=(event&& other) noexcept {
        _native = other._native;

        return *this;
    }


    const event::native_event& event::native() const {
        return _native;
    }


    window::window(const std::wstring& cls) {
        static std::unordered_set<std::wstring> class_registry;

        if (!class_registry.contains(cls)) {
            WNDCLASSEXW wc{
                .cbSize = sizeof(wc),
                .lpfnWndProc = &window::wnd_proc_fwd,
                .hInstance = GetModuleHandleW(nullptr),
                .hCursor = LoadCursorW(nullptr, IDC_ARROW),
                .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
                .lpszClassName = cls.c_str()
            };

            if (RegisterClassExW(&wc) == 0) {
                throw std::runtime_error("failed to register class");
            }

            class_registry.insert(cls);
        }


        _handle = CreateWindowExW(
            0,
            cls.c_str(),
            L"",
            WS_VISIBLE | WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            nullptr, nullptr,
            GetModuleHandleW(nullptr),
            this
        );
    }


    window::~window() {
        DestroyWindow(_handle);
    }


    window::window(window&& other) noexcept {
        *this = std::forward<window>( other);
    }


    window& window::operator=(window&& other) noexcept {
        _handle = std::exchange(other._handle, nullptr);
        return *this;
    }


    event_result window::on_event(event& e) {

        const auto& native = e.native();

        if (native.msg == WM_DESTROY) {
            PostQuitMessage(EXIT_SUCCESS);
            return event_result::ok;
        }

        DefWindowProcW(native.hwnd, native.msg, native.wparam, native.lparam);

        return event_result::ok;
    }


    LRESULT window::wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        auto* _this = reinterpret_cast<window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        // May need to be set
        if (_this) {
            event e { { hwnd, msg, wparam, lparam }};

            switch (_this->on_event(e)) {
                case event_result::ok:
                    return 0;
            }

            return 0;
        }

        if (msg == WM_CREATE) {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lparam);

            if (auto* param = static_cast<window*>(cs->lpCreateParams)) {
                param->_handle = hwnd;
                SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(param));

                return 0;
            }
        }

        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }



}