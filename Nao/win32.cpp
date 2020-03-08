#include "win32.h"

#include <unordered_set>

#include "utils.h"

#include "ui_element.h"

namespace win32 {
    inline namespace raii {
        prop_variant::prop_variant() {
            PropVariantInit(&_prop);
        }

        prop_variant::~prop_variant() {
            HASSERT(PropVariantClear(&_prop));
        }

        prop_variant::operator PROPVARIANT* () noexcept {
            return &_prop;
        }



        icon::icon(HICON handle, bool destroy) : _handle(handle), _destroy(destroy) { }

        icon::icon(icon&& other) noexcept : _handle(other._handle), _destroy(other._destroy) { }

        icon& icon::operator=(icon&& other) noexcept {
            _handle = other._handle;
            _destroy = other._destroy;

            other._handle = nullptr;
            other._destroy = false;

            return *this;
        }

        icon::~icon() {
            if (_destroy) {
                DeleteObject(_handle);
            }
        }

        icon::operator HICON() const noexcept {
            return _handle;
        }

        HICON icon::handle() const noexcept {
            return _handle;
        }



        dynamic_library::dynamic_library(const std::string& name)
            : _handle(LoadLibraryW(utils::utf16(name).c_str())) {
            ASSERT(_handle);
        }

        dynamic_library::~dynamic_library() {
            if (_handle) {
                FreeLibrary(_handle);
            }
        }

        icon dynamic_library::load_icon_scaled(int resource, int width, int height) const {
            HICON _icon;
            HASSERT(LoadIconWithScaleDown(_handle, MAKEINTRESOURCEW(resource), width, height, &_icon));
            return icon(_icon, true);
        }
    }

    inline namespace resource {
        HINSTANCE instance() {
            static HINSTANCE inst;
            if (!inst) {
                inst = GetModuleHandleW(nullptr);
            }

            return inst;
        }

        std::wstring load_wstring(int resource) {
            union {
                LPCWSTR str;
                WCHAR buf[sizeof(str) / sizeof(WCHAR)];
            } pun { };

            int length = LoadStringW(GetModuleHandleW(nullptr), resource, pun.buf, 0) + 1;
            std::wstring str(length, L'\0');
            wcsncpy_s(str.data(), length, pun.str, length - 1i64);

            return str;
        }

        icon load_icon(int resource) {
            return icon { LoadIconW(instance(), MAKEINTRESOURCEW(resource)), true };
        }

        HGDIOBJ stock_object(int obj) {
            return GetStockObject(obj);
        }


    }

    inline namespace ui {
        HWND create_window(const std::wstring& class_name, const std::wstring& window_name, DWORD style, const ::rectangle& at, ui_element* parent, void* param) {
            return create_window_ex(class_name, window_name, style, at, parent, 0, param);
        }

        HWND create_window_ex(const std::wstring& class_name, const std::wstring& window_name, DWORD style, const ::rectangle& at, ui_element* parent, DWORD ex_style, void* param) {
            return CreateWindowExW(
                ex_style,
                class_name.c_str(),
                window_name.c_str(),
                style,
                utils::narrow<int>(at.x), utils::narrow<int>(at.y),
                utils::narrow<int>(at.width), utils::narrow<int>(at.height),

                parent ? parent->handle() : nullptr,
                
                nullptr,
                instance(),
                param);
        }

        std::wstring register_once(int class_id) {
            std::wstring class_name = load_wstring(class_id);

            return register_once({
                .cbSize = sizeof(WNDCLASSEXW),
                .style = CS_HREDRAW | CS_VREDRAW,
                .lpfnWndProc = &ui_element::wnd_proc_fwd,
                .hInstance = instance(),
                .hCursor = LoadCursorW(nullptr, IDC_ARROW),
                .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
                .lpszClassName = class_name.c_str()
                });
        }

        std::wstring register_once(const WNDCLASSEXW& wcx) {
            static std::unordered_set<std::wstring> registered_classes;

            if (!registered_classes.contains(wcx.lpszClassName)) {
                ASSERT(RegisterClassExW(&wcx) != 0);

                registered_classes.insert(wcx.lpszClassName);
            }

            return wcx.lpszClassName;
        }
    }

    namespace comm_ctrl {
        bool init(DWORD flags) {
            INITCOMMONCONTROLSEX picce {
                    .dwSize = sizeof(INITCOMMONCONTROLSEX),
                    .dwICC = flags
            };

            return InitCommonControlsEx(&picce);
        }
    }
}
