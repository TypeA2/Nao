#include "nao_presenter.h"

#include <libnao/libnao_ui.h>

int main(int, char**) {
    if (!nao::libnao_ui::init()) {
        return EXIT_FAILURE;
    }

    nao_presenter p;
    return p.run();
}
