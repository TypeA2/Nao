// SPDX-License-Identifier: LGPL-3.0-or-later

#define FUSE_USE_VERSION 314

#include <cstddef>
#include <array>
#include <string_view>
#include <iostream>

#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <fuse.h>
#include <fuse_opt.h>

#include "naofs.hpp"

struct options {
    const char* source;
    int help;
    int archive_only;
    int loglevel = 2;
};

static naofs& get_fs() {
    return *static_cast<naofs*>(fuse_get_context()->private_data);
}

static int naofs_getattr(const char* path, struct stat* stbuf, struct fuse_file_info* fi) {
    (void) fi;
    spdlog::info("getattr '{}'", path);

    std::memset(stbuf, 0, sizeof(*stbuf));
    return get_fs().getattr(path, *stbuf);
}

static int naofs_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
    off_t offset, fuse_file_info* fi, fuse_readdir_flags flags) {
    (void) fi;
    (void) flags;

    spdlog::info("readdir {}", path);

    return get_fs().readdir(path, offset, [&](std::string name, struct stat* stbuf, off_t offset) -> bool {
        return filler(buf, name.c_str(), stbuf, offset, fuse_fill_dir_flags{}) == 0;
    });
}

static int show_help(fuse_args& args) {
    fmt::print(std::cerr, "usage: {} [options] <mountpoint>\n\n", args.argv[0]);
    fmt::print(std::cerr,
        "naofs options:\n"
        "    --source=<path>     Path of a file or directory to use as a data source\n"
        "    -a, --archive_only  Only show archive contents, perform no other conversions\n"
        "    --loglevel=<0-6>    Specify log level, with lower numbers being more verbose\n"
        "\n"
    );

    /* Make fuse show it's options */
    fuse_opt_add_arg(&args, "--help");

    /* Prevent "usage" line in fuse output */
    args.argv[0][0] = '\0';

    return fuse_main(args.argc, args.argv, static_cast<fuse_operations*>(nullptr), nullptr);
}

int main(int argc, char** argv) {
    /* This could be done with fancy constexpr stuff, or just with this */
#define OPT(t, m) { t, offsetof(options, m), 1 }
    static constexpr fuse_opt options_spec[] {
        OPT("--source=%s", source),
        OPT("--help", help),
        OPT("-a", archive_only),
        OPT("--archive_only", archive_only),
        OPT("--loglevel=%i", loglevel),
        FUSE_OPT_END,
    };
#undef OPT

    options options_vals{};
    fuse_args args = FUSE_ARGS_INIT(argc, argv);

    if (fuse_opt_parse(&args, &options_vals, options_spec, nullptr) == -1) {
        return EXIT_FAILURE;
    }

    struct fuse_args_raii {
        fuse_args& args;
        ~fuse_args_raii() {
            fuse_opt_free_args(&args);
        }
    } raii { args };

    if (options_vals.help) {
        return show_help(args);
    }

    if (options_vals.loglevel < 0 || options_vals.loglevel >= spdlog::level::n_levels) {
        fmt::print(std::cerr, "Log level out of range: {}\n", options_vals.loglevel);
        return EXIT_FAILURE;
    }
    
    spdlog::set_level(static_cast<spdlog::level::level_enum>(options_vals.loglevel));

    try {
        if (!options_vals.source) {
            throw std::runtime_error("--source is required");
        }

        archive_mode mode = options_vals.archive_only ? archive_mode::archive_only : archive_mode::all;
        naofs fs(options_vals.source, mode);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

        static const fuse_operations operations {
            .getattr = naofs_getattr,
            .readdir = naofs_readdir,
        };

#pragma GCC diagnostic pop

        return fuse_main(args.argc, args.argv, &operations, &fs);
    } catch (std::runtime_error& e) {
        fmt::print(std::cerr, "Error: {}\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;    
}
