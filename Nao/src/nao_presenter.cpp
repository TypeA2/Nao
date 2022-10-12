#include "nao_presenter.h"

#include <libnao/ui/message_loop.h>
#include <libnao/ui/file_dialog.h>

nao_presenter::nao_presenter()
    : _model{ *this }, _window{ *this } {
    
    _worker.push([this] {
        _model.set_path("/");
        });
}

int nao_presenter::run() {
    _main_thread_id = GetCurrentThreadId();

    nao::ui::message_loop loop;

    loop.add_filter([this](nao::ui::event& e) {
        return event_filter(e);
    });

    return loop.run();
}

bool nao_presenter::event_filter(nao::ui::event& e) {
    auto& native = e.native();
    if (native.msg > TM_FIRST && native.msg < TM_LAST) {
        logger().debug("Got thread message {}",
            magic_enum::enum_name<thread_message>(static_cast<thread_message>(native.msg)));

        switch (native.msg) {
            case TM_PATH_CHANGED:
                _window.set_path(_model.path().get(true));

                if (_model.path().top_level()) {
                    _window.disable_up();
                } else {
                    _window.enable_up();
                }

                break;
        }

        return true;
    }

    return false;
}

void nao_presenter::up() {
    _worker.push([&] {
        _model.up();
    });
}

void nao_presenter::refresh() {
    logger().debug("Refresh");          
}

void nao_presenter::browse() {
    auto res = nao::ui::file_dialog::get_directory();
    if (res.has_value()) {
        _new_path(std::move(res.value()));
    }
}

void nao_presenter::path_changed(std::string_view new_path) {
    _new_path(std::string { new_path });
}

void nao_presenter::path_changed() const {
    if (_main_thread_id == GetCurrentThreadId()) {
        throw std::runtime_error { "nao_presenter::path_changed was called from the GUI thread" };
    }

    PostThreadMessageW(_main_thread_id, TM_PATH_CHANGED, 0, 0);
}

void nao_presenter::_new_path(std::string path) {
    _worker.push([&, path = std::move(path)] {
        logger().info("Changing path to \"{}\"", path);
        _model.set_path(path);
    });
}
