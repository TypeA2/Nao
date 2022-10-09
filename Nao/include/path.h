#pragma once

#include <libnao/util/logging.h>

#include <vector>
#include <string>

/* Path object for the model */
class path {
    NAO_LOGGER(path);

    std::vector<std::string> _components;

    public:
    static constexpr char pathsep = '\\';

    [[nodiscard]] std::string get() const;
    void set(std::string_view path);
};
