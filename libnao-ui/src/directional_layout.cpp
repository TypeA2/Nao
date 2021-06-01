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

#include "directional_layout.h"

#include <libnao/util/ranges.h>

nao::directional_layout::directional_layout(window& parent, layout_direction dir)
    : layout{ parent }
    , _direction{ dir } {
    
}


void nao::directional_layout::add_element(window& element) {
    logger().debug("Adding child {}", fmt::ptr(&element));

    if (GetParent(element.handle()) != _handle) {
        SetParent(element.handle(), _handle);
    }

    _children.push_back(&element);
    reposition();
}


nao::event_result nao::directional_layout::on_resize(resize_event& e) {
    event_result res = layout::on_resize(e);

    reposition();
    return res;
}


void nao::directional_layout::reposition() {
    switch (_direction) {
        case layout_direction::horizontal:
            _reposition_horizontal();
            break;

        case layout_direction::vertical:
            _reposition_vertical();
            break;
    }
}

void nao::directional_layout::_reposition_horizontal() {
    int count = static_cast<int>(_children.size());
    logger().trace("Repositioning {} children horizontall to fit in {}", count, client_size());

    auto [w, h] = client_size();
    auto [top, right, bot, left] = content_margins();
    long spacing = content_spacing();

    // Remove all margins from the width/height
    w = std::max<long>(0, w - left - right - ((count - 1) * spacing));
    h = std::max<long>(0, h - top - bot);

    HDWP dwp = BeginDeferWindowPos(count);

    long remaining_pixels = w;

    for (long pos_x = left; auto [win, i] : _children | with_index) {
        HWND hwnd = win->handle();

        size proposed{
            .w = remaining_pixels / (count - static_cast<long>(i)),
            .h = h,
        };

        size min = win->minimum_size();
        size max = win->maximum_size();

        // Deal with minimums and maximums, this can be done better right?
        proposed.w = std::max<long>(std::min<long>(proposed.w, max.w), min.w);
        proposed.h = std::max<long>(std::min<long>(proposed.h, max.h), min.h);

        dwp = DeferWindowPos(dwp, hwnd, nullptr, pos_x, top, proposed.w, proposed.h, 0);

        pos_x += proposed.w + spacing;
        remaining_pixels -= proposed.w;
    }

    if (!EndDeferWindowPos(dwp)) {
        logger().critical("Failed to reposition {} children", count);
    }
}

void nao::directional_layout::_reposition_vertical() {
    int count = static_cast<int>(_children.size());
    logger().trace("Repositioning {} children vertically to fit in {}", count, client_size());
    
    auto [w, h] = client_size();
    auto [top, right, bot, left] = content_margins();
    long spacing = content_spacing();

    // Calculate space remaining for children
    w = std::max<long>(0, w - left - right);
    h = std::max<long>(0, h - top - bot - ((count - 1) * spacing));

    HDWP dwp = BeginDeferWindowPos(count);

    long remaining_pixels = h;

    for (long pos_y = top; auto [win, i] : _children | with_index) {
        HWND hwnd = win->handle();

        size proposed{
            .w = w,
            .h = remaining_pixels / (count - static_cast<long>(i)),
        };

        size min = win->minimum_size();
        size max = win->maximum_size();

        proposed.w = std::max<long>(std::min<long>(proposed.w, max.w), min.w);
        proposed.h = std::max<long>(std::min<long>(proposed.h, max.h), min.h);

        dwp = DeferWindowPos(dwp, hwnd, nullptr, left, pos_y, proposed.w, proposed.h, 0);

        pos_y += proposed.h + spacing;
        remaining_pixels -= proposed.h;
    }

    if (!EndDeferWindowPos(dwp)) {
        logger().critical("Failed to reposition {} children", count);
    }
}


nao::horizontal_layout::horizontal_layout(window& parent)
    : directional_layout{ parent, layout_direction::horizontal } {

}

nao::vertical_layout::vertical_layout(window& parent)
    : directional_layout{ parent, layout_direction::vertical } {
    
}
