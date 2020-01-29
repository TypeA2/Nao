#pragma once

#include "frameworks.h"

#include <string>
#include <filesystem>

class file_info {
    public:
    explicit file_info();
    explicit file_info(const std::string& path);
    explicit file_info(const std::filesystem::path& path);
    ~file_info() = default;

    operator bool() const;

    const std::string& path() const;
    int64_t size() const;

    bool invalid() const;

    bool archive() const;
    bool compressed() const;
    bool directory() const;
    bool encrypted() const;
    bool hidden() const;
    bool normal() const;
    bool readonly() const;
    bool sparse() const;
    bool system() const;
    bool temporary() const;

    private:
    std::string _m_path;
    WIN32_FILE_ATTRIBUTE_DATA _m_data;
};

