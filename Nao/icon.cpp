#include "icon.h"

icon::icon() : _m_handle(nullptr), _m_destroy(false) {

}

icon::icon(HICON handle, bool destroy) : _m_handle(handle), _m_destroy(destroy) {
    
}

icon::icon(icon&& other) noexcept : _m_handle(other._m_handle), _m_destroy(other._m_destroy) {
    
}


icon::~icon() {
    if (_m_destroy) {
        DeleteObject(_m_handle);
    }
}

icon::operator HICON() const noexcept {
    return _m_handle;
}

HICON icon::handle() const noexcept {
    return _m_handle;
}
