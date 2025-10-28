// whatsmycli - Plugin Loader Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include "whatsmy/plugin_loader.h"
#include "whatsmy/plugin_validator.h"
#include "whatsmy/helpers.h"
#include <filesystem>
#include <string>
#include <cstdlib>

namespace whatsmy {
namespace backend {

// Forward declaration of platform-specific function
// This will be implemented in platform-specific source files
namespace platform {
    int load_and_execute_plugin(const std::string& plugin_path);
}

std::string PluginLoader::get_plugin_directory() {
    // Check for environment variable override (for testing)
    const char* env_plugin_dir = std::getenv("WHATSMY_PLUGIN_DIR");
    if (env_plugin_dir) {
        return std::string(env_plugin_dir);
    }
    
    #ifdef _WIN32
        return "C:\\Program Files\\whatsmy\\plugins\\";
    #elif __APPLE__
        return "/usr/local/lib/whatsmy/plugins/";
    #else
        return "/usr/lib/whatsmy/plugins/";
    #endif
}

std::string PluginLoader::get_library_extension() {
    #ifdef _WIN32
        return ".dll";
    #elif __APPLE__
        return ".dylib";
    #else
        return ".so";
    #endif
}

int PluginLoader::load_and_run(const std::string& plugin_name) {
    // 1. Get plugin directory
    std::string plugin_dir = get_plugin_directory();
    
    // 2. Get platform-specific library extension
    std::string extension = get_library_extension();
    
    // 3. Construct full plugin path
    // Plugins are organized as: /usr/lib/whatsmy/plugins/<plugin-name>/<platform>.<ext>
    // For example: /usr/lib/whatsmy/plugins/gpu/linux.so
    
    std::string platform_name;
    #ifdef _WIN32
        platform_name = "windows";
    #elif __APPLE__
        platform_name = "macos";
    #else
        platform_name = "linux";
    #endif
    
    std::filesystem::path plugin_path = std::filesystem::path(plugin_dir) / 
                                        plugin_name / 
                                        (platform_name + extension);
    
    // 4. Check if plugin directory exists
    std::filesystem::path plugin_folder = std::filesystem::path(plugin_dir) / plugin_name;
    if (!std::filesystem::exists(plugin_folder)) {
        helpers::output::print_error("Plugin '" + plugin_name + "' not found");
        helpers::output::print_info("Plugin directory does not exist: " + plugin_folder.string());
        return 1;
    }
    
    // 5. Check if platform-specific plugin binary exists
    if (!std::filesystem::exists(plugin_path)) {
        helpers::output::print_error("Plugin '" + plugin_name + "' not available for this platform");
        helpers::output::print_info("Expected plugin at: " + plugin_path.string());
        return 1;
    }
    
    // 6. Validate plugin before loading
    helpers::error::debug_log("Validating plugin: " + plugin_path.string());
    ValidationResult validation = PluginValidator::validate(plugin_path.string());
    
    if (!validation.valid) {
        helpers::output::print_error("Plugin validation failed for '" + plugin_name + "'");
        helpers::output::print_error("Reason: " + validation.error_message);
        helpers::error::error_log("Plugin validation failed: " + validation.error_message);
        return 1;
    }
    
    // Display any validation warnings
    if (validation.has_warnings()) {
        for (const auto& warning : validation.warnings) {
            helpers::output::print_warning("Plugin warning: " + warning);
            helpers::error::warning_log(warning);
        }
    }
    
    helpers::error::debug_log("Plugin validation successful");
    
    // 7. Load and execute the plugin using platform-specific implementation
    int result = platform::load_and_execute_plugin(plugin_path.string());
    
    // 8. Validate return code
    if (!PluginValidator::validate_return_code(result)) {
        helpers::output::print_warning("Plugin '" + plugin_name + "' returned invalid exit code: " + 
                                      std::to_string(result));
    }
    
    if (result != 0) {
        std::string description = PluginValidator::get_return_code_description(result);
        helpers::error::debug_log("Plugin exited with code " + std::to_string(result) + ": " + description);
    }
    
    return result;
}

} // namespace backend
} // namespace whatsmy

