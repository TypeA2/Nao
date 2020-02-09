#pragma once

#include "frameworks.h"

#include "icon.h"
#include "concepts.h"

#include <memory>
#include <functional>

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

    template <concepts::pointer T>
    static T stock_object(int obj) {
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
    virtual [[maybe_unused]] LRESULT send_message(UINT msg, WPARAM wparam = 0, LPARAM lparam = 0) const;
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

    virtual bool post_message(UINT msg, WPARAM wparam, LPARAM lparam) const;

    template <concepts::pointer_or_integral W, concepts::pointer_or_integral L>
    bool post_message(UINT msg, W wparam, L lparam) const {
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
        template <std::derived_from<ui_element> T>
        wnd_init(T* element, LRESULT(T::* proc)(HWND, UINT, WPARAM, LPARAM), void* replacement = nullptr) {

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

class defer_window_pos {
    struct move_entry {
        dimensions to;
        ui_element* ptr;
    };

    std::vector<move_entry> _m_entries;

    public:
    defer_window_pos() = default;
    ~defer_window_pos() {
        HDWP dwp = BeginDeferWindowPos(static_cast<int>(_m_entries.size()));

        for (const move_entry& entry : _m_entries) {
            entry.ptr->move_dwp(dwp, entry.to.x, entry.to.y, entry.to.width, entry.to.height);
        }

        EndDeferWindowPos(dwp);
    }

    defer_window_pos& move(ui_element* element, const dimensions& at) {
        _m_entries.push_back({ .to = at, .ptr = element });
        return *this;
    }

    template <concepts::smart_pointer Ptr> requires std::derived_from<typename Ptr::element_type, ui_element>
    auto move(const Ptr& element, const dimensions& at) {
        return move(element.get(), at);
    }
};
