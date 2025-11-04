// whatsmycli - Windows Platform Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include <windows.h>
#undef ERROR  // Undefine Windows ERROR macro to avoid conflict with helpers::error::Level::ERROR
#include <string>
#include <filesystem>
#include <fstream>
#include "whatsmy/helpers.h"
#include "whatsmy/plugin_validator.h"

// Platform-specific bullet character for better Windows console compatibility
#ifdef _WIN32
    #define BULLET "  - "
#else
    #define BULLET "  • "
#endif

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
        HMODULE handle = LoadLibraryA(plugin_path.c_str());
        if (!handle) {
            DWORD error = GetLastError();
            std::string error_msg = "Failed to load plugin (error " + 
                                   std::to_string(error) + "): " + plugin_path;
            helpers::output::print_error(error_msg);
            return 1;
        }

        // Resolve the plugin_run symbol
        typedef int (*plugin_run_func)(int, char**);
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
            helpers::error::debug_log("Executing plugin_run() with " + std::to_string(argc) + " arguments");
            result = plugin_run(argc, argv);
            helpers::error::debug_log("Plugin returned exit code: " + std::to_string(result));
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
int load_and_execute_plugin(const std::string& plugin_path, int argc, char* argv[]) {
    return WindowsPluginLoader::load_and_execute(plugin_path, argc, argv);
}

/**
 * Platform-specific plugin validation for Windows
 */
namespace validator {

/**
 * Check if plugin has required symbols using LoadLibrary/GetProcAddress
 */
ValidationResult check_symbols(const std::string& plugin_path) {
    ValidationResult result;
    
    // Load the library to check for symbols
    HMODULE handle = LoadLibraryA(plugin_path.c_str());
    if (!handle) {
        DWORD error = GetLastError();
        result.add_error("Failed to open plugin for symbol verification (error " + 
                        std::to_string(error) + ")");
        return result;
    }
    
    // Check for required symbol: plugin_run
    void* symbol = reinterpret_cast<void*>(GetProcAddress(handle, "plugin_run"));
    
    if (!symbol) {
        DWORD error = GetLastError();
        std::string error_msg = "Required symbol 'plugin_run' not found in plugin";
        if (error != 0) {
            error_msg += " (error " + std::to_string(error) + ")";
        }
        result.add_error(error_msg);
        FreeLibrary(handle);
        return result;
    }
    
    // Symbol found successfully
    FreeLibrary(handle);
    
    return result;
}

/**
 * Verify binary format is PE (Portable Executable) for Windows
 */
ValidationResult verify_binary_format(const std::string& plugin_path) {
    ValidationResult result;
    
    // Open the file and check for PE magic number
    std::ifstream file(plugin_path, std::ios::binary);
    if (!file) {
        result.add_error("Failed to open plugin file for binary format verification");
        return result;
    }
    
    // Read DOS header magic number (first 2 bytes should be 'M' 'Z')
    unsigned char dos_magic[2];
    file.read(reinterpret_cast<char*>(dos_magic), 2);
    
    if (!file) {
        result.add_error("Failed to read plugin file header");
        return result;
    }
    
    // Check DOS magic number
    if (dos_magic[0] != 'M' || dos_magic[1] != 'Z') {
        result.add_error("Plugin is not a valid PE binary (expected Windows DLL)");
        return result;
    }
    
    // Read PE offset (at offset 0x3C in DOS header)
    file.seekg(0x3C);
    if (!file) {
        result.add_error("Failed to read PE header offset");
        return result;
    }
    
    uint32_t pe_offset;
    file.read(reinterpret_cast<char*>(&pe_offset), 4);
    
    if (!file) {
        result.add_error("Failed to read PE header offset");
        return result;
    }
    
    // Read PE signature
    file.seekg(pe_offset);
    if (!file) {
        result.add_error("Invalid PE header offset");
        return result;
    }
    
    unsigned char pe_signature[4];
    file.read(reinterpret_cast<char*>(pe_signature), 4);
    
    if (!file) {
        result.add_error("Failed to read PE signature");
        return result;
    }
    
    // Check PE signature ('P' 'E' 0x00 0x00)
    if (pe_signature[0] != 'P' || pe_signature[1] != 'E' || 
        pe_signature[2] != 0x00 || pe_signature[3] != 0x00) {
        result.add_error("Invalid PE signature");
        return result;
    }
    
    // Read machine type (next 2 bytes after PE signature)
    uint16_t machine_type;
    file.read(reinterpret_cast<char*>(&machine_type), 2);
    
    if (!file) {
        result.add_error("Failed to read machine type");
        return result;
    }
    
    // Verify architecture matches system
    #if defined(_M_X64) || defined(__x86_64__)
        // 64-bit system
        if (machine_type != 0x8664) {  // IMAGE_FILE_MACHINE_AMD64
            result.add_warning("Plugin is not 64-bit (may not be compatible with 64-bit system)");
        }
    #elif defined(_M_IX86) || defined(__i386__)
        // 32-bit system
        if (machine_type != 0x014c) {  // IMAGE_FILE_MACHINE_I386
            result.add_warning("Plugin is not 32-bit (may not be compatible with 32-bit system)");
        }
    #endif
    
    file.close();
    
    return result;
}

} // namespace validator

} // namespace platform
} // namespace backend
} // namespace whatsmy

