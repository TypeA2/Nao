// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <memory>

#include <fmt/format.h>

class file_stream;

class archive_error : public std::runtime_error {
    public:
    template <typename... Args>
    archive_error(fmt::format_string<Args...> fmt, Args&&... args)
        : std::runtime_error(fmt::format(fmt, std::forward<Args>(args)...)) {

    }
};

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

#endif /* ARCHIVE_H */
