#pragma once

#include "frameworks.h"

#include "utils.h"
#include "win32.h"

#include <memory>
#include <functional>

class ui_element {
    ui_element* _parent = nullptr;
    HWND _handle = nullptr;

    public:
    explicit ui_element(ui_element* parent);

    // Create from a registered class
    ui_element(ui_element* parent,
        const std::wstring& classname,
        const rectangle& rect, DWORD style, DWORD ex_style = 0);

    ui_element(ui_element* parent,
        const std::wstring& classname,
        DWORD style, DWORD ex_style = 0);

    // Win32 resource
    ui_element(ui_element* parent, int string,
        const rectangle& rect, DWORD style, DWORD ex_style = 0);

    ui_element(ui_element* parent, int string,
        DWORD style, DWORD ex_style = 0);
    
    // Register and create
    ui_element(ui_element* parent,
        const win32::wnd_class& wc,
        DWORD style, DWORD ex_style = 0);

    ui_element(ui_element* parent,
        const win32::wnd_class& wc,
        const rectangle& rect, DWORD style, DWORD ex_style = 0);

    ui_element() = default;

    ui_element(const ui_element&) = delete;
    ui_element& operator=(const ui_element&) noexcept = delete;

    virtual ~ui_element();

    // Call DestroyWindow on the window handle
    bool destroy();

    ui_element* parent() const;
    HWND handle() const;
    win32::device_context dc() const;

    dimensions text_extent_point(const std::string& str) const;

    int64_t width() const;
    int64_t height() const;

    dimensions dims() const;
    dimensions screen_dims() const;

    coordinates coords() const;
    coordinates screen_coords() const;

    rectangle rect() const;
    rectangle screen_rect() const;

    // Move using SetWindowPos
    void move(const rectangle& rect) const;

    // Move using DeferWindowPos, for multiple windows
    HDWP& move_dwp(HDWP& dwp, const rectangle& rect) const;

    // Set the window style
    void set_style(DWORD style, bool enable = true) const;

    // Set an extended style
    void set_ex_style(DWORD style, bool enable = true) const;

    // Set window font
    void set_font(HFONT font) const;

    // Specifically enable or disable
    void set_enabled(bool enabled = true) const;

    // SetWindowText forward
    void set_window_text(const std::wstring& text) const;

    // Sets a control's text
    void set_text(const std::string& text) const;
    void set_text(const std::wstring& text) const;

    // Set focus on a window
    void set_focus() const;
    void activate() const;

    // Redraw entire window
    bool redraw(UINT flags) const;

    // Manually send or post messages
    [[maybe_unused]] LRESULT send_message(UINT msg, WPARAM wparam = 0, LPARAM lparam = 0) const;
    template <concepts::pointer_or_integral W, concepts::pointer_or_integral L>
    [[maybe_unused]] LRESULT send_message(UINT msg, W wparam, L lparam) const {
        static_assert(sizeof(WPARAM) == sizeof(void*) && sizeof(LPARAM) == sizeof(void*));

        WPARAM _wparam;
        if constexpr (concepts::pointer<W>) {
            _wparam = reinterpret_cast<WPARAM>(wparam);
        } else {
            _wparam = static_cast<WPARAM>(wparam);
        }

        LPARAM _lparam;
        if constexpr (concepts::pointer<L>) {
            _lparam = reinterpret_cast<LPARAM>(lparam);
        } else {
            _lparam = static_cast<LPARAM>(lparam);
        }

        return send_message(msg, _wparam, _lparam);
    }

    [[maybe_unused]] bool post_message(UINT msg, WPARAM wparam = 0, LPARAM lparam = 0) const;

    template <concepts::pointer_or_integral W, concepts::pointer_or_integral L>
    [[maybe_unused]] bool post_message(UINT msg, W wparam, L lparam) const {
        static_assert(sizeof(WPARAM) == sizeof(void*) && sizeof(LPARAM) == sizeof(void*));

        WPARAM _wparam;
        if constexpr (concepts::pointer<W>) {
            _wparam = reinterpret_cast<WPARAM>(wparam);
        } else {
            _wparam = static_cast<WPARAM>(wparam);
        }

        LPARAM _lparam;
        if constexpr (concepts::pointer<L>) {
            _lparam = reinterpret_cast<LPARAM>(lparam);
        } else {
            _lparam = static_cast<LPARAM>(lparam);
        }

        return post_message(msg, _wparam, _lparam);
    }

    protected:
    void set_handle(HWND handle);

    // Overridable message handlers
    virtual void wm_destroy();
    virtual void wm_size(int type, const dimensions& dims);
    virtual void wm_paint();
    virtual void wm_command(WORD id, WORD code, HWND target);
    virtual void wm_notify(WPARAM id, NMHDR* hdr);
    virtual void wm_mousemove(WPARAM which, const coordinates& at);
    virtual void wm_lbuttondown(WPARAM which, const coordinates& at);
    virtual void wm_lbuttonup(WPARAM which, const coordinates& at);

    // Called after the previous wm_* functions
    virtual LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    static LRESULT CALLBACK
        wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    friend std::wstring win32::register_once(int);
    friend win32::wnd_class::operator WNDCLASSEXW() const;
};

class defer_window_pos {
    struct move_entry {
        rectangle to;
        ui_element* ptr;
    };

    std::vector<move_entry> _entries;

    public:
    defer_window_pos() = default;
    ~defer_window_pos() {
        HDWP dwp = BeginDeferWindowPos(utils::narrow<unsigned int>(_entries.size()));

        for (const move_entry& entry : _entries) {
            if (entry.ptr->handle() != nullptr) {
                entry.ptr->move_dwp(dwp, entry.to);
            }
        }

        EndDeferWindowPos(dwp);
    }

    defer_window_pos& move(ui_element* element, const rectangle& at) {
        if (element) {
            _entries.push_back({ .to = at, .ptr = element });
        }
        return *this;
    }

    defer_window_pos& move(ui_element& element, const rectangle& at) {
        _entries.push_back({ .to = at, .ptr = &element });
        return *this;
    }

    template <concepts::smart_pointer Ptr> requires std::derived_from<typename Ptr::element_type, ui_element>
    auto move(const Ptr& element, const rectangle& at) {
        return move(element.get(), at);
    }
};
