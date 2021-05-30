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

#include <projectedfslib.h>

projection::projection(const path& source, const path& root)
    : _src{ canonical(source) }, _root{ weakly_canonical(root) } {
    _prepare_root();
    _start();
}


void projection::_prepare_root() {
    if (!is_directory(_root)) {
        // Root dir does not exist, create it
        if (std::error_code err; !create_directories(_root, err)) {
            throw std::filesystem::filesystem_error{
                "failed to create root dir", _root, err
            };
        }
    }

    // Both directories now exist

    // Extract existing or create new instance GUID
    if (auto instance_path = _root / instance_file; exists(instance_path)) {
        // Instance file exists, read data
        std::ifstream inst{ instance_path, std::ios::binary };
        inst.read(reinterpret_cast<char*>(&_instance), sizeof(_instance));

        if (inst.gcount() != sizeof(_instance)) {
            // Failed to read entire GUID, abort
            throw std::filesystem::filesystem_error{
                "failed to read instance id from instance file", instance_path,
                std::make_error_code(std::errc::io_error)
            };
        }

        logger().info("Found existing session: {}", _instance);

        // Success
    } else {
        // New session ID required
        if (CoCreateGuid(&_instance) != S_OK) {
            throw std::runtime_error{ "failed to create session guid" };
        }

        // Write it to the file
        std::ofstream inst{ instance_path, std::ios::binary };
        inst.write(reinterpret_cast<char*>(&_instance), sizeof(_instance));

        if (!inst) {
            throw std::filesystem::filesystem_error{
                "failed to write instance id to file", instance_path,
                std::make_error_code(std::errc::io_error)
            };
        }

        // Done!
        logger().info("Created new session: {}", _instance);
    }
}


void projection::_start() {
    // We don't want to receive any notifications
    PRJ_STARTVIRTUALIZING_OPTIONS opts {
        .NotificationMappingsCount = 0,
    };

    PRJ_CALLBACKS callbacks = {
        
    };
}