#pragma once

#include "frameworks.h"

class icon {
    public:
    explicit icon();
    explicit icon(HICON handle, bool destroy = true);

    icon(const icon& other) = delete;
    icon(icon&& other) noexcept;

    ~icon();

    operator HICON() const noexcept;
    HICON handle() const noexcept;

    private:
    HICON _m_handle;
    bool _m_destroy;
};

