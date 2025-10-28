// whatsmycli - Plugin Validator Header
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#ifndef WHATSMY_PLUGIN_VALIDATOR_H
#define WHATSMY_PLUGIN_VALIDATOR_H

#include <string>
#include <vector>

namespace whatsmy {
namespace backend {

/**
 * Plugin validation result
 */
struct ValidationResult {
    bool valid;
    std::string error_message;
    std::vector<std::string> warnings;
    
    ValidationResult() : valid(true) {}
    
    void add_error(const std::string& message) {
        valid = false;
        error_message = message;
    }
    
    void add_warning(const std::string& message) {
        warnings.push_back(message);
    }
    
    bool has_warnings() const {
        return !warnings.empty();
    }
};

/**
 * Plugin validator for verifying plugin integrity and compatibility
 */
class PluginValidator {
public:
    /**
     * Validate a plugin before loading
     * 
     * This performs comprehensive validation including:
     * - File existence and permissions
     * - Binary format verification
     * - Required symbol checking
     * - Platform compatibility verification
     * 
     * @param plugin_path Full path to the plugin binary
     * @return ValidationResult with validation status and details
     */
    static ValidationResult validate(const std::string& plugin_path);
    
    /**
     * Check if plugin file exists and is readable
     * 
     * @param plugin_path Full path to the plugin binary
     * @return ValidationResult with file check status
     */
    static ValidationResult check_file_access(const std::string& plugin_path);
    
    /**
     * Verify plugin has required symbols (platform-specific)
     * 
     * @param plugin_path Full path to the plugin binary
     * @return ValidationResult with symbol check status
     */
    static ValidationResult check_required_symbols(const std::string& plugin_path);
    
    /**
     * Verify plugin binary format and compatibility
     * 
     * @param plugin_path Full path to the plugin binary
     * @return ValidationResult with binary format check status
     */
    static ValidationResult check_binary_format(const std::string& plugin_path);
    
    /**
     * Validate plugin return code is within expected range
     * 
     * @param return_code The return code from plugin execution
     * @return true if return code is valid, false otherwise
     */
    static bool validate_return_code(int return_code);
    
    /**
     * Get human-readable description of plugin return code
     * 
     * @param return_code The return code from plugin execution
     * @return String description of the return code
     */
    static std::string get_return_code_description(int return_code);

private:
    /**
     * Maximum allowed plugin file size (100MB)
     * This prevents attempting to load obviously invalid files
     */
    static constexpr size_t MAX_PLUGIN_SIZE = 100 * 1024 * 1024;
    
    /**
     * Minimum allowed plugin file size (1KB)
     * Valid shared libraries should be at least this size
     */
    static constexpr size_t MIN_PLUGIN_SIZE = 1024;
};

/**
 * Platform-specific validator namespace
 * Implemented in platform-specific files (linux.cpp, windows.cpp, macos.cpp)
 */
namespace platform {
namespace validator {
    /**
     * Check if plugin has required symbols using platform-specific tools
     * 
     * @param plugin_path Full path to the plugin binary
     * @return ValidationResult with symbol check status
     */
    ValidationResult check_symbols(const std::string& plugin_path);
    
    /**
     * Verify binary format is correct for the platform
     * 
     * @param plugin_path Full path to the plugin binary
     * @return ValidationResult with binary format check status
     */
    ValidationResult verify_binary_format(const std::string& plugin_path);
}
}

} // namespace backend
} // namespace whatsmy

#endif // WHATSMY_PLUGIN_VALIDATOR_H

