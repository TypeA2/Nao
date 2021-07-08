#pragma once

#include "left_window.h"

#include <libnao/ui/main_window.h>

#include <libnao/ui/directional_layout.h>
#include <libnao/ui/push_button.h>

class nao_presenter;

class nao_window : public nao::ui::main_window {
    NAO_LOGGER(nao_window);

    nao_presenter& _presenter;

    nao::ui::horizontal_layout _layout;

    left_window _left;
    nao::ui::push_button _right;

    public:
    explicit nao_window(nao_presenter& presenter);
};
