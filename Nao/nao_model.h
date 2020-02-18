#pragma once

#include "file_handler.h"

#include <deque>

class nao_view;
class nao_controller;

class provider_for_wrapper;

class nao_model {
    std::string _m_path;
    public:
    explicit nao_model(nao_view& view, nao_controller& controller);
    nao_model() = delete;

    void setup();

    void move_to(std::string path);

    // Move up one level, deleting the current provider
    void move_up();

    // Move down to the given element, creating a provider if needed
    void move_down(item_data* to);

    // Try to fetch a preview for the given item, notify the controller if one is found.
    void fetch_preview(item_data* item);

    // Free up the current preview provider
    void clear_preview();

    const std::string& current_path() const;
    const item_file_handler_ptr& current_provider() const;
    const file_handler_ptr& preview_provider() const;
    const item_file_handler_ptr& parent_provider() const;

    // Whether we can "open" the given item
    bool can_open(item_data* data);

    private:
    void _create_tree(const std::string& to);

    file_handler_ptr _provider_for(std::string path, bool* result = nullptr, file_handler_tag* tag = nullptr);

    protected:
    nao_view& view;
    nao_controller& controller;

    private:
    using istream_type = file_handler::istream_type;

    std::deque<item_file_handler_ptr> _m_tree;
    file_handler_ptr _m_preview_provider;
};
