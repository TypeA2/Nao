// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ARCHIVE_HPP
#define ARCHIVE_HPP

#include <memory>
#include <functional>
#include <string_view>
#include <filesystem>
#include <span>
#include <optional>

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
     * @return int 
     */
    [[nodiscard]] virtual int stat(std::string_view name, struct stat& stbuf) = 0;

    /**
     * @brief Request opening a file with specific access flags
     * 
     * @param name 
     * @param flags 
     * @return int 
     */
    [[nodiscard]] virtual int open(std::string_view name, int flags) = 0;

    /**
     * @brief Read at most `buf.size()` bytes into `buf` from `offset` and return the number of bytes read
     * 
     * @param name 
     * @param buf 
     * @param offset 
     * @return int 
     */
    [[nodiscard]] virtual int read(std::string_view name, std::span<std::byte> buf, off_t offset) = 0;

    /**
     * @brief Check whether a path represents a parseable archive
     * 
     * @param path 
     * @param fs 
     * @return true 
     * @return false 
     */
    [[nodiscard]] static bool is_archive(const std::filesystem::path& path, file_stream* fs);

    /**
     * @brief Get the archive for the given path, or throw an exception
     * 
     * @param path 
     * @param fs 
     * @return std::unique_ptr<archive> 
     */
    [[nodiscard]] static std::unique_ptr<archive> get_archive(const std::filesystem::path& path, std::unique_ptr<file_stream> fs);

    private:
    /**
     * @brief Internal 2-for-1 helper for is_archive and get_archive
     * 
     */
    using resolve_type = std::pair<bool, std::unique_ptr<archive>>;
    [[nodiscard]] static resolve_type resolve_internal(const std::filesystem::path& path, file_stream* fs, bool make_archive);
};

class file_archive : public archive {
    std::unique_ptr<file_stream> _fs;

    protected:
    file_stream& fs;

    public:
    explicit file_archive(std::unique_ptr<file_stream> fs);
};

#endif /* ARCHIVE_HPP */
