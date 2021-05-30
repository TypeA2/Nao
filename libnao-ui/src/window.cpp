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

#include "window.h"

#include <unordered_set>
#include <utility>

#include <libnao/encoding.h>

nao::window::window(const window_descriptor& w) {
    _create_window(w);
}


nao::window::~window() {
    DestroyWindow(_handle);
}


nao::window::window(window&& other) noexcept {
    *this = std::forward<window>(other);
}


nao::window& nao::window::operator=(window&& other) noexcept {
    _handle = std::exchange(other._handle, nullptr);
    return *this;
}


nao::event_result nao::window::on_event(event& e) {
    const auto& native = e.native();

    _last_msg_result = 0;

    switch (native.msg) {
        case WM_DESTROY:
            PostQuitMessage(EXIT_SUCCESS);
            break;

        case WM_SIZE: {
            if (auto* ev = dynamic_cast<resize_event*>(&e)) {
                return on_resize(*ev);
            }

            resize_event ev{ native };
            return on_resize(ev);
        }

        case WM_GETMINMAXINFO: {
            auto info = reinterpret_cast<MINMAXINFO*>(native.lparam);
            info->ptMinTrackSize.x = _min_size.w;
            info->ptMinTrackSize.y = _min_size.h;
            _last_msg_result = 0;
            break;
        }

        default:
            _last_msg_result = native.call_default();
    }


    return event_result::ok;
}


nao::event_result nao::window::on_resize(resize_event& e) {
    _last_msg_result = e.native().call_default();

    return event_result::ok;
}


HWND nao::window::handle() const {
    return _handle;
}


nao::size nao::window::client_size() const {
    RECT rect;
    GetClientRect(_handle, &rect);
    return { rect.right - rect.left, rect.bottom - rect.top };
}


nao::position nao::window::client_pos() const {
    RECT rect;
    GetClientRect(_handle, &rect);

    // Coordinates relative to parent
    MapWindowPoints(_handle, GetParent(_handle), reinterpret_cast<POINT*>(&rect), 2);
    return { rect.left, rect.top };
}


void nao::window::set_minimum_size(const size& size) {
    assert(size.w > 0 && size.h > 0);
    _min_size = size;
}


void nao::window::set_minimum_size(long w, long h) {
    assert(w > 0 && h > 0);
    _min_size = { w, h };
}


nao::size nao::window::minimum_size() const {
    return _min_size;
}


void nao::window::_create_window(const window_descriptor& w) {
    static std::unordered_set<std::wstring> class_registry;

    std::wstring cls_wide = utf8_to_wide(w.cls);

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

LRESULT nao::window::wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    auto* _this = reinterpret_cast<window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    // May need to be set
    if (_this) {
        logger().trace("Handling message: msg={:<#8x} wparam={:<#16x} lparam={:<#16x}", msg, wparam, lparam);

        event e { { hwnd, msg, wparam, lparam } };
        (void)_this->on_event(e);

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
