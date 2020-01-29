#include "filesystem_utils.h"

namespace fs_utils {
    bool is_child(const std::string& base, const std::string& child) {
        // Child path must be longer
        if (child.size() <= base.size()) {
            return false;
        }

        // Child should start with exactly base
        if (child.substr(0, base.size()) != base) {
            return false;
        }

        return true;
    }

    bool is_direct_child(const std::string& base, const std::string& child) {
        // Must at least be any kind of child
        if (!is_child(base, child)) {
            return false;
        }

        // child's path relative to base must not contain a separator
        if (child.substr(base.size()).find('\\') != std::string::npos) {
            return false;
        }

        return true;
    }


}
