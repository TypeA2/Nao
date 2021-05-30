#include "nao_window.h"

#include <libnao/libnao_ui.h>
#include <libnao/ui/message_loop.h>


int main(int, char**) {
    if (!nao::libnao_ui::init()) {
        return EXIT_FAILURE;
    }

    nao_window w;

    nao::message_loop loop;

    return loop.run();
}
