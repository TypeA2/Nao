#pragma once

// Defines main to libnao_main
// ReSharper disable once CppUnusedIncludeDirective
#include <libnao/libnao_util.h>

// Required for common controls
#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace nao::libnao_ui {
    [[nodiscard]] bool init();
}
