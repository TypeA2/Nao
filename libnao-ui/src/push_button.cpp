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
#include <libnao/util/encoding.h>


nao::push_button::push_button(window& parent) : window{
    {
        .builtin = true,
        .cls = WC_BUTTONA,
        .style = WS_VISIBLE | WS_CHILD | WS_TABSTOP,
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

nao::push_button::push_button(window& parent, std::string_view text) : push_button{ parent } {
    set_text(text);
}

void nao::push_button::set_icon(icon icon) {
    // Fix styles
    // If there is no text, show icon only, else show both
    win32::set_style(_handle, BS_ICON, _text.empty());
    
    _icon = std::move(icon);

    SendMessageW(_handle, BM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(_icon.handle()));
}

void nao::push_button::set_text(std::string_view text) {
    // Disable icon if only icon is set
    win32::set_style(_handle, BS_ICON, _icon.handle());

    _text = text;
    
    auto wide = utf8_to_wide(_text);
    SendMessageW(_handle, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(wide.c_str()));
}
