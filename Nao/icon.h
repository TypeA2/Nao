#pragma once

#include "frameworks.h"
#include "concepts.h"

class icon {
    public:
    explicit icon();
    explicit icon(HICON handle, bool destroy = true);

    icon(const icon& other) = delete;
    icon(icon&& other) noexcept;

    ~icon();

    operator HICON() const noexcept;
    HICON handle() const noexcept;

    template <concepts::pointer_or_integral T>
    operator T() const noexcept {
        if constexpr (concepts::pointer<T>) {
            return static_cast<T>(_m_handle);
        } else {
            return reinterpret_cast<T>(_m_handle);
        }
    }

    private:
    HICON _m_handle;
    bool _m_destroy;
};

