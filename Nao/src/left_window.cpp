#include "left_window.h"

#include <CommCtrl.h>

#include <libnao/util/shared_library.h>


left_window::left_window(window& parent)
    : window{ parent }
    , _layout{ *this }
    , _controls_layout{ _layout }
    , _up{ _controls_layout }
    , _refresh{ _controls_layout }
    , _path{ _controls_layout, "foo" }
    , _browse{ _controls_layout, "Browse" }
    , _filler2{ _layout, "bar" } {

    _controls_layout.set_maximum_size(nao::layout::fill, 23);

    _up.set_maximum_size(23, nao::layout::fill);
    _refresh.set_maximum_size(23, nao::layout::fill);
    _path.set_padding(1, 1, 1, 1);
    _browse.set_maximum_size(73, nao::layout::fill);

    nao::shared_library shell32{ "shell32.dll" };

    HICON up_icon;
    HRESULT res = LoadIconWithScaleDown(shell32.handle(),
                                        MAKEINTRESOURCEW(16817), 16, 16, &up_icon);
    assert(res == S_OK);
    _up.set_icon({ up_icon });

    HICON refresh_icon;
    res = LoadIconWithScaleDown(shell32.handle(), MAKEINTRESOURCEW(16739), 16, 16, &refresh_icon);
    assert(res == S_OK);
    _refresh.set_icon({ refresh_icon });

    HICON browse_icon;
    res = LoadIconWithScaleDown(shell32.handle(), MAKEINTRESOURCEW(4), 16, 16, &browse_icon);
    assert(res == S_OK);

    _browse.set_icon({ browse_icon });
}
