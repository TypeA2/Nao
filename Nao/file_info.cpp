#include "file_info.h"
#include "utils.h"

file_info::file_info() : _m_data { .dwFileAttributes = INVALID_FILE_ATTRIBUTES } {
    
}


file_info::file_info(const std::string& path) : _m_path(path) {
    if (!GetFileAttributesExW(utils::utf16(path).c_str(), GetFileExInfoStandard, &_m_data)) {
        _m_data.dwFileAttributes = INVALID_FILE_ATTRIBUTES;
    }
}

file_info::file_info(const std::filesystem::path& path) : _m_path(path.string()) {
    if (!GetFileAttributesExW(path.wstring().c_str(), GetFileExInfoStandard, &_m_data)) {
        _m_data.dwFileAttributes = INVALID_FILE_ATTRIBUTES;
    }
}


file_info::operator bool() const {
    return !invalid();
}

const std::string& file_info::path() const {
    return _m_path;
}

int64_t file_info::size() const {
    return utils::make_quad(_m_data.nFileSizeLow, _m_data.nFileSizeHigh);
}

bool file_info::invalid() const {
    return _m_data.dwFileAttributes == INVALID_FILE_ATTRIBUTES;
}

bool file_info::archive() const {
    return _m_data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE;
}

bool file_info::compressed() const {
    return _m_data.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED;
}

bool file_info::directory() const {
    return _m_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}

bool file_info::encrypted() const {
    return _m_data.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED;
}

bool file_info::hidden() const {
    return _m_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN;
}

bool file_info::normal() const {
    return _m_data.dwFileAttributes & FILE_ATTRIBUTE_NORMAL;
}

bool file_info::readonly() const {
    return _m_data.dwFileAttributes & FILE_ATTRIBUTE_READONLY;
}

bool file_info::sparse() const {
    return _m_data.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE;
}

bool file_info::system() const {
    return _m_data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM;
}

bool file_info::temporary() const {
    return _m_data.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY;
}
