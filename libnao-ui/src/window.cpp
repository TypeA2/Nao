#include "window.h"

#include <unordered_set>
#include <utility>

#include <libnao/encoding.h>

namespace nao {
    LRESULT event::native_event::call_default() const {
        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }



    event::event(const native_event& native) : _native{ native } { }


    event::event(event&& other) noexcept {
        *this = std::forward<event>(other);
    }


    event& event::operator=(event&& other) noexcept {
        _native = other._native;

        return *this;
    }


    event::operator native_event() const {
        return _native;
    }


    const event::native_event& event::native() const {
        return _native;
    }


    size resize_event::new_size() const {
        return {
            .w = LOWORD(_native.lparam),
            .h = HIWORD(_native.lparam)
        };
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

        _last_msg_result = 0;

        switch (native.msg) {
            case WM_DESTROY:
                PostQuitMessage(EXIT_SUCCESS);
                break;

            case WM_SIZE: {
                resize_event ev{ native };
                return on_resize(ev);
            }

            default:
                _last_msg_result = native.call_default();
        }

        
        return event_result::ok;
    }


    event_result window::on_resize(resize_event& e) {
        _last_msg_result = e.native().call_default();

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

        log.debug("Window: {}", fmt::ptr(GetModuleHandleW(nullptr)));

        if (!w.builtin && !class_registry.contains(cls_wide)) {
            logger().info("Registering class \"{}\" for first-time use", w.cls);

            WNDCLASSEXW wc{
                .cbSize = sizeof(WNDCLASSEXW),
                .lpfnWndProc = &window::wnd_proc_fwd,
                .hInstance = GetModuleHandleW(nullptr),
                .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
                .lpszClassName = cls_wide.c_str(),
            };

            if (RegisterClassExW(&wc) == 0) {
                logger().critical("Failed to register class {}", w.cls);
            }

            class_registry.insert(cls_wide);
        }
        logger().debug("Creating instance of {} for {}", w.cls, fmt::ptr(this));
  
        _handle = CreateWindowExW(
            w.ex_style,
            cls_wide.c_str(),
            utf8_to_wide(w.name).c_str(),
            w.style,
            w.pos.x, w.pos.y, w.size.w, w.size.h,
            w.parent ? w.parent->_handle : nullptr, nullptr,
            GetModuleHandleW(nullptr),
            this
        );

        if (!_handle) {
            logger().critical("Handle is null: {}, name: {}", w.cls, w.name);
        }
    }


    LRESULT window::wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        auto* _this = reinterpret_cast<window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        // May need to be set
        if (_this) {
            logger().trace("Handling message: msg={:<#8x} wparam={:<#16x} lparam={:<#16x}", msg, wparam, lparam);

            event e { { hwnd, msg, wparam, lparam } };
            (void) _this->on_event(e);

            return _this->_last_msg_result;
        }

        if (msg == WM_CREATE) {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lparam);

            if (auto* param = static_cast<window*>(cs->lpCreateParams)) {
                logger().debug("Attaching callback for {}", fmt::ptr(hwnd));

                SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(param));

                return 0;
            }

            logger().debug("Created without `this` pointer.");
        }

        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }



}
