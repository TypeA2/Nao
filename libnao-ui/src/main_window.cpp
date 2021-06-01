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

#include <libnao/util/encoding.h>

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
}
