#pragma once

#include "frameworks.h"

#include "utils.h"

#include <functional>

struct auto_wrapper {
    std::function<void()> before;
    std::function<void()> after;

    explicit auto_wrapper(const std::function<void()>& before, const std::function<void()>& after = {})
         : before(before), after(after) {
        if (before) {
            before();
        }
    }
    
    // Bind a function and run it in with the specified wrapper active
    template <typename Wrapper, typename Func, typename... Args>
    static auto bind(Func&& func, Args&&... args) {
        return [func, ... args = std::forward<Args>(args)] {
            Wrapper _wrapper;
            (void) _wrapper;
            
            return func(std::forward<Args>(args)...);
        };
    }

    template <template <auto...> typename Wrapper, typename Func, typename... Args>
    static auto bind(Func&& func, Args&&... args) {
        return [func, ... args = std::forward<Args>(args)] {
            Wrapper _wrapper;
            (void) _wrapper;
            
            return func(std::forward<Args>(args)...);
        };
    }

    template <typename Wrapper, typename... Args>
    static auto bind_obj(Args&&... args) {
        return bind<Wrapper>(std::bind(std::forward<Args>(args)...));
    }

    template <template <auto...> typename Wrapper, typename... Args>
    static auto bind_obj(Args&&... args) {
        return bind<Wrapper>(std::bind(std::forward<Args>(args)...));
    }

    ~auto_wrapper() {
        if (after) {
            after();
        }
    }
};

template <DWORD _Init = COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE>
struct com_wrapper : auto_wrapper {

    static constexpr DWORD flags = _Init;

    explicit com_wrapper() : auto_wrapper(
        [] { HASSERT(CoInitializeEx(nullptr, _Init)); },
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