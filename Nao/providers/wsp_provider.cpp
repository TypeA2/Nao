#include "wsp_provider.h"
#include "item_provider_factory.h"

wsp_provider::wsp_provider(const stream& stream,
    const std::string& path, data_model& model)
    : item_provider(stream, path, model) {
    _populate();
}


size_t wsp_provider::count() const {
    return 0;
}

item_provider::item_data& wsp_provider::data(size_t index) {
    return _m_contents[index];
}

void wsp_provider::_populate() {
    
}

item_provider* wsp_provider::_create(const stream& file, const std::string& name, data_model& model) {
    if (name.substr(name.size() - 4) == ".wsp") {
        std::string fcc(4, '\0');
        file->read(fcc.data(), std::size(fcc));
        file->seekg(-4, std::ios::cur);

        if (fcc == "RIFF") {
            return new wsp_provider(file, name, model);
        }
    }

    return nullptr;
}

size_t wsp_provider::_id = item_provider_factory::register_class(_create);
