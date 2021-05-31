#include "nao_window.h"

nao_window::nao_window()
    : main_window{ "Nao" }
    , _layout { *this }
    , _button1{ "Browse", _layout }
    , _button2{ "bar", _layout } {

    set_minimum_size(480, 320);
    _button1.set_maximum_size(73, 22);
    _layout.set_content_margins(2, 2, 2, 2);
    _layout.set_content_spacing(2);

    
}
