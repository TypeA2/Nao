#pragma once

#include "frameworks.h"

#include "icon.h"

#include <memory>
#include <functional>
#include <type_traits>

struct coords {
    long x;
    long y;
};

struct dimensions {
    long x;
    long y;
    long width;
    long height;
};

class ui_element : public std::enable_shared_from_this<ui_element> {
    static HINSTANCE _instance;

    public:
    // Public utility functions
    static std::wstring load_wstring(int resource);
    static icon load_icon(int resource);

    static HGDIOBJ stock_object(int obj);

    template <typename T>
    static std::enable_if_t<std::is_pointer_v<T>, T>
        stock_object(int obj) {
            return static_cast<T>(stock_object(obj));
    }

    static void set_instance(HINSTANCE instance);
    static HINSTANCE instance();

    static HWND create_window(
        const std::wstring& class_name,
        const std::wstring& window_name,
        DWORD style, const dimensions& at,
        ui_element* parent, void* param = nullptr);

    static HWND create_window_ex(
        const std::wstring& class_name,
        const std::wstring& window_name,
        DWORD style, const dimensions& at,
        ui_element* parent, DWORD ex_style, void* param = nullptr);

    explicit ui_element(ui_element* parent);

    ui_element() = delete;

    virtual ~ui_element();

    // Call DestroyWindow on the window handle
    bool destroy();

    ui_element* parent() const;
    HWND handle() const;

    virtual long width() const;
    virtual long height() const;
    virtual coords position() const;
    virtual dimensions dimensions() const;

    // Move using SetWindowPos
    virtual void move(int x, int y, int width, int height);

    // Move using DeferWindowPos, for multiple windows
    virtual HDWP& move_dwp(HDWP& dwp, int x, int y, int width, int height);

    // Set the window style
    virtual void set_style(DWORD style, bool enable = true);

    // Specifically enable or disable
    virtual void set_enabled(bool enabled = true);

    // Manually send or post messages
    virtual LRESULT send_message(UINT msg, WPARAM wparam, LPARAM lparam) const;
    [[maybe_unused]] LRESULT send_message(UINT msg, WPARAM wparam, const void* lparam) const;
    template <typename _LPARAM>
    [[maybe_unused]] std::enable_if_t<
        !std::is_same_v<_LPARAM, LPARAM> && std::is_convertible_v<_LPARAM, LPARAM>, LRESULT>
        send_message(UINT msg, WPARAM wparam, _LPARAM lparam) const {
        return send_message(msg, wparam, static_cast<LPARAM>(lparam));
    }

    virtual bool post_message(UINT msg, WPARAM wparam, LPARAM lparam) const;
    bool post_message(UINT msg, WPARAM wparam, const void* lparam) const;
    template <typename _LPARAM>
    std::enable_if_t<
        !std::is_same_v<_LPARAM, LPARAM> &&  std::is_convertible_v<_LPARAM, LPARAM>, bool>
        post_message(UINT msg, WPARAM wparam, _LPARAM lparam) const {
        return post_message(msg, wparam, static_cast<LPARAM>(lparam));
    }

    protected:
    void set_handle(HWND handle);

    // Overridable message handlers
    virtual bool wm_create(CREATESTRUCTW* create);
    virtual void wm_destroy();
    virtual void wm_size(int type, int width, int height);
    virtual void wm_paint();
    virtual void wm_command(WPARAM wparam, LPARAM lparam);

    /*
     * Overridable WndProc
     */
    using wnd_proc_func = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;

    // Struct passed as lparam to CreateWindow to automate setup
    struct wnd_init {
        ui_element* element;
        std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> proc;
        void* replacement;

        wnd_init(ui_element* element, const wnd_proc_func& proc, void* replacement = nullptr);

        // Shorthand to construct for a member function taking only the
        // default arguments
        template <typename T>
        wnd_init(T* element, LRESULT(T::* proc)(HWND, UINT, WPARAM, LPARAM), void* replacement = nullptr) {
            static_assert(std::is_base_of_v<ui_element, T>);

            using namespace std::placeholders;
            this->element = element;
            this->proc = std::bind(proc, element, _1, _2, _3, _4);
            this->replacement = replacement;
        }
    };

    static LRESULT CALLBACK wnd_proc_fwd(HWND hwnd, UINT msg,
        WPARAM wparam, LPARAM lparam);

    private:
    ui_element* _m_parent;
    HWND _m_handle;

    wnd_proc_func _m_wnd_proc;

    bool _m_created;
};

template <int window_count>
class defer_window_pos {

    HDWP _m_dwp;
    bool _m_committed;

    public:
    defer_window_pos() : _m_dwp(BeginDeferWindowPos(window_count)), _m_committed(false) { }
    ~defer_window_pos() { commit(); }

    void commit() {
        if (!_m_committed) {
            EndDeferWindowPos(_m_dwp);
            _m_committed = true;
        }
    }

    defer_window_pos& move(ui_element* element, const dimensions& at) {
        element->move_dwp(_m_dwp, at.x, at.y, at.width, at.height);
        return *this;
    }

    template <typename Cont>
    defer_window_pos& move(const Cont& element, const dimensions& at,
        std::enable_if_t<
            std::is_pointer_v<
                std::remove_cvref_t<decltype(element.get())>>
            && std::is_base_of_v<ui_element,
                std::remove_pointer_t<decltype(element.get())>>> * = nullptr) {
        return move(element.get(), at);
    }
};
