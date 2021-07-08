#include "nao_presenter.h"

#include <libnao/ui/message_loop.h>

nao_presenter::nao_presenter()
    : _model{ *this }, _window{ *this } {
    
    _worker.push([this] {
        _model.set_path("/");
        });
}

int nao_presenter::run() {
    (void)this;
    nao::ui::message_loop loop;

    loop.add_filter([this](nao::ui::event& e) {
        return event_filter(e);
    });

    return loop.run();
}

bool nao_presenter::event_filter(nao::ui::event& e) {
    auto& native = e.native();
    if (native.msg > TM_FIRST && native.msg < TM_LAST) {
        logger().debug("Got thread message {}", native.msg);
        return true;
    }

    logger().debug("Event {}", native.msg);

    return false;
}

void nao_presenter::up() {
    logger().debug("Up");
}

void nao_presenter::refresh() {
    logger().debug("Refresh");          
}

void nao_presenter::path_changed() const {
    (void)this;
    PostMessageW(nullptr, TM_PATH_CHANGED, 0, 0);
}
