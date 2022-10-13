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
#pragma once

#include <libnao/util/win32.h>

#include <memory>

namespace nao::ui {
    class icon_backend;

    struct icon_backend_deleter {
        void operator()(icon_backend* ptr) const;
    };

    using icon_backend_ptr = std::unique_ptr<icon_backend, icon_backend_deleter>;

    class icon {
        icon_backend_ptr _d;

        public:
        icon();
        explicit(false) icon(win32::gdi_object obj);
        ~icon() = default;

        icon(const icon& other);
        icon& operator=(const icon& other);

        icon(icon&& other) noexcept;
        icon& operator=(icon&& other) noexcept;

        [[nodiscard]] HICON handle() const;
    };
}