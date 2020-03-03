#pragma once

#include <Windows.h>

#include <CommCtrl.h>
#include <commoncontrols.h>
#include <Uxtheme.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>

#include <Windowsx.h>
#include <comip.h>

#include <wincodec.h>

template <typename Interface>
using com_ptr = _com_ptr_t<_com_IIID<Interface, &__uuidof(Interface)>>;

template <typename Interface>
class com_interface {
    protected:
    com_ptr<Interface> com_object { };

    public:
    explicit com_interface() = default;
    explicit com_interface(const com_ptr<Interface>& ptr) : com_object { ptr } {}
    Interface* object() { return com_object; }
    const Interface* object() const { return com_object; }
};
