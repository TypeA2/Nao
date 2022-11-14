#include "left_window.h"

#include "nao_presenter.h"

#include <CommCtrl.h>

#include <libnao/util/shared_library.h>

left_window::left_window(nao_presenter& presenter, window& parent)
    : window { parent }
    , _presenter { presenter }
    , _layout { *this }
    , _controls_layout { _layout }
    , _up { _controls_layout }
    , _refresh { _controls_layout }
    , _path { _controls_layout }
    , _browse { _controls_layout, "Browse" }
    , _view { _layout } {

    _controls_layout.set_maximum_size(nao::ui::layout::fill, 23);

    _up.set_maximum_size(23, nao::ui::layout::fill);
    _refresh.set_maximum_size(23, nao::ui::layout::fill);
    _path.set_padding(1, 1, 1, 1);
    _browse.set_maximum_size(73, nao::ui::layout::fill);

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
    _browse.on_click.add(&nao_presenter::browse, _presenter);

    _up.on_click.add(&nao_presenter::up, _presenter);
    _refresh.on_click.add(&nao_presenter::refresh, _presenter);
    _refresh.set_enabled(false);

    _path.on_enter.add(static_cast<void(nao_presenter::*)(std::string_view)>(&nao_presenter::path_changed), _presenter);
    _path.on_enter.add([this](std::string_view) {
        _path.unfocus();
    });

    auto hdr = std::make_unique<nao::ui::list_view::item>();
    hdr->set_columns(3);
    hdr->set_text(0, "foo");
    hdr->set_text(1, "bar");
    hdr->set_text(2, "baz");

    _view.set_header(std::move(hdr));
}

void left_window::set_path(std::string_view new_path) {
    _path.set_text(new_path);
}

void left_window::set_up_enabled(bool state) {
    _up.set_enabled(state);
}
