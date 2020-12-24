#include "push_button.h"

namespace nao {
    push_button::push_button(window* parent, std::string_view text) : window{
        {
            .builtin = true,
            .cls = "BUTTON",
            .style = WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            .name = text,
            .pos = { 50, 50 },
            .dims = { 50, 50 },
            .parent = parent,
        }} {

        //SetParent(_handle, parent->handle());
    }
}