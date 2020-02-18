#pragma once

#include "ui_element.h"

#include <memory>

class left_window;
class right_window;
class nao_view;

class main_window : public ui_element {
    public:
    explicit main_window(nao_view* view);
    ~main_window() override = default;

    left_window* left() const;
    right_window* right() const;

    protected:
    bool wm_create(CREATESTRUCTW* create) override;
    void wm_destroy() override;
    void wm_size(int type, int width, int height) override;
    void wm_command(WPARAM wparam, LPARAM lparam) override;
    
    private:
    // Forwarded class WndProc function
    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    // Windows
    std::unique_ptr<left_window> _m_left;
    std::unique_ptr<right_window> _m_right;
};
