#include "direct2d.h"

#include "utils.h"

#include "ui_element.h"

static const com_ptr<ID2D1Factory>& static_factory() {
    static com_ptr<ID2D1Factory> factory;
    if (!factory) {
        HASSERT(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory));
    }

    return factory;
}

namespace direct2d {
    factory::factory() : factory { static_factory() } { }
    factory::factory(const com_ptr<ID2D1Factory>& ptr) : _factory { ptr } { }

    com_ptr<ID2D1HwndRenderTarget> factory::create_hwnd_render_target(ui_element* element) const {
        return create_hwnd_render_target(element->handle(), element->dims());
    }

    com_ptr<ID2D1HwndRenderTarget> factory::create_hwnd_render_target(HWND handle, dimensions dims) const {
        com_ptr<ID2D1HwndRenderTarget> target;

        HRESULT hr = _factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(handle,
                D2D1::SizeU(utils::narrow<UINT>(dims.width), utils::narrow<UINT>(dims.height))),
            &target);

        return SUCCEEDED(hr) ? target : nullptr;
    }


}
