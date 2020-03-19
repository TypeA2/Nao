#pragma once

#include "ui_element.h"
#include "com.h"

#include <d2d1.h>
#include <d2d1_1.h>

class direct2d_image_display : public ui_element {
    dimensions _dims;
    com_ptr<ID2D1HwndRenderTarget> _target;
    com_ptr<ID2D1DeviceContext> _device_context;
    com_ptr<ID2D1Bitmap> _bitmap;
    com_ptr<ID2D1Effect> _effect;

    float _ratio {};

    char* _data;

    public:
    explicit direct2d_image_display(ui_element* parent, char* data, const dimensions& dims);

    protected:
    bool wm_create(CREATESTRUCTW*) override;
    void wm_paint() override;
    void wm_size(int type, int width, int height) override;

    private:
    void _create_resources();
    void _free_resources() const;

    void _update_scaling();
};
