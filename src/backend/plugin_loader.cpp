// whatsmycli - Plugin Loader Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include "whatsmy/plugin_loader.h"
#include "whatsmy/helpers.h"

#ifdef _WIN32
    // Windows headers
    // #include <windows.h>
#elif __APPLE__
    // macOS headers
    // #include <dlfcn.h>
#else
    // Linux headers
    // #include <dlfcn.h>
#endif

namespace whatsmy {
namespace backend {

std::string PluginLoader::get_plugin_directory() {
    // TODO: Implement in Phase 1
    #ifdef _WIN32
        return "C:\\Program Files\\whatsmy\\plugins\\";
    #elif __APPLE__
        return "/usr/local/lib/whatsmy/plugins/";
    #else
        return "/usr/lib/whatsmy/plugins/";
    #endif
}

std::string PluginLoader::get_library_extension() {
    // TODO: Implement in Phase 1
    #ifdef _WIN32
        return ".dll";
    #elif __APPLE__
        return ".dylib";
    #else
        return ".so";
    #endif
}

int PluginLoader::load_and_run(const std::string& plugin_name) {
    // TODO: Implement in Phase 1
    // This will handle:
    // 1. Construct plugin path
    // 2. Load dynamic library
    // 3. Resolve plugin_run symbol
    // 4. Execute plugin
    // 5. Unload library
    // 6. Return result
    
    helpers::output::print_error("Plugin loader not yet implemented");
    return 1;
}

} // namespace backend
} // namespace whatsmy

