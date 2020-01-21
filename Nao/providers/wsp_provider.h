#pragma once

#include "item_provider.h"

#include <vector>

class wsp_provider : public item_provider {
    public:
    wsp_provider(const stream& stream,
        const std::string& path, data_model& model);

    ~wsp_provider() override = default;

    size_t count() const override;
    item_data& data(size_t index) override;

    private:
    void _populate();

    std::vector<item_data> _m_contents;

    static item_provider* _create(const stream& file,
        const std::string& name, data_model& model);
    static size_t _id;
};
