// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CRIPAK_HPP
#define CRIPAK_HPP

#include "archive/archive.hpp"

#include <memory>

#include "util/cripak_utf.hpp"

class cripak_archive : public archive {
    std::unique_ptr<utf_table> _utf;

    public:
    explicit cripak_archive(std::string_view name, file_stream& cripak_fs);
};

#endif /* CRIPAK_HPP */
