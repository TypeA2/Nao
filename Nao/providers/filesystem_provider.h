#pragma once

#include "item_provider.h"

#include <vector>

class ui_element;

class filesystem_provider : public item_provider {
    public:
    explicit filesystem_provider(std::wstring path, ui_element* window);
    ~filesystem_provider() override = default;
    
    size_t count() const override;
    std::wstring& name(size_t index) override;
    int64_t size(size_t index) const override;
    std::wstring& size_str(size_t index) override;
    std::wstring& type(size_t index) override;
    double compression(size_t index) const override;
    int icon(size_t index) const override;
    bool dir(size_t index) const override;
    
    private:
    void _populate();

    // Current filesystem files and folders
    struct fs_entry {
        bool is_dir;
        std::wstring name;
        std::wstring type;
        int64_t size;
        std::wstring size_str;
        int icon_index;
    };
    std::vector<fs_entry> _m_contents;
    
    std::wstring _m_path;
    ui_element* _m_window;
    
    // Factory registration
    static item_provider* _create(std::istream& file, ui_element* window);
    static size_t _id;
};
