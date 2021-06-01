/**
 *  This file is part of libnao-ui.
 *
 *  libnao-ui is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libnao-ui is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libnao-ui.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "push_button.h"

#include <CommCtrl.h>

#include "layout.h"

nao::push_button::push_button(window& parent, std::string_view text) : window{
        {
            .builtin = true,
            .cls = WC_BUTTONA,
            .style = WS_VISIBLE | WS_CHILD | WS_TABSTOP,
            .name = text,
            .pos = { 0, 0 },
            .size = { 50, 50 },
            .parent = &parent,
        } } {

    // Unset font is weird, fix it
    NONCLIENTMETRICS metrics{
        .cbSize = sizeof(metrics),
    };
    BOOL res = SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);
    assert(res);

    _font = CreateFontIndirectW(&metrics.lfCaptionFont);
    SendMessageW(_handle, WM_SETFONT, reinterpret_cast<WPARAM>(_font.handle()), 0);

    parent.set_window(*this);
}

void nao::push_button::set_icon(icon icon) {
    _icon = std::move(icon);

    SendMessageW(_handle, BM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(_icon.handle()));
}
