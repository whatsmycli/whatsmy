// plugin_manager.h - Plugin management system for downloading and installing plugins
// Copyright (C) 2025 enXov
// Licensed under the GNU General Public License v3.0

#ifndef WHATSMY_PLUGIN_MANAGER_H
#define WHATSMY_PLUGIN_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <optional>

namespace whatsmy {
namespace plugin_manager {

// Plugin information structure
struct PluginInfo {
    std::string name;
    std::string description;
    std::string version;
    std::string author;
    std::string license;
    std::vector<std::string> platforms;
    std::map<std::string, std::string> checksums;  // platform -> checksum
    std::map<std::string, size_t> sizes;           // platform -> size in bytes
    std::vector<std::string> tags;
};

// Repository configuration
struct RepositoryConfig {
    std::string base_url;
    std::string metadata_url;
    
    RepositoryConfig() 
        : base_url("https://raw.githubusercontent.com/whatsmycli/plugins/main")
        , metadata_url(base_url + "/plugins.json") {}
};

// Download result
struct DownloadResult {
    bool success;
    std::string error_message;
    std::string file_path;  // Path to downloaded file if successful
};

/**
 * Fetch the plugin metadata from the repository
 * @return Vector of available plugins, or empty vector on error
 */
std::vector<PluginInfo> fetch_plugin_list();

/**
 * Get list of installed plugins
 * @param plugin_dir Path to plugin directory
 * @return Vector of installed plugin names
 */
std::vector<std::string> get_installed_plugins(const std::string& plugin_dir);

/**
 * Download a plugin binary from the repository
 * @param plugin_name Name of the plugin to download
 * @param platform Target platform (linux, windows, macos)
 * @param dest_path Destination path for the downloaded binary
 * @return DownloadResult with success status and error message if failed
 */
DownloadResult download_plugin(const std::string& plugin_name, 
                                const std::string& platform,
                                const std::string& dest_path);

/**
 * Install a plugin (download and set up in plugin directory)
 * @param plugin_name Name of the plugin to install
 * @param plugin_dir Base plugin directory
 * @return true if installation successful, false otherwise
 */
bool install_plugin(const std::string& plugin_name, const std::string& plugin_dir);

/**
 * Remove an installed plugin
 * @param plugin_name Name of the plugin to remove
 * @param plugin_dir Base plugin directory
 * @return true if removal successful, false otherwise
 */
bool remove_plugin(const std::string& plugin_name, const std::string& plugin_dir);

/**
 * Update a plugin to the latest version
 * @param plugin_name Name of the plugin to update
 * @param plugin_dir Base plugin directory
 * @return true if update successful, false otherwise
 */
bool update_plugin(const std::string& plugin_name, const std::string& plugin_dir);

/**
 * Search for plugins by name or description
 * @param query Search term
 * @param available_plugins List of available plugins to search
 * @return Vector of matching plugins
 */
std::vector<PluginInfo> search_plugins(const std::string& query,
                                        const std::vector<PluginInfo>& available_plugins);

/**
 * Verify checksum of a downloaded file
 * @param file_path Path to the file
 * @param expected_checksum Expected checksum (format: "sha256:hash")
 * @return true if checksum matches, false otherwise
 */
bool verify_checksum(const std::string& file_path, const std::string& expected_checksum);

/**
 * Get the current platform name (linux, windows, macos)
 * @return Platform name as string
 */
std::string get_platform_name();

/**
 * Get the plugin binary filename for current platform
 * @return Binary filename (e.g., "linux.so", "windows.dll", "macos.dylib")
 */
std::string get_platform_binary_name();

/**
 * Display list of available plugins in a formatted table
 * @param plugins Vector of plugin information
 */
void display_plugin_list(const std::vector<PluginInfo>& plugins);

/**
 * Display list of installed plugins
 * @param plugin_names Vector of installed plugin names
 * @param plugin_dir Base plugin directory
 */
void display_installed_plugins(const std::vector<std::string>& plugin_names,
                                const std::string& plugin_dir);

/**
 * Display detailed information about a specific plugin
 * @param plugin Plugin information
 */
void display_plugin_info(const PluginInfo& plugin);

} // namespace plugin_manager
} // namespace whatsmy

#endif // WHATSMY_PLUGIN_MANAGER_H

