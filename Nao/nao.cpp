#include "nao_model.h"
#include "nao_controller.h"

#include "com.h"
#include "sdl2.h"

#include <CommCtrl.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd) {
    (void) hInstance;
    (void) hPrevInstance;
    (void) lpCmdLine;
    (void) nShowCmd;

    sdl::lock sdl_lock;

    com::com_wrapper com;
    ASSERT(win32::comm_ctrl::init());

    nao_controller controller;

    return controller.pump();
}
