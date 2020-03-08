#pragma once

#include <Windows.h>
#include <CommCtrl.h>
#include <PropIdl.h>

#include <string>

#include "concepts.h"

namespace win32 {
    // RAII-ing Win32 stuff
    inline namespace raii {
        class prop_variant {
            PROPVARIANT _prop;

            public:
            prop_variant();
            ~prop_variant();

            operator PROPVARIANT* () noexcept;
        };

        class icon {
            HICON _handle = nullptr;
            bool _destroy = false;

            public:
            icon() = default;
            icon(HICON handle, bool destroy = true);

            icon(const icon&) = delete;
            icon& operator=(const icon&) = delete;

            icon(icon&& other) noexcept;
            icon& operator=(icon&& other) noexcept;

            ~icon();

            operator HICON() const noexcept;
            HICON handle() const noexcept;

            template <concepts::pointer_or_integral T>
            operator T() const noexcept {
                if constexpr (concepts::pointer<T>) {
                    return static_cast<T>(_handle);
                } else {
                    return reinterpret_cast<T>(_handle);
                }
            }
        };

        class dynamic_library {
            HMODULE _handle = nullptr;

            public:
            explicit dynamic_library(const std::string& name);
            ~dynamic_library();

            icon load_icon_scaled(int resource, int width, int height) const;
        };
    }

    inline namespace resource {
        HINSTANCE instance();



        std::wstring load_wstring(int resource);

        icon load_icon(int resource);



        HGDIOBJ stock_object(int obj);

        template <concepts::pointer T>
        static T stock_object(int obj) {
            return static_cast<T>(stock_object(obj));
        }
    }

    namespace comm_ctrl {
        bool init(DWORD flags =
            ICC_ANIMATE_CLASS | ICC_BAR_CLASSES | ICC_COOL_CLASSES |
            ICC_DATE_CLASSES | ICC_HOTKEY_CLASS | ICC_LINK_CLASS |
            ICC_LISTVIEW_CLASSES | ICC_NATIVEFNTCTL_CLASS | ICC_PAGESCROLLER_CLASS |
            ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES | ICC_TAB_CLASSES |
            ICC_UPDOWN_CLASS | ICC_USEREX_CLASSES | ICC_WIN95_CLASSES);
    }
}
