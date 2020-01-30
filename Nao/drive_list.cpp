#include "drive_list.h"

#include "utils.h"

drive_list::drive_list()
    : _m_drives(GetLogicalDrives()) {
    
    _m_drive_info.reserve(_m_drives.count());

    DWORD required = GetLogicalDriveStringsW(0, nullptr);
    std::vector str(required, L'\0');

    LPWSTR s = str.data();

    GetLogicalDriveStringsW(required, s);

    SHFILEINFOW finfo { };
    DWORD drives = _m_drives.to_ulong();
    while (drives) {
        if (drives & 1) {
            DWORD_PTR hr = SHGetFileInfoW(s, 0, &finfo, sizeof(finfo),
                SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_ADDOVERLAYS);

            ASSERT(hr);

            ULARGE_INTEGER total;
            ULARGE_INTEGER free;
            GetDiskFreeSpaceExW(s, nullptr, &total, &free);
            
            _m_drive_info.push_back({
                .letter     = utils::utf8(s).front(),
                .name       = utils::utf8(finfo.szDisplayName),
                .icon       = finfo.iIcon,
                .total_size = total.QuadPart,
                .free_size  = free.QuadPart
                });

            s += wcslen(s) + 1;
        }

        drives >>= 1;
    }
}

size_t drive_list::count() const {
    return _m_drives.count();
}

drive_list::array_type::iterator drive_list::begin() {
    return _m_drive_info.begin();
}

drive_list::array_type::const_iterator drive_list::begin() const {
    return _m_drive_info.begin();
}

drive_list::array_type::const_iterator drive_list::cbegin() const {
    return _m_drive_info.cbegin();
}

drive_list::array_type::reverse_iterator drive_list::rbegin() {
    return _m_drive_info.rbegin();
}

drive_list::array_type::const_reverse_iterator drive_list::rbegin() const {
    return _m_drive_info.rbegin();
}

drive_list::array_type::const_reverse_iterator drive_list::crbegin() const {
    return _m_drive_info.crbegin();
}

drive_list::array_type::iterator drive_list::end() {
    return _m_drive_info.end();
}

drive_list::array_type::const_iterator drive_list::end() const {
    return _m_drive_info.end();
}


drive_list::array_type::const_iterator drive_list::cend() const {
    return _m_drive_info.cend();
}

drive_list::array_type::reverse_iterator drive_list::rend() {
    return _m_drive_info.rend();
}

drive_list::array_type::const_reverse_iterator drive_list::rend() const {
    return _m_drive_info.rend();
}

drive_list::array_type::const_reverse_iterator drive_list::crend() const {
    return _m_drive_info.crend();
}
