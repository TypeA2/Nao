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

#include "main_window.h"

#include <libnao/encoding.h>

#include "layout.h"

namespace nao {
    main_window::main_window(std::string_view title) : window{
        {
            .cls = "nao_main_window",
            .style = WS_VISIBLE | WS_OVERLAPPEDWINDOW,
            .name = title,
        } } {
    }


    void main_window::set_title(std::string_view title) const {
        logger().trace("Setting window title to {}", title);

        SetWindowTextW(_handle, utf8_to_wide(title).c_str());
    }


    void main_window::set_layout(layout& l) {
        if (_layout) {
            logger().warn("Overwriting layout {} with {}", fmt::ptr(_layout), fmt::ptr(&l));
        } else {
            logger().debug("Setting layout to {}", fmt::ptr(&l));
        }

        l._set_parent(*this);

        _layout = &l;
    }


    event_result main_window::on_resize(resize_event& e) {
        if (_layout) {
            event_result res = _layout->on_event(e);

            RedrawWindow(_layout->handle(), nullptr, nullptr, RDW_ALLCHILDREN);

            return res;
        }

        return window::on_resize(e);
    }
}
