/**
 *  This file is part of libnao-util.
 *
 *  libnao-util is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libnao-util is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libnao-util.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "win32.h"

nao::win32::gdi_object::gdi_object(HGDIOBJ obj) : _obj { obj } { }

nao::win32::gdi_object::~gdi_object() {
    if (_obj) {
        DeleteObject(_obj);
    }
}

nao::win32::gdi_object::gdi_object(gdi_object&& other) noexcept {
    _obj = std::exchange(other._obj, nullptr);
}

nao::win32::gdi_object& nao::win32::gdi_object::operator=(gdi_object&& other) noexcept {
    _obj = std::exchange(other._obj, nullptr);

    return *this;
}

HGDIOBJ nao::win32::gdi_object::handle() const {
    return _obj;
}

void nao::win32::set_style(HWND hwnd, DWORD style, bool enable) {
    auto old_style = static_cast<DWORD>(GetWindowLongPtrW(hwnd, GWL_STYLE));

    // Don't re-apply style
    if (enable && !(old_style & style)) {
        SetWindowLongPtrW(hwnd, GWL_STYLE, old_style | style);
    } else if (!enable && (old_style & style)) {
        SetWindowLongPtrW(hwnd, GWL_STYLE, old_style & ~style);
    }
}

