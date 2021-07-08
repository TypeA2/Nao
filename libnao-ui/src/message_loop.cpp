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

#include "message_loop.h"

#include <Windows.h>

int nao::ui::message_loop::run() {
    (void)this;

    MSG msg{};

    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);

        event e { { msg.hwnd, msg.message, msg.wParam, msg.lParam }};

        // Discard event if 1 event filter returns true
        bool skip = false;
        for (auto& f : _filters) {
            if (f(e)) {
                skip = true;
                break;
            }
        }

        if (!skip) {
            DispatchMessageW(&msg);
        }
    }

    return EXIT_SUCCESS;
}

void nao::ui::message_loop::add_filter(event_filter f) {
    _filters.emplace_back(std::move(f));
}
