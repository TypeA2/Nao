#pragma once

#include <libnao/main_window.h>

#include <libnao/horizontal_layout.h>
#include <libnao/push_button.h>

class nao_window : public nao::main_window {
    NAO_LOGGER(nao_window);

    nao::horizontal_layout _layout;
    nao::push_button _button1;
    nao::push_button _button2;

    public:
    explicit nao_window();
};
