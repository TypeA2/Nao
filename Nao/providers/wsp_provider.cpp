#include "wsp_provider.h"
#include "item_provider_factory.h"
#include "utils.h"
#include "data_model.h"

#include <iomanip>

wsp_provider::wsp_provider(const stream& stream,
    const std::string& path, data_model& model)
    : item_provider(stream, path, model) {
    _populate();
}


size_t wsp_provider::count() const {
    return _m_riff_files.size();
}

item_provider::item_data& wsp_provider::data(size_t index) {
    return _m_contents[index];
}

void wsp_provider::_populate() {
    while (!eof()) {
        wwriff_file f;
        f.offset = tellg();

        std::string fcc(4, '\0');
        read(fcc);

        if (fcc != "RIFF") {
            utils::coutln(tellg(), fcc);
            MessageBoxW(model.handle(), L"Invalid RIFF signature",
                L"Error", MB_ICONEXCLAMATION | MB_OK);
            return;
        }

        f.size = read<uint32_t>() + 8i64;

        _m_riff_files.push_back(f);

        rseek(f.size);
        ignore(std::numeric_limits<std::streamsize>::max(), 'R');

        if (!eof()) {
            rseek(-1);
        }
    }

    _m_contents.reserve(_m_riff_files.size());

    

    std::streamsize name_width = std::streamsize(log10(_m_riff_files.size()) + 1);

    size_t i = 0;

    for (const auto& wwriff : _m_riff_files) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(name_width) << i++;

        _m_contents.push_back({
            .name     = ss.str() + ".wem",
            .type     = "WEM audio",
            .size     = wwriff.size,
            .size_str = utils::bytes(wwriff.size),
            .stream   = nullptr,
            .data     = std::make_shared<wwriff_file>(wwriff)
            });

    }
}

item_provider* wsp_provider::_create(const stream& file, const std::string& name, data_model& model) {
    if (name.substr(name.size() - 4) == ".wsp") {
        
        std::string fcc(4, '\0');
        file->read(fcc.data(), fcc.size());
        file->seekg(-4, std::ios::cur);

        if (fcc == "RIFF") {
            return new wsp_provider(file, name, model);
        }
    }

    return nullptr;
}

size_t wsp_provider::_id = item_provider_factory::register_class(_create);
