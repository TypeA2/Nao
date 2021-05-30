#include "nao_window.h"

nao_window::nao_window()
    : main_window{ "Nao" }
    , _layout { *this }
    , _button1{ "foo", _layout }
    , _button2{ "bar", _layout } {

    set_minimum_size(480, 320);

}
