#pragma once

#include <string>

class data_model;

class item_provider {
    public:
    // Returned item data
    struct item_data {
        std::wstring name;
        std::wstring type;

        int64_t size {};
        std::wstring size_str;

        double compression {};

        int icon {};

        bool dir {};
        bool drive {};

        wchar_t drive_letter {};
    };

    item_provider(std::string name, data_model& model);
    
    virtual ~item_provider() = default;

    virtual size_t count() const = 0;
    
    virtual item_data& data(size_t index) = 0;
    virtual const item_data& data(size_t index) const;

    const std::string& get_name() const;

    protected:
    std::string name;
    data_model& model;
};
