#pragma once

#include <libnao/ui/main_window.h>

#include <libnao/ui/directional_layout.h>
#include <libnao/ui/push_button.h>

class nao_window : public nao::main_window {
    NAO_LOGGER(nao_window);

    nao::horizontal_layout _layout;
    nao::push_button _button1;
    nao::push_button _button2;
    nao::push_button _button3;

    public:
    explicit nao_window();
};
