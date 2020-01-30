#pragma once

#include "frameworks.h"

#include <bitset>
#include <vector>

class drive_list {
    struct drive_info {
        char letter;
        std::string name;
        int icon;

        uintmax_t total_size;
        uintmax_t free_size;
    };

    public:
    using value_type = drive_info;
    using array_type = std::vector<value_type>;

    explicit drive_list();
    ~drive_list() = default;

    size_t count() const;

    array_type::iterator begin();
    array_type::const_iterator begin() const;
    array_type::const_iterator cbegin() const;

    array_type::reverse_iterator rbegin();
    array_type::const_reverse_iterator rbegin() const;
    array_type::const_reverse_iterator crbegin() const;

    array_type::iterator end();
    array_type::const_iterator end() const;
    array_type::const_iterator cend() const;

    array_type::reverse_iterator rend();
    array_type::const_reverse_iterator rend() const;
    array_type::const_reverse_iterator crend() const;

    private:
    std::bitset<sizeof(DWORD) * CHAR_BIT> _m_drives;
    array_type _m_drive_info;
};

