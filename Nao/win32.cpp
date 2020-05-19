#include "win32.h"

#include <unordered_set>

#include "utils.h"

#include "ui_element.h"

#include <strings.h>

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



        icon::icon(HICON handle, bool destroy) : object { handle }, _destroy(destroy) { }

        icon::icon(icon&& other) noexcept : object { other.obj }, _destroy(other._destroy) { }

        icon& icon::operator=(icon&& other) noexcept {
            obj = other.obj;
            _destroy = other._destroy;

            other.obj = nullptr;
            other._destroy = false;

            return *this;
        }



        dynamic_library::dynamic_library(const std::string& name)
            : _handle(LoadLibraryW(strings::to_utf16(name).c_str())) {
            ASSERT(_handle);
        }

        dynamic_library::~dynamic_library() {
            if (_handle) {
                FreeLibrary(_handle);
            }
        }

        icon dynamic_library::load_icon_scaled(int resource, const dimensions& dims) const {
            HICON _icon;
            HASSERT(LoadIconWithScaleDown(_handle, MAKEINTRESOURCEW(resource),
                utils::narrow<int>(dims.width), utils::narrow<int>(dims.height), &_icon));
            return icon(_icon, true);
        }



        device_context::temporary_release::temporary_release(const device_context* ctx, HGDIOBJ obj) : _ctx { ctx }, _obj { obj } { }

        device_context::temporary_release::~temporary_release() {
            _ctx->select(_obj);
        }


        device_context::device_context(HWND hwnd, HDC hdc, bool release)
            : object { hdc, false }, _hwnd { hwnd }, _release { release } { }

        device_context::~device_context() {
            if (_release) {
                ReleaseDC(_hwnd, obj);
            }
        }

        HGDIOBJ device_context::select(HGDIOBJ obj) const {
            return SelectObject(this->obj, obj);
        }

        device_context::temporary_release device_context::temporary_select(HGDIOBJ obj) const {
            return temporary_release { this, select(obj) };
        }

        void device_context::rectangle(const ::rectangle& rect) const {
            Rectangle(obj,
                utils::narrow<int>(rect.x),
                utils::narrow<int>(rect.y),
                utils::narrow<int>(rect.width),
                utils::narrow<int>(rect.height));
        }



        paint_struct::paint_struct(ui_element* element)
            : device_context { element->handle(), BeginPaint(element->handle(), &_ps), false }, _element { element } { }

        paint_struct::~paint_struct() {
            EndPaint(_element->handle(), &_ps);
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
        wnd_class wnd_class::create(int resource_id) {
            return {
                .class_name = load_wstring(resource_id)
            };
        }

        wnd_class::operator WNDCLASSEXW() const {
            return {
                .cbSize = sizeof(WNDCLASSEXW),
                .style = style,
                .lpfnWndProc = &ui_element::wnd_proc_fwd,
                .hInstance = instance(),
                .hIcon = icon.handle(),
                .hCursor = cursor.handle(),
                .hbrBackground = background.handle(),
                .lpszMenuName = menu_name.empty() ? MAKEINTRESOURCEW(menu_resource) : menu_name.c_str(),
                .lpszClassName = class_name.c_str(),
                .hIconSm = icon.handle()
            };
        }

        HWND create_window(const std::wstring& class_name, const std::wstring& window_name, DWORD style, const rectangle& at, ui_element* parent, void* param) {
            return create_window_ex(class_name, window_name, style, at, parent, 0, param);
        }

        HWND create_window_ex(const std::wstring& class_name, const std::wstring& window_name, DWORD style, const rectangle& at, ui_element* parent, DWORD ex_style, void* param) {
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

        static std::unordered_set<std::wstring>& class_registry() {
            static std::unordered_set<std::wstring> reg;
            return reg;
        }

        bool registered(const std::wstring& classname) {
            if (class_registry().contains(classname)) {
                return true;
            }

            // Maybe registered with Win32
            WNDCLASSW wc;
            return GetClassInfoW(instance(), classname.c_str(), &wc) != 0;
        }

        std::wstring register_once(int class_id) {
            return register_once(wnd_class::create(class_id));
            std::wstring class_name = load_wstring(class_id);

            return register_once({
                .cbSize = sizeof(WNDCLASSEXW),
                .lpfnWndProc = &ui_element::wnd_proc_fwd,
                .hInstance = instance(),
                .hCursor = LoadCursorW(nullptr, IDC_ARROW),
                .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
                .lpszClassName = class_name.c_str()
                });
        }

        std::wstring register_once(const WNDCLASSEXW& wcx) {
            if (!class_registry().contains(wcx.lpszClassName)) {
                ASSERT(RegisterClassExW(&wcx) != 0);

                class_registry().insert(wcx.lpszClassName);
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
