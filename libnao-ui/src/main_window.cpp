#include "main_window.h"


#include <bitset>
#include <libnao/encoding.h>

#include "layout.h"

namespace nao {
    main_window::main_window(std::string_view title) : window{
        {
            .cls = "nao_main_window",
            .style = WS_VISIBLE | WS_OVERLAPPEDWINDOW,
            .name = title,
        } } {
    }


    void main_window::set_title(std::string_view title) const {
        logger().trace("Setting window title to {}", title);

        SetWindowTextW(_handle, utf8_to_wide(title).c_str());
    }


    void main_window::add_layout(layout& l) {
        logger().debug("Adding layout {}", fmt::ptr(&l));

        l._set_parent(*this);

        _layouts.push_back(&l);
    }

}
