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

#include "horizontal_layout.h"

#include <libnao/util/ranges.h>

void nao::horizontal_layout::add_element(window& element) {
    logger().debug("Adding child {}", fmt::ptr(&element));

    if (GetParent(element.handle()) != _handle) {
        SetParent(element.handle(), _handle);
    }

    _children.push_back(&element);
    reposition();
}


nao::event_result nao::horizontal_layout::on_resize(resize_event& e) {
    event_result res = layout::on_resize(e);

    reposition();
    return res;
}


void nao::horizontal_layout::reposition() {
    int count = static_cast<int>(_children.size());
    logger().debug("Repositioning {} children to fit in {}", count, client_size());
    logger().debug("Margins: {}", content_margins());

    auto [w, h] = client_size();
    auto [top, right, bot, left] = content_margins();

    // Remove all margins from the width/height
    w = std::max<long>(0, w - right - left);
    h = std::max<long>(0, h - top - bot);

    HDWP dwp = BeginDeferWindowPos(count);

    for (long pos_x = left;
         auto [hwnd, i] : _children | member_transform(&window::handle) | with_index) {
        int this_width = w / count;

        dwp = DeferWindowPos(dwp, hwnd, nullptr, pos_x, top, this_width, h, 0);

        pos_x += this_width;
    }

    if (!EndDeferWindowPos(dwp)) {
        logger().critical("Failed to reposition {} children", count);
    }
}
