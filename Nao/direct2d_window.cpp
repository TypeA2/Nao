#include "direct2d_window.h"

#include "resource.h"

#include "direct2d.h"

direct2d_window::direct2d_window(ui_element* parent) : ui_element(parent) {
    std::wstring class_name = load_wstring(IDS_D2DWINDOW);
    WNDCLASSEXW wcex {
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = wnd_proc_fwd,
        .hInstance = instance(),
        .hCursor = LoadCursorW(nullptr, IDC_ARROW),
        .hbrBackground = nullptr,
        .lpszClassName = class_name.c_str()
    };

    std::wstring classname = register_once(wcex);

    auto [width, height] = this->parent()->dims();

    HWND handle = create_window(classname, L"", WS_CHILD | WS_VISIBLE | WS_OVERLAPPED,
        { 0, 0, width, height }, this->parent(), new wnd_init(this, &direct2d_window::_wnd_proc));

    ASSERT(handle);
}

void direct2d_window::set_bitmap(char* data, const dimensions& dims) {
    D2D1_BITMAP_PROPERTIES props {
        .pixelFormat = {
            .format = DXGI_FORMAT_B8G8R8A8_UNORM,
            .alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED
        },
        .dpiX = 96.f,
        .dpiY = 96.f
    };

    if (_bitmap) { _bitmap.Release(); }

    HRESULT hr =_target->CreateBitmap(
        D2D1::SizeU(utils::narrow<UINT32>(dims.width), utils::narrow<UINT32>(dims.height)),
        data, utils::narrow<UINT32>(dims.width * 4i64), props, &_bitmap);
    
    HASSERT(hr);

    com_ptr<ID2D1DeviceContext> device_context;
    HASSERT(_target->QueryInterface(&device_context));

    HASSERT(device_context->CreateEffect(CLSID_D2D1Scale, &_effect));
    _effect->SetInput(0, _bitmap);
    _effect->SetValue(D2D1_SCALE_PROP_INTERPOLATION_MODE, D2D1_SCALE_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);

    auto size = _bitmap->GetSize();
    float ratio = static_cast<float>(width()) / size.width;
    _effect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(ratio, ratio));
}

bool direct2d_window::wm_create(CREATESTRUCTW*) {
    
    _create_resources();

    return true;
}

void direct2d_window::wm_paint() {
    _create_resources();

    _target->BeginDraw();
    _target->SetTransform(D2D1::Matrix3x2F::Identity());
    _target->Clear(D2D1::ColorF(D2D1::ColorF::White));

    if (_bitmap) {
        _device_context->DrawImage(_effect, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);
    }


    /*_target->DrawBitmap(_bitmap,
        { utils::narrow<float>(x), utils::narrow<float>(y),
            utils::narrow<float>(width), size.height * ratio },); */
    
    HRESULT hr = _target->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        _free_resources();
    }
}

void direct2d_window::wm_size(int, int width, int height) {
    if (_target) {
        _target->Resize(D2D1::SizeU(width, height));

        if (_bitmap) {
            auto size = _bitmap->GetSize();
            float ratio = static_cast<float>(width) / size.width;
            _effect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(ratio, ratio));
        }
    }
}

LRESULT direct2d_window::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

void direct2d_window::_create_resources() {
    if (!_target) {
        _target = direct2d::factory().create_hwnd_render_target(this);
        ASSERT(_target);
        HASSERT(_target->QueryInterface(&_device_context));
    }
}

void direct2d_window::_free_resources() const {
    _device_context->Release();
    _target->Release();
}
