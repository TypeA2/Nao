#include "nao_controller.h"

nao_controller::nao_controller() : view(*this), model(view) {
    view.setup();
}
