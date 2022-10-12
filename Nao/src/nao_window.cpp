#include "nao_window.h"

nao_window::nao_window(nao_presenter& presenter)
    : main_window{ "Nao" }
    , _presenter{ presenter }
    , _layout { *this }
    , _left{ _presenter, _layout }
    , _right{ _layout, "baz" }{

    set_minimum_size(480, 320);
    
    _layout.set_content_margins(2, 2, 2, 2);
    _layout.set_content_spacing(2);
}

void nao_window::set_path(std::string_view new_path) {
    _left.set_path(new_path);
}

void nao_window::disable_up() {
    _left.set_up_enabled(false);
}

void nao_window::enable_up() {
    _left.set_up_enabled(true);
}
