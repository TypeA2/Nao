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
    nao::message_loop loop;
    return loop.run();
}

void nao_presenter::up() {
    logger().debug("Up");
}

void nao_presenter::refresh() {
    logger().debug("Refresh");
}
