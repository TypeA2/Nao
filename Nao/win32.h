#pragma once

#include <Windows.h>
#include <CommCtrl.h>
#include <PropIdl.h>

#include <string>

#include "concepts.h"

class ui_element;
struct rectangle;

namespace win32 {
    // RAII-ing Win32 stuff
    inline namespace raii {
        // Generic object that can be DeleteObject'ed
        template <typename T>
        class object {
            bool _release;

            protected:
            T obj = nullptr;

            public:
            object() = default;
            explicit object(T handle, bool release = true) : _release { release }, obj { handle } { }
            virtual ~object() {
                if (obj && _release) {
                    DeleteObject(obj);
                }
            }

            T handle() const noexcept {
                return obj;
            }

            operator T() const noexcept {
                return obj;
            }

            template <concepts::pointer_or_integral P>
            operator P() const noexcept {
                if constexpr (concepts::pointer<P>) {
                    return static_cast<P>(obj);
                } else {
                    return reinterpret_cast<P>(obj);
                }
            }
        };

        class prop_variant {
            PROPVARIANT _prop;

            public:
            prop_variant();
            ~prop_variant();

            operator PROPVARIANT* () noexcept;
        };

        class icon : public object<HICON> {
            bool _destroy = false;

            public:
            icon() = default;
            icon(HICON handle, bool destroy = true);

            icon(const icon&) = delete;
            icon& operator=(const icon&) = delete;

            icon(icon&& other) noexcept;
            icon& operator=(icon&& other) noexcept;
        };

        class dynamic_library {
            HMODULE _handle = nullptr;

            public:
            explicit dynamic_library(const std::string& name);
            ~dynamic_library();

            icon load_icon_scaled(int resource, int width, int height) const;
        };

        class device_context : public object<HDC> {
            HWND _hwnd;
            bool _release;

            public:
            // RAII release, re-select at end of life
            class temporary_release {
                const device_context* _ctx;
                HGDIOBJ _obj;

                public:
                temporary_release(const device_context* ctx, HGDIOBJ obj);
                ~temporary_release();
            };

            explicit device_context(HWND hwnd, HDC hdc, bool release = true);
            ~device_context();

            template <typename T>
            [[maybe_unused]] HGDIOBJ select(const object<T>& obj) const {
                return SelectObject(this->obj, obj);
            }

            [[maybe_unused]] HGDIOBJ select(HGDIOBJ obj) const;

            template <typename T>
            [[nodiscard]] temporary_release temporary_select(const object<T>& obj) const {
                return temporary_release { this, select(obj.handle()) };
            }

            [[nodiscard]] temporary_release temporary_select(HGDIOBJ obj) const;

            void rectangle(const rectangle& rect) const;
        };

        class paint_struct : public device_context {
            ui_element* _element;

            PAINTSTRUCT _ps { };

            public:
            explicit paint_struct(ui_element* element);
            ~paint_struct();
        };

        using pen = object<HPEN>;
        using brush = object<HBRUSH>;
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

    inline namespace ui {

        // Window creation parameters
        struct wnd_class {
            // Default params, from a resource ID
            static wnd_class create(int resource_id);

            std::wstring class_name;

            DWORD style = 0;
            HCURSOR cursor = LoadCursorW(nullptr, IDC_ARROW);
            HBRUSH background = nullptr;

            operator WNDCLASSEXW() const;
            
        };

        HWND create_window(const std::wstring& class_name, const std::wstring& window_name,
            DWORD style, const rectangle& at, ui_element* parent, void* param = nullptr);

        HWND create_window_ex(const std::wstring& class_name, const std::wstring& window_name,
            DWORD style, const rectangle& at, ui_element* parent, DWORD ex_style, void* param = nullptr);

        std::wstring register_once(int class_id);
        std::wstring register_once(const WNDCLASSEXW& wcx);
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
