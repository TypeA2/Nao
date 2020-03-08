#include "com.h"

#include "utils.h"

namespace com {
    inline namespace raii {
        com_wrapper::com_wrapper(DWORD flags) {
            HASSERT(CoInitializeEx(nullptr, flags));
        }

        com_wrapper::~com_wrapper() {
            CoUninitialize();
        }

    }
}
