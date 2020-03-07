#pragma once

#include <PropIdl.h>

namespace win32 {
    class prop_variant {
        PROPVARIANT _prop;

        public:
        prop_variant();
        ~prop_variant();

        operator PROPVARIANT* () noexcept;
    };
}
