#include "file_handler_factory.h"

#include "binary_stream.h"

#include "utils.h"

size_t file_handler_factory::register_class(const factory_entry& entry) {
    _registered_classes().emplace_back(factory_registry(entry, _next_id()));

    utils::coutln("[ITEM FACTORY] Registered class", entry.name, "with ID", _next_id());

    return _next_id()++;
}

file_handler_ptr file_handler_factory::create(size_t id, const istream_ptr& stream, const std::string& path) {
    ASSERT(id < _next_id());
    
    if (stream != nullptr) {
        stream->seekg(0);
    }

    return _registered_classes()[id].creator(stream, path);
}

file_handler_ptr file_handler_factory::create(const std::string& name, const istream_ptr& stream, const std::string& path) {
    if (stream != nullptr) {
        stream->seekg(0);
    }

    if (auto it = std::find_if(_registered_classes().begin(),
        _registered_classes().end(),
        [name](const factory_registry& reg) { return reg.name == name; });
        it != _registered_classes().end()) {
        return it->creator(stream, path);
    }

    return nullptr;
}

file_handler_ptr file_handler_factory::create(const istream_ptr& stream, const std::string& path) {
    for (const factory_registry& reg : _registered_classes()) {
        // Should support this setup
        bool provider = reg.supports(stream, path);

        if (stream != nullptr) {
            stream->seekg(0);
        }

        if (!provider) {
            continue;
        }

        auto p = reg.creator(stream, path);

        if (p != nullptr) {
            return p;
        }
    }

    return nullptr;
}

size_t file_handler_factory::supports(const istream_ptr& stream, const std::string& path) {
    for (const factory_registry& reg : _registered_classes()) {
        // Should support this setup
        bool supported = reg.supports(stream, path);

        if (stream != nullptr) {
            stream->seekg(0);
        }

        if (supported) {
            return reg.id;
        }
    }

    return npos;
}

size_t file_handler_factory::supports(const istream_ptr& stream, const std::string& path, file_handler_tag& tag) {
    if (stream != nullptr) {
        stream->seekg(0);
    }

    size_t id = supports(stream, path);

    if (id != npos) {
        tag = _registered_classes()[id].tag;
    }

    return id;
}


file_handler_factory::factory_registry::factory_registry(const factory_entry& entry, size_t id)
    : factory_entry(entry), id(id) {
    
}

file_handler_factory::factory_registry::factory_registry(factory_registry&& other) noexcept
    : factory_entry(std::move(other)), id { other.id }{
    
}

std::vector<file_handler_factory::factory_registry>& file_handler_factory::_registered_classes() {
    static std::vector<factory_registry> registry;

    return registry;
}

size_t& file_handler_factory::_next_id() {
    static size_t next = 0;
    return next;
}
