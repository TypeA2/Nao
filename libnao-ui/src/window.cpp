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

#include <libnao/util/encoding.h>

#include <magic_enum.hpp>

#include "layout.h"

nao::ui::window::window(const window_descriptor& w)
    : _min_size{ GetSystemMetrics(SM_CXMINTRACK), GetSystemMetrics(SM_CYMINTRACK) }
    , _max_size{ GetSystemMetrics(SM_CXMAXTRACK), GetSystemMetrics(SM_CYMAXTRACK) } {
    _create_window(w);
}

nao::ui::window::window(window& w) : window{
    {
        .builtin = false,
        .cls = "libnao_generic_window",
        .style = WS_VISIBLE | WS_CHILD,
        .pos = { 0, 0 },
        .size = w.client_size(),
        .parent = &w,

    } } {
    w.set_window(*this);
}

nao::ui::window::~window() {
    DestroyWindow(_handle);
}


nao::ui::window::window(window&& other) noexcept {
    *this = std::forward<window>(other);
}

nao::ui::window& nao::ui::window::operator=(window&& other) noexcept {
    _handle = std::exchange(other._handle, nullptr);
    return *this;
}

nao::ui::event_result nao::ui::window::on_event(event& e) {
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
            info->ptMaxTrackSize.x = _max_size.w;
            info->ptMaxTrackSize.y = _max_size.h;
            _last_msg_result = 0;
            break;
        }

        case WM_COMMAND: {
            // Forward to child
            if (_child && _child->handle() == reinterpret_cast<HWND>(native.lparam)) {
                return _child->on_event(e);
            }

            break;
        }

        case WM_KEYDOWN: {
            key_event ev { native, static_cast<key_code>(native.wparam) };
            return on_keydown(ev);
        }

        default:
            _last_msg_result = native.call_default();
    }


    return event_result::ok;
}

nao::ui::event_result nao::ui::window::on_resize(resize_event& e) {
    if (_child) {
        auto [x, y] = _child->constrain_size(e.new_size());
        SetWindowPos(_child->handle(), nullptr, 0, 0, x, y, 0);
        
        return event_result::ok;
    }

    _last_msg_result = e.native().call_default();

    return event_result::ok;
}

nao::ui::event_result nao::ui::window::on_keydown(key_event& e) {
    _last_msg_result = e.native().call_default();

    return event_result::ok;
}

nao::ui::event_result nao::ui::window::send_event(window& win, event& e) {
    return win.on_event(e);
}

HWND nao::ui::window::handle() const {
    return _handle;
}

nao::ui::window* nao::ui::window::parent() const {
    return _parent;
}

nao::size nao::ui::window::client_size() const {
    RECT rect;
    GetClientRect(_handle, &rect);
    return { rect.right - rect.left, rect.bottom - rect.top };
}

nao::position nao::ui::window::client_pos() const {
    RECT rect;
    GetClientRect(_handle, &rect);

    // Coordinates relative to parent
    MapWindowPoints(_handle, GetParent(_handle), reinterpret_cast<POINT*>(&rect), 2);
    return { rect.left, rect.top };
}

nao::size nao::ui::window::constrain_size(size s) const {
    // Also constrain to parent
    return {
        .w = std::max<long>(std::min<long>(s.w, _max_size.w), _min_size.w),
        .h = std::max<long>(std::min<long>(s.h, _max_size.h), _min_size.h),
    };
}

void nao::ui::window::set_minimum_size(const size& size) {
    _min_size = size;

    if (_min_size.w < 0) {
        // Reset to default
        _min_size.w = GetSystemMetrics(SM_CXMINTRACK);
    }

    if (_min_size.h < 0) {
        _min_size.h = GetSystemMetrics(SM_CYMINTRACK);
    }


    // Adjust max sizes to make sense
    _max_size.w = std::max<long>(_min_size.w, _max_size.w);
    _max_size.h = std::max<long>(_min_size.h, _max_size.h);
    if (_min_size.w > _max_size.w) {
        _max_size.w = _min_size.w;
    }

    if (_min_size.h > _max_size.h) {
        _max_size.h = _min_size.h;
    }
}

void nao::ui::window::set_minimum_size(long w, long h) {
    set_minimum_size({ w, h });
}

nao::size nao::ui::window::minimum_size() const {
    return _min_size;
}

void nao::ui::window::set_maximum_size(const size& size) {
    _max_size = size;

    if (_max_size.w < 0) {
        // Reset to default
        _max_size.w = GetSystemMetrics(SM_CXMAXTRACK);
    }

    if (_max_size.h < 0) {
        _max_size.h = GetSystemMetrics(SM_CYMAXTRACK);
    }

    // Adjust max sizes to make sense
    _min_size.w = std::min<long>(_min_size.w, _max_size.w);
    _min_size.h = std::min<long>(_min_size.h, _max_size.h);
}

void nao::ui::window::set_maximum_size(long w, long h) {
    set_maximum_size({ w, h });
}


nao::size nao::ui::window::maximum_size() const {
    return _max_size;
}

void nao::ui::window::set_padding(const margins& padding) {
    _padding = padding;
}

void nao::ui::window::set_padding(long top, long right, long bot, long left) {
    _padding = { top, right, bot, left };
}

nao::margins nao::ui::window::padding() const {
    return _padding;
}

void nao::ui::window::set_name(std::string_view name) {
    _name = name;
}

std::string_view nao::ui::window::name() const {
    return _name;
}

void nao::ui::window::set_enabled(bool enabled) {
    (void)this;
    EnableWindow(_handle, enabled);
}

bool nao::ui::window::enabled() const {
    return IsWindowEnabled(_handle);
}

void nao::ui::window::set_window(window& w) {
    w.set_parent(*this);
    _child = &w;
}

void nao::ui::window::set_parent(window& win) {
    logger().trace("Attaching to {}", fmt::ptr(&win));

    _parent = &win;
    SetParent(_handle, _parent->handle());

    auto [w, h] = _parent->client_size();
    SetWindowPos(_handle, nullptr, 0, 0, w, h, 0);
}

void nao::ui::window::_create_window(const window_descriptor& w) {
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

    logger().trace("Creating instance of {} for {}", w.cls, fmt::ptr(this));

    _handle = CreateWindowExW(
        w.ex_style,
        cls_wide.c_str(),
        w.name.empty() ? nullptr : utf8_to_wide(w.name).c_str(),
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

LRESULT nao::ui::window::wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    auto* _this = reinterpret_cast<window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    // May need to be set
    if (_this) {
        logger().trace("Handling message: msg={:<#8x} wparam={:<#16x} lparam={:<#16x}",
                       msg, wparam, lparam);

        event e { { hwnd, msg, wparam, lparam } };
        (void)_this->on_event(e);

        return _this->_last_msg_result;
    }

    if (msg == WM_CREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lparam);

        if (auto* param = static_cast<window*>(cs->lpCreateParams)) {
            logger().trace("Attaching callback for {}", fmt::ptr(hwnd));

            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(param));

            return 0;
        }

        logger().debug("Created without `this` pointer.");
    }

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}
