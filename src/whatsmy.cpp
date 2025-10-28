// whatsmycli - Main Application Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include "whatsmy/whatsmy.h"
#include "whatsmy/plugin_loader.h"
#include "whatsmy/helpers.h"
#include <iostream>
#include <string>

namespace whatsmy {

void show_help() {
    std::cout << APP_NAME << " - System Information Tool\n"
              << "\nUsage:\n"
              << "  " << APP_NAME << " <component>  Run plugin for component\n"
              << "  " << APP_NAME << " help         Show this help message\n"
              << "  " << APP_NAME << " version      Show version information\n"
              << "\nExamples:\n"
              << "  " << APP_NAME << " gpu          Display GPU information\n"
              << "  " << APP_NAME << " cpu          Display CPU information\n"
              << std::endl;
}

void show_version() {
    std::cout << APP_NAME << " version " << VERSION << "\n"
              << "Licensed under GPLv3\n"
              << "Copyright (C) 2025 enXov\n"
              << std::endl;
}

int run(int argc, char* argv[]) {
    // No arguments provided
    if (argc < 2) {
        show_help();
        return static_cast<int>(ExitCode::INVALID_ARGS);
    }
    
    std::string command = argv[1];
    
    // Handle built-in commands
    if (command == "help" || command == "--help" || command == "-h") {
        show_help();
        return static_cast<int>(ExitCode::SUCCESS);
    }
    
    if (command == "version" || command == "--version" || command == "-v") {
        show_version();
        return static_cast<int>(ExitCode::SUCCESS);
    }
    
    // Try to load and run plugin
    // TODO: Implement plugin loading in Phase 1
    helpers::output::print_error("Plugin system not yet implemented");
    helpers::output::print("Requested component: " + command);
    
    return static_cast<int>(ExitCode::PLUGIN_NOT_FOUND);
}

} // namespace whatsmy

