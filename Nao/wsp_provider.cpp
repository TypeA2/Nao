#include "wsp_provider.h"

#include "item_provider_factory.h"
#include "binary_stream.h"
#include "utils.h"
#include "preview.h"

wsp_provider::wsp_provider(const istream_type& stream, const std::string& path) : item_provider(stream, path) {
    utils::coutln("[WSP] creating for", path);

    while (!stream->eof()) {
        wwriff_file f;
        f.offset = stream->tellg();

        std::string fcc(4, '\0');
        stream->read(fcc);

        if (fcc != "RIFF") {
            utils::coutln("invalid fourcc at", stream->tellg(), fcc);
            return;
        }

        f.size = stream->read<uint32_t>() + 8i64;

        _m_riff.push_back(f);

        stream->rseek(f.size);
        stream->ignore(std::numeric_limits<std::streamsize>::max(), 'R');

        if (!stream->eof()) {
            stream->rseek(-1);
        }
    }

    items.reserve(_m_riff.size());

    std::streamsize name_width = std::streamsize(log10(_m_riff.size()) + 1);

    size_t i = 0;

    SHFILEINFOW finfo {};

    std::string filename = std::filesystem::path(path).stem().string();

    for (const auto& wwriff : _m_riff) {
        DWORD_PTR hr = SHGetFileInfoW(L".wem", FILE_ATTRIBUTE_NORMAL, &finfo, sizeof(finfo),
            SHGFI_TYPENAME | SHGFI_ICON | SHGFI_ICONLOCATION | SHGFI_ADDOVERLAYS | SHGFI_USEFILEATTRIBUTES);

        if (hr == 0) {
            continue;
        }

        std::stringstream ss;
        ss << filename << "_" << std::setfill('0') << std::setw(name_width) << i++;

        items.push_back(item_data {
            .provider = this,
            .name     = ss.str() + ".wem",
            .type     = utils::utf8(finfo.szTypeName),
            .size     = wwriff.size,
            .icon     = finfo.iIcon,
            .stream   = nullptr,
            .data     = std::make_shared<wwriff_file>(wwriff)
            });

    }
}

preview_element_type wsp_provider::preview_type() const {
    return PREVIEW_LIST_VIEW;
}

std::unique_ptr<preview> wsp_provider::make_preview(nao_view& view) {
    return std::make_unique<list_view_preview>(view, this);
}

static item_provider_ptr create(const item_provider::istream_type& stream, const std::string& path) {
    return std::make_shared<wsp_provider>(stream, path);
}

static bool provide(const item_provider::istream_type& stream, const std::string& path) {
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

static size_t id = item_provider_factory::register_class({
    .creator = create,
    .provide = provide,
    .preview = provide,
    .name = "wsp"
});
