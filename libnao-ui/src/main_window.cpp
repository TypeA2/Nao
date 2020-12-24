#include "main_window.h"

#include <libnao/encoding.h>

namespace nao {
    main_window::main_window(std::string_view title) : window{
        {
            .cls = "nao_main_window",
            .style = WS_VISIBLE | WS_OVERLAPPEDWINDOW,
            .name = title
        } } {
        //set_title(title);
    }


    void main_window::set_title(std::string_view title) const {
        SetWindowTextW(_handle, utf8_to_wide(title).c_str());
    }

}
