// whatsmycli - Error Helper Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include "whatsmy/helpers.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <cstdlib>

namespace whatsmy {
namespace helpers {
namespace error {

namespace {
    // Global log level (default: WARNING)
    Level current_log_level = Level::WARNING;

    /**
     * Get string representation of log level
     */
    const char* get_level_string(Level level) {
        switch (level) {
            case Level::DEBUG:    return "DEBUG";
            case Level::INFO:     return "INFO";
            case Level::WARNING:  return "WARNING";
            case Level::ERROR:    return "ERROR";
            case Level::CRITICAL: return "CRITICAL";
            default:              return "UNKNOWN";
        }
    }

    /**
     * Get color for log level
     */
    output::Color get_level_color(Level level) {
        switch (level) {
            case Level::DEBUG:    return output::Color::BRIGHT_BLACK;
            case Level::INFO:     return output::Color::CYAN;
            case Level::WARNING:  return output::Color::YELLOW;
            case Level::ERROR:    return output::Color::RED;
            case Level::CRITICAL: return output::Color::BRIGHT_RED;
            default:              return output::Color::RESET;
        }
    }

    /**
     * Check if debug mode is enabled via environment variable
     */
    bool is_debug_enabled() {
        const char* debug_env = std::getenv("WHATSMY_DEBUG");
        if (debug_env) {
            std::string debug_str(debug_env);
            return debug_str == "1" || debug_str == "true" || debug_str == "TRUE";
        }
        return false;
    }

    /**
     * Get current timestamp string
     */
    std::string get_timestamp() {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

} // anonymous namespace

void set_log_level(Level level) {
    current_log_level = level;
}

Level get_log_level() {
    return current_log_level;
}

std::string format_error(const std::string& context, const std::string& details) {
    std::ostringstream oss;
    oss << "[" << context << "] " << details;
    return oss.str();
}

std::string format_error(Code code, const std::string& details) {
    std::ostringstream oss;
    oss << get_error_description(code);
    if (!details.empty()) {
        oss << ": " << details;
    }
    return oss.str();
}

std::string get_error_description(Code code) {
    switch (code) {
        case Code::SUCCESS:
            return "Success";
        case Code::INVALID_ARGS:
            return "Invalid arguments";
        case Code::PLUGIN_NOT_FOUND:
            return "Plugin not found";
        case Code::PLUGIN_LOAD_ERROR:
            return "Failed to load plugin";
        case Code::PLUGIN_EXEC_ERROR:
            return "Plugin execution failed";
        case Code::UNKNOWN_ERROR:
            return "Unknown error";
        default:
            return "Unspecified error";
    }
}

void log(Level level, const std::string& message) {
    // Check if message should be logged based on current log level
    if (level < current_log_level) {
        return;
    }

    // Format log message
    std::ostringstream oss;
    
    // Add timestamp for ERROR and CRITICAL levels
    if (level >= Level::ERROR) {
        oss << "[" << get_timestamp() << "] ";
    }

    // Add level indicator
    std::string level_str = std::string("[") + get_level_string(level) + "]";
    level_str = output::colorize(level_str, get_level_color(level));
    oss << level_str << " " << message;

    // Output to appropriate stream
    if (level >= Level::ERROR) {
        std::cerr << oss.str() << std::endl;
    } else {
        std::cout << oss.str() << std::endl;
    }
}

void debug_log(const std::string& message) {
    // Only log debug messages if:
    // 1. Built with DEBUG flag, OR
    // 2. WHATSMY_DEBUG environment variable is set, OR
    // 3. Current log level is DEBUG
    #ifdef DEBUG
    log(Level::DEBUG, message);
    #else
    if (is_debug_enabled() || current_log_level == Level::DEBUG) {
        log(Level::DEBUG, message);
    }
    #endif
}

void info_log(const std::string& message) {
    log(Level::INFO, message);
}

void warning_log(const std::string& message) {
    log(Level::WARNING, message);
}

void error_log(const std::string& message) {
    log(Level::ERROR, message);
}

void critical_log(const std::string& message) {
    log(Level::CRITICAL, message);
}

std::string format_exception(const std::exception& e) {
    std::ostringstream oss;
    oss << "Exception: " << e.what();
    return oss.str();
}

} // namespace error
} // namespace helpers
} // namespace whatsmy

