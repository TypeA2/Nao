#pragma once

#include "left_window.h"
#include "right_window.h"

#include <memory>


class nao_view;

class main_window : public ui_element {
    nao_view& _view;
    left_window _left;
    right_window _right;
    
    public:
    explicit main_window(nao_view& view);
    ~main_window() override = default;

    left_window& left();
    right_window& right();

    protected:
    void wm_destroy() override;
    void wm_size(int, const dimensions& dims) override;
    void wm_command(WORD id, WORD code, HWND target) override;
};
