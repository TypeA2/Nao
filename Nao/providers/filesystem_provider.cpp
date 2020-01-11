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
    WIN32_FIND_DATAW data;
    HANDLE f = FindFirstFileW((_m_path + L"\\*").data(), &data);

    if (f == INVALID_HANDLE_VALUE) {
        MessageBoxW(_m_window->handle(), L"Failed to get directory contents", L"Error",
            MB_OK | MB_ICONWARNING);
        return;
    }

    std::wstring file_path;

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

        file_path = _m_path + L'\\' + data.cFileName;

        SHFILEINFOW finfo { };
        DWORD_PTR hr = SHGetFileInfoW(file_path.data(), 0, &finfo,
            sizeof(finfo), SHGFI_TYPENAME | SHGFI_ICON | SHGFI_ICONLOCATION);

        if (hr == 0) {
            std::wstringstream ss;
            ss << L"Failed to get icon, error code " << GetLastError();
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

    std::string str = dynamic_cast<std::stringstream&>(file).str();

    size_t required;
    mbstowcs_s(&required, nullptr, 0, str.data(), 0);
    std::wstring path(required - 1, L'\0');
    mbstowcs_s(&required, path.data(), required, str.data(), _TRUNCATE);
    
    if (GetFileAttributesW(path.data()) & FILE_ATTRIBUTE_DIRECTORY) {
        return new filesystem_provider(path, window);
    }

    return nullptr;
}

size_t filesystem_provider::_id = item_provider_factory::register_class(_create);

