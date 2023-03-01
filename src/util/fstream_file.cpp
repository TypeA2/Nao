#include "util/fstream_file.hpp"

fstream_file::fstream_file(const std::filesystem::path& path)
    : _stream { path }, _size { static_cast<std::streamsize>(std::filesystem::file_size(path)) } {
    if (!_stream) {
        throw io_error("Could not open file \"{}\"", path.string());
    }
}

std::streamsize fstream_file::size() const {
    return _size;
}

void fstream_file::seek(std::streamoff new_pos) {
    _stream.seekg(new_pos, std::ios::beg);
    pos = new_pos;
}

std::streamsize fstream_file::read(std::span<std::byte> dest) {
    /* Clamp to end of file */
    auto target_read = std::min<std::streamsize>(pos + dest.size(), _size) - pos;

    /* Read remaining data */
    _stream.read(reinterpret_cast<char*>(dest.data()), target_read);

    /* Clear flags if needed */
    if (_stream.eof()) {
        _stream.clear();
    }

    pos += target_read;
    return target_read;
}

std::byte fstream_file::getb() {
    int res = _stream.get();
    if (res != std::char_traits<char>::eof()) {
        pos += 1;
        return static_cast<std::byte>(res);
    }

    throw io_error("eof");
}
