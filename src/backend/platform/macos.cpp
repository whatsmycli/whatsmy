// whatsmycli - macOS Platform Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include <dlfcn.h>
#include <string>
#include <filesystem>
#include "whatsmy/helpers.h"

namespace whatsmy {
namespace backend {
namespace platform {

/**
 * macOS-specific plugin loading using dlopen/dlsym
 * Similar to Linux but may have different behaviors
 */
class MacOSPluginLoader {
public:
    /**
     * Load and execute a plugin on macOS
     * 
     * @param plugin_path Full path to the plugin .dylib file
     * @return Exit code from plugin (0 on success, non-zero on error)
     */
    static int load_and_execute(const std::string& plugin_path) {
        // Check if plugin file exists
        if (!std::filesystem::exists(plugin_path)) {
            helpers::output::print_error("Plugin file not found: " + plugin_path);
            return 1;
        }

        // Load the dynamic library
        // RTLD_LAZY: Resolve symbols as needed
        // RTLD_LOCAL: Don't make symbols globally available
        void* handle = dlopen(plugin_path.c_str(), RTLD_LAZY | RTLD_LOCAL);
        if (!handle) {
            std::string error_msg = "Failed to load plugin: ";
            error_msg += dlerror();
            helpers::output::print_error(error_msg);
            return 1;
        }

        // Clear any existing errors
        dlerror();

        // Resolve the plugin_run symbol
        typedef int (*plugin_run_func)();
        plugin_run_func plugin_run = reinterpret_cast<plugin_run_func>(
            dlsym(handle, "plugin_run")
        );

        // Check for symbol resolution errors
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
            std::string error_msg = "Failed to find plugin_run symbol: ";
            error_msg += dlsym_error;
            helpers::output::print_error(error_msg);
            dlclose(handle);
            return 1;
        }

        if (!plugin_run) {
            helpers::output::print_error("plugin_run symbol is null");
            dlclose(handle);
            return 1;
        }

        // Execute the plugin
        int result = 0;
        try {
            result = plugin_run();
        } catch (const std::exception& e) {
            std::string error_msg = "Plugin execution failed: ";
            error_msg += e.what();
            helpers::output::print_error(error_msg);
            dlclose(handle);
            return 1;
        } catch (...) {
            helpers::output::print_error("Plugin execution failed with unknown exception");
            dlclose(handle);
            return 1;
        }

        // Unload the library
        dlclose(handle);

        return result;
    }
};

// Export the function for use by plugin_loader.cpp
int load_and_execute_plugin(const std::string& plugin_path) {
    return MacOSPluginLoader::load_and_execute(plugin_path);
}

} // namespace platform
} // namespace backend
} // namespace whatsmy

