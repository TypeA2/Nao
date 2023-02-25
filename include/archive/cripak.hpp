// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CRIPAK_H
#define CRIPAK_H

#include "archive/archive.hpp"

class cripak_archive : public archive {
    public:
    explicit cripak_archive(std::string_view name, file_stream& cripak_fs);
};

#endif /* CRIPAK_H */
