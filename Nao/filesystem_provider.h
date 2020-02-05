#pragma once

#include "item_provider.h"

class filesystem_provider : public item_provider {
    public:
    explicit filesystem_provider(const std::string& path);

    preview_element_type preview_type() const override;
    std::unique_ptr<preview> make_preview(nao_view& view) override;
};
