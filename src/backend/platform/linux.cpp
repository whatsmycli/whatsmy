// whatsmycli - Linux Platform Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include <dlfcn.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring>
#include "whatsmy/helpers.h"
#include "whatsmy/plugin_validator.h"

namespace whatsmy {
namespace backend {
namespace platform {

/**
 * Linux-specific plugin loading using dlopen/dlsym
 */
class LinuxPluginLoader {
public:
    /**
     * Load and execute a plugin on Linux
     * 
     * @param plugin_path Full path to the plugin .so file
     * @return Exit code from plugin (0 on success, non-zero on error)
     */
    static int load_and_execute(const std::string& plugin_path) {
        // Check if plugin file exists
        if (!std::filesystem::exists(plugin_path)) {
            helpers::output::print_error("Plugin file not found: " + plugin_path);
            return 1;
        }

        // Load the dynamic library
        void* handle = dlopen(plugin_path.c_str(), RTLD_LAZY);
        if (!handle) {
            const char* dl_error = dlerror();
            helpers::output::print_error("Failed to load plugin");
            helpers::error::error_log("dlopen() failed: " + std::string(dl_error ? dl_error : "unknown error"));
            
            // Provide detailed diagnostics
            std::cout << "\nLoad Failure Diagnostics:\n";
            std::cout << "  • Plugin path: " << plugin_path << "\n";
            std::cout << "  • Error: " << (dl_error ? dl_error : "unknown") << "\n";
            
            // Common error explanations
            if (dl_error) {
                std::string error_str(dl_error);
                
                if (error_str.find("cannot open shared object file") != std::string::npos) {
                    std::cout << "\n💡 This usually means:\n";
                    std::cout << "  • The plugin file is missing or corrupted\n";
                    std::cout << "  • File permissions prevent loading\n";
                    std::cout << "  • The file is not a valid shared library\n";
                } else if (error_str.find("wrong ELF class") != std::string::npos) {
                    std::cout << "\n💡 Architecture mismatch detected:\n";
                    std::cout << "  • Plugin was built for different architecture (32-bit vs 64-bit)\n";
                    std::cout << "  • Rebuild the plugin for your system architecture\n";
                } else if (error_str.find("undefined symbol") != std::string::npos) {
                    std::cout << "\n💡 Missing dependencies:\n";
                    std::cout << "  • Plugin requires libraries that are not installed\n";
                    std::cout << "  • Check plugin documentation for required dependencies\n";
                } else if (error_str.find("cannot allocate memory") != std::string::npos) {
                    std::cout << "\n💡 System resource issue:\n";
                    std::cout << "  • Insufficient memory to load plugin\n";
                    std::cout << "  • Close other applications and try again\n";
                }
            }
            
            std::cout << "\nFor more help, run with: WHATSMY_DEBUG=1 whatsmy <component>\n";
            std::cout << "Or use: whatsmy --debug <component>\n";
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
            helpers::output::print_error("Failed to find plugin_run symbol");
            helpers::error::error_log("dlsym() failed: " + std::string(dlsym_error));
            
            std::cout << "\nSymbol Resolution Diagnostics:\n";
            std::cout << "  • Required symbol: plugin_run\n";
            std::cout << "  • Error: " << dlsym_error << "\n";
            std::cout << "\n💡 This means:\n";
            std::cout << "  • Plugin does not export the required 'plugin_run' function\n";
            std::cout << "  • Plugin may be outdated or incompatible\n";
            std::cout << "  • Make sure plugin was built with 'extern \"C\"' for plugin_run\n";
            std::cout << "\nFor plugin developers:\n";
            std::cout << "  See: https://github.com/whatsmycli/plugin-template\n";
            
            dlclose(handle);
            return 1;
        }

        if (!plugin_run) {
            helpers::output::print_error("plugin_run symbol resolved to null");
            helpers::error::error_log("plugin_run function pointer is null");
            
            std::cout << "\n💡 This is unusual and may indicate:\n";
            std::cout << "  • Plugin corruption\n";
            std::cout << "  • Incompatible plugin version\n";
            std::cout << "  • System or library issue\n";
            
            dlclose(handle);
            return 1;
        }

        // Execute the plugin
        int result = 0;
        try {
            helpers::error::debug_log("Executing plugin_run()");
            result = plugin_run();
            helpers::error::debug_log("Plugin returned exit code: " + std::to_string(result));
        } catch (const std::exception& e) {
            helpers::output::print_error("Plugin crashed during execution");
            helpers::error::error_log("Exception caught: " + std::string(e.what()));
            
            std::cout << "\nRuntime Error Diagnostics:\n";
            std::cout << "  • Exception type: std::exception\n";
            std::cout << "  • Error message: " << e.what() << "\n";
            std::cout << "\n💡 The plugin encountered an error:\n";
            std::cout << "  • This is a bug in the plugin code\n";
            std::cout << "  • Report this to the plugin developer\n";
            std::cout << "  • Include the error message above\n";
            
            helpers::error::debug_log("Stack trace information may be available in debug builds");
            
            dlclose(handle);
            return 4;  // PLUGIN_EXEC_ERROR
        } catch (...) {
            helpers::output::print_error("Plugin crashed with unknown exception");
            helpers::error::error_log("Unknown exception caught during plugin execution");
            
            std::cout << "\nRuntime Error Diagnostics:\n";
            std::cout << "  • Exception type: unknown (not std::exception)\n";
            std::cout << "\n💡 The plugin crashed unexpectedly:\n";
            std::cout << "  • This is a serious bug in the plugin\n";
            std::cout << "  • The plugin may have:\n";
            std::cout << "    - Accessed invalid memory\n";
            std::cout << "    - Thrown a non-standard exception\n";
            std::cout << "    - Triggered undefined behavior\n";
            std::cout << "  • Report this to the plugin developer immediately\n";
            
            dlclose(handle);
            return 4;  // PLUGIN_EXEC_ERROR
        }

        // Unload the library
        dlclose(handle);

        return result;
    }
};

// Export the function for use by plugin_loader.cpp
int load_and_execute_plugin(const std::string& plugin_path) {
    return LinuxPluginLoader::load_and_execute(plugin_path);
}

/**
 * Platform-specific plugin validation for Linux
 */
namespace validator {

/**
 * Check if plugin has required symbols using nm or dlopen
 */
ValidationResult check_symbols(const std::string& plugin_path) {
    ValidationResult result;
    
    // Try to load the library without executing it to check for symbols
    void* handle = dlopen(plugin_path.c_str(), RTLD_LAZY | RTLD_NOLOAD);
    if (!handle) {
        // Library not already loaded, try to load it
        handle = dlopen(plugin_path.c_str(), RTLD_LAZY);
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
 * Verify binary format is ELF for Linux
 */
ValidationResult verify_binary_format(const std::string& plugin_path) {
    ValidationResult result;
    
    // Open the file and check for ELF magic number
    std::ifstream file(plugin_path, std::ios::binary);
    if (!file) {
        result.add_error("Failed to open plugin file for binary format verification");
        return result;
    }
    
    // Read ELF magic number (first 4 bytes should be 0x7F 'E' 'L' 'F')
    unsigned char magic[4];
    file.read(reinterpret_cast<char*>(magic), 4);
    
    if (!file) {
        result.add_error("Failed to read plugin file header");
        return result;
    }
    
    // Check ELF magic number
    if (magic[0] != 0x7F || magic[1] != 'E' || magic[2] != 'L' || magic[3] != 'F') {
        result.add_error("Plugin is not a valid ELF binary (expected Linux shared library)");
        return result;
    }
    
    // Read ELF class (32-bit or 64-bit)
    unsigned char elf_class;
    file.read(reinterpret_cast<char*>(&elf_class), 1);
    
    if (!file) {
        result.add_error("Failed to read ELF class information");
        return result;
    }
    
    // Verify architecture matches system
    #if defined(__x86_64__) || defined(__aarch64__)
        // 64-bit system
        if (elf_class != 2) {  // ELFCLASS64 = 2
            result.add_warning("Plugin is 32-bit but system is 64-bit (may not be compatible)");
        }
    #else
        // 32-bit system
        if (elf_class != 1) {  // ELFCLASS32 = 1
            result.add_warning("Plugin is 64-bit but system is 32-bit (may not be compatible)");
        }
    #endif
    
    file.close();
    
    return result;
}

} // namespace validator

} // namespace platform
} // namespace backend
} // namespace whatsmy

