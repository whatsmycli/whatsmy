// plugin_manager.cpp - Plugin management system implementation
// Copyright (C) 2025 enXov
// Licensed under the GNU General Public License v3.0

#include "whatsmy/plugin_manager.h"
#include "whatsmy/helpers.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace whatsmy {
namespace plugin_manager {

namespace {
    // Repository configuration (singleton)
    RepositoryConfig& get_repo_config() {
        static RepositoryConfig config;
        return config;
    }
    
    // Simple JSON parser for plugins.json
    // Note: This is a minimal implementation. For production, consider using a JSON library.
    std::vector<PluginInfo> parse_plugins_json(const std::string& json_content) {
        std::vector<PluginInfo> plugins;
        
        // Find "plugins" array
        size_t plugins_start = json_content.find("\"plugins\"");
        if (plugins_start == std::string::npos) {
            return plugins;
        }
        
        // Find the opening bracket of the plugins array
        size_t array_start = json_content.find('[', plugins_start);
        if (array_start == std::string::npos) {
            return plugins;
        }
        
        // Parse each plugin object
        size_t pos = array_start + 1;
        while (pos < json_content.length()) {
            // Find next plugin object
            size_t obj_start = json_content.find('{', pos);
            if (obj_start == std::string::npos) break;
            
            size_t obj_end = json_content.find('}', obj_start);
            if (obj_end == std::string::npos) break;
            
            std::string obj = json_content.substr(obj_start, obj_end - obj_start + 1);
            
            // Parse plugin info (simple string extraction)
            PluginInfo info;
            
            // Extract name
            size_t name_pos = obj.find("\"name\"");
            if (name_pos != std::string::npos) {
                size_t name_start = obj.find('\"', name_pos + 7);
                size_t name_end = obj.find('\"', name_start + 1);
                if (name_start != std::string::npos && name_end != std::string::npos) {
                    info.name = obj.substr(name_start + 1, name_end - name_start - 1);
                }
            }
            
            // Extract description
            size_t desc_pos = obj.find("\"description\"");
            if (desc_pos != std::string::npos) {
                size_t desc_start = obj.find('\"', desc_pos + 14);
                size_t desc_end = obj.find('\"', desc_start + 1);
                if (desc_start != std::string::npos && desc_end != std::string::npos) {
                    info.description = obj.substr(desc_start + 1, desc_end - desc_start - 1);
                }
            }
            
            // Extract version
            size_t ver_pos = obj.find("\"version\"");
            if (ver_pos != std::string::npos) {
                size_t ver_start = obj.find('\"', ver_pos + 10);
                size_t ver_end = obj.find('\"', ver_start + 1);
                if (ver_start != std::string::npos && ver_end != std::string::npos) {
                    info.version = obj.substr(ver_start + 1, ver_end - ver_start - 1);
                }
            }
            
            // Extract author
            size_t auth_pos = obj.find("\"author\"");
            if (auth_pos != std::string::npos) {
                size_t auth_start = obj.find('\"', auth_pos + 9);
                size_t auth_end = obj.find('\"', auth_start + 1);
                if (auth_start != std::string::npos && auth_end != std::string::npos) {
                    info.author = obj.substr(auth_start + 1, auth_end - auth_start - 1);
                }
            }
            
            // Extract platforms
            size_t plat_pos = obj.find("\"platforms\"");
            if (plat_pos != std::string::npos) {
                size_t plat_start = obj.find('[', plat_pos);
                size_t plat_end = obj.find(']', plat_start);
                if (plat_start != std::string::npos && plat_end != std::string::npos) {
                    std::string plat_array = obj.substr(plat_start + 1, plat_end - plat_start - 1);
                    size_t p = 0;
                    while (p < plat_array.length()) {
                        size_t start = plat_array.find('\"', p);
                        if (start == std::string::npos) break;
                        size_t end = plat_array.find('\"', start + 1);
                        if (end == std::string::npos) break;
                        info.platforms.push_back(plat_array.substr(start + 1, end - start - 1));
                        p = end + 1;
                    }
                }
            }
            
            if (!info.name.empty()) {
                plugins.push_back(info);
            }
            
            pos = obj_end + 1;
            
            // Check if this was the last plugin
            size_t next_obj = json_content.find('{', pos);
            size_t array_end = json_content.find(']', pos);
            if (array_end != std::string::npos && (next_obj == std::string::npos || next_obj > array_end)) {
                break;
            }
        }
        
        return plugins;
    }
    
    // Execute a command and capture output
    std::string exec_command(const std::string& command) {
        std::string result;
        
#ifdef _WIN32
        FILE* pipe = _popen(command.c_str(), "r");
#else
        FILE* pipe = popen(command.c_str(), "r");
#endif
        
        if (!pipe) {
            return "";
        }
        
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        
#ifdef _WIN32
        _pclose(pipe);
#else
        pclose(pipe);
#endif
        
        return result;
    }
    
    // Download file using curl or wget
    bool download_file_http(const std::string& url, const std::string& dest_path) {
        // Try curl first
        std::string curl_cmd = "curl -sSL -o \"" + dest_path + "\" \"" + url + "\" 2>&1";
        std::string curl_result = exec_command(curl_cmd);
        
        // Check if download succeeded
        if (fs::exists(dest_path) && fs::file_size(dest_path) > 0) {
            return true;
        }
        
        // Try wget as fallback
        std::string wget_cmd = "wget -q -O \"" + dest_path + "\" \"" + url + "\" 2>&1";
        std::string wget_result = exec_command(wget_cmd);
        
        return fs::exists(dest_path) && fs::file_size(dest_path) > 0;
    }
    
    // Calculate SHA256 checksum of a file
    std::string calculate_sha256(const std::string& file_path) {
#ifdef _WIN32
        // Windows: Use certutil
        std::string cmd = "certutil -hashfile \"" + file_path + "\" SHA256";
#else
        // Unix: Try sha256sum, then shasum
        std::string cmd = "sha256sum \"" + file_path + "\" 2>/dev/null || shasum -a 256 \"" + file_path + "\"";
#endif
        
        std::string output = exec_command(cmd);
        
        // Extract hash from output
        std::istringstream iss(output);
        std::string hash;
        
#ifdef _WIN32
        // certutil output: skip first line, hash is on second line
        std::string line;
        std::getline(iss, line); // Skip "SHA256 hash of file..."
        std::getline(iss, line); // Get hash line
        // Remove spaces from hash
        line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        hash = line;
#else
        // Unix: hash is first token
        iss >> hash;
#endif
        
        // Convert to lowercase
        std::transform(hash.begin(), hash.end(), hash.begin(), ::tolower);
        
        return hash;
    }
}

std::vector<PluginInfo> fetch_plugin_list() {
    const auto& config = get_repo_config();
    
    // Create temporary file for download
    fs::path temp_path = fs::temp_directory_path() / "whatsmy_plugins.json";
    std::string temp_file = temp_path.string();
    
    helpers::error::log(helpers::error::Level::DEBUG, "Fetching plugin list from: " + config.metadata_url);
    
    // Download plugins.json
    if (!download_file_http(config.metadata_url, temp_file)) {
        helpers::output::print_error("Failed to download plugin metadata from repository");
        return {};
    }
    
    // Read the downloaded file
    std::ifstream file(temp_file);
    if (!file) {
        helpers::output::print_error("Failed to read plugin metadata");
        return {};
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    // Clean up temp file
    fs::remove(temp_file);
    
    // Parse JSON
    auto plugins = parse_plugins_json(content);
    
    helpers::error::log(helpers::error::Level::DEBUG, "Found " + std::to_string(plugins.size()) + " plugins");
    
    return plugins;
}

std::vector<std::string> get_installed_plugins(const std::string& plugin_dir) {
    std::vector<std::string> installed;
    
    if (!fs::exists(plugin_dir) || !fs::is_directory(plugin_dir)) {
        return installed;
    }
    
    // Iterate through plugin directory
    for (const auto& entry : fs::directory_iterator(plugin_dir)) {
        if (entry.is_directory()) {
            std::string plugin_name = entry.path().filename().string();
            
            // Check if the plugin has a binary for the current platform
            std::string binary_name = get_platform_binary_name();
            fs::path binary_path_obj = entry.path() / binary_name;
            std::string binary_path = binary_path_obj.string();
            
            if (fs::exists(binary_path)) {
                installed.push_back(plugin_name);
            }
        }
    }
    
    return installed;
}

DownloadResult download_plugin(const std::string& plugin_name,
                                const std::string& platform,
                                const std::string& dest_path) {
    DownloadResult result{false, "", ""};
    
    const auto& config = get_repo_config();
    
    // Construct URL: base_url/plugin_name/platform.ext
    std::string ext;
    if (platform == "linux") ext = ".so";
    else if (platform == "windows") ext = ".dll";
    else if (platform == "macos") ext = ".dylib";
    else {
        result.error_message = "Unknown platform: " + platform;
        return result;
    }
    
    std::string url = config.base_url + "/" + plugin_name + "/" + platform + ext;
    
    helpers::error::log(helpers::error::Level::DEBUG, "Downloading from: " + url);
    
    // Download the binary
    if (!download_file_http(url, dest_path)) {
        result.error_message = "Failed to download plugin binary";
        return result;
    }
    
    result.success = true;
    result.file_path = dest_path;
    return result;
}

bool install_plugin(const std::string& plugin_name, const std::string& plugin_dir) {
    helpers::output::print_info("Installing plugin: " + plugin_name);
    
    // Check if plugin directory is writable
    fs::path base_dir(plugin_dir);
    if (fs::exists(base_dir)) {
        // Check if we can write to the directory
#ifndef _WIN32
        if (access(base_dir.c_str(), W_OK) != 0) {
            helpers::output::print_error("Permission denied: Cannot write to plugin directory");
            std::cout << "\nPlugin directory: " << base_dir.generic_string() << "\n";
            std::cout << "\nTo fix this, try one of the following:\n";
            std::cout << "  1. Run with sudo: " << helpers::output::colorize("sudo whatsmy plugin install " + plugin_name, helpers::output::Color::GREEN) << "\n";
            std::cout << "  2. Change directory permissions: " << helpers::output::colorize("sudo chmod -R 775 " + base_dir.generic_string(), helpers::output::Color::GREEN) << "\n";
            std::cout << "  3. Set a user-writable plugin directory:\n";
            std::cout << "     " << helpers::output::colorize("export WHATSMY_PLUGIN_DIR=~/.local/lib/whatsmy/plugins", helpers::output::Color::GREEN) << "\n";
            std::cout << "     " << helpers::output::colorize("mkdir -p ~/.local/lib/whatsmy/plugins", helpers::output::Color::GREEN) << "\n";
            return false;
        }
#endif
    } else {
        // Try to create the base directory to check permissions
        try {
            fs::create_directories(base_dir);
        } catch (const std::exception& e) {
            helpers::output::print_error("Failed to create plugin directory: " + std::string(e.what()));
#ifndef _WIN32
            std::cout << "\nTry running with sudo: " << helpers::output::colorize("sudo whatsmy plugin install " + plugin_name, helpers::output::Color::GREEN) << "\n";
            std::cout << "Or set a user-writable plugin directory with WHATSMY_PLUGIN_DIR environment variable.\n";
#endif
            return false;
        }
    }
    
    // Fetch plugin list to get metadata
    auto plugins = fetch_plugin_list();
    if (plugins.empty()) {
        helpers::output::print_error("Failed to fetch plugin list");
        return false;
    }
    
    // Find the plugin
    auto it = std::find_if(plugins.begin(), plugins.end(),
                          [&plugin_name](const PluginInfo& p) { return p.name == plugin_name; });
    
    if (it == plugins.end()) {
        helpers::output::print_error("Plugin '" + plugin_name + "' not found in repository");
        return false;
    }
    
    const PluginInfo& plugin = *it;
    
    // Check if platform is supported
    std::string platform = get_platform_name();
    if (std::find(plugin.platforms.begin(), plugin.platforms.end(), platform) == plugin.platforms.end()) {
        helpers::output::print_error("Plugin '" + plugin_name + "' is not available for " + platform);
        std::cout << "Available platforms: ";
        for (size_t i = 0; i < plugin.platforms.size(); ++i) {
            std::cout << plugin.platforms[i];
            if (i < plugin.platforms.size() - 1) std::cout << ", ";
        }
        std::cout << "\n";
        return false;
    }
    
    // Create plugin directory
    fs::path plugin_path = fs::path(plugin_dir) / plugin_name;
    if (!fs::exists(plugin_path)) {
        try {
            fs::create_directories(plugin_path);
        } catch (const std::exception& e) {
            helpers::output::print_error("Failed to create plugin directory: " + std::string(e.what()));
            return false;
        }
    }
    
    // Download plugin binary
    std::string binary_name = get_platform_binary_name();
    fs::path dest_path_obj = plugin_path / binary_name;
    std::string dest_path = dest_path_obj.string();
    
    auto result = download_plugin(plugin_name, platform, dest_path);
    if (!result.success) {
        helpers::output::print_error(result.error_message);
        return false;
    }
    
    // Verify checksum if available
    if (plugin.checksums.count(platform) > 0) {
        std::string expected_checksum = plugin.checksums.at(platform);
        if (expected_checksum != "sha256:placeholder" && !expected_checksum.empty()) {
            if (!verify_checksum(dest_path, expected_checksum)) {
                helpers::output::print_warning("Checksum verification failed for plugin '" + plugin_name + "'");
                helpers::output::print_warning("The plugin may be corrupted or tampered with");
                // Don't fail installation, just warn
            } else {
                helpers::error::log(helpers::error::Level::DEBUG, "Checksum verification passed");
            }
        }
    }
    
    // Set execute permissions on Unix
#ifndef _WIN32
    chmod(dest_path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
    
    helpers::output::print_success("Plugin '" + plugin_name + "' installed successfully");
    // Use generic_string() for consistent path display across platforms
    std::cout << "Location: " << plugin_path.generic_string() << "\n";
    
    return true;
}

bool remove_plugin(const std::string& plugin_name, const std::string& plugin_dir) {
    fs::path plugin_path = fs::path(plugin_dir) / plugin_name;
    
    if (!fs::exists(plugin_path)) {
        helpers::output::print_error("Plugin '" + plugin_name + "' is not installed");
        return false;
    }
    
    helpers::output::print_info("Removing plugin: " + plugin_name);
    
    try {
        fs::remove_all(plugin_path);
        helpers::output::print_success("Plugin '" + plugin_name + "' removed successfully");
        return true;
    } catch (const std::exception& e) {
        helpers::output::print_error("Failed to remove plugin: " + std::string(e.what()));
        return false;
    }
}

bool update_plugin(const std::string& plugin_name, const std::string& plugin_dir) {
    // Check if plugin is installed
    auto installed = get_installed_plugins(plugin_dir);
    if (std::find(installed.begin(), installed.end(), plugin_name) == installed.end()) {
        helpers::output::print_error("Plugin '" + plugin_name + "' is not installed");
        return false;
    }
    
    helpers::output::print_info("Updating plugin: " + plugin_name);
    
    // Remove and reinstall
    if (!remove_plugin(plugin_name, plugin_dir)) {
        return false;
    }
    
    return install_plugin(plugin_name, plugin_dir);
}

std::vector<PluginInfo> search_plugins(const std::string& query,
                                        const std::vector<PluginInfo>& available_plugins) {
    std::vector<PluginInfo> results;
    std::string query_lower = query;
    std::transform(query_lower.begin(), query_lower.end(), query_lower.begin(), ::tolower);
    
    for (const auto& plugin : available_plugins) {
        std::string name_lower = plugin.name;
        std::string desc_lower = plugin.description;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
        std::transform(desc_lower.begin(), desc_lower.end(), desc_lower.begin(), ::tolower);
        
        if (name_lower.find(query_lower) != std::string::npos ||
            desc_lower.find(query_lower) != std::string::npos) {
            results.push_back(plugin);
        }
    }
    
    return results;
}

bool verify_checksum(const std::string& file_path, const std::string& expected_checksum) {
    if (!fs::exists(file_path)) {
        return false;
    }
    
    // Extract hash from expected format: "sha256:hash"
    std::string expected_hash = expected_checksum;
    if (expected_hash.substr(0, 7) == "sha256:") {
        expected_hash = expected_hash.substr(7);
    }
    
    std::string actual_hash = calculate_sha256(file_path);
    
    helpers::error::log(helpers::error::Level::DEBUG, "Expected: " + expected_hash);
    helpers::error::log(helpers::error::Level::DEBUG, "Actual:   " + actual_hash);
    
    return actual_hash == expected_hash;
}

std::string get_platform_name() {
#ifdef _WIN32
    return "windows";
#elif __APPLE__
    return "macos";
#else
    return "linux";
#endif
}

std::string get_platform_binary_name() {
#ifdef _WIN32
    return "windows.dll";
#elif __APPLE__
    return "macos.dylib";
#else
    return "linux.so";
#endif
}

void display_plugin_list(const std::vector<PluginInfo>& plugins) {
    if (plugins.empty()) {
        std::cout << "No plugins found.\n";
        return;
    }
    
    std::cout << "\n";
    helpers::output::print_heading("Available Plugins");
    std::cout << "\n";
    
    // Create table
    helpers::output::Table table;
    // Use SIMPLE border style on Windows for better compatibility
#ifdef _WIN32
    table.set_border_style(helpers::output::BorderStyle::SIMPLE);
#else
    table.set_border_style(helpers::output::BorderStyle::ROUNDED);
#endif
    
    // Add header
    table.add_row({"Name", "Version", "Description", "Platforms"});
    
    // Add plugins
    for (const auto& plugin : plugins) {
        std::string platforms;
        for (size_t i = 0; i < plugin.platforms.size(); ++i) {
            platforms += plugin.platforms[i];
            if (i < plugin.platforms.size() - 1) platforms += ", ";
        }
        
        table.add_row({plugin.name, plugin.version, plugin.description, platforms});
    }
    
    table.print();
    
    std::cout << "\n" << helpers::output::colorize("Total: ", helpers::output::Color::CYAN) 
              << plugins.size() << " plugins\n";
    std::cout << "\nUse " << helpers::output::colorize("whatsmy plugin install <name>", helpers::output::Color::GREEN)
              << " to install a plugin\n";
}

void display_installed_plugins(const std::vector<std::string>& plugin_names,
                                const std::string& plugin_dir) {
    if (plugin_names.empty()) {
        std::cout << "No plugins installed.\n";
        std::cout << "\nUse " << helpers::output::colorize("whatsmy plugin list", helpers::output::Color::GREEN)
                  << " to see available plugins\n";
        return;
    }
    
    std::cout << "\n";
    helpers::output::print_heading("Installed Plugins");
    std::cout << "\n";
    
    // Create table
    helpers::output::Table table;
    // Use SIMPLE border style on Windows for better compatibility
#ifdef _WIN32
    table.set_border_style(helpers::output::BorderStyle::SIMPLE);
#else
    table.set_border_style(helpers::output::BorderStyle::ROUNDED);
#endif
    
    // Add header
    table.add_row({"Name", "Location"});
    
    // Add plugins
    for (const auto& name : plugin_names) {
        // Use generic_string() for consistent path display across platforms
        std::string location = (fs::path(plugin_dir) / name).generic_string();
        table.add_row({name, location});
    }
    
    table.print();
    
    std::cout << "\n" << helpers::output::colorize("Total: ", helpers::output::Color::CYAN) 
              << plugin_names.size() << " plugins installed\n";
}

void display_plugin_info(const PluginInfo& plugin) {
    std::cout << "\n";
    helpers::output::print_heading("Plugin: " + plugin.name);
    std::cout << "\n";
    
    std::cout << helpers::output::colorize("Version: ", helpers::output::Color::CYAN) << plugin.version << "\n";
    std::cout << helpers::output::colorize("Author: ", helpers::output::Color::CYAN) << plugin.author << "\n";
    std::cout << helpers::output::colorize("License: ", helpers::output::Color::CYAN) << plugin.license << "\n";
    std::cout << helpers::output::colorize("Description: ", helpers::output::Color::CYAN) << plugin.description << "\n";
    
    std::cout << helpers::output::colorize("Platforms: ", helpers::output::Color::CYAN);
    for (size_t i = 0; i < plugin.platforms.size(); ++i) {
        std::cout << plugin.platforms[i];
        if (i < plugin.platforms.size() - 1) std::cout << ", ";
    }
    std::cout << "\n";
    
    if (!plugin.tags.empty()) {
        std::cout << helpers::output::colorize("Tags: ", helpers::output::Color::CYAN);
        for (size_t i = 0; i < plugin.tags.size(); ++i) {
            std::cout << plugin.tags[i];
            if (i < plugin.tags.size() - 1) std::cout << ", ";
        }
        std::cout << "\n";
    }
}

} // namespace plugin_manager
} // namespace whatsmy

