#include "push_button.h"

#include "layout.h"

namespace nao {
    push_button::push_button(std::string_view text, layout& parent) : window{
        {
            .builtin = true,
            .cls = "BUTTON",
            .style = WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            .name = text,
            .pos = { 50, 50 },
            .size = { 50, 50 },
            .parent = &parent,
        }} {

        parent.add_element(*this);
    }
}
