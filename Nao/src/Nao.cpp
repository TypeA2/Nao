#include <libnao/libnao_ui.h>

#include <libnao/message_loop.h>
#include <libnao/main_window.h>
#include <libnao/push_button.h>
#include <libnao/horizontal_layout.h>

int main(int, char**) {
    if (!nao::libnao_ui::init()) {
        return EXIT_FAILURE;
    }

    nao::main_window w{ "Nao" };
    nao::horizontal_layout layout{ w };
    nao::push_button button{ "wooo", layout };
    nao::push_button button2{ "yeet", layout };
    nao::push_button button3{ "button3", layout };

    nao::message_loop loop;

    return loop.run();
}
