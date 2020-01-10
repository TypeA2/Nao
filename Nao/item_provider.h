#pragma once

#include <string>

class item_provider {
    public:
    item_provider() = default;
    
    virtual ~item_provider() = 0;

    virtual size_t count() const = 0;
    
    virtual std::wstring& name(size_t index) = 0;
    virtual const std::wstring& name(size_t index) const;
    
    virtual int64_t size(size_t index) const = 0;
    
    virtual std::wstring& type(size_t index) = 0;
    virtual const std::wstring& type(size_t index) const;
    
    virtual double compression(size_t index) const = 0;
    
    virtual int icon(size_t index) const = 0;

    virtual bool dir(size_t index) const = 0;
};
