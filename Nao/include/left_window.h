#pragma once

#include <libnao/ui/window.h>
#include <libnao/ui/push_button.h>
#include <libnao/ui/directional_layout.h>

class left_window : public nao::window {
    nao::vertical_layout _layout;
    nao::horizontal_layout _controls_layout;
    nao::push_button _browse;
    nao::push_button _filler, _filler2;

    public:
    explicit left_window(window& parent);
};
