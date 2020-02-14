#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <CommCtrl.h>
#include <commoncontrols.h>
#include <Uxtheme.h>
#include <shellapi.h>
#include <ShlObj.h>

#include <Shlwapi.h>

#include <mfidl.h>
#include <mfapi.h>
#include <Mferror.h>
#include <mfmediaengine.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>

#include <endpointvolume.h>

#include <Windowsx.h>
#include <comip.h>

template <typename Interface>
using com_ptr = _com_ptr_t<_com_IIID<Interface, &__uuidof(Interface)>>;