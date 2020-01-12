#pragma once

#include "frameworks.h"

#include "ui_element.h"

#include <string>

class item_provider;
class left_window;
class data_model;

class main_window : public ui_element {
    public:
    main_window(HINSTANCE inst, int show_cmd, data_model& model);
    ~main_window();

    // Program loop
    int run() const;

    // Some public types
    using icon_index = std::pair<std::wstring, int>;

    protected:
    bool wm_create(CREATESTRUCTW* create) override;
    void wm_destroy() override;
    void wm_size(int type, int width, int height) override;
    void wm_command(WPARAM wparam, LPARAM lparam) override;
    
    private:

    friend class left_window;

    // Initialise stuff needed for the Windows API
    bool _init(int show_cmd);
    bool _register_class() const;
    bool _init_instance(int show_cmd, const std::wstring& title);

    // Forwarded class WndProc function
    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    // Message processing
    //void _list_sort(int col);
    

    // Set left list sort arrow
    //
    //void _set_left_sort_arrow(int col, sort_arrow type) const;

    // Forwards the message processing to the member function
    static LRESULT CALLBACK _right_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    
    // Static, doesn't require the member variables
    static INT_PTR CALLBACK _about(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    // Whether init succeeded
    bool _m_success;

    // Main instance
    HINSTANCE _m_inst;

    // Windows
    left_window* _m_left;
    HWND _m_right;

    std::wstring _m_window_class;
    std::wstring _m_right_class;

    // Accelerators
    HACCEL _m_accel;

    // Current path
    std::wstring _m_path;
    data_model& _m_model;
    
    // Constants
    static constexpr int gutter_size = 2;
    static constexpr int control_height = 22;
    static constexpr int control_button_width = 26;
    static constexpr int browse_button_width = 73;
    static constexpr int path_x_offset = control_button_width * 2 + gutter_size * 3;

    
};
