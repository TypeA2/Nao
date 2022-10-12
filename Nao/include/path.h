#pragma once

#include <libnao/util/logging.h>

#include <vector>
#include <string>
#include <filesystem>

/* Path object for the model */
class path {
    NAO_LOGGER(path);

    std::vector<std::filesystem::path> _components;

    public:
    static constexpr char path_prefix[] = R"(\\?\)";
    static constexpr char pathsep = '\\';

    [[nodiscard]] std::string get(bool pretty = false) const;
    void set(std::string_view path);

    [[nodiscard]] bool top_level() const;

    void up();
};
