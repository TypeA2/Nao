#pragma once

#include "frameworks.h"

#include <thread>

class com_thread {
    public:
    explicit com_thread(
        DWORD init = COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    ~com_thread();

    com_thread(const com_thread&) = delete;
    com_thread(com_thread&&) = delete;

    template<typename Func, typename... Args>
    static void run(Func fun, Args&&... args) {
        std::thread(bind(fun, args)).detach();
    }

    template<typename CondFunc, typename Func, typename... Args>
    static void run_cond(CondFunc cond, Func fun, Args&&... args) {
        std::thread(bind_cond(cond, fun, args)).detach();
    }

    template<typename Func, typename... Args>
    static auto bind(Func fun, Args&&... args) {
        return [fun, args...] {
            com_thread __com;
            fun(std::forward<Args>(args)...);
        };
    }

    template<typename CondFunc, typename Func, typename... Args>
    static auto bind_cond(CondFunc cond, Func fun, Args&&... args) {
        return [cond, fun, args...] {
            if (!cond()) {
                return;
            }

            com_thread __com;
            fun(std::forward<Args>(args)...);
        };
    }
};

