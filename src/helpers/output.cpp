// whatsmycli - Output Helper Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include "whatsmy/helpers.h"
#include <iostream>

namespace whatsmy {
namespace helpers {
namespace output {

void print(const std::string& message) {
    std::cout << message << std::endl;
}

void print_info(const std::string& message) {
    std::cout << "Info: " << message << std::endl;
}

void print_error(const std::string& message) {
    std::cerr << "Error: " << message << std::endl;
}

} // namespace output
} // namespace helpers
} // namespace whatsmy

