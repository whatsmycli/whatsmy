// whatsmycli - Main Application Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include "whatsmy/whatsmy.h"
#include "whatsmy/plugin_loader.h"
#include "whatsmy/plugin_manager.h"
#include "whatsmy/helpers.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>

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
    std::cout << "Usage:\n"
              << "  " << APP_NAME << " <plugin name>           Run plugin\n"
              << "\nCOMMANDS:\n"
              << "  help                      Show this help message\n"
              << "  version                   Show version information\n"
              << "\nPLUGIN MANAGEMENT:\n"
              << "  plugin list               List all available plugins\n"
              << "  plugin installed          Show installed plugins\n"
              << "  plugin install <name>     Install a plugin\n"
              << "  plugin remove <name>      Remove a plugin\n"
              << "  plugin update <name>      Update a plugin\n"
              << "  plugin search <term>      Search for plugins\n"
              << std::endl;
}

void show_version() {
    std::cout << APP_NAME << " version " << VERSION << "\n"
              << "Licensed under GPLv3\n"
              << "Copyright (C) 2025 enXov\n"
              << std::endl;
}

int run(int argc, char* argv[]) {
    // Check for debug flag early
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--debug" || arg == "-d") {
            helpers::error::set_log_level(helpers::error::Level::DEBUG);
            helpers::error::debug_log("Debug mode enabled");
            break;
        }
    }
    
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
    
    // Handle plugin management commands
    if (command == "plugin") {
        if (argc < 3) {
            helpers::output::print_error("'plugin' command requires a subcommand");
            std::cout << "\nAvailable subcommands:\n"
                      << "  list               List all available plugins\n"
                      << "  installed          Show installed plugins\n"
                      << "  install <name>     Install a plugin\n"
                      << "  remove <name>      Remove a plugin\n"
                      << "  update <name>      Update a plugin\n"
                      << "  search <term>      Search for plugins\n"
                      << "\nTry: " << APP_NAME << " plugin list\n";
            return static_cast<int>(ExitCode::INVALID_ARGS);
        }
        
        std::string subcommand = argv[2];
        
        // Get plugin directory (use platform-specific default)
        const char* plugin_dir_env = std::getenv("WHATSMY_PLUGIN_DIR");
        std::string plugin_dir = plugin_dir_env ? plugin_dir_env : backend::PluginLoader::get_plugin_directory();
        
        if (subcommand == "list") {
            auto plugins = plugin_manager::fetch_plugin_list();
            if (plugins.empty()) {
                helpers::output::print_error("Failed to fetch plugin list");
                return static_cast<int>(ExitCode::PLUGIN_LOAD_ERROR);
            }
            plugin_manager::display_plugin_list(plugins);
            return static_cast<int>(ExitCode::SUCCESS);
        }
        
        else if (subcommand == "installed") {
            auto installed = plugin_manager::get_installed_plugins(plugin_dir);
            plugin_manager::display_installed_plugins(installed, plugin_dir);
            return static_cast<int>(ExitCode::SUCCESS);
        }
        
        else if (subcommand == "install") {
            if (argc < 4) {
                helpers::output::print_error("'plugin install' requires a plugin name");
                std::cout << "Usage: " << APP_NAME << " plugin install <name>\n";
                return static_cast<int>(ExitCode::INVALID_ARGS);
            }
            std::string plugin_name = argv[3];
            bool success = plugin_manager::install_plugin(plugin_name, plugin_dir);
            return success ? static_cast<int>(ExitCode::SUCCESS) 
                          : static_cast<int>(ExitCode::PLUGIN_LOAD_ERROR);
        }
        
        else if (subcommand == "remove") {
            if (argc < 4) {
                helpers::output::print_error("'plugin remove' requires a plugin name");
                std::cout << "Usage: " << APP_NAME << " plugin remove <name>\n";
                return static_cast<int>(ExitCode::INVALID_ARGS);
            }
            std::string plugin_name = argv[3];
            bool success = plugin_manager::remove_plugin(plugin_name, plugin_dir);
            return success ? static_cast<int>(ExitCode::SUCCESS) 
                          : static_cast<int>(ExitCode::PLUGIN_LOAD_ERROR);
        }
        
        else if (subcommand == "update") {
            if (argc < 4) {
                helpers::output::print_error("'plugin update' requires a plugin name");
                std::cout << "Usage: " << APP_NAME << " plugin update <name>\n";
                return static_cast<int>(ExitCode::INVALID_ARGS);
            }
            std::string plugin_name = argv[3];
            bool success = plugin_manager::update_plugin(plugin_name, plugin_dir);
            return success ? static_cast<int>(ExitCode::SUCCESS) 
                          : static_cast<int>(ExitCode::PLUGIN_LOAD_ERROR);
        }
        
        else if (subcommand == "search") {
            if (argc < 4) {
                helpers::output::print_error("'plugin search' requires a search term");
                std::cout << "Usage: " << APP_NAME << " plugin search <term>\n";
                return static_cast<int>(ExitCode::INVALID_ARGS);
            }
            std::string query = argv[3];
            auto plugins = plugin_manager::fetch_plugin_list();
            if (plugins.empty()) {
                helpers::output::print_error("Failed to fetch plugin list");
                return static_cast<int>(ExitCode::PLUGIN_LOAD_ERROR);
            }
            auto results = plugin_manager::search_plugins(query, plugins);
            if (results.empty()) {
                std::cout << "No plugins found matching '" << query << "'\n";
            } else {
                std::cout << "\nSearch results for '" << query << "':\n";
                plugin_manager::display_plugin_list(results);
            }
            return static_cast<int>(ExitCode::SUCCESS);
        }
        
        else {
            helpers::output::print_error("Unknown plugin subcommand: " + subcommand);
            std::cout << "Try: " << APP_NAME << " plugin list\n";
            return static_cast<int>(ExitCode::INVALID_ARGS);
        }
    }
    
    // Handle debug flag (skip it if it's the first argument)
    if (command == "--debug" || command == "-d") {
        if (argc < 3) {
            helpers::output::print_error("--debug flag requires a component name");
            std::cout << "Usage: " << APP_NAME << " --debug <component>\n";
            return static_cast<int>(ExitCode::INVALID_ARGS);
        }
        command = argv[2];
        
        // Check for built-in commands again after extracting command from --debug
        if (command == "help" || command == "--help" || command == "-h") {
            show_help();
            return static_cast<int>(ExitCode::SUCCESS);
        }
        
        if (command == "version" || command == "--version" || command == "-v") {
            show_version();
            return static_cast<int>(ExitCode::SUCCESS);
        }
    }
    
    // Route to plugin system
    helpers::error::debug_log("Attempting to load plugin: " + command);
    
    // Prepare arguments for plugin
    // argv[0] = "whatsmy"
    // argv[1] = plugin name (e.g., "example")
    // argv[2+] = additional arguments
    // 
    // We want to pass to plugin:
    // plugin_argv[0] = plugin name
    // plugin_argv[1+] = additional arguments
    //
    // So plugin gets: argc-1 args starting from argv[1]
    
    int plugin_result = backend::PluginLoader::load_and_run(command, argc - 1, argv + 1);
    
    // If plugin execution succeeded, return its exit code
    if (plugin_result == 0) {
        return static_cast<int>(ExitCode::SUCCESS);
    }
    
    // If plugin was not found, show suggestions before returning
    if (plugin_result == static_cast<int>(ExitCode::PLUGIN_NOT_FOUND)) {
        // Find and display similar commands
        auto suggestions = find_similar_commands(command);
        if (!suggestions.empty()) {
            std::cerr << "\nDid you mean one of these?\n";
            for (const auto& suggestion : suggestions) {
                std::cerr << "  " << APP_NAME << " " << suggestion << "\n";
            }
            std::cerr << std::endl;
        }
        
        std::cerr << "Run '" << APP_NAME << " help' for available commands.\n" << std::endl;
        return plugin_result;
    }
    
    // For other plugin errors (load error, exec error), return as-is
    if (plugin_result == static_cast<int>(ExitCode::PLUGIN_LOAD_ERROR) ||
        plugin_result == static_cast<int>(ExitCode::PLUGIN_EXEC_ERROR)) {
        return plugin_result;
    }
    
    // Unknown error code - shouldn't happen, but handle gracefully
    helpers::output::print_error("Unknown error occurred while loading plugin");
    return static_cast<int>(ExitCode::PLUGIN_LOAD_ERROR);
}

} // namespace whatsmy

