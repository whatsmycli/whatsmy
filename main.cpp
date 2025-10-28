// whatsmycli - Unified system information CLI tool
// Copyright (C) 2025 whatsmycli contributors
// Licensed under GPLv3

#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "whatsmy - System Information Tool" << std::endl;
    
    if (argc < 2) {
        std::cout << "Usage: whatsmy <component>" << std::endl;
        std::cout << "Example: whatsmy gpu" << std::endl;
        return 1;
    }
    
    // TODO: Implement plugin loading and command routing
    std::cout << "Requested component: " << argv[1] << std::endl;
    
    return 0;
}

