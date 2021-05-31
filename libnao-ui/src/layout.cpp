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

#include "layout.h"

#include "main_window.h"

nao::layout::layout(main_window& w) : window{
        {
            .cls = "nao_layout",
            .style = WS_VISIBLE | WS_CHILD,
            .pos = { 0, 0 },
            .parent = &w
        } } {
    w.set_layout(*this);
}


void nao::layout::set_content_margins(const margins& margins) {
    assert(margins.top >= 0 && margins.right >= 0 && margins.bot >= 0 && margins.left >= 0);
    _content_margins = margins;
    reposition();
}


void nao::layout::set_content_margins(long top, long right, long bot, long left) {
    set_content_margins({ top, right, bot, left });
}


nao::margins nao::layout::content_margins() const {
    return _content_margins;
}


void nao::layout::set_content_spacing(long spacing) {
    assert(spacing >= 0);
    _content_spacing = spacing;
    reposition();
}


long nao::layout::content_spacing() const {
    return _content_spacing;
}


nao::event_result nao::layout::on_resize(resize_event& e) {
    auto [x, y] = client_pos();
    auto [w, h] = e.new_size();

    return (SetWindowPos(_handle, nullptr, x, y, w, h, 0) != 0)
        ? event_result::ok : event_result::err;
}


void nao::layout::_set_parent(main_window& win) const {
    logger().debug("Attaching to {}", fmt::ptr(&win));

    SetParent(_handle, win.handle());

    auto [w, h] = win.client_size();
    SetWindowPos(_handle, nullptr, 0, 0, w, h, 0);
}
