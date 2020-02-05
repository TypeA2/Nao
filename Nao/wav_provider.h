#pragma once

#include "item_provider.h"

class wav_provider : public item_provider {
    public:
    wav_provider(const istream_type& stream, const std::string& path);

    ~wav_provider() override = default;

    preview_element_type preview_type() const override;
    std::unique_ptr<preview> make_preview(nao_view& view) override;
};
