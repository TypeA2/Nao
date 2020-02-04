#include "filesystem_provider.h"

#include "item_provider_factory.h"

#include "frameworks.h"

#include <filesystem>

#include "file_info.h"
#include "utils.h"
#include "drive_list.h"
#include "binary_stream.h"

filesystem_provider::filesystem_provider(const std::string& path) : item_provider(nullptr, path){
    utils::coutln("[FILESYSTEM] creating for", path);
    
    if (path == "\\") {
        // Devices list

        SHFILEINFOW finfo { };
        for (auto [letter, name, icon, total, free] : drive_list()) {

            ASSERT(SHGetFileInfoW(utils::utf16({ letter, ':', '\\'}).c_str(), 0, &finfo, sizeof(finfo), SHGFI_TYPENAME));

            std::stringstream ss;
            ss << utils::bytes(total) << " ("
                << utils::bytes(free) << " free)";
            
            items.push_back(item_data {
                .provider     = this,
                .name         = name,
                .type         = utils::utf8(finfo.szTypeName),
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
            file_info info(entry);

            if (info.invalid() || info.system() || info.hidden()) {
                continue;
            }

            if (entry.path() == "." || entry.path() == "..") {
                continue;
            }

            ASSERT(SHGetFileInfoW(entry.path().c_str(), 0, &finfo, sizeof(finfo),
                SHGFI_TYPENAME | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX));

            items.push_back({
                .provider     = this,
                .name   = entry.path().filename().string(),
                .type   = utils::utf8(finfo.szTypeName),
                .size   = entry.is_directory() ? 0 : entry.file_size(),
                .icon   = finfo.iIcon,
                .dir    = entry.is_directory(),
                .stream = entry.is_directory() ? nullptr : std::make_shared<binary_istream>(entry.path())
            });
        }
    }
}

item_provider_ptr filesystem_provider::_create(const istream_type& stream, const std::string& path) {
    file_info finfo(path);

    if (!finfo.invalid() && finfo.directory()) {
        return std::make_shared<filesystem_provider>(path);
    }

    return nullptr;
}

size_t filesystem_provider::_id = item_provider_factory::register_class(_create, "filesystem");

