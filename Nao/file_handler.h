#pragma once

#include "item_data.h"

#include "pcm_provider.h"
#include "image_provider.h"

enum file_handler_tag : uintmax_t {
    TAG_FILE  = 0b0000,
    TAG_ITEMS = 0b0001,
    TAG_PCM   = 0b0010,
    TAG_IMAGE = 0b0100,
    TAG_AV    = 0b1000
};

file_handler_tag operator|(file_handler_tag left, file_handler_tag right) noexcept;
file_handler_tag operator&(file_handler_tag left, file_handler_tag right) noexcept;

class file_handler;
class pcm_file_handler;
class item_file_handler;
class image_file_handler;
class av_file_handler;

template <file_handler_tag> struct file_handler_type { };
template <> struct file_handler_type<TAG_FILE>  { using type = file_handler;       };
template <> struct file_handler_type<TAG_ITEMS> { using type = item_file_handler;  };
template <> struct file_handler_type<TAG_PCM>   { using type = pcm_file_handler;   };
template <> struct file_handler_type<TAG_IMAGE> { using type = image_file_handler; };
template <> struct file_handler_type<TAG_AV>    { using type = av_file_handler;    };

template <file_handler_tag tag>
using file_handler_t = typename file_handler_type<tag>::type;

class file_handler {
    public:
    template <file_handler_tag tag>
    file_handler_t<tag>* query() {
        if (!(this->tag() & tag)) {
            return nullptr;
        }

        return dynamic_cast<file_handler_t<tag>*>(this);
    }

    template <file_handler_tag tag>
    file_handler_t<tag>* query() const {
        if (!(this->tag() & tag)) {
            return nullptr;
        }

        return dynamic_cast<const file_handler_t<tag>*>(this);
    }

    template <file_handler_tag tag, std::derived_from<file_handler> Src>
    static std::shared_ptr<file_handler_t<tag>> query(const std::shared_ptr<Src>& ptr) {
        if (!(ptr->tag() & tag)) {
            return nullptr;
        }

        return std::dynamic_pointer_cast<file_handler_t<tag>>(ptr);
    }

    file_handler(const istream_ptr& stream, const std::string& path);

    virtual file_handler_tag tag() const = 0;

    virtual ~file_handler();

    const istream_ptr& get_stream() const;
    const std::string& get_path() const;

    protected:

    const istream_ptr stream;
    const std::string path;
};

using file_handler_ptr = std::shared_ptr<file_handler>;

class item_file_handler : public virtual file_handler {
    public:
    using file_handler::file_handler;
    virtual ~item_file_handler() = default;

    // Number of items to display in this item
    size_t count() const;

    // Access item data
    item_data& data(size_t index);
    const std::vector<item_data>& data() const;

    protected:
    std::vector<item_data> items;
};

using item_file_handler_ptr = std::shared_ptr<item_file_handler>;

class pcm_file_handler : public virtual file_handler {
    public:
    using file_handler::file_handler;
    virtual ~pcm_file_handler() = default;

    virtual pcm_provider_ptr make_provider() = 0;
};

using pcm_file_handler_ptr = std::shared_ptr<pcm_file_handler>;

class image_file_handler : public virtual file_handler {
    public:
    using file_handler::file_handler;
    virtual ~image_file_handler() = default;

    virtual image_provider_ptr make_provider() = 0;
};

using image_file_handler_ptr = std::shared_ptr<image_file_handler>;

class av_file_handler : public virtual file_handler {
    public:
    using file_handler::file_handler;
    virtual ~av_file_handler() = default;
};

using av_file_handler_ptr = std::shared_ptr<av_file_handler>;
