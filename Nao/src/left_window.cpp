#include "left_window.h"

#include <CommCtrl.h>

#include <libnao/util/shared_library.h>


left_window::left_window(window& parent)
    : window{ parent }
    , _layout{ *this }
    , _controls_layout{ _layout }
    , _browse{ _controls_layout, "Browse" }
    , _filler{ _controls_layout, "foo" }
    , _filler2{ _layout, "bar" } {

    _controls_layout.set_maximum_size(-1, 23);
    _browse.set_maximum_size(73, -1);

    nao::shared_library shell32{ "shell32.dll" };

    HICON icon;
    HRESULT res = LoadIconWithScaleDown(shell32.handle(), MAKEINTRESOURCEW(4), 16, 16, &icon);
    assert(res == S_OK);

    _browse.set_icon({ icon });
}
