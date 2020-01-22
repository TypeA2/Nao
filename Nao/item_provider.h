#pragma once

#include "item_provider_factory.h"

#include "binary_stream.h"

#include <string>
#include <variant>
#include <memory>

class data_model;

class item_provider : protected binary_stream {
    public:
    using stream = item_provider_factory::stream;

    // Returned item data
    struct item_data {
        std::string name;
        std::string type;

        int64_t size {};
        std::string size_str;

        double compression {};

        int icon {};

        bool dir {};
        bool drive {};

        char drive_letter {};

        stream stream;

        std::shared_ptr<void> data;
    };

    item_provider(stream file, std::string name, data_model& model);
    
    virtual ~item_provider() = default;

    virtual size_t count() const = 0;
    
    virtual item_data& data(size_t index) = 0;
    virtual const item_data& data(size_t index) const;

    const std::string& get_name() const;

    protected:
    binary_stream& file;
    std::string name;
    data_model& model;
};
