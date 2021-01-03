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

#include <libnao/ranges.h>

namespace nao {
    void horizontal_layout::add_element(window& element) {
        logger().debug("Adding child {}", fmt::ptr(&element));

        if (GetParent(element.handle()) != _handle) {
            SetParent(element.handle(), _handle);
        }

        SetWindowPos(element.handle(), nullptr, 
            static_cast<int>(_children.size()) * 50, 50, 50, 50, 0);
        _children.push_back(&element);
        _reposition();
    }


    event_result horizontal_layout::on_resize(resize_event& e) {
        _reposition();
        return event_result::ok;
    }


    void horizontal_layout::_reposition() {
        logger().trace("Repositioning {} children", _children.size());

        HDWP dwp = BeginDeferWindowPos(static_cast<int>(_children.size()));


        int i = 0;
        auto transformer = [](window* w) {
            return w->handle();
        };

        for (HWND w : _children | member_transform(&window::handle)) {
            dwp = DeferWindowPos(dwp, w, nullptr,i * 50, 50, 50, 50, 0);
            ++i;
        }

        if (!EndDeferWindowPos(dwp)) {
            logger().critical("Failed to reposition {} children", _children.size());
        }
    }
}
