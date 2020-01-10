#pragma once

#include "frameworks.h"

#include <type_traits>

class ui_element {
    public:
    explicit ui_element(ui_element* parent);
	
    ui_element() = delete;

    virtual ~ui_element();

    // Call DestroyWindow on the window handle
    bool destroy();

    ui_element* parent() const;
    HWND handle() const;

    virtual long width() const;
    virtual long height() const;

    // Move using SetWindowPos
    virtual bool move(int x, int y, int width, int height) const;

    // Move using DeferWindowPos, for multiple windows
    virtual HDWP& move_dwp(HDWP& dwp, int x, int y, int width, int height) const;

    protected:
    void set_handle(HWND handle);


	// Overridable message handlers
    virtual bool wm_create(CREATESTRUCTW* create);
    virtual void wm_destroy();
    virtual void wm_size(int type, int width, int height);
    virtual void wm_paint();

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

	// Struct passed as lparam to CreateWindow to automate setup
    struct wnd_init {
        ui_element* element;
        member_wnd_proc<> proc;

        template <typename T = ui_element>
    	wnd_init(ui_element* element, member_wnd_proc<T> proc)
    		: wnd_init(element, member_wnd_proc<>(proc)) {
            static_assert(std::is_base_of_v<ui_element, T>);
        }

        wnd_init(ui_element* element, member_wnd_proc<> proc);
    };

    void use_wnd_proc(member_wnd_proc<> new_proc);

    static LRESULT CALLBACK wnd_proc_fwd(HWND hwnd, UINT msg,
        WPARAM wparam, LPARAM lparam);

    private:
    ui_element* _m_parent;
    HWND _m_handle;

    member_wnd_proc<> _m_mem_wnd_proc;

    bool _m_created;
};

