#pragma once

#include "left_window.h"

#include <libnao/ui/main_window.h>

#include <libnao/ui/directional_layout.h>
#include <libnao/ui/push_button.h>

class nao_window : public nao::main_window {
    NAO_LOGGER(nao_window)

    nao::horizontal_layout _layout;

    left_window _left;
    nao::push_button _right;

    public:
    explicit nao_window();
};
