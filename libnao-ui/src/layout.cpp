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

namespace nao {
    layout::layout(main_window& w) : window{
        {
            .cls = "nao_layout",
            .style = WS_VISIBLE | WS_CHILD,
            .pos = { 0, 0 },
            .parent = &w
        } } {
        w.set_layout(*this);
    }


    event_result layout::on_resize(resize_event& e) {
        auto [x, y] = client_pos();
        auto [w, h] = e.new_size();

        return (MoveWindow(_handle, x, y, w, h, false) != 0) ? event_result::ok : event_result::err;
    }


    void layout::_set_parent(main_window& win) const {
        logger().debug("Attaching to {}", fmt::ptr(&win));

        SetParent(_handle, win.handle());

        auto [w, h] = win.client_size();
        SetWindowPos(_handle, nullptr, 0, 0, w, h, 0);
    }

     
}