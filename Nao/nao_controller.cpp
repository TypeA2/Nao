#include "nao_controller.h"
#include "utils.h"

#include "auto_wrapper.h"

nao_controller::nao_controller() : view(*this), model(view)
        , _m_worker(1)
        , _m_main_threadid(GetCurrentThreadId()) {

    view.setup();

    _m_worker.push(auto_wrapper::bind<com_wrapper>([this] {
        model.setup();
        view.set_path(model.current_path());
        }));
}
    
int nao_controller::pump() {
    if (GetCurrentThreadId() != _m_main_threadid) {
        throw std::runtime_error("message pump called from outside main thread");
    }

    // Event loop
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        if (!msg.hwnd && msg.message > TM_FIRST && msg.message < TM_LAST) {
            _handle_message(static_cast<nao_thread_message>(msg.message), msg.wParam, msg.lParam);

            continue;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

DWORD nao_controller::main_threadid() const {
    return _m_main_threadid;
}

void nao_controller::_handle_message(nao_thread_message msg, WPARAM wparam, LPARAM lparam) {
    utils::coutln("thread message:", msg, wparam, lparam);
}
