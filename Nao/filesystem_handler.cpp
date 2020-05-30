#include "filesystem_handler.h"

#include "file_handler_factory.h"

#include "frameworks.h"

#include "utils.h"
#include "filesystem_utils.h"

#include "binary_stream.h"

#include <nao/strings.h>

filesystem_handler::filesystem_handler(const std::string& path)
    : file_handler(nullptr, path), item_file_handler(nullptr, path) {
    if (path == "\\") {
        // Devices list

        SHFILEINFOW finfo { };
        for (auto [letter, name, icon, total, free] : fs_utils::drive_list()) {

            ASSERT(SHGetFileInfoW(nao::string(std::string { letter, ':', '\\'}).wide().c_str(), 0, &finfo, sizeof(finfo), SHGFI_TYPENAME));

            std::stringstream ss;
            ss << nao::bytes(total).c_str() << " ("
                << nao::bytes(free).c_str() << " free)";

            items.push_back({
                .handler      = this,
                .name         = name,
                .type         = nao::wstring(finfo.szTypeName).narrow().c_str(),
                .size         = free,
                .size_str     = ss.str(),
                .icon         = icon,
                .drive        = true,
                .drive_letter = letter
                });
        }
    } else {
        // File list

        SHFILEINFOW finfo { };
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path)) {
            fs_utils::file_info info(entry);

            if (info.invalid() || info.system() || info.hidden()) {
                continue;
            }

            if (entry.path() == "." || entry.path() == "..") {
                continue;
            }

            ASSERT(SHGetFileInfoW(entry.path().c_str(), 0, &finfo, sizeof(finfo),
                SHGFI_TYPENAME | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX));

            items.push_back({
                .handler = this,
                .name    = entry.path().filename().string(),
                .type    = nao::wstring(finfo.szTypeName).narrow().c_str(),
                .size    = entry.is_directory() ? 0 : static_cast<std::streamsize>(entry.file_size()),
                .icon    = finfo.iIcon,
                .dir     = entry.is_directory(),
                .stream  = entry.is_directory() ? nullptr : std::make_shared<binary_istream>(entry.path())
            });
        }
    }
}

file_handler_tag filesystem_handler::tag() const {
    return TAG_ITEMS;
}

static file_handler_ptr create(const istream_ptr&, const std::string& path) {
    return std::make_shared<filesystem_handler>(path);
}

static bool supports(const istream_ptr&, const std::string& path) {
    fs_utils::file_info finfo(path);

    return !finfo.invalid() && finfo.directory();
}

[[maybe_unused]] static size_t id = file_handler_factory::register_class({
    .tag = TAG_ITEMS,
    .creator = create,
    .supports = supports,
    .name = "filesystem"
});


