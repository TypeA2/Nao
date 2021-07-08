#pragma once

#include <libnao/ui/window.h>
#include <libnao/ui/push_button.h>
#include <libnao/ui/line_edit.h>
#include <libnao/ui/directional_layout.h>

class nao_presenter;

class left_window : public nao::ui::window {
    NAO_LOGGER(left_window);

    nao_presenter& _presenter;

    nao::ui::vertical_layout _layout;

    nao::ui::horizontal_layout _controls_layout;
    nao::ui::push_button _up;
    nao::ui::push_button _refresh;
    nao::ui::line_edit _path;
    nao::ui::push_button _browse;

    nao::ui::push_button _filler2;

    public:
    explicit left_window(nao_presenter& presenter, window& parent);
};
