// whatsmycli - Main Application Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include "whatsmy/whatsmy.h"
#include "whatsmy/plugin_loader.h"
#include "whatsmy/helpers.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

namespace whatsmy {

namespace {
    // Common components that might be available as plugins
    const std::vector<std::string> KNOWN_COMPONENTS = {
        "cpu", "gpu", "ram", "disk", "os", "network", 
        "battery", "display", "audio", "usb"
    };

    /**
     * Calculate Levenshtein distance between two strings
     * Used for command suggestions
     */
    int levenshtein_distance(const std::string& s1, const std::string& s2) {
        const size_t len1 = s1.size(), len2 = s2.size();
        std::vector<std::vector<int>> d(len1 + 1, std::vector<int>(len2 + 1));

        for (size_t i = 0; i <= len1; ++i) d[i][0] = i;
        for (size_t i = 0; i <= len2; ++i) d[0][i] = i;

        for (size_t i = 1; i <= len1; ++i) {
            for (size_t j = 1; j <= len2; ++j) {
                int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
                d[i][j] = std::min({
                    d[i - 1][j] + 1,      // deletion
                    d[i][j - 1] + 1,      // insertion
                    d[i - 1][j - 1] + cost // substitution
                });
            }
        }

        return d[len1][len2];
    }

    /**
     * Find similar commands to suggest
     */
    std::vector<std::string> find_similar_commands(const std::string& command) {
        std::vector<std::pair<int, std::string>> scored_commands;
        
        for (const auto& known : KNOWN_COMPONENTS) {
            int distance = levenshtein_distance(command, known);
            // Only suggest if distance is small (3 or less)
            if (distance <= 3) {
                scored_commands.push_back({distance, known});
            }
        }

        // Sort by distance (closest first)
        std::sort(scored_commands.begin(), scored_commands.end());

        // Extract just the command names
        std::vector<std::string> suggestions;
        for (const auto& pair : scored_commands) {
            suggestions.push_back(pair.second);
        }

        return suggestions;
    }
} // anonymous namespace

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
    
    // Route to plugin system
    helpers::error::debug_log("Attempting to load plugin: " + command);
    
    int plugin_result = backend::PluginLoader::load_and_run(command);
    
    // If plugin execution succeeded, return its exit code
    if (plugin_result == 0) {
        return static_cast<int>(ExitCode::SUCCESS);
    }
    
    // Handle plugin not found - provide suggestions
    helpers::output::print_error("Unknown component: '" + command + "'");
    
    // Find and display similar commands
    auto suggestions = find_similar_commands(command);
    if (!suggestions.empty()) {
        std::cout << "\nDid you mean one of these?\n";
        for (const auto& suggestion : suggestions) {
            std::cout << "  " << APP_NAME << " " << suggestion << "\n";
        }
        std::cout << std::endl;
    }
    
    std::cout << "Run '" << APP_NAME << " help' for available commands.\n" << std::endl;
    
    return static_cast<int>(ExitCode::PLUGIN_NOT_FOUND);
}

} // namespace whatsmy

