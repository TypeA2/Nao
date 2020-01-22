#pragma once

#include "item_provider.h"

#include <vector>

class wsp_provider : public item_provider {
    public:
    wsp_provider(const stream& stream,
        const std::string& path, data_model& model);

    ~wsp_provider() override;

    size_t count() const override;
    item_data& data(size_t index) override;

    private:
    // Represents a sequenced file
    struct wwriff_file {
        int64_t size;
        int64_t offset;
    };

    void _populate();

    std::vector<item_data> _m_contents;
    std::vector<wwriff_file> _m_riff_files;

    static item_provider* _create(const stream& file,
        const std::string& name, data_model& model);
    static size_t _id;
};
