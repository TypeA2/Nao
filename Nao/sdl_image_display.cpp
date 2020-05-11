#include "sdl_image_display.h"

#include <SDL.h>

#include "resource.h"

sdl_image_display::sdl_image_display(ui_element* parent, const char* data, const dimensions& dims)
    : ui_element(parent, IDS_SDL_WINDOW, parent->dims().rect(), win32::style | WS_OVERLAPPED) {
    _win = SDL_CreateWindowFrom(handle());
    ASSERT(_win);

    SDL_Renderer* renderer = SDL_CreateRenderer(_win, -1, 0);
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    utils::coutln("displaying");
    SDL_Delay(2500);
    utils::coutln("done");
}

