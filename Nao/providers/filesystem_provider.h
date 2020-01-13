#pragma once

#include "item_provider.h"

#include <vector>

class filesystem_provider : public item_provider {
    public:
    filesystem_provider(const std::string& path, data_model& model);
    ~filesystem_provider() override = default;
    
    size_t count() const override;
    
    item_data& data(size_t index) override;
    
    private:
    void _populate();

    // Current filesystem files and folders
    std::vector<item_data> _m_contents;
    
    std::wstring _m_path;
    
    // Factory registration
    static item_provider* _create(std::istream& file,
        const std::string& name, data_model& model);
    static size_t _id;
};
