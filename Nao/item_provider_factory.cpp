#include "item_provider_factory.h"

#include "binary_stream.h"

#include "utils.h"

size_t item_provider_factory::register_class(const factory_entry& entry) {
    _registered_classes().emplace_back(factory_registry(entry, _next_id()));

    utils::coutln("[ITEM FACTORY] Registered class", entry.name, "with ID", _next_id());

    return _next_id()++;
}

item_provider_ptr item_provider_factory::create(size_t id, const istream_type& stream, const std::string& path) {
    ASSERT(id < _next_id());

    return _registered_classes()[id].creator(stream, path);
}

item_provider_ptr item_provider_factory::create(const std::string& name, const istream_type& stream, const std::string& path) {
    if (auto it = std::find_if(_registered_classes().begin(),
        _registered_classes().end(),
        [name](const factory_registry& reg) { return reg.name == name; });
        it != _registered_classes().end()) {
        return it->creator(stream, path);
    }

    return nullptr;
}

item_provider_ptr item_provider_factory::create(const istream_type& stream, const std::string& path) {
    const bool should_reset = !!stream;
    const auto start_pos = should_reset ? stream->tellg() : std::istream::pos_type(0);

    for (const factory_registry& reg : _registered_classes()) {
        // Should support this setup
        bool provider = reg.provide(stream, path);

        if (should_reset && stream->tellg() != start_pos) {
            stream->seekg(start_pos);
        }

        if (!provider) {
            continue;
        }


        auto p = reg.creator(stream, path);

        if (p != nullptr) {
            return p;
        }

        if (should_reset && stream->tellg() != start_pos) {
            stream->seekg(start_pos);
        }
    }

    return nullptr;
}

size_t item_provider_factory::provide(const istream_type& stream, const std::string& path) {
    const bool should_reset = !!stream;
    const auto start_pos = should_reset ? stream->tellg() : std::istream::pos_type(0);

    for (const factory_registry& reg : _registered_classes()) {
        // Should support this setup
        bool supported = reg.provide(stream, path);

        if (should_reset && stream->tellg() != start_pos) {
            stream->seekg(start_pos);
        }

        if (supported) {
            return reg.id;
        }
    }

    return npos;
}

size_t item_provider_factory::preview(const istream_type& stream, const std::string& path) {
    const bool should_reset = !!stream;
    const auto start_pos = should_reset ? stream->tellg() : std::istream::pos_type(0);

    for (const factory_registry& reg : _registered_classes()) {
        // Should support this setup
        bool supported = reg.preview(stream, path);

        if (should_reset && stream->tellg() != start_pos) {
            stream->seekg(start_pos);
        }

        if (supported) {
            return reg.id;
        }
    }

    return npos;
}

item_provider_factory::factory_registry::factory_registry(const factory_entry& entry, size_t id)
    : factory_entry(entry), id(id) {
    
}

item_provider_factory::factory_registry::factory_registry(factory_registry&& other) noexcept
    : factory_entry(std::move(other)), id { other.id }{
    
}

std::vector<item_provider_factory::factory_registry>& item_provider_factory::_registered_classes() {
    static std::vector<factory_registry> registry;

    return registry;
}

size_t& item_provider_factory::_next_id() {
    static size_t next = 0;
    return next;
}
