// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ARCHIVE_HPP
#define ARCHIVE_HPP

#include <memory>

class file_stream;
class archive {
    protected:
    file_stream& fs;

    explicit archive(file_stream& fs);
    public:
    virtual ~archive() = default;

    /**
     * @brief Based on all information required, return an appropriate archive instance for the input file
     * 
     * @param name 
     * @param fs 
     * @return std::unique_ptr<archive> 
     */
    [[nodiscard]] static std::unique_ptr<archive> resolve(std::string_view name, file_stream& fs);
};

#endif /* ARCHIVE_HPP */
