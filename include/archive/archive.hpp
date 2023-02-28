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
    public:
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
     * @brief 
     * 
     * @param name 
     * @param fs 
     * @return std::unique_ptr<archive> 
     */

    /**
     * @brief Check whether a path represents a valid archive, and optionally createes an `archive` instance for it
     * 
     * @param path 
     * @param fs 
     * @param archive 
     * @return true 
     * @return false 
     */
    [[nodiscard]] static bool resolve(const std::filesystem::path& path,
        std::unique_ptr<file_stream> fs = nullptr, std::unique_ptr<archive>* archive = nullptr);
};

class file_archive : public archive {
    std::unique_ptr<file_stream> _fs;

    protected:
    file_stream& fs;

    public:
    explicit file_archive(std::unique_ptr<file_stream> fs);
};

#endif /* ARCHIVE_HPP */
