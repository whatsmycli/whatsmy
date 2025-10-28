// whatsmycli - Helper Functions Header
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#ifndef WHATSMY_HELPERS_H
#define WHATSMY_HELPERS_H

#include <string>

namespace whatsmy {
namespace helpers {

/**
 * Output formatting functions
 */
namespace output {
    /**
     * Print formatted message
     */
    void print(const std::string& message);
    
    /**
     * Print error message to stderr
     */
    void print_error(const std::string& message);
}

/**
 * Error handling functions
 */
namespace error {
    /**
     * Format error message
     */
    std::string format_error(const std::string& context, const std::string& details);
    
    /**
     * Log debug message (if debug mode enabled)
     */
    void debug_log(const std::string& message);
}

} // namespace helpers
} // namespace whatsmy

#endif // WHATSMY_HELPERS_H

