#pragma once

#include "item_provider.h"

#include <functional>

class item_provider_factory {
    public:
    using istream_type = item_data::istream_type;

    // Function signature for an object that creates an item_provider
    using create_func = std::function<item_provider_ptr(const istream_type&, const std::string&)>;

    // Register a class's creator functor (and optionally non-unique name), return it's unique id
    static size_t register_class(const create_func& creator, const std::string& name = "unknown");

    // Create by id
    static item_provider_ptr create(size_t id, const istream_type& stream, const std::string& path);
    // Create by name
    static item_provider_ptr create(const std::string& name, const istream_type& stream, const std::string& path);

    // Find and create if found, else return null
    static item_provider_ptr create(const istream_type& stream, const std::string& path);

    private:
    item_provider_factory() = default;

    struct factory_registry {
        create_func creator;
        std::string name;
        size_t id;
    };

    static std::vector<factory_registry>& _registered_classes();
    static size_t& _next_id();
};

