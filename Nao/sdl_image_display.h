#pragma once

#include "ui_element.h"

struct SDL_Window;

class sdl_image_display : public ui_element {
    SDL_Window* _win;

    public:
    explicit sdl_image_display(ui_element* parent, const char* data, const dimensions& dims);
};