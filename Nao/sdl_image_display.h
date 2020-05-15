#pragma once

#include <sdl2.h>

#include "ui_element.h"

struct SDL_Window;

class sdl_image_display : public ui_element {
    SDL_Renderer* _renderer;
    SDL_Texture* _texture;
    dimensions _dims;
    public:
    explicit sdl_image_display(ui_element* parent, const char* data, const dimensions& dims);

    protected:
    void wm_paint() override;
};