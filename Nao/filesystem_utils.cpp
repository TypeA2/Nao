#include "filesystem_utils.h"

#include "utils.h"

namespace fs_utils {
    bool is_child(const std::string& base, const std::string& child) {
        // Child path must be longer
        if (child.size() <= base.size()) {
            return false;
        }

        // Child should start with exactly base
        if (child.substr(0, base.size()) != base) {
            return false;
        }

        return true;
    }

    bool is_direct_child(const std::string& base, const std::string& child) {
        // Must at least be any kind of child
        if (!is_child(base, child)) {
            return false;
        }

        // child's path relative to base must not contain a separator
        if (child.substr(base.size()).find('\\') != std::string::npos) {
            return false;
        }

        return true;
    }

    bool same_path(const std::string& left, const std::string& right) {
        if (left.back() == '\\' && right.back() != '\\') {
            return left == (right + '\\');
        }

        if (left.back() != '\\' && right.back() == '\\') {
            return (left + '\\') == right;
        }

        return left == right;
    }

    inline namespace classes {
        file_info::file_info(const std::string& path) : _path(path) {
            if (!GetFileAttributesExW(utils::utf16(path).c_str(), GetFileExInfoStandard, &_data)) {
                _data.dwFileAttributes = INVALID_FILE_ATTRIBUTES;
            }
        }

        file_info::file_info(const std::filesystem::path& path) : _path(path.string()) {
            if (!GetFileAttributesExW(path.wstring().c_str(), GetFileExInfoStandard, &_data)) {
                _data.dwFileAttributes = INVALID_FILE_ATTRIBUTES;
            }
        }

        file_info::operator bool() const {
            return !invalid();
        }

        const std::string& file_info::path() const {
            return _path;
        }

        int64_t file_info::size() const {
            return utils::make_quad(_data.nFileSizeLow, _data.nFileSizeHigh);
        }

        bool file_info::invalid() const {
            return _data.dwFileAttributes == INVALID_FILE_ATTRIBUTES;
        }

        bool file_info::archive() const {
            return _data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE;
        }

        bool file_info::compressed() const {
            return _data.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED;
        }

        bool file_info::directory() const {
            return _data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        }

        bool file_info::encrypted() const {
            return _data.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED;
        }

        bool file_info::hidden() const {
            return _data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN;
        }

        bool file_info::normal() const {
            return _data.dwFileAttributes & FILE_ATTRIBUTE_NORMAL;
        }

        bool file_info::readonly() const {
            return _data.dwFileAttributes & FILE_ATTRIBUTE_READONLY;
        }

        bool file_info::sparse() const {
            return _data.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE;
        }

        bool file_info::system() const {
            return _data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM;
        }

        bool file_info::temporary() const {
            return _data.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY;
        }



        drive_list::drive_list() : _drives(GetLogicalDrives()) {

            _drive_info.reserve(_drives.count());

            DWORD required = GetLogicalDriveStringsW(0, nullptr);
            std::vector str(required, L'\0');

            LPWSTR s = str.data();

            GetLogicalDriveStringsW(required, s);

            SHFILEINFOW finfo { };
            DWORD drives = _drives.to_ulong();
            while (drives) {
                if (drives & 1) {
                    DWORD_PTR hr = SHGetFileInfoW(s, 0, &finfo, sizeof(finfo),
                        SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_ADDOVERLAYS);

                    ASSERT(hr);

                    ULARGE_INTEGER total;
                    ULARGE_INTEGER free;
                    GetDiskFreeSpaceExW(s, nullptr, &total, &free);

                    _drive_info.push_back({
                        .letter     = utils::utf8(s).front(),
                        .name       = utils::utf8(finfo.szDisplayName),
                        .icon       = finfo.iIcon,
                        .total_size = static_cast<std::streamsize>(total.QuadPart),
                        .free_size  = static_cast<std::streamsize>(free.QuadPart)
                        });

                    s += wcslen(s) + 1;
                }

                drives >>= 1;
            }
        }

        size_t drive_list::count() const {
            return _drives.count();
        }

        drive_list::array_type::iterator drive_list::begin() {
            return _drive_info.begin();
        }

        drive_list::array_type::const_iterator drive_list::begin() const {
            return _drive_info.begin();
        }

        drive_list::array_type::iterator drive_list::end() {
            return _drive_info.end();
        }

        drive_list::array_type::const_iterator drive_list::end() const {
            return _drive_info.end();
        }
    }
}
