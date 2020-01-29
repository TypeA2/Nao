#include "nao_model.h"
#include "nao_controller.h"

#include "auto_wrapper.h"

#include <CommCtrl.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd) {
    (void) hInstance;
    (void) hPrevInstance;
    (void) lpCmdLine;
    (void) nShowCmd;

    com_wrapper com;
    comm_ctrl_wrapper comm_ctrl;

    nao_controller controller;

    return controller.pump();
}
