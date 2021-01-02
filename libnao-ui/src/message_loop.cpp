#include "message_loop.h"

#include <Windows.h>

namespace nao {
    int message_loop::run() {
        (void)this;

        MSG msg{};

        while (GetMessageW(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        return EXIT_SUCCESS;
    }

}