#include "wsp_provider.h"
#include "item_provider_factory.h"
#include "utils.h"
#include "data_model.h"

#include <iomanip>

wsp_provider::wsp_provider(const stream& stream,
    const std::string& path, data_model& model)
    : item_provider(stream, path + '\\', model) {
    utils::coutln("[WSP] creating for", path);
    _populate();
}

wsp_provider::~wsp_provider() {
    utils::coutln("[WSP] deleting for", name);
}

size_t wsp_provider::count() const {
    return _m_contents.size();
}

item_provider::item_data& wsp_provider::data(size_t index) {
    return _m_contents[index];
}

void wsp_provider::_populate() {
    while (!file->eof()) {
        wwriff_file f;
        f.offset = file->tellg();

        std::string fcc(4, '\0');
        file->read(fcc);

        if (fcc != "RIFF") {
            utils::coutln(file->tellg(), fcc);
            MessageBoxW(model.handle(), L"Invalid RIFF signature",
                L"Error", MB_ICONEXCLAMATION | MB_OK);
            return;
        }

        f.size = file->read<uint32_t>() + 8i64;

        _m_riff_files.push_back(f);

        file->rseek(f.size);
        file->ignore(std::numeric_limits<std::streamsize>::max(), 'R');

        if (!file->eof()) {
            file->rseek(-1);
        }
    }

    _m_contents.reserve(_m_riff_files.size());

    std::streamsize name_width = std::streamsize(log10(_m_riff_files.size()) + 1);

    size_t i = 0;

    SHFILEINFOW finfo {};

    for (const auto& wwriff : _m_riff_files) {
        DWORD_PTR hr = SHGetFileInfoW(L".wem", FILE_ATTRIBUTE_NORMAL, &finfo, sizeof(finfo),
            SHGFI_TYPENAME | SHGFI_ICON | SHGFI_ICONLOCATION | SHGFI_ADDOVERLAYS | SHGFI_USEFILEATTRIBUTES);

        if (hr == 0) {
            MessageBoxW(model.handle(),
                L"Failed to get file info", L"Error", MB_OK | MB_ICONEXCLAMATION);
            continue;
        }

        std::stringstream ss;
        ss << std::setfill('0') << std::setw(name_width) << i++;

        _m_contents.push_back({
            .name     = ss.str() + ".wem",
            .type     = utils::utf8(finfo.szTypeName),
            .size     = wwriff.size,
            .size_str = utils::bytes(wwriff.size),
            .icon     = finfo.iIcon,
            .stream   = nullptr,
            .data     = std::make_shared<wwriff_file>(wwriff)
            });

    }
}

item_provider* wsp_provider::_create(const stream& file, const std::string& name, data_model& model) {
    if (name.substr(name.size() - 4) == ".wsp") {
        
        std::string fcc(4, '\0');
        file->read(fcc);
        file->seekg(-4, std::ios::cur);

        if (fcc == "RIFF") {
            return new wsp_provider(file, name, model);
        }
    }

    return nullptr;
}

size_t wsp_provider::_id = item_provider_factory::register_class(_create);
