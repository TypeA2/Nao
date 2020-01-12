#include "filesystem_provider.h"

#include "item_provider_factory.h"
#include "ui_element.h"
#include "utils.h"

filesystem_provider::filesystem_provider(std::wstring path, ui_element* window)
    : _m_path(std::move(path))
    , _m_window(window) {
    _populate();
}

size_t filesystem_provider::count() const {
    return _m_contents.size();
}

std::wstring& filesystem_provider::name(size_t index) {
    return _m_contents[index].name;
}

int64_t filesystem_provider::size(size_t index) const {
    return _m_contents[index].size;
}

std::wstring& filesystem_provider::size_str(size_t index) {
    return _m_contents[index].size_str;
}

std::wstring& filesystem_provider::type(size_t index) {
    return _m_contents[index].type;
}

double filesystem_provider::compression(size_t index) const {
    (void) index;
    return 0;
}

int filesystem_provider::icon(size_t index) const {
    return _m_contents[index].icon_index;
}

bool filesystem_provider::dir(size_t index) const {
    return _m_contents[index].is_dir;
}

void filesystem_provider::_populate() {
    // List devices
    if (_m_path == L"\\") {
        DWORD drives = GetLogicalDrives();

        DWORD required = GetLogicalDriveStringsW(0, nullptr);
        std::vector str(required, L'\0');

        GetLogicalDriveStringsW(required, str.data());

        LPCWSTR s = str.data();
        SHFILEINFOW finfo { };

        while (drives) {
            if (drives & 1) {
                DWORD_PTR hr = SHGetFileInfoW(s, 0, &finfo,
                    sizeof(finfo),
                    SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_ICON | SHGFI_ICONLOCATION | SHGFI_ADDOVERLAYS);

                if (hr == 0) {
                    MessageBoxW(_m_window->handle(),
                        L"Failed to get drive name", L"Error", MB_OK | MB_ICONEXCLAMATION);
                } else {
                    DWORD sectors_per_cluster, bytes_per_sector, free_clusters, total_clusters;
                    GetDiskFreeSpaceW(s, &sectors_per_cluster, &bytes_per_sector, &free_clusters, &total_clusters);

                    int64_t cluster_size = int64_t(sectors_per_cluster) * bytes_per_sector;

                    std::wstringstream ss;
                    ss << utils::wbytes(cluster_size * free_clusters)
                        << L" free of "
                        << utils::wbytes(cluster_size * total_clusters);

                    _m_contents.push_back({
                        true,
                        finfo.szDisplayName,
                        finfo.szTypeName,
                        cluster_size * free_clusters, ss.str(),
                        finfo.iIcon
                        });
                }

                s += wcslen(s) + 1;
            }

            drives >>= 1;
        }

        return;
    }
    WIN32_FIND_DATAW data;
    HANDLE f = FindFirstFileW((_m_path + L"\\*").c_str(), &data);

    if (f == INVALID_HANDLE_VALUE) {
        MessageBoxW(_m_window->handle(), L"Failed to get directory contents", L"Error",
            MB_OK | MB_ICONWARNING);
        return;
    }

    std::wstring file_path;

    SetLastError(0);

    do {
        if (data.dwFileAttributes == INVALID_FILE_ATTRIBUTES ||
            data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ||
            data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
            continue;
        }

        // Skip these
        if (wcscmp(data.cFileName, L".") == 0 || wcscmp(data.cFileName, L"..") == 0) {
            continue;
        }

        if (_m_path.back() != L'\\') {
            file_path = _m_path + L'\\' + data.cFileName;
        } else {
            file_path = _m_path + data.cFileName;
        }

        SHFILEINFOW finfo { };
        DWORD_PTR hr = SHGetFileInfoW(file_path.c_str(), 0, &finfo,
            sizeof(finfo), SHGFI_TYPENAME | SHGFI_ICON | SHGFI_ICONLOCATION | SHGFI_ADDOVERLAYS);

        if (hr == 0) {
            std::wstringstream ss;
            ss << L"Failed to get icon for " << file_path << ", error code " << GetLastError();
            MessageBoxW(_m_window->handle(), ss.str().c_str(), L"Error",
                MB_OK | MB_ICONEXCLAMATION);
            continue;
        }

        int64_t size = (int64_t(data.nFileSizeHigh) << 32) | data.nFileSizeLow;

        _m_contents.push_back({
            !!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY),
            data.cFileName,
            finfo.szTypeName,
            size,
            utils::wbytes(size),
            finfo.iIcon
            });
    } while (FindNextFileW(f, &data) != 0);

    if (GetLastError() != ERROR_NO_MORE_FILES) {
        MessageBoxW(_m_window->handle(), L"Unexpected error", L"Error",
            MB_OK | MB_ICONWARNING);
        utils::coutln("Error:", GetLastError());
    }

    FindClose(f);
}

item_provider* filesystem_provider::_create(std::istream& file, ui_element* window) {
    if (!dynamic_cast<std::stringstream*>(&file)) {
        return nullptr;
    }

    std::wstring path = utils::utf16(dynamic_cast<std::stringstream&>(file).str());
    
    if (GetFileAttributesW(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY) {
        return new filesystem_provider(path, window);
    }

    return nullptr;
}

size_t filesystem_provider::_id = item_provider_factory::register_class(_create);

