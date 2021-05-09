/**
 *  This file is part of naofs.
 *
 *  naofs is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  naofs is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with naofs.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "projection.h"

#include <fstream>

#include <Windows.h>
#include <comutil.h>

projection::projection(const path& source, const path& root)
    : _src{ canonical(source) }, _root{ canonical(root) } {
    _prepare_root();
}


void projection::_prepare_root() {
    
    if (!is_directory(_src)) {
        throw std::filesystem::filesystem_error{
            "source directory does not exist", _src,
            std::make_error_code(std::errc::no_such_file_or_directory)
        };
    }

    if (!is_directory(_root)) {
        // Root dir does not exist, create it
        if (std::error_code err; !create_directories(_root, err)) {
            throw std::filesystem::filesystem_error{
                "failed to create root dir", _root, err
            };
        }
    }

    // Both directories now exist

    GUID instance;
    if (auto instance_path = _root / instance_file; exists(instance_path)) {
        // Instance file exists, read data
        std::ifstream inst{ instance_path, std::ios::binary };
        inst.read(reinterpret_cast<char*>(&instance), sizeof(instance));

        if (inst.gcount() != sizeof(instance)) {
            // Failed to read entire GUID, abort
            throw std::filesystem::filesystem_error{
                "failed to read instance id from instance file", instance_path,
                std::make_error_code(std::errc::io_error)
            };
        }

        // Success
    } else {
        // New session ID required
        if (CoCreateGuid(&instance) != S_OK) {
            throw std::runtime_error{ "failed to create session guid" };
        }

        // Write it to the file
        std::ofstream inst{ instance_path, std::ios::binary };
        inst.write(reinterpret_cast<char*>(&instance), sizeof(instance));

        if (!inst) {
            throw std::filesystem::filesystem_error{
                "failed to write instance id to file", instance_path,
                std::make_error_code(std::errc::io_error)
            };
        }

        // Done!
    }

    
}
