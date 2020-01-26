#pragma once

#include "item_provider.h"

class wav_provider : public item_provider {
    public:
    wav_provider(const file_stream& stream,
        const std::string& path, data_model& model);

    ~wav_provider() override;

    //preview_type preview() const override;
    std::shared_ptr<ui_element> preview_element(const std::shared_ptr<ui_element>& parent) const override;

    private:

    static item_provider* _create(const file_stream& file,
        const std::string& name, data_model& model);
    static size_t _id;
};