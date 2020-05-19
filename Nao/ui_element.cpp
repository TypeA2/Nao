#include "ui_element.h"

#include "utils.h"

#include <unordered_set>

#include <strings.h>

ui_element::ui_element(ui_element* parent) : _parent { parent } {

}

ui_element::ui_element(ui_element* parent, const std::wstring& classname, const rectangle& rect,
    DWORD style, DWORD ex_style) : ui_element(parent) {

    // Default if not registered
    if (!win32::registered(classname)) {
        win32::register_once(win32::wnd_class { .class_name = classname });
    }

    _handle = win32::create_window_ex(classname, L"", style, rect, parent, ex_style, this);

    ASSERT(_handle);
}

ui_element::ui_element(ui_element* parent, const std::wstring& classname, DWORD style, DWORD ex_style)
    : ui_element(parent, classname, { }, style, ex_style) {
    
}

ui_element::ui_element(ui_element* parent, int string, const rectangle& rect, DWORD style, DWORD ex_style)
    : ui_element(parent, win32::load_wstring(string), rect, style, ex_style) {
    
}

ui_element::ui_element(ui_element* parent, int string, DWORD style, DWORD ex_style)
    : ui_element(parent, string, { }, style, ex_style) {
    
}

ui_element::ui_element(ui_element* parent, const win32::wnd_class& wc, const rectangle& rect, DWORD style, DWORD ex_style)
    : ui_element(parent, register_once(wc), rect, style, ex_style) {

}

ui_element::ui_element(ui_element* parent, const win32::wnd_class& wc, DWORD style, DWORD ex_style)
    : ui_element(parent, wc, rectangle { }, style, ex_style) {
    
}

ui_element::~ui_element() {
    destroy();
}

bool ui_element::destroy() {
    HWND handle = _handle;
    _handle = nullptr;
    return DestroyWindow(handle);
}

ui_element* ui_element::parent() const {
    return _parent;
}

HWND ui_element::handle() const {
    return _handle;
}

win32::device_context ui_element::dc() const {
    return win32::device_context { _handle, GetDC(_handle), true };
}

dimensions ui_element::text_extent_point(const std::string& str) const {
    std::wstring wide = strings::to_utf16(str);
    SIZE size;
    ASSERT(GetTextExtentPoint32W(dc(), wide.c_str(), utils::narrow<uint32_t>(wide.size()), &size));

    return { .width = size.cx, .height = size.cy };
}

int64_t ui_element::width() const {
    RECT rect;
    bool res = GetClientRect(handle(), &rect);
    ASSERT(res);

    return static_cast<int64_t>(rect.right) - rect.left;
}

int64_t ui_element::height() const {
    RECT rect;
    bool res = GetClientRect(handle(), &rect);
    ASSERT(res);

    return static_cast<int64_t>(rect.bottom) - rect.top;
}

coordinates ui_element::coords() const {
    RECT rect;
    GetClientRect(handle(), &rect);

    return {
        .x = rect.left,
        .y = rect.top
    };
}

coordinates ui_element::screen_coords() const {
    RECT rect;
    GetWindowRect(handle(), &rect);

    return {
        .x = rect.left,
        .y = rect.top
    };
}

dimensions ui_element::dims() const {
    RECT rect;
    GetClientRect(handle(), &rect);

    return {
        .width = rect.right,
        .height = rect.bottom
    };
}

dimensions ui_element::screen_dims() const {
    RECT rect;
    GetWindowRect(handle(), &rect);

    return {
        .width = rect.right,
        .height = rect.bottom
    };
}

rectangle ui_element::rect() const {
    RECT rect;
    GetClientRect(handle(), &rect);

    return {
        .x = rect.left,
        .y = rect.top,
        .width = rect.right,
        .height = rect.bottom
    };
}

rectangle ui_element::screen_rect() const {
    RECT rect;
    GetWindowRect(handle(), &rect);

    return {
        .x = rect.left,
        .y = rect.top,
        .width = rect.right,
        .height = rect.bottom
    };
}

void ui_element::move(const rectangle& rect) const {
    SetWindowPos(handle(), nullptr,
        utils::narrow<int>(rect.x), utils::narrow<int>(rect.y),
        utils::narrow<int>(rect.width), utils::narrow<int>(rect.height), 0);
}

HDWP& ui_element::move_dwp(HDWP& dwp, const rectangle& rect) const {
    dwp = DeferWindowPos(dwp, _handle, nullptr,
        utils::narrow<int>(rect.x), utils::narrow<int>(rect.y),
        utils::narrow<int>(rect.width), utils::narrow<int>(rect.height), 0);

    return dwp;
}

void ui_element::set_style(DWORD style, bool enable) const {
    DWORD old_style = static_cast<DWORD>(GetWindowLongPtrW(handle(), GWL_STYLE));

    // Don't re-apply style
    if (enable && !(old_style & style)) {
        SetWindowLongPtrW(handle(), GWL_STYLE, old_style | style);
    } else if (!enable && (old_style & style)) {
        SetWindowLongPtrW(handle(), GWL_STYLE, old_style & ~style);
    }
}

void ui_element::set_ex_style(DWORD style, bool enable) const {
    DWORD old_ex_style = utils::narrow<DWORD>(GetWindowLongPtrW(handle(), GWL_EXSTYLE));

    if (enable && !(old_ex_style & style)) {
        SetWindowLongPtrW(handle(), GWL_EXSTYLE, old_ex_style | style);
    } else if (!enable && (old_ex_style & style)) {
        SetWindowLongPtrW(handle(), GWL_EXSTYLE, old_ex_style & ~style);
    }
}

void ui_element::set_font(HFONT font) const {
    send_message(WM_SETFONT, WPARAM(font), true);
}

void ui_element::set_enabled(bool enabled) const {
    EnableWindow(handle(), enabled);
}

void ui_element::set_window_text(const std::wstring& text) const {
    SetWindowTextW(_handle, text.c_str());
}

void ui_element::set_text(const std::string& text) const {
    set_text(strings::to_utf16(text));
}

void ui_element::set_text(const std::wstring& text) const {
    (void) send_message(WM_SETTEXT, 0, text.c_str());
}

void ui_element::set_focus() const {
    SetFocus(handle());
}

void ui_element::activate() const {
    SetActiveWindow(handle());
}

bool ui_element::redraw(UINT flags) const {
    return RedrawWindow(_handle, nullptr, nullptr, flags) != 0;
}

LRESULT ui_element::send_message(UINT msg, WPARAM wparam, LPARAM lparam) const {
    return SendMessageW(handle(), msg, wparam, lparam);
}

bool ui_element::post_message(UINT msg, WPARAM wparam, LPARAM lparam) const {
    return PostMessageW(handle(), msg, wparam, lparam);
}

void ui_element::set_handle(HWND handle) {
    ASSERT(!_handle && handle);

    _handle = handle;
}

void ui_element::wm_destroy() {
    wnd_proc(_handle, WM_DESTROY, 0, 0);
}

void ui_element::wm_size(int type, const dimensions& dims) {
    wnd_proc(_handle, WM_SIZE, type,
        MAKELPARAM(utils::narrow<int>(dims.width), utils::narrow<int>(dims.height)));
}

void ui_element::wm_paint() {
    // API dislikes no painting being done
    win32::paint_struct { this };
}

void ui_element::wm_command(WORD id, WORD code, HWND target) {
    wnd_proc(_handle, WM_COMMAND, MAKEWPARAM(id, code), reinterpret_cast<LPARAM>(target));
}

void ui_element::wm_notify(WPARAM id, NMHDR* hdr) {
    wnd_proc(_handle, WM_NOTIFY, id, reinterpret_cast<LPARAM>(hdr));
}

void ui_element::wm_mousemove(WPARAM which, const coordinates& at) {
    wnd_proc(_handle, WM_MOUSEMOVE, which,
        MAKELPARAM(utils::narrow<int>(at.x), utils::narrow<int>(at.y)));
}

void ui_element::wm_lbuttondown(WPARAM which, const coordinates& at) {
    wnd_proc(_handle, WM_LBUTTONDOWN, which,
        MAKELPARAM(utils::narrow<int>(at.x), utils::narrow<int>(at.y)));
}

void ui_element::wm_lbuttonup(WPARAM which, const coordinates& at) {
    wnd_proc(_handle, WM_LBUTTONUP, which,
        MAKELPARAM(utils::narrow<int>(at.x), utils::narrow<int>(at.y)));
}

LRESULT ui_element::wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

LRESULT ui_element::wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    // Retrieve instance
    ui_element* _this = reinterpret_cast<ui_element*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (_this) {
        // Predefined handlers
        switch (msg) {
            case WM_DESTROY:
                _this->wm_destroy();
                return 0;
            
            case WM_SIZE:
                _this->wm_size(utils::narrow<int>(wparam), dimensions::from_lparam(lparam));
                return 0;

            case WM_PAINT:
                _this->wm_paint();
                return 0;

            case WM_COMMAND:
                _this->wm_command(LOWORD(wparam), HIWORD(wparam), reinterpret_cast<HWND>(lparam));
                return 0;

            case WM_NOTIFY:
                _this->wm_notify(wparam, reinterpret_cast<NMHDR*>(lparam));
                return 0;

            case WM_MOUSEMOVE:
                _this->wm_mousemove(wparam, coordinates::from_lparam(lparam));
                return 0;

            case WM_LBUTTONDOWN:
                _this->wm_lbuttondown(wparam, coordinates::from_lparam(lparam));
                return 0;

            case WM_LBUTTONUP:
                _this->wm_lbuttonup(wparam, coordinates::from_lparam(lparam));
                return 0;

            default: break;
        }
        
        // Custom callback
        return _this->wnd_proc(hwnd, msg, wparam, lparam);
    }

    if (msg == WM_CREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lparam);
        auto element = static_cast<ui_element*>(cs->lpCreateParams);

        // Initialisation given
        if (element) {
            // Set handle and `this` pointer
            element->set_handle(hwnd);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(element));

            return 0;
        }
    }
    
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}
