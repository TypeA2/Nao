#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include "archive.hpp"

#include <filesystem>
#include <map>

class directory_archive : public archive {
    std::filesystem::path _path;

    std::map<std::string, std::unique_ptr<archive>> _files;

    public:
    explicit directory_archive(const std::filesystem::path& path);

    void contents(fill_func filler) override;

    [[nodiscard]] bool contains_archive(std::string_view name) override;
    [[nodiscard]] archive& get_archive(std::string_view name) override;
    [[nodiscard]] int stat(std::string_view name, struct stat& stbuf) override;
};

#endif /* DIRECTORY_HPP */
