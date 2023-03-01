#ifndef FSTREAM_FILE_HPP
#define FSTREAM_FILE_HPP

#include "util/file_stream.hpp"

#include <fstream>
#include <filesystem>

class fstream_file : public file_stream {
    std::ifstream _stream;
    std::streamsize _size;

    public:
    explicit fstream_file(const std::filesystem::path& path);

    ~fstream_file() override = default;

    [[nodiscard]] std::streamsize size() const override;

    void seek(std::streamoff new_pos) override;

    [[nodiscard]] std::streamsize read(std::span<std::byte> dest) override;

    [[nodiscard]] std::byte getb();
};


#endif /* FSTREAM_FILE_HPP */
