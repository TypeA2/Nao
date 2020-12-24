#include <Windows.h>
#include <CommCtrl.h>

#include <libnao/libnao-ui.h>

#include <libnao/message_loop.h>
#include <libnao/main_window.h>
#include <libnao/push_button.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {

    if (!nao::libnao_ui::init()) {
        return EXIT_FAILURE;
    }

    nao::main_window w{ "Nao" };

    nao::push_button button{ &w, "wooo" };

    nao::message_loop loop;

    return loop.run();
}
