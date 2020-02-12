#pragma once

#include "item_provider.h"

#include <functional>

// Function signature for an object that creates an item_provider
using create_func = std::function<item_provider_ptr(const item_data::istream_type&, const std::string&)>;

// Can the provider provide items for this setup
using provider_func = std::function<bool(const item_data::istream_type&, const std::string&)>;

// Function signature for an object that checks if an item has a preview
using preview_func = std::function<bool(const item_data::istream_type&, const std::string&)>;

struct factory_entry {
    create_func creator;
    provider_func provide = [](const item_data::istream_type&, const std::string&) { return false; };
    preview_func preview = [](const item_data::istream_type&, const std::string&) { return false; };
    std::string name = "unknown";
};

class item_provider_factory {
    public:
    using istream_type = item_data::istream_type;

    static constexpr size_t npos = -1;

    // Register a class's creator functor (and optionally non-unique name), return it's unique id
    static size_t register_class(const factory_entry& entry);

    // Create by id
    static item_provider_ptr create(size_t id, const istream_type& stream, const std::string& path);
    // Create by name
    static item_provider_ptr create(const std::string& name, const istream_type& stream, const std::string& path);

    // Find and create if found, else return null
    static item_provider_ptr create(const istream_type& stream, const std::string& path);

    // Return an ID if the setup is supported, else npos
    static size_t provide(const istream_type& stream, const std::string& path);

    // Return an ID if the setup has a provider, else npos
    static size_t preview(const istream_type& stream, const std::string& path);

    private:
    item_provider_factory() = default;

    struct factory_registry : factory_entry {
        explicit factory_registry(const factory_entry& entry, size_t id);
        factory_registry(const factory_registry& other) = default;
        factory_registry(factory_registry&& other) noexcept;

        size_t id;
    };

    static std::vector<factory_registry>& _registered_classes();
    static size_t& _next_id();
};

