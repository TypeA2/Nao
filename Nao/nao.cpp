#include "main_window.h"

#include <CommCtrl.h>

#include "utils.h"
#include "data_model.h"
#include "steam_utils.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd) {
    (void) hPrevInstance;
    (void) lpCmdLine;

    INITCOMMONCONTROLSEX picce {
        sizeof(INITCOMMONCONTROLSEX),
        ICC_ANIMATE_CLASS | ICC_BAR_CLASSES | ICC_COOL_CLASSES |
        ICC_DATE_CLASSES | ICC_HOTKEY_CLASS | ICC_LINK_CLASS |
        ICC_LISTVIEW_CLASSES | ICC_NATIVEFNTCTL_CLASS | ICC_PAGESCROLLER_CLASS |
        ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES | ICC_TAB_CLASSES |
        ICC_UPDOWN_CLASS | ICC_USEREX_CLASSES | ICC_WIN95_CLASSES
    };

    InitCommonControlsEx(&picce);

    data_model model(utils::utf8(steam_utils::game_path(L"NieRAutomata", L"data")));
    
    main_window nao(hInstance, nShowCmd, model);

    model.startup();

    return nao.run();
}
