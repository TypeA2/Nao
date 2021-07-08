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
#include "line_edit.h"

#include <libnao/util/encoding.h>

#include <CommCtrl.h>


nao::ui::line_edit::line_edit(window& parent)
    : window{
        {
            .builtin = true,
            .cls = WC_EDITA,
            .style = WS_VISIBLE | WS_CHILD,
            .ex_style = WS_EX_CLIENTEDGE,
            .parent = &parent,
        }} {

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

nao::ui::line_edit::line_edit(window& parent, std::string_view text)
    : line_edit{ parent } {
    set_text(text);
}

void nao::ui::line_edit::set_text(std::string_view text) {
    _text = text;
    std::wstring wide = utf8_to_wide(text);
    SendMessageW(_handle, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(wide.c_str()));
}

std::string_view nao::ui::line_edit::text() const {
    return _text;
}

nao::ui::event_result nao::ui::line_edit::on_event(event& e) {
    const event::native_event& native = e.native();

    switch (native.msg) {
        case WM_COMMAND: {
            if (HIWORD(native.wparam) == EN_CHANGE
                && native.lparam == reinterpret_cast<LPARAM>(_handle)) {
                // Text changed
                int len = GetWindowTextLengthW(_handle) + 1;
                std::vector<wchar_t> text_buf(len);
                GetWindowTextW(_handle, text_buf.data(), len);

                _text = wide_to_utf8(text_buf.data());
                logger().info("Text changed to: {}", _text);
            }
            break;
        }

        default: break;
    }

    return window::on_event(e);
}
