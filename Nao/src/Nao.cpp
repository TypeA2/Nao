#include <libnao/libnao_ui.h>

#include <libnao/message_loop.h>
#include <libnao/main_window.h>
#include <libnao/push_button.h>
#include <libnao/horizontal_layout.h>

#include "nao_window.h"

int main(int, char**) {
    if (!nao::libnao_ui::init()) {
        return EXIT_FAILURE;
    }

    nao_window w;

    nao::message_loop loop;

    return loop.run();
}
