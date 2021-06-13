#include "nao_presenter.h"

#include <libnao/ui/message_loop.h>


nao_presenter::nao_presenter()
    : _model{ *this }, _window{ *this } {
    
}

int nao_presenter::run() {
    (void)this;
    nao::message_loop loop;
    return loop.run();
}
