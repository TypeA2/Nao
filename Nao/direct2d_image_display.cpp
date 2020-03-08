#include "direct2d_image_display.h"

#include "resource.h"

#include "direct2d.h"

direct2d_image_display::direct2d_image_display(ui_element* parent, char* data, const dimensions& dims)
    : ui_element(parent), _dims { dims } {
    std::wstring class_name = win32::load_wstring(IDS_D2DWINDOW);
    WNDCLASSEXW wcex {
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = wnd_proc_fwd,
        .hInstance = win32::instance(),
        .hCursor = LoadCursorW(nullptr, IDC_ARROW),
        .hbrBackground = nullptr,
        .lpszClassName = class_name.c_str()
    };

    std::wstring classname = win32::register_once(wcex);

    auto [width, height] = this->parent()->dims();

    HWND handle = win32::create_window(classname, L"", WS_CHILD | WS_VISIBLE | WS_OVERLAPPED,
        { 0, 0, width, height }, this->parent(),
        new wnd_init(this, &direct2d_image_display::_wnd_proc, data));

    ASSERT(handle);
}

bool direct2d_image_display::wm_create(CREATESTRUCTW* create) {
    char* data = static_cast<char*>(create->lpCreateParams);

    _create_resources();

    auto props = D2D1::BitmapProperties(
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

    HASSERT(_target->CreateBitmap(
        D2D1::SizeU(utils::narrow<UINT32>(_dims.width), utils::narrow<UINT32>(_dims.height)),
        data, utils::narrow<UINT32>(_dims.width * 4i64), props, &_bitmap));
    
    HASSERT(_device_context->CreateEffect(CLSID_D2D1Scale, &_effect));
    _effect->SetInput(0, _bitmap);
    _effect->SetValue(D2D1_SCALE_PROP_INTERPOLATION_MODE, D2D1_SCALE_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);

    _update_scaling();

    return true;
}

void direct2d_image_display::wm_paint() {
    PAINTSTRUCT ps;
    BeginPaint(handle(), &ps);
    _create_resources();

    _target->BeginDraw();
    _target->Clear(D2D1::ColorF(D2D1::ColorF::White));
    

    if (_bitmap) {
        int64_t width = this->width();

        float scalar = (_dims.width > width) ? _ratio : 1.f;

        auto offset = D2D1::Point2F((width - (_dims.width * scalar)) / 2.f, 0.f);
        _device_context->DrawImage(_effect, &offset, nullptr, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);
    }

    HRESULT hr = _target->EndDraw();

    if (hr == D2DERR_RECREATE_TARGET) {
        _free_resources();
    }

    EndPaint(handle(), &ps);
}

void direct2d_image_display::wm_size(int, int width, int height) {
    if (_target) {
        _target->Resize(D2D1::SizeU(width, height));
    }

    _update_scaling();
}

LRESULT direct2d_image_display::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

void direct2d_image_display::_create_resources() {
    if (!_target) {
        _target = direct2d::factory().create_hwnd_render_target(handle(), dims());
        ASSERT(_target);

        HASSERT(_target->QueryInterface(&_device_context));
    }
}

void direct2d_image_display::_free_resources() const {
    _device_context->Release();
    _target->Release();
}

void direct2d_image_display::_update_scaling() {
    if (_bitmap && _effect) {
        auto size = _bitmap->GetSize();
        auto [width, height] = dims();

        float ratio_width = static_cast<float>(width) / size.width;
        float ratio_height = static_cast<float>(height) / size.height;

        _ratio = (size.width * ratio_height) > width ? ratio_width : ratio_height;
        

        // Don't upscale smaller images
        float ratio = std::clamp(_ratio, 0.f, 1.f);
        
        _effect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(ratio, ratio));
    }
}

