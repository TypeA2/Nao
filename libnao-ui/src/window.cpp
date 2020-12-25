#include "window.h"

#include <stdexcept>
#include <unordered_set>
#include <utility>

#include <libnao/encoding.h>

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


    window::window(const window_descriptor& w) {
        _create_window(w);
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


    HWND window::handle() const {
        return _handle;
    }


    size window::size() const {
        RECT rect;
        GetWindowRect(_handle, &rect);
        return { rect.right, rect.bottom };
    }


    void window::_create_window(const window_descriptor& w) {
        static std::unordered_set<std::wstring> class_registry;

        std::wstring cls_wide = utf8_to_wide(w.cls);

        if (!w.builtin && !class_registry.contains(cls_wide)) {
            logger().info("Registering class \"{}\" for first-time use", w.cls);

            WNDCLASSEXW wc{
                .cbSize = sizeof(wc),
                .lpfnWndProc = &window::wnd_proc_fwd,
                .hInstance = GetModuleHandleW(nullptr),
                .hCursor = LoadCursorW(nullptr, IDC_ARROW),
                .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
                .lpszClassName = cls_wide.c_str()
            };

            if (RegisterClassExW(&wc) == 0) {
                logger().critical("Failed to register class \"{}\"", w.cls);
                throw std::runtime_error("failed to register class");
            }

            class_registry.insert(cls_wide);
        }


        _handle = CreateWindowExW(
            0,
            cls_wide.c_str(),
            utf8_to_wide(w.name).c_str(),
            w.style,
            w.pos.x, w.pos.y,
            w.size.w, w.size.h,
            w.parent ? w.parent->_handle : nullptr, nullptr,
            GetModuleHandleW(nullptr),
            this
        );

        if (!_handle) {
            logger().critical("Handle is null: {}", w.cls, w.name);
        }
    }


    LRESULT window::wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        auto* _this = reinterpret_cast<window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        // May need to be set
        if (_this) {
            event e { { hwnd, msg, wparam, lparam }};

            switch (_this->on_event(e)) {
                case event_result::ok:
                    return 1;
            }

            return 0;
        }

        if (msg == WM_CREATE) {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lparam);

            if (auto* param = static_cast<window*>(cs->lpCreateParams)) {
                logger().debug("Attaching callback for {}", fmt::ptr(param));

                param->_handle = hwnd;
                SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(param));

                return 0;
            }

            logger().debug("Created without this pointer.");
        }

        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }



}
