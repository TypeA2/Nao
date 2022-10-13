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
#include "icon.h"

#include <libnao/util/logging.h>

namespace nao::ui {
    class icon_backend {
        public:
        virtual ~icon_backend() = default;

        [[nodiscard]] virtual icon_backend_ptr copy() = 0;

        [[nodiscard]] virtual HICON native_handle() = 0;
    };

    void icon_backend_deleter::operator()(icon_backend* ptr) const {
        delete ptr;
    }

    class null_icon_backend : public nao::ui::icon_backend {
        public:
        null_icon_backend() = default;
        ~null_icon_backend() override = default;

        [[nodiscard]] icon_backend_ptr copy() override {
            return icon_backend_ptr{ new null_icon_backend{} };
        }

        [[nodiscard]] HICON native_handle() override {
            return nullptr;
        }
    };

    class gdi_icon_backend : public nao::ui::icon_backend {
        win32::gdi_object _obj;

        public:
        explicit gdi_icon_backend(win32::gdi_object obj) : _obj{ std::move(obj) } {

        }

        ~gdi_icon_backend() override = default;

        [[nodiscard]] icon_backend_ptr copy() override {
            return icon_backend_ptr{ new gdi_icon_backend{ CopyIcon(_obj.handle<HICON>()) }};
        }

        [[nodiscard]] HICON native_handle() override {
            return _obj.handle<HICON>();
        }
    };
}

nao::ui::icon::icon() : _d{ new null_icon_backend{} } {

}

nao::ui::icon::icon(win32::gdi_object obj)
    : _d{ new gdi_icon_backend{ std::move(obj)} } {

}

nao::ui::icon::icon(const icon& other) : _d{ other._d->copy() } {
}

nao::ui::icon& nao::ui::icon::operator=(const icon& other) {
    _d = other._d->copy();
    return *this;
}

nao::ui::icon::icon(icon&& other) noexcept : _d{ std::move(other._d) } {
    _d = std::move(other._d);
}

nao::ui::icon& nao::ui::icon::operator=(icon&& other) noexcept {
    _d = std::move(other._d);
    return *this;
}

HICON nao::ui::icon::handle() const {
    return _d->native_handle();
}
