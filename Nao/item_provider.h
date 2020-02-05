#pragma once

#include "item_data.h"

#include <vector>

enum preview_element_type {
    PREVIEW_NONE,
    PREVIEW_LIST_VIEW,
    PREVIEW_PLAY_AUDIO
};

class preview;
class nao_view;

class item_provider {
    public:
    using istream_type = item_data::istream_type;

    virtual ~item_provider();

    virtual preview_element_type preview_type() const = 0;
    virtual std::unique_ptr<preview> make_preview(nao_view& view) = 0;

    // Number of items to display in this item
    size_t count() const;

    // Access item data
    item_data& data(size_t index);
    const std::vector<item_data>& data() const;

    const istream_type& get_stream() const;
    const std::string& get_path() const;

    protected:
    item_provider(const istream_type& stream, const std::string& path);

    std::vector<item_data> items;

    const istream_type stream;
    const std::string path;
};

using item_provider_ptr = std::shared_ptr<item_provider>;
