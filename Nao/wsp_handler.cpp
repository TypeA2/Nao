#include "wsp_handler.h"

#include "file_handler_factory.h"
#include "binary_stream.h"
#include "utils.h"
#include "frameworks.h"
#include "partial_file_streambuf.h"
#include "riff.h"

wsp_handler::wsp_handler(const istream_ptr& stream, const std::string& path)
    : file_handler(stream, path), item_file_handler(stream, path) {
    while (!stream->eof()) {
        wwriff_file f;
        f.offset = stream->tellg();

        std::string fcc(4, '\0');
        stream->read(fcc);

        if (fcc == "RIFF") {
            f.size = stream->read<uint32_t>() + 8i64;

            stream->read(fcc);
            ASSERT(fcc == "WAVE");
            _m_riff.push_back(f);

            stream->seekg(f.size, std::ios::cur);
        }

        stream->ignore(std::numeric_limits<std::streamsize>::max(), 'R');

        if (!stream->eof()) {
            stream->rseek(-1);
        }
    }

    items.reserve(_m_riff.size());

    std::streamsize name_width = std::streamsize(log10(_m_riff.size()) + 1);

    size_t i = 0;

    SHFILEINFOW finfo_wem {};
    DWORD_PTR hr = SHGetFileInfoW(L".wem", FILE_ATTRIBUTE_NORMAL, &finfo_wem, sizeof(finfo_wem),
        SHGFI_TYPENAME | SHGFI_ICON | SHGFI_ICONLOCATION | SHGFI_ADDOVERLAYS | SHGFI_USEFILEATTRIBUTES);
    ASSERT(hr != 0);

    SHFILEINFOW finfo_wav {};
    hr = SHGetFileInfoW(L".wav", FILE_ATTRIBUTE_NORMAL, &finfo_wav, sizeof(finfo_wav),
        SHGFI_TYPENAME | SHGFI_ICON | SHGFI_ICONLOCATION | SHGFI_ADDOVERLAYS | SHGFI_USEFILEATTRIBUTES);
    ASSERT(hr != 0);

    std::string filename = std::filesystem::path(path).stem().string();

    for (const auto& wwriff : _m_riff) {
        std::stringstream ss;
        ss << filename << "_" << std::setfill('0') << std::setw(name_width) << i++;

        stream->seekg(wwriff.offset + 12);
        riff_header hdr;
        stream->read(&hdr, sizeof(hdr));
        ASSERT(std::string(hdr.header, 4) == "fmt ");

        fmt_chunk fmt;
        stream->read(&fmt, sizeof(fmt));
        ASSERT(stream->gcount() == sizeof(fmt));

        std::reference_wrapper<SHFILEINFOW> finfo { finfo_wem };
        switch (fmt.format) {
            case 0xFFFF:
                ss << ".wem";
                break;

            case 0xFFFE:
                finfo = finfo_wav;
                ss << ".wav";
                break;

            default: ASSERT(false);
        }

        items.push_back(item_data {
            .handler = this,
            .name    = ss.str(),
            .type    = utils::utf8(finfo.get().szTypeName),
            .size    = wwriff.size,
            .icon    = finfo.get().iIcon,
            .stream  = std::make_shared<binary_istream>(std::make_unique<partial_file_streambuf>(stream, wwriff.offset, wwriff.size)),
            .data    = std::make_shared<wwriff_file>(wwriff)
            });
    }
}

file_handler_tag wsp_handler::tag() const {
    return TAG_ITEMS;
}

static file_handler_ptr create(const istream_ptr& stream, const std::string& path) {
    return std::make_shared<wsp_handler>(stream, path);
}

static bool supports(const istream_ptr& stream, const std::string& path) {
    if (path.substr(path.size() - 4) == ".wsp") {

        std::string fcc(4, '\0');
        stream->read(fcc);
        stream->seekg(-4, std::ios::cur);

        if (fcc == "RIFF") {
            return true;
        }
    }

    return false;
}

[[maybe_unused]] static size_t id = file_handler_factory::register_class({
    .tag = TAG_ITEMS,
    .creator = create,
    .supports = supports,
    .name = "wsp"
});
