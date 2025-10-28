// whatsmycli - Windows Platform Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include <windows.h>
#include <string>
#include <filesystem>
#include "whatsmy/helpers.h"

namespace whatsmy {
namespace backend {
namespace platform {

/**
 * Windows-specific plugin loading using LoadLibrary/GetProcAddress
 */
class WindowsPluginLoader {
public:
    /**
     * Load and execute a plugin on Windows
     * 
     * @param plugin_path Full path to the plugin .dll file
     * @return Exit code from plugin (0 on success, non-zero on error)
     */
    static int load_and_execute(const std::string& plugin_path) {
        // Check if plugin file exists
        if (!std::filesystem::exists(plugin_path)) {
            helpers::output::print_error("Plugin file not found: " + plugin_path);
            return 1;
        }

        // Load the dynamic library
        HMODULE handle = LoadLibraryA(plugin_path.c_str());
        if (!handle) {
            DWORD error = GetLastError();
            std::string error_msg = "Failed to load plugin (error " + 
                                   std::to_string(error) + "): " + plugin_path;
            helpers::output::print_error(error_msg);
            return 1;
        }

        // Resolve the plugin_run symbol
        typedef int (*plugin_run_func)();
        plugin_run_func plugin_run = reinterpret_cast<plugin_run_func>(
            GetProcAddress(handle, "plugin_run")
        );

        if (!plugin_run) {
            DWORD error = GetLastError();
            std::string error_msg = "Failed to find plugin_run symbol (error " + 
                                   std::to_string(error) + ")";
            helpers::output::print_error(error_msg);
            FreeLibrary(handle);
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
            FreeLibrary(handle);
            return 1;
        } catch (...) {
            helpers::output::print_error("Plugin execution failed with unknown exception");
            FreeLibrary(handle);
            return 1;
        }

        // Unload the library
        FreeLibrary(handle);

        return result;
    }
};

// Export the function for use by plugin_loader.cpp
int load_and_execute_plugin(const std::string& plugin_path) {
    return WindowsPluginLoader::load_and_execute(plugin_path);
}

} // namespace platform
} // namespace backend
} // namespace whatsmy

