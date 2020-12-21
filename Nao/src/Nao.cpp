#include <window.h>
#include <message_loop.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
    nao::window w;

    nao::message_loop loop;

    return loop.run();
}
