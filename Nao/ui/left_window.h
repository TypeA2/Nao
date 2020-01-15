#pragma once

#include <string>

#include "ui_element.h"

class list_view;
class line_edit;
class push_button;

class data_model;

class left_window : public ui_element {
    public:
    left_window(ui_element* parent, data_model& model);
    ~left_window();

    protected:
    bool wm_create(CREATESTRUCTW* create) override;
    void wm_size(int type, int width, int height) override;
    
    private:
    void _init();

    // Open a new folder
    void _open_folder();

    // Sort the left list view
    void _sort(NMLISTVIEW* view) const;

    // An item has been double-clicked
    void _dblclick(NMITEMACTIVATE* item) const;

    // Right-clicked
    void _rclick(NMITEMACTIVATE* item) const;

    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    list_view* _m_list;
    push_button* _m_up;
    push_button* _m_refresh;
    push_button* _m_browse;
    line_edit* _m_path;

    data_model& _m_model;

    std::wstring _m_current_path;
};
