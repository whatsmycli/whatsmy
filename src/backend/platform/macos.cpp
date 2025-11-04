// whatsmycli - macOS Platform Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include <dlfcn.h>
#include <string>
#include <filesystem>
#include <fstream>
#include "whatsmy/helpers.h"
#include "whatsmy/plugin_validator.h"

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
     * @param argc Number of arguments to pass to the plugin
     * @param argv Array of argument strings to pass to the plugin
     * @return Exit code from plugin (0 on success, non-zero on error)
     */
    static int load_and_execute(const std::string& plugin_path, int argc, char* argv[]) {
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
        typedef int (*plugin_run_func)(int, char**);
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
            helpers::error::debug_log("Executing plugin_run() with " + std::to_string(argc) + " arguments");
            result = plugin_run(argc, argv);
            helpers::error::debug_log("Plugin returned exit code: " + std::to_string(result));
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
int load_and_execute_plugin(const std::string& plugin_path, int argc, char* argv[]) {
    return MacOSPluginLoader::load_and_execute(plugin_path, argc, argv);
}

/**
 * Platform-specific plugin validation for macOS
 */
namespace validator {

/**
 * Check if plugin has required symbols using dlopen/dlsym
 */
ValidationResult check_symbols(const std::string& plugin_path) {
    ValidationResult result;
    
    // Try to load the library without executing it to check for symbols
    void* handle = dlopen(plugin_path.c_str(), RTLD_LAZY | RTLD_NOLOAD);
    if (!handle) {
        // Library not already loaded, try to load it
        handle = dlopen(plugin_path.c_str(), RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        result.add_error("Failed to open plugin for symbol verification: " + std::string(dlerror()));
        return result;
    }
    
    // Clear any existing errors
    dlerror();
    
    // Check for required symbol: plugin_run
    void* symbol = dlsym(handle, "plugin_run");
    const char* dlsym_error = dlerror();
    
    if (dlsym_error || !symbol) {
        std::string error_msg = "Required symbol 'plugin_run' not found in plugin";
        if (dlsym_error) {
            error_msg += ": " + std::string(dlsym_error);
        }
        result.add_error(error_msg);
        dlclose(handle);
        return result;
    }
    
    // Symbol found successfully
    dlclose(handle);
    
    return result;
}

/**
 * Verify binary format is Mach-O for macOS
 */
ValidationResult verify_binary_format(const std::string& plugin_path) {
    ValidationResult result;
    
    // Open the file and check for Mach-O magic number
    std::ifstream file(plugin_path, std::ios::binary);
    if (!file) {
        result.add_error("Failed to open plugin file for binary format verification");
        return result;
    }
    
    // Read Mach-O magic number (first 4 bytes)
    uint32_t magic;
    file.read(reinterpret_cast<char*>(&magic), 4);
    
    if (!file) {
        result.add_error("Failed to read plugin file header");
        return result;
    }
    
    // Check Mach-O magic numbers
    // 0xfeedface = 32-bit Mach-O
    // 0xfeedfacf = 64-bit Mach-O
    // 0xcafebabe = Universal binary (fat binary)
    // 0xcffaedfe = 32-bit Mach-O (reverse byte order)
    // 0xcefaedfe = 64-bit Mach-O (reverse byte order)
    
    bool is_macho = (magic == 0xfeedface || magic == 0xfeedfacf || 
                     magic == 0xcafebabe || magic == 0xcffaedfe || 
                     magic == 0xcefaedfe);
    
    if (!is_macho) {
        result.add_error("Plugin is not a valid Mach-O binary (expected macOS dylib)");
        return result;
    }
    
    // Check architecture compatibility
    #if defined(__x86_64__)
        // 64-bit Intel system
        if (magic == 0xfeedface || magic == 0xcffaedfe) {
            result.add_warning("Plugin is 32-bit but system is 64-bit Intel (may not be compatible)");
        }
    #elif defined(__aarch64__) || defined(__arm64__)
        // Apple Silicon (ARM64)
        if (magic == 0xfeedface || magic == 0xcffaedfe) {
            result.add_warning("Plugin is 32-bit but system is ARM64 (may not be compatible)");
        }
        // Note: Universal binaries (0xcafebabe) should work on both Intel and ARM
    #endif
    
    file.close();
    
    return result;
}

} // namespace validator

} // namespace platform
} // namespace backend
} // namespace whatsmy

