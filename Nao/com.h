#pragma once

#include <comip.h>

#include <CommCtrl.h>

template <typename Interface>
using com_ptr = _com_ptr_t<_com_IIID<Interface, &__uuidof(Interface)>>;

namespace com {
    template <typename Interface>
    class com_interface {
        protected:
        com_ptr<Interface> com_object { };

        public:
        explicit com_interface() = default;
        explicit com_interface(const com_ptr<Interface>& ptr) : com_object { ptr } {}
        explicit com_interface(Interface* ptr, bool addref) : com_interface(com_ptr<Interface> { ptr, addref && ptr != nullptr } ) { }
        Interface* object() { return com_object; }
        Interface* object() const { return com_object; }
        Interface** operator&() { return &com_object; }

        operator bool() const { return com_object; }
    };

    inline namespace raii {
        class com_wrapper {
            public:
            com_wrapper(DWORD flags = COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            ~com_wrapper();
        };
    }
}
