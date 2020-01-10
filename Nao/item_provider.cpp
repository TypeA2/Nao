#include "item_provider.h"

// Linker wants this
item_provider::~item_provider() {
    
}

const std::wstring& item_provider::name(size_t index) const {
    return const_cast<const item_provider*>(this)->name(index);
}

const std::wstring& item_provider::type(size_t index) const {
    return const_cast<const item_provider*>(this)->type(index);
}

