#include "sdl_image_display.h"

#include <SDL.h>
#include <SDL_syswm.h>

#include "resource.h"

#include <nao/logging.h>

sdl_image_display::sdl_image_display(ui_element* parent, const char* data, const dimensions& dims)
    : ui_element(parent, IDS_SDL_WINDOW, parent->dims().rect(), win32::style | WS_OVERLAPPED)
    , _dims { dims } {
    SDL_Window* win = SDL_CreateWindowFrom(handle());
    ASSERT(win);

    SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, "best", SDL_HINT_OVERRIDE);

    _renderer = SDL_CreateRenderer(win, -1, 0);
    SDL_SetRenderDrawColor(_renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);

    _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_BGRA32,
        SDL_TEXTUREACCESS_STATIC, utils::narrow<int>(dims.width), utils::narrow<int>(dims.height));
    ASSERT(_texture);
    ASSERT(SDL_UpdateTexture(_texture, nullptr, data, utils::narrow<int>(dims.width * 4)) == 0);

    SDL_RenderClear(_renderer);
    SDL_Rect dest {
        .x = 0,
        .y = 0,
        .w = utils::narrow<int>(width()),
        .h = utils::narrow<int>((dims.height / static_cast<double>(dims.width)) * width())
    };
    SDL_RenderCopy(_renderer, _texture, nullptr, &dest);
    SDL_RenderPresent(_renderer);
    nao::coutln("displaying");
    //SDL_Delay(2500);
    nao::coutln("done");
}

void sdl_image_display::wm_paint() {
    nao::coutln("PAINT!");
    SDL_RenderClear(_renderer);
    SDL_Rect dest {
        .x = 0,
        .y = 0,
        .w = utils::narrow<int>(width()),
        .h = utils::narrow<int>((_dims.height / static_cast<double>(_dims.width)) * width())
    };
    SDL_RenderCopy(_renderer, _texture, nullptr, &dest);
    SDL_RenderPresent(_renderer);
}

