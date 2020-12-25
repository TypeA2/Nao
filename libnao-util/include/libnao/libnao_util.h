#pragma once

#ifdef main
#undef main
#endif

#define main libnao_main

extern "C" int libnao_main(int argc, char** argv);

namespace nao::libnao_util {
    [[nodiscard]] bool init();
}
