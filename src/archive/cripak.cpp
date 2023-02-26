// SPDX-License-Identifier: LGPL-3.0-or-later

#include "archive/cripak.hpp"

#include <array>
#include <iostream>

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

#include "util/file_stream.hpp"

cripak_archive::cripak_archive(std::string_view name, file_stream& cripak_fs)
    : archive(cripak_fs) {

    spdlog::info("Opening .cpk: {}", name);

    std::array<char, 4> fourcc;
    if (fs.read(fourcc) != fourcc.size()) {
        throw io_error("unexpected EOF while reading magic number");
    }

    if (fourcc != std::array { 'C', 'P', 'K', ' ' }) {
        throw archive_error("unexpected magic number: expected 'CPK ', got \"{} {} {} {}\"",
            fourcc[0], fourcc[1], fourcc[2], fourcc[3]);
    }

    if (uint32_t tag = fs.read_u32le(); tag != 0xFF) {
        throw archive_error("expected tag 0xFF, got {:#x}", tag);
    }

    [[maybe_unused]] uint64_t utf_size = fs.read_u64le();

    _utf = std::make_unique<utf_table>(fs);

    if (_utf->rows() != 1) {
        throw archive_error("expected 1 row in CPK @UTF");
    }

    if (!_utf->has_field("TocOffset")) {
        throw archive_error("TocOffset is required");
    }

    auto toc_offset = _utf->get<uint64_t>(0, "TocOffset");
    
    if (toc_offset > 2048) {
        spdlog::trace("Clamping toc_offset from {}", toc_offset);
        toc_offset = 2048;
    }

    /* First 16 bytes of the archive are not counted */
    toc_offset += 16;

    uint64_t extra_offset;
    if (!_utf->has_field("ContentOffset")) {
        spdlog::trace("No ContentOffset");
        extra_offset = toc_offset;
    } else {
        auto content_offset = _utf->get<uint64_t>(0, "ContentOffset");

        if (content_offset >= toc_offset) {
            extra_offset = toc_offset;

        } else {
            spdlog::trace("Used ContentOffset");
            extra_offset = content_offset;
        }
    }
    
    fs.seek(toc_offset);
    utf_table files(fs);

    for (uint32_t i = 0; i < files.rows(); ++i) {
        spdlog::info("File: {} ({} bytes -> {} bytes)",
            files.get<std::string>(i, "FileName"),
            files.get<uint32_t>(i, "FileSize"),
            files.get<uint32_t>(i, "ExtractSize")
        );
    }
}
