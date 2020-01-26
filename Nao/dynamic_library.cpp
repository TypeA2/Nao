#include "dynamic_library.h"

#include "utils.h"

dynamic_library::dynamic_library() : _m_handle(nullptr) {
    
}

dynamic_library::dynamic_library(const std::string& name)
    : _m_handle(LoadLibraryW(utils::utf16(name).c_str())) {
    ASSERT(_m_handle);
}

dynamic_library::~dynamic_library() {
    if (_m_handle) {
        FreeLibrary(_m_handle);
    }
}

icon dynamic_library::load_icon_scaled(int resource, int width, int height) const {
    HICON _icon;
    HASSERT(LoadIconWithScaleDown(_m_handle, MAKEINTRESOURCEW(resource), width, height, &_icon));
    return icon(_icon, true);
}

