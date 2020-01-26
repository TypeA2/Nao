#pragma once

#include "item_provider_factory.h"
#include "item_data.h"
#include "ui_element.h"

#include <string>
#include <memory>

class item_provider {
    public:

    using file_stream = item_provider_factory::stream;
    //using preview_type = data_model::preview_type;

    virtual ~item_provider() = default;

    virtual size_t count() const;
    
    virtual item_data& data(size_t index);
    virtual const item_data& data(size_t index) const;

    const std::string& get_name() const;
    const file_stream& get_stream() const;

    //virtual preview_type preview() const;

    virtual std::shared_ptr<ui_element>
        preview_element(const std::shared_ptr<ui_element>& parent) const;

    template <typename T>
    std::enable_if_t<std::is_base_of_v<ui_element, T>, std::shared_ptr<T>>
        preview_element(const std::shared_ptr<ui_element>& parent) const {
        return std::dynamic_pointer_cast<T>(preview_element(parent));
    }

    virtual file_stream stream() const;

    protected:
    item_provider(file_stream file, std::string name, data_model& model);

    file_stream file;
    std::string name;
    data_model& model;
};
