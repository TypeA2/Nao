#include "direct2d_image_display.h"

#include "resource.h"

#include "direct2d.h"

direct2d_image_display::direct2d_image_display(ui_element* parent, const char* data, const dimensions& dims)
    : ui_element(parent, IDS_D2DWINDOW, parent->dims().rect(), win32::style | WS_OVERLAPPED)
    , _dims { dims } {

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
}

void direct2d_image_display::wm_paint() {
    win32::paint_struct ps { this };
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
}

void direct2d_image_display::wm_size(int, const dimensions& dims) {
    if (_target) {
        _target->Resize(D2D1::SizeU(utils::narrow<UINT32>(dims.width), utils::narrow<UINT32>(dims.height)));
    }

    _update_scaling();
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

