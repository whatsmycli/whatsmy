// whatsmycli - Plugin Validator Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include "whatsmy/plugin_validator.h"
#include "whatsmy/helpers.h"
#include <filesystem>
#include <fstream>

namespace whatsmy {
namespace backend {

// Forward declaration of platform-specific validator functions
namespace platform {
namespace validator {
    ValidationResult check_symbols(const std::string& plugin_path);
    ValidationResult verify_binary_format(const std::string& plugin_path);
}
}

ValidationResult PluginValidator::validate(const std::string& plugin_path) {
    ValidationResult result;
    
    // Step 1: Check file access
    ValidationResult access_result = check_file_access(plugin_path);
    if (!access_result.valid) {
        return access_result;
    }
    
    // Collect warnings from file access check
    for (const auto& warning : access_result.warnings) {
        result.add_warning(warning);
    }
    
    // Step 2: Verify binary format
    ValidationResult format_result = check_binary_format(plugin_path);
    if (!format_result.valid) {
        return format_result;
    }
    
    // Collect warnings from format check
    for (const auto& warning : format_result.warnings) {
        result.add_warning(warning);
    }
    
    // Step 3: Check required symbols
    ValidationResult symbol_result = check_required_symbols(plugin_path);
    if (!symbol_result.valid) {
        return symbol_result;
    }
    
    // Collect warnings from symbol check
    for (const auto& warning : symbol_result.warnings) {
        result.add_warning(warning);
    }
    
    helpers::error::debug_log("Plugin validation successful: " + plugin_path);
    
    return result;
}

ValidationResult PluginValidator::check_file_access(const std::string& plugin_path) {
    ValidationResult result;
    
    // Check if file exists
    if (!std::filesystem::exists(plugin_path)) {
        result.add_error("Plugin file does not exist: " + plugin_path);
        return result;
    }
    
    // Check if it's a regular file
    if (!std::filesystem::is_regular_file(plugin_path)) {
        result.add_error("Plugin path is not a regular file: " + plugin_path);
        return result;
    }
    
    // Check file size
    std::error_code ec;
    std::uintmax_t file_size = std::filesystem::file_size(plugin_path, ec);
    
    if (ec) {
        result.add_error("Failed to determine plugin file size: " + ec.message());
        return result;
    }
    
    if (file_size == 0) {
        result.add_error("Plugin file is empty");
        return result;
    }
    
    if (file_size < MIN_PLUGIN_SIZE) {
        result.add_error("Plugin file is too small (" + std::to_string(file_size) + 
                        " bytes). Minimum expected size: " + std::to_string(MIN_PLUGIN_SIZE) + " bytes");
        return result;
    }
    
    if (file_size > MAX_PLUGIN_SIZE) {
        result.add_error("Plugin file is too large (" + std::to_string(file_size) + 
                        " bytes). Maximum allowed size: " + std::to_string(MAX_PLUGIN_SIZE) + " bytes");
        return result;
    }
    
    // Check if file is readable
    auto perms = std::filesystem::status(plugin_path, ec).permissions();
    if (ec) {
        result.add_error("Failed to check plugin file permissions: " + ec.message());
        return result;
    }
    
    using std::filesystem::perms;
    bool is_readable = (perms & perms::owner_read) != perms::none ||
                       (perms & perms::group_read) != perms::none ||
                       (perms & perms::others_read) != perms::none;
    
    if (!is_readable) {
        result.add_error("Plugin file is not readable");
        return result;
    }
    
    // Warn if file is writable by others (security concern)
    bool world_writable = (perms & perms::others_write) != perms::none;
    if (world_writable) {
        result.add_warning("Plugin file is writable by others (potential security risk)");
    }
    
    helpers::error::debug_log("Plugin file access check passed: " + plugin_path);
    
    return result;
}

ValidationResult PluginValidator::check_required_symbols(const std::string& plugin_path) {
    // Delegate to platform-specific implementation
    ValidationResult result = platform::validator::check_symbols(plugin_path);
    
    if (result.valid) {
        helpers::error::debug_log("Plugin symbol check passed: " + plugin_path);
    } else {
        helpers::error::debug_log("Plugin symbol check failed: " + result.error_message);
    }
    
    return result;
}

ValidationResult PluginValidator::check_binary_format(const std::string& plugin_path) {
    // Delegate to platform-specific implementation
    ValidationResult result = platform::validator::verify_binary_format(plugin_path);
    
    if (result.valid) {
        helpers::error::debug_log("Plugin binary format check passed: " + plugin_path);
    } else {
        helpers::error::debug_log("Plugin binary format check failed: " + result.error_message);
    }
    
    return result;
}

bool PluginValidator::validate_return_code(int return_code) {
    // Standard convention:
    // 0 = success
    // 1-7 = standard error codes (as documented in plugin API)
    // 8-255 = custom plugin error codes
    // Negative values are not valid
    
    if (return_code < 0) {
        helpers::error::warning_log("Plugin returned negative exit code: " + std::to_string(return_code));
        return false;
    }
    
    if (return_code > 255) {
        helpers::error::warning_log("Plugin returned exit code > 255: " + std::to_string(return_code));
        return false;
    }
    
    return true;
}

std::string PluginValidator::get_return_code_description(int return_code) {
    // Standard return codes as documented in plugin API
    switch (return_code) {
        case 0:
            return "Success";
        case 1:
            return "General error";
        case 2:
            return "Invalid arguments or usage";
        case 3:
            return "Required data or resource not found";
        case 4:
            return "Permission denied or access error";
        case 5:
            return "I/O error or data read failure";
        case 6:
            return "Unsupported platform or feature";
        case 7:
            return "Initialization or setup failed";
        default:
            if (return_code > 7 && return_code <= 255) {
                return "Plugin-specific error code: " + std::to_string(return_code);
            } else if (return_code < 0) {
                return "Invalid negative return code: " + std::to_string(return_code);
            } else {
                return "Invalid return code (> 255): " + std::to_string(return_code);
            }
    }
}

} // namespace backend
} // namespace whatsmy

