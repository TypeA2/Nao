#include "nao_window.h"

#include <CommCtrl.h>

#include <libnao/util/shared_library.h>

nao_window::nao_window()
    : main_window{ "Nao" }
    , _layout { *this }
    , _button1{ _layout, "Browse" }
    , _button2{ _layout, "bar" }
    , _button3{ _layout, "baz" } {

    set_minimum_size(480, 320);
    _button1.set_maximum_size(73, 23);
    _layout.set_content_margins(2, 2, 2, 2);
    _layout.set_content_spacing(2);

    nao::shared_library shell32{ "shell32.dll" };
    
    HICON icon;
    HRESULT res = LoadIconWithScaleDown(shell32.handle(), MAKEINTRESOURCEW(4), 16, 16, &icon);
    assert(res == S_OK);

    _button1.set_icon({ icon });
}
