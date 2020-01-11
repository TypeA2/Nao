#pragma once

#include <string>

#include "ui_element.h"

class data_model;

class list_view;
class line_edit;
class push_button;

class left_window : public ui_element {
    public:
    explicit left_window(ui_element* parent, data_model& model);
    ~left_window();

    protected:
    bool wm_create(CREATESTRUCTW* create) override;
    void wm_size(int type, int width, int height) override;
    
    private:
    void _init();

    // Open a new folder
    void _open_folder();

    // Move to a specific (non-relative) path
    void _move_to(std::wstring path);

    // Update the left window's contents
    void _update_view();
    //void _get_provider();
    //void _fill_view();
    
    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    list_view* _m_list;
    HWND _m_up;
    HWND _m_refresh;
    push_button* _m_browse;
    line_edit* _m_path;

    data_model& _m_model;

    std::wstring _m_current_path;

    // Column sort
    enum sort_order : int8_t {
        NONE,
        NORMAL,
        REVERSE
    };
    sort_order _m_sort_order[4];
    int _m_selected_col;

    // Default order when unselected columns get selected
    static constexpr sort_order default_order[4]
        = { NORMAL, NORMAL, REVERSE, REVERSE };
};
