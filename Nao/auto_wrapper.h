#pragma once

#include "frameworks.h"

#include "utils.h"

#include <functional>

struct auto_wrapper {
    private:
    std::function<void()> _after;

    public:
    explicit auto_wrapper(const std::function<void()>& before, const std::function<void()>& after = {})
         : _after(after) {
        before();
    }

    ~auto_wrapper() {
        if (_after) {
            _after();
        }
    }

};

template <DWORD _Init = COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE>
struct com_wrapper : auto_wrapper {
    explicit com_wrapper()
        : auto_wrapper(
            [] { ASSERT(CoInitializeEx(nullptr, _Init) == S_OK); },
            [] { CoUninitialize(); }) { }
};

template <DWORD _IIC = ICC_ANIMATE_CLASS | ICC_BAR_CLASSES | ICC_COOL_CLASSES |
    ICC_DATE_CLASSES | ICC_HOTKEY_CLASS | ICC_LINK_CLASS |
    ICC_LISTVIEW_CLASSES | ICC_NATIVEFNTCTL_CLASS | ICC_PAGESCROLLER_CLASS |
    ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES | ICC_TAB_CLASSES |
    ICC_UPDOWN_CLASS | ICC_USEREX_CLASSES | ICC_WIN95_CLASSES>
    struct comm_ctrl_wrapper : auto_wrapper {
    explicit comm_ctrl_wrapper()
        : auto_wrapper(
            []{
                INITCOMMONCONTROLSEX picce {
                    .dwSize = sizeof(INITCOMMONCONTROLSEX),
                    .dwICC = _IIC
                };
                ASSERT(InitCommonControlsEx(&picce));
            }) { }
};