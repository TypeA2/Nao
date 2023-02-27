// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ARCHIVE_HPP
#define ARCHIVE_HPP

#include <memory>
#include <functional>
#include <string_view>
#include <filesystem>

#include <sys/stat.h>

class file_stream;
class archive {
    protected:
    file_stream& fs;

    public:
    explicit archive(file_stream& fs);
    virtual ~archive() = default;

    enum file_type {
        file, dir,
    };

    /**
     * Function called for every top-level element in this archive, with the name and type as parameters
     */
    using fill_func = std::function<void(std::string_view, file_type)>;

    /**
     * @brief Calls `filler` for every top-level element of this archive
     * 
     * @param filler 
     */
    virtual void contents(fill_func filler) = 0;

    /**
     * @brief Query whether the archive contains a specific sub-archive
     * 
     * @param name 
     * @return true 
     * @return false 
     */
    [[nodiscard]] virtual bool contains_archive(std::string_view name) = 0;

    /**
     * @brief Retrieve archive instance for a specific sub-archive
     * 
     * @param name 
     * @return archive& 
     */
    [[nodiscard]] virtual archive& get_archive(std::string_view name) = 0;

    /**
     * @brief Retrieve a top-level file's attributes
     * 
     * @param name 
     * @param stbuf 
     * @return * int 
     */
    [[nodiscard]] virtual int stat(std::string_view name, struct stat& stbuf) = 0;

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
