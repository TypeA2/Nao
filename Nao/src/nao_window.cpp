#include "nao_window.h"

nao_window::nao_window()
    : main_window{ "Nao" }
    , _layout { *this }
    , _left{ _layout }
    , _right{ _layout, "baz" }{

    set_minimum_size(480, 320);
    
    _layout.set_content_margins(2, 2, 2, 2);
    _layout.set_content_spacing(2);
}
