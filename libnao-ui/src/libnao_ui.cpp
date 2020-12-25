#include "libnao_ui.h"

#include <Windows.h>
#include <CommCtrl.h>

#include <libnao/logging.h>

namespace nao::libnao_ui {
    bool init() {
        if (!libnao_util::init()) {
            return false;
        }

        INITCOMMONCONTROLSEX picce{
            .dwSize = sizeof(picce),
            .dwICC = ICC_ANIMATE_CLASS | ICC_BAR_CLASSES | ICC_COOL_CLASSES |
                ICC_DATE_CLASSES | ICC_HOTKEY_CLASS | ICC_LINK_CLASS |
                ICC_LISTVIEW_CLASSES | ICC_NATIVEFNTCTL_CLASS | ICC_PAGESCROLLER_CLASS |
                ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES | ICC_TAB_CLASSES |
                ICC_UPDOWN_CLASS | ICC_USEREX_CLASSES | ICC_WIN95_CLASSES,
        };

        if (!InitCommonControlsEx(&picce)) {
            log.critical("Failed to init common controls");
            return false;
        }

        return true;
    }
}
