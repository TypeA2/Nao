/**
 *  This file is part of libnao-ui.
 *
 *  libnao-ui is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libnao-ui is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libnao-ui.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "file_dialog.h"

#include <window.h>
#include <libnao/util/encoding.h>

#include <stdexcept>

#include <comdef.h>
#include <comip.h>
#include <shobjidl_core.h>

_COM_SMARTPTR_TYPEDEF(IFileOpenDialog, __uuidof(IFileOpenDialog));
_COM_SMARTPTR_TYPEDEF(IShellItem, __uuidof(IShellItem));

std::optional<std::string> nao::ui::file_dialog::get_directory(window* parent) {
    IFileOpenDialogPtr dialog;
    
    if (FAILED(dialog.CreateInstance(CLSID_FileOpenDialog))) {
        throw std::runtime_error{ "Failed to create FileOpenDialog" };
    }

    FILEOPENDIALOGOPTIONS options;
    HRESULT hr = dialog->GetOptions(&options);
    if (FAILED(hr)) {
        throw std::runtime_error{ "Failed to get FileOpenDialog options" };
    }

    dialog->SetOptions(options | FOS_PICKFOLDERS);

    hr = dialog->Show(parent ? parent->handle() : nullptr);
    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
        // Nothing was selected
        return {};

    } else if (FAILED(hr)) {
        throw std::runtime_error{ "Failed to show file dialog" };
    }

    IShellItemPtr item;
    hr = dialog->GetResult(&item);
    if (FAILED(hr)) {
        throw std::runtime_error{ "Failed to get file dialog result" };
    }

    LPWSTR path;
    hr = item->GetDisplayName(SIGDN_FILESYSPATH, &path);
    if (FAILED(hr)) {
        throw std::runtime_error{ "Failed to get item display name" };
    }

    return wide_to_utf8(path);
}