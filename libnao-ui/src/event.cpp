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

#include "event.h"

#include <type_traits>

namespace nao {
    LRESULT event::native_event::call_default() const {
        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }


    event::event(const native_event& native) : _native{ native } {}


    event::event(event&& other) noexcept {
        *this = std::forward<event>(other);
    }


    event& event::operator=(event&& other) noexcept {
        _native = other._native;

        return *this;
    }


    event::operator native_event() const {
        return _native;
    }


    const event::native_event& event::native() const {
        return _native;
    }


    dimensions resize_event::new_dimensions() const {
        return {
            .w = LOWORD(_native.lparam),
            .h = HIWORD(_native.lparam)
        };
    }
}
