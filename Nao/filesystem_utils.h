#pragma once

#include <Windows.h>

#include <string>
#include <bitset>
#include <filesystem>

namespace fs_utils {
    bool is_child(const std::string& base, const std::string& child);

    bool is_direct_child(const std::string& base, const std::string& child);

    bool same_path(const std::string& left, const std::string& right);

    inline namespace classes {
        class file_info {
            std::string _path;
            WIN32_FILE_ATTRIBUTE_DATA _data { .dwFileAttributes = INVALID_FILE_ATTRIBUTES };

            public:
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
        };

        class drive_list {
            public:
            struct drive_info {
                char letter;
                std::string name;
                int icon;

                std::streamsize total_size;
                std::streamsize free_size;
            };

            using array_type = std::vector<drive_info>;

            std::bitset<sizeof(DWORD) * CHAR_BIT> _drives;
            array_type _drive_info;

            drive_list();
            ~drive_list() = default;

            size_t count() const;

            array_type::iterator begin();
            array_type::const_iterator begin() const;

            array_type::iterator end();
            array_type::const_iterator end() const;
        };
    }
}
