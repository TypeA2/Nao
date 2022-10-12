#include "path.h"

#include <algorithm>
#include <string_view>
#include <ranges>

std::string path::get(bool pretty) const {
    if (pretty && _components.empty()) {
        return std::string(1, pathsep);
    }

    std::string res = pretty ? "" : path_prefix;
    for (auto& component : _components) {
        res.append(component.string()).append(1, pathsep);
    }

    return res;
}

void path::set(std::string_view path) {
    if (path.starts_with(path_prefix)) {
        path = path.substr(sizeof(path_prefix) - 1);
    }

    auto canonical = std::filesystem::path { path }.make_preferred();

    if (canonical != std::string { pathsep }) {
        // Special case for Win32, have / show all drives
        canonical = weakly_canonical(canonical);
    }

    std::vector<std::filesystem::path> new_components;

    logger().trace("Elements of \"{}\":", canonical.string());
    for (auto e : canonical) {
        if (*e.c_str() == pathsep) {
            continue;
        }

        logger().trace(" > {}", e.string());
        new_components.push_back(e);
    }

    if (new_components != _components) {
        logger().debug("Replaced path");
        _components = std::move(new_components);
    }
}

bool path::top_level() const {
    return _components.empty();
}

void path::up() {
    if (!_components.empty()) {
        _components.pop_back();
    }
}
