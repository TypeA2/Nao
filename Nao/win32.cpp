#include "win32.h"

#include "utils.h"

namespace win32 {
    inline namespace raii {
        prop_variant::prop_variant() {
            PropVariantInit(&_prop);
        }

        prop_variant::~prop_variant() {
            HASSERT(PropVariantClear(&_prop));
        }

        prop_variant::operator PROPVARIANT* () noexcept {
            return &_prop;
        }



        icon::icon(HICON handle, bool destroy) : _handle(handle), _destroy(destroy) { }

        icon::icon(icon&& other) noexcept : _handle(other._handle), _destroy(other._destroy) { }

        icon& icon::operator=(icon&& other) noexcept {
            _handle = other._handle;
            _destroy = other._destroy;

            other._handle = nullptr;
            other._destroy = false;

            return *this;
        }

        icon::~icon() {
            if (_destroy) {
                DeleteObject(_handle);
            }
        }

        icon::operator HICON() const noexcept {
            return _handle;
        }

        HICON icon::handle() const noexcept {
            return _handle;
        }



        dynamic_library::dynamic_library(const std::string& name)
            : _handle(LoadLibraryW(utils::utf16(name).c_str())) {
            ASSERT(_handle);
        }

        dynamic_library::~dynamic_library() {
            if (_handle) {
                FreeLibrary(_handle);
            }
        }

        icon dynamic_library::load_icon_scaled(int resource, int width, int height) const {
            HICON _icon;
            HASSERT(LoadIconWithScaleDown(_handle, MAKEINTRESOURCEW(resource), width, height, &_icon));
            return icon(_icon, true);
        }
    }

    namespace comm_ctrl {
        bool init(DWORD flags) {
            INITCOMMONCONTROLSEX picce {
                    .dwSize = sizeof(INITCOMMONCONTROLSEX),
                    .dwICC = flags
            };

            return InitCommonControlsEx(&picce);
        }
    }
}
