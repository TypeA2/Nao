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

#include <libnao/libnao_util.h>
#include <libnao/logging.h>

#include "projection.h"

spdlog::logger logger = nao::make_logger("naofs");

static void usage(const char* prog) {
    logger.info("Usage:");
    logger.info("  {} <source> <root>", prog);
    logger.info("      <source> Game root directory.");
    logger.info("      <root>   Virtualization root directory.");
}

int main(int argc, char** argv) {
    if (!nao::libnao_util::init()) {
       return EXIT_FAILURE;
    }
    
    if (argc != 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    try {
        projection p{ argv[1], argv[2] };

    } catch (const std::exception& e) {
        logger.error("Exception caught: {}", e.what());
    }

    return EXIT_SUCCESS;
}
