#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include "archive.hpp"

#include <filesystem>
#include <map>

class directory_archive : public archive {
    std::filesystem::path _path;

    std::map<std::string, std::unique_ptr<archive>> _archives;
    std::map<std::string , std::unique_ptr<file_stream>> _files;

    public:
    explicit directory_archive(const std::filesystem::path& path);

    void contents(fill_func filler) override;

    [[nodiscard]] bool contains_archive(std::string_view name) override;
    [[nodiscard]] archive& get_archive(std::string_view name) override;
    [[nodiscard]] int stat(std::string_view name, struct stat& stbuf) override;
    [[nodiscard]] int open(std::string_view name, int flags) override;
    [[nodiscard]] int read(std::string_view name, std::span<std::byte> buf, off_t offset) override;
};

#endif /* DIRECTORY_HPP */
