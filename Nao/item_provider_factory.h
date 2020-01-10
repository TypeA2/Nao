#pragma once

#include <vector>
#include <fstream>

class item_provider;
class main_window;

class item_provider_factory {
    public:
    using create_func = item_provider* (*)(std::istream&, main_window*);

    static size_t register_class(create_func creator);
    static item_provider* create(size_t id, std::istream& file,
        main_window* window);
    static item_provider* create(std::istream& file, main_window* window);

    private:
    item_provider_factory() = default;
    
    static std::vector<create_func> _registered_classes;
    static size_t _next_id;
};

