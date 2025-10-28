// whatsmycli - Error Helper Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include "whatsmy/helpers.h"
#include <iostream>
#include <sstream>

namespace whatsmy {
namespace helpers {
namespace error {

std::string format_error(const std::string& context, const std::string& details) {
    std::ostringstream oss;
    oss << "[" << context << "] " << details;
    return oss.str();
}

void debug_log(const std::string& message) {
    // TODO: Implement debug mode check in Phase 1
    // For now, this is a no-op
    #ifdef DEBUG
    std::cerr << "[DEBUG] " << message << std::endl;
    #endif
}

} // namespace error
} // namespace helpers
} // namespace whatsmy

