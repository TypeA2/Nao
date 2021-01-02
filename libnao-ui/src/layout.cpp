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
        w.add_layout(*this);
    }


    void layout::_set_parent(main_window& w) const {
        logger().debug("Attaching to {}", fmt::ptr(&w));

        SetParent(_handle, w.handle());

        auto target_size = w.size();
        SetWindowPos(_handle, nullptr,
            5, 5, target_size.w - 10, target_size.h - 10, 0);
    }


}