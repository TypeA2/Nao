#pragma once

#include "file_handler.h"

#include <functional>

// Function signature for an object that creates a file_handler
using create_func = std::function<file_handler_ptr(const item_data::istream_type&, const std::string&)>;

// Can the provider be created for this setup
using supports_func = std::function<bool(const item_data::istream_type&, const std::string&)>;

struct factory_entry {
    file_handler_tag tag;
    create_func creator;
    supports_func supports = [](const item_data::istream_type&, const std::string&) { return false; };
    std::string name = "unknown";
};

class file_handler_factory {
    public:
    using istream_type = item_data::istream_type;

    static constexpr size_t npos = -1;

    // Register a class's creator functor (and optionally non-unique name), return it's unique id
    static size_t register_class(const factory_entry& entry);

    // Create by id
    static file_handler_ptr create(size_t id, const istream_type& stream, const std::string& path);
    // Create by name
    static file_handler_ptr create(const std::string& name, const istream_type& stream, const std::string& path);

    // Find and create if found, else return null
    static file_handler_ptr create(const istream_type& stream, const std::string& path);

    // Return an ID if the setup is supported, else npos
    static size_t supports(const istream_type& stream, const std::string& path);

    // Also get the handler's tag
    static size_t supports(const istream_type& stream, const std::string& path, file_handler_tag& tag);

    private:
    file_handler_factory() = default;

    struct factory_registry : factory_entry {
        explicit factory_registry(const factory_entry& entry, size_t id);
        factory_registry(const factory_registry& other) = default;
        factory_registry(factory_registry&& other) noexcept;

        size_t id;
    };

    static std::vector<factory_registry>& _registered_classes();
    static size_t& _next_id();
};

