#include "layout.h"

#include "main_window.h"

namespace nao {
    layout::layout(main_window& w) : window{
        {
            .cls = "nao_layout",
            .style = WS_VISIBLE | WS_CHILD,
            .pos = { 0, 0 },
            .parent = &w
        } } {
        w.add_layout(*this);
    }


    void layout::_set_parent(main_window& w) const {
        logger().debug("Attaching to {}", fmt::ptr(&w));

        SetParent(_handle, w.handle());

        auto target_size = w.size();
        SetWindowPos(_handle, nullptr,
            0, 0, target_size.w, target_size.h, 0);
    }


}