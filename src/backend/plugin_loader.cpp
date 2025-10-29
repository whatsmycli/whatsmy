// whatsmycli - Plugin Loader Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include "whatsmy/plugin_loader.h"
#include "whatsmy/plugin_validator.h"
#include "whatsmy/helpers.h"
#include <filesystem>
#include <iostream>
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
    
    std::filesystem::path plugin_path;
    std::filesystem::path plugin_folder;
    
    // Wrap filesystem operations in try-catch to handle extremely long paths
    try {
        plugin_path = std::filesystem::path(plugin_dir) / 
                      plugin_name / 
                      (platform_name + extension);
        plugin_folder = std::filesystem::path(plugin_dir) / plugin_name;
    } catch (const std::filesystem::filesystem_error& e) {
        helpers::output::print_error("Invalid plugin name: path too long or contains invalid characters");
        helpers::error::error_log("Filesystem error constructing plugin path: " + std::string(e.what()));
        return 2; // PLUGIN_NOT_FOUND
    } catch (const std::exception& e) {
        helpers::output::print_error("Error constructing plugin path");
        helpers::error::error_log("Exception: " + std::string(e.what()));
        return 2; // PLUGIN_NOT_FOUND
    }
    
    // 4. Check if plugin directory exists
    try {
        if (!std::filesystem::exists(plugin_folder)) {
            helpers::output::print_error("Plugin '" + plugin_name + "' not found");
            helpers::error::error_log("Plugin directory does not exist: " + plugin_folder.string());
            
            // Provide helpful diagnostics
            std::cout << "\nDiagnostics:\n";
            std::cout << "  • Plugin directory searched: " << plugin_dir << "\n";
            std::cout << "  • Expected plugin folder: " << plugin_folder.string() << "\n";
            
            // List available plugins if plugin directory exists
            if (std::filesystem::exists(plugin_dir)) {
                std::cout << "\nAvailable plugins:\n";
                bool found_any = false;
                try {
                    for (const auto& entry : std::filesystem::directory_iterator(plugin_dir)) {
                        if (entry.is_directory()) {
                            std::cout << "  • " << entry.path().filename().string() << "\n";
                            found_any = true;
                        }
                    }
                } catch (const std::exception& e) {
                    helpers::error::debug_log("Failed to list plugin directory: " + std::string(e.what()));
                }
                
                if (!found_any) {
                    std::cout << "  (no plugins installed)\n";
                }
            } else {
                std::cout << "\n⚠️  Plugin directory doesn't exist: " << plugin_dir << "\n";
                std::cout << "   You may need to install plugins or set WHATSMY_PLUGIN_DIR\n";
            }
            
            std::cout << "\nFor help, see: https://github.com/enxov/whatsmycli/blob/main/docs/troubleshooting.md\n";
            return 2; // PLUGIN_NOT_FOUND
        }
    } catch (const std::filesystem::filesystem_error& e) {
        helpers::output::print_error("Error checking plugin directory");
        helpers::error::error_log("Filesystem error: " + std::string(e.what()));
        return 2; // PLUGIN_NOT_FOUND
    }
    
    // 5. Check if platform-specific plugin binary exists
    try {
        if (!std::filesystem::exists(plugin_path)) {
            helpers::output::print_error("Plugin '" + plugin_name + "' not available for this platform");
            helpers::error::error_log("Platform-specific binary not found: " + plugin_path.string());
            
            // Provide helpful diagnostics
            std::cout << "\nDiagnostics:\n";
            std::cout << "  • Plugin: " << plugin_name << "\n";
            std::cout << "  • Platform: " << platform_name << "\n";
            std::cout << "  • Expected binary: " << plugin_path.string() << "\n";
            
            // List available platform binaries for this plugin
            std::cout << "\nAvailable builds for '" << plugin_name << "':\n";
            bool found_any = false;
            try {
                for (const auto& entry : std::filesystem::directory_iterator(plugin_folder)) {
                    if (entry.is_regular_file()) {
                        std::string filename = entry.path().filename().string();
                        std::cout << "  • " << filename << "\n";
                        found_any = true;
                    }
                }
            } catch (const std::exception& e) {
                helpers::error::debug_log("Failed to list plugin builds: " + std::string(e.what()));
            }
            
            if (!found_any) {
                std::cout << "  (no platform binaries found)\n";
            }
            
            std::cout << "\nThis plugin may not support your platform yet.\n";
            std::cout << "For help, see: https://github.com/enxov/whatsmycli/blob/main/docs/troubleshooting.md\n";
            return 3; // PLUGIN_LOAD_ERROR
        }
    } catch (const std::filesystem::filesystem_error& e) {
        helpers::output::print_error("Error checking plugin binary");
        helpers::error::error_log("Filesystem error: " + std::string(e.what()));
        return 3; // PLUGIN_LOAD_ERROR
    }
    
    // 6. Validate plugin before loading
    helpers::error::debug_log("Validating plugin: " + plugin_path.string());
    ValidationResult validation = PluginValidator::validate(plugin_path.string());
    
    if (!validation.valid) {
        helpers::output::print_error("Plugin validation failed for '" + plugin_name + "'");
        helpers::output::print_error("Reason: " + validation.error_message);
        helpers::error::error_log("Plugin validation failed: " + validation.error_message);
        return 3; // PLUGIN_LOAD_ERROR
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

