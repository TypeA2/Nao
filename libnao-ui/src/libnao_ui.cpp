/**
 *  This file is part of libnao-ui.
 *
 *  libnao-ui is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libnao-ui is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libnao-ui.  If not, see <https://www.gnu.org/licenses/>.
 */

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
