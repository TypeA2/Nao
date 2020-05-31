/*  This file is part of libnao-util.

    libnao-util is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libnao-util is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libnao-util.  If not, see <https://www.gnu.org/licenses/>.   */
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "nao/steam.h"

#include "nao/windows_min.h"
#include "nao/strings.h"
#include "nao/logging.h"
#include "nao/vector.h"

#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <sstream>


// TODO in-house VDF parser
#include "vdf_parser.h"

namespace {
    // Simple auto-destructor
    struct hkey_lock {
        HKEY key = nullptr;
        ~hkey_lock() {
            if (key) {
                RegCloseKey(key);
            }
        }
    };
}

namespace nao::steam {
    expected<string> path() {
        hkey_lock key;
        LSTATUS status = RegOpenKeyExW(HKEY_CURRENT_USER,
            L"Software\\Valve\\Steam", 0, KEY_READ, &key.key);

        if (status != ERROR_SUCCESS) {
            return { except::runtime_error, __FUNCTION__": Key opening failed" };
        }

        DWORD type, size;
        status = RegQueryValueExW(key.key,
            L"SteamPath", nullptr,
            &type, nullptr, &size);

        if (status != ERROR_SUCCESS) {
            return { except::runtime_error, __FUNCTION__": Key length retrieval failed" };
        }

        if (type != REG_SZ) {
            return { except::runtime_error, __FUNCTION__": Expected REG_SZ" };
        }

        if (size == 0) {
            return { except::runtime_error, __FUNCTION__": No size" };
        }

        // Size includes null terminator
        wstring str((size / sizeof(WCHAR)) - 1, 0);
        status = RegQueryValueExW(key.key, L"SteamPath",
            nullptr, nullptr, reinterpret_cast<uint8_t*>(str.data()), &size);

        if (status != ERROR_SUCCESS) {
            return { except::runtime_error, __FUNCTION__": Key retrieval failed" };
        }

        return str.narrow();
    }

    vector<string> install_folders() {
        auto p = path() + "\\SteamApps\\libraryfolders.vdf";
        auto vdf_path = std::filesystem::path {
            p.c_str() }.lexically_normal();

        std::ifstream in { vdf_path };
        tyti::vdf::object root = tyti::vdf::read(in);

        //in.seekg(0);
        //in.clear();
        //auto root1 = vdf::parse(in);
        //std::stringstream ss;
        //root1.print(ss);
        //logging::coutln(ss.str());

        vector<string> folders { root.attribs.size() - 1 };

        folders[0] = path();

        for (size_t i = 1; i < folders.size(); ++i) {
            folders[i] = root.attribs.at(std::to_string(i));
        }

        std::transform(folders.begin(), folders.end(), folders.begin(), [](auto& path) {
            return std::filesystem::absolute(path.c_str()).lexically_normal().string();
            });

        return folders;
    }


    expected<string> game_path(string_view game) {
        for (const string& dir : install_folders()) {
            try {
                std::filesystem::directory_iterator it { (dir + "\\SteamApps\\common").c_str() };
                for (const auto& entry : it) {
                    if (is_directory(entry) && entry.path().filename() == game.data()) {
                        return absolute(entry).lexically_normal().string();
                    }
                }
            }  catch (const std::filesystem::filesystem_error&) {
                // Directory does not exist
            }
        }

        return { except::runtime_error, string { __FUNCTION__": Path not found:" } + game };
    }

    namespace vdf {
        class object::opaque {
            public:
            std::string name;
            std::unordered_map<std::string, std::string> attributes;
            std::unordered_map<std::string, object> children;
        };

        object::object() {
            d = new opaque;
        }

        object::~object() {
            delete d;
        }

        std::string& object::name() {
            return d->name;
        }

        std::unordered_map<std::string, std::string>& object::attributes() {
            return d->attributes;
        }

        std::unordered_map<std::string, object>& object::children() {
            return d->children;
        }

        void object::print(std::ostream& os, size_t indent) const {
            std::string indent_child(indent + 4, ' ');
            os << std::string(indent, ' ') << std::quoted(d->name) << "\n{\n";
            for (const auto& [key, val] : d->attributes) {
                os << std::quoted(key) << ' ' << std::quoted(val) << '\n';
            }

            for (const auto& [key, obj] : d->children) {
                obj.print(os, indent + 4);
            }

            os << std::string(indent, ' ') << "}\n";
        }


        static void check_eof(std::istream& in) {
            if (in.eof() || !in) {
                throw std::runtime_error(__FUNCTION__ ": unexpected EOF");
            }
        }

        static void consume_whitespace(std::istream& in) {
            while (in.peek() == ' ' || in.peek() == '\n') {
                in.get();
                check_eof(in);
            }
        }

        static std::string next_token(std::istream& in) {
            consume_whitespace(in);

            char peeked = in.peek();
            switch (peeked) {
                case '{': return "{";
                case '}': return "}";
                case '\"': {
                    std::string token;
                    in >> std::quoted(token);

                    return token;
                }

                default: throw std::runtime_error(__FUNCTION__ ": invalid next token");
            }
            
        }

        static void parse_recursive(object& parent, std::istream& in) {
            std::string name = next_token(in);

            std::string next;
            do {
                next = next_token(in);

                if (next == "{") {
                    // sub-block
                    parent.children()[name].name() = name;
                    parse_recursive(parent.children()[name], in);
                } else {
                    // Key-value pair
                    parent.attributes()[name] = next;
                }
            } while (next != "}");
        }

        object parse(std::istream& in) {
            if (!in) {
                throw std::invalid_argument(__FUNCTION__ ": stream not open");
            }

            object root;
            parse_recursive(root, in);

            return root;
        }

        

    }
}
