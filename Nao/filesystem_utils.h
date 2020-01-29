#pragma once

#include <string>

namespace fs_utils {
    bool is_child(const std::string& base, const std::string& child);

    bool is_direct_child(const std::string& base, const std::string& child);
}
