#include "win32.h"

#include "utils.h"

namespace win32 {
    prop_variant::prop_variant() {
        PropVariantInit(&_prop);
    }

    prop_variant::~prop_variant() {
        HASSERT(PropVariantClear(&_prop));
    }

    prop_variant::operator PROPVARIANT*() noexcept {
        return &_prop;
    }

}
