#pragma once

#include "ui_element.h"

#include <d2d1.h>
#include <d2d1_1.h>

class direct2d_image_display : public ui_element {
    dimensions _dims;
    com_ptr<ID2D1HwndRenderTarget> _target;
    com_ptr<ID2D1DeviceContext> _device_context;
    com_ptr<ID2D1Bitmap> _bitmap;
    com_ptr<ID2D1Effect> _effect;

    float _ratio;

    public:
    explicit direct2d_image_display(ui_element* parent, char* data, const dimensions& dims);

    protected:
    bool wm_create(CREATESTRUCTW* create) override;
    void wm_paint() override;
    void wm_size(int type, int width, int height) override;

    private:
    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    void _create_resources();
    void _free_resources() const;

    void _update_scaling();
};
