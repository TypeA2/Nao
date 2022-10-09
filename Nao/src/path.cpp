#include "path.h"

#include <algorithm>
#include <filesystem>
#include <string_view>
#include <ranges>

std::string path::get() const {
    auto fold = [](std::string res, std::string_view e) {
        return std::move(res) + pathsep + std::string { e };
    };
    return "";
}

void path::set(std::string_view path) {
    for (auto e : std::string{ path } | std::views::split(pathsep)) {
        logger().debug(std::string_view { e.begin(), e.end() });
    }
}