// whatsmycli - Plugin Loader Header
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#ifndef WHATSMY_PLUGIN_LOADER_H
#define WHATSMY_PLUGIN_LOADER_H

#include <string>

namespace whatsmy {
namespace backend {

/**
 * Plugin loader for dynamic library loading
 */
class PluginLoader {
public:
    /**
     * Load and execute a plugin with arguments
     * 
     * @param plugin_name Name of the plugin to load
     * @param argc Number of arguments to pass to the plugin
     * @param argv Array of argument strings to pass to the plugin
     * @return Exit code from plugin (0 on success)
     */
    static int load_and_run(const std::string& plugin_name, int argc, char* argv[]);
    
    /**
     * Get plugin directory path based on platform
     */
    static std::string get_plugin_directory();
    
    /**
     * Get platform-specific library extension
     */
    static std::string get_library_extension();
};

} // namespace backend
} // namespace whatsmy

#endif // WHATSMY_PLUGIN_LOADER_H

