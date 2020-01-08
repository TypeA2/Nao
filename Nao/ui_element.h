#pragma once

#include "frameworks.h"

#include <cstdint>
#include <type_traits>

class ui_element {
    public:
    explicit ui_element(ui_element* parent);
    ui_element() = delete;

    virtual ~ui_element();

    // Call DestroyWindow on the window handle
    bool destroy();

    virtual ui_element* parent() const;
    virtual HWND handle() const;

    virtual long width() const;
    virtual long height() const;

    // Move using SetWindowPos
    virtual bool move(int x, int y, int width, int height) const;

    // Move using DeferWindowPos, for multiple windows
    virtual HDWP& move_dwp(HDWP& dwp, int x, int y, int width, int height) const;

    protected:
    void set_handle(HWND handle);

    /*
     * Overridable WndProc,
     * By calling _use_wnd_proc(member_func), the specified member will be called,
     * instead of the default WndProc for the window. The original must still be
     * called for unprocessed messages (this is returned)
     */
    template <typename T = ui_element>
    using member_wnd_proc = LRESULT(T::*)(HWND, UINT, WPARAM, LPARAM);

    template <typename T = ui_element>
    void use_wnd_proc(member_wnd_proc<T> new_proc) {
        static_assert(std::is_base_of_v<ui_element, T>, "T must derive from ui_element");
        use_wnd_proc(member_wnd_proc<>(new_proc));
    }

    //template <typename T = ui_element>
    //void use_wnd_proc(T* ctx, member_wnd_proc<T> new_proc) {
    //    static_assert(std::is_base_of_v<ui_element, T>, "T must derive from ui_element");
    //    use_wnd_proc(ctx, member_wnd_proc<>(new_proc));
    //}

    void use_wnd_proc(member_wnd_proc<> new_proc);
    //void use_wnd_proc(ui_element* ctx, member_wnd_proc<> new_proc);
    
    WNDPROC default_wnd_proc() const;

    static LRESULT CALLBACK wnd_proc_fwd(HWND hwnd, UINT msg,
        WPARAM wparam, LPARAM lparam);

    private:
    ui_element* _m_parent;
    HWND _m_handle;

    member_wnd_proc<> _m_mem_wnd_proc;
    WNDPROC _m_default_wnd_proc;
    //ui_element* _m_wnd_proc_ctx;
};

