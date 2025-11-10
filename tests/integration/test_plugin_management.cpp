// test_plugin_management.cpp - Integration tests for plugin management system
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include <gtest/gtest.h>
#include "whatsmy/plugin_manager.h"
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <thread>
#include <chrono>
#ifdef _WIN32
    #include <process.h>
    #define getpid _getpid
    #define setenv(name, value, overwrite) _putenv_s(name, value)
    #define unsetenv(name) _putenv_s(name, "")
#else
    #include <unistd.h>
#endif

namespace fs = std::filesystem;

class PluginManagementTest : public ::testing::Test {
protected:
    std::string temp_plugin_dir;
    
    void SetUp() override {
        // Create temporary plugin directory for testing
        temp_plugin_dir = (fs::temp_directory_path() / ("whatsmy_test_" + std::to_string(getpid()))).string();
        fs::create_directories(temp_plugin_dir);
        
        // Set environment variable to use temp directory
        setenv("WHATSMY_PLUGIN_DIR", temp_plugin_dir.c_str(), 1);
    }
    
    void TearDown() override {
        // Clean up temporary directory
        if (fs::exists(temp_plugin_dir)) {
            fs::remove_all(temp_plugin_dir);
        }
        unsetenv("WHATSMY_PLUGIN_DIR");
    }
    
    // Helper to check if we have internet connectivity
    // Try to fetch plugin list - if it works, we have internet
    bool has_internet() {
        try {
            auto plugins = whatsmy::plugin_manager::fetch_plugin_list();
            return !plugins.empty();
        } catch (...) {
            return false;
        }
    }
    
    // Helper to find a plugin compatible with current platform
    std::string find_compatible_plugin() {
        auto plugins = whatsmy::plugin_manager::fetch_plugin_list();
        std::string current_platform = whatsmy::plugin_manager::get_platform_name();
        
        for (const auto& plugin : plugins) {
            if (std::find(plugin.platforms.begin(), plugin.platforms.end(), current_platform) != plugin.platforms.end()) {
                return plugin.name;
            }
        }
        return "";  // No compatible plugin found
    }
};

// Test: Fetch plugin list from real repository
TEST_F(PluginManagementTest, FetchPluginListFromRepository) {
    if (!has_internet()) {
        GTEST_SKIP() << "Skipping test: No internet connection";
    }
    
    auto plugins = whatsmy::plugin_manager::fetch_plugin_list();
    
    // Should have at least one plugin (gpu)
    EXPECT_GT(plugins.size(), 0) << "Plugin list should not be empty";
    
    // Check if we got valid plugin data
    if (!plugins.empty()) {
        const auto& first_plugin = plugins[0];
        EXPECT_FALSE(first_plugin.name.empty()) << "Plugin should have a name";
        EXPECT_FALSE(first_plugin.description.empty()) << "Plugin should have a description";
        EXPECT_FALSE(first_plugin.author.empty()) << "Plugin should have an author";
    }
}

// Test: Search for existing plugin
TEST_F(PluginManagementTest, SearchForPlugin) {
    if (!has_internet()) {
        GTEST_SKIP() << "Skipping test: No internet connection";
    }
    
    auto all_plugins = whatsmy::plugin_manager::fetch_plugin_list();
    ASSERT_GT(all_plugins.size(), 0) << "Need plugins to test search";
    
    // Search for "example" (we know this exists in the repository)
    auto results = whatsmy::plugin_manager::search_plugins("example", all_plugins);
    
    EXPECT_GT(results.size(), 0) << "Should find at least one result for 'example'";
    
    // Verify the found plugin contains "example" in name or description
    bool found_example = false;
    for (const auto& plugin : results) {
        std::string name_lower = plugin.name;
        std::string desc_lower = plugin.description;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
        std::transform(desc_lower.begin(), desc_lower.end(), desc_lower.begin(), ::tolower);
        
        if (name_lower.find("example") != std::string::npos || 
            desc_lower.find("example") != std::string::npos) {
            found_example = true;
            break;
        }
    }
    EXPECT_TRUE(found_example) << "Search results should contain example plugin";
}

// Test: Search for non-existent plugin
TEST_F(PluginManagementTest, SearchForNonExistentPlugin) {
    if (!has_internet()) {
        GTEST_SKIP() << "Skipping test: No internet connection";
    }
    
    auto all_plugins = whatsmy::plugin_manager::fetch_plugin_list();
    ASSERT_GT(all_plugins.size(), 0) << "Need plugins to test search";
    
    // Search for something that definitely doesn't exist
    auto results = whatsmy::plugin_manager::search_plugins("nonexistentpluginxyz12345", all_plugins);
    
    EXPECT_EQ(results.size(), 0) << "Should find no results for non-existent plugin";
}

// Test: Get installed plugins (empty initially)
TEST_F(PluginManagementTest, GetInstalledPluginsEmpty) {
    auto installed = whatsmy::plugin_manager::get_installed_plugins(temp_plugin_dir);
    
    EXPECT_EQ(installed.size(), 0) << "Should have no installed plugins initially";
}

// Test: Install plugin from repository
TEST_F(PluginManagementTest, InstallPluginFromRepository) {
    if (!has_internet()) {
        GTEST_SKIP() << "Skipping test: No internet connection";
    }
    
    // Find a plugin compatible with current platform
    std::string plugin_name = find_compatible_plugin();
    if (plugin_name.empty()) {
        GTEST_SKIP() << "No plugins available for current platform";
    }
    
    bool success = whatsmy::plugin_manager::install_plugin(plugin_name, temp_plugin_dir);
    
    EXPECT_TRUE(success) << "Plugin installation should succeed";
    
    // Check that plugin was installed
    auto installed = whatsmy::plugin_manager::get_installed_plugins(temp_plugin_dir);
    EXPECT_EQ(installed.size(), 1) << "Should have one installed plugin";
    EXPECT_EQ(installed[0], plugin_name) << "Installed plugin should be '" << plugin_name << "'";
    
    // Verify binary exists
    std::string platform = whatsmy::plugin_manager::get_platform_name();
    std::string binary_name = whatsmy::plugin_manager::get_platform_binary_name();
    fs::path binary_path = fs::path(temp_plugin_dir) / plugin_name / binary_name;
    
    EXPECT_TRUE(fs::exists(binary_path)) << "Plugin binary should exist after installation";
    EXPECT_GT(fs::file_size(binary_path), 0) << "Plugin binary should not be empty";
}

// Test: Install non-existent plugin
TEST_F(PluginManagementTest, InstallNonExistentPlugin) {
    if (!has_internet()) {
        GTEST_SKIP() << "Skipping test: No internet connection";
    }
    
    // Try to install a plugin that doesn't exist
    bool success = whatsmy::plugin_manager::install_plugin("nonexistentpluginxyz", temp_plugin_dir);
    
    EXPECT_FALSE(success) << "Installing non-existent plugin should fail";
    
    // Verify nothing was installed
    auto installed = whatsmy::plugin_manager::get_installed_plugins(temp_plugin_dir);
    EXPECT_EQ(installed.size(), 0) << "Should have no installed plugins after failed install";
}

// Test: Remove installed plugin
TEST_F(PluginManagementTest, RemoveInstalledPlugin) {
    if (!has_internet()) {
        GTEST_SKIP() << "Skipping test: No internet connection";
    }
    
    // Find a plugin compatible with current platform
    std::string plugin_name = find_compatible_plugin();
    if (plugin_name.empty()) {
        GTEST_SKIP() << "No plugins available for current platform";
    }
    
    // First install a plugin
    bool install_success = whatsmy::plugin_manager::install_plugin(plugin_name, temp_plugin_dir);
    ASSERT_TRUE(install_success) << "Plugin installation should succeed for test setup";
    
    // Verify it's installed
    auto installed_before = whatsmy::plugin_manager::get_installed_plugins(temp_plugin_dir);
    ASSERT_EQ(installed_before.size(), 1) << "Should have one installed plugin before removal";
    
    // Remove the plugin
    bool remove_success = whatsmy::plugin_manager::remove_plugin(plugin_name, temp_plugin_dir);
    EXPECT_TRUE(remove_success) << "Plugin removal should succeed";
    
    // Verify it's gone
    auto installed_after = whatsmy::plugin_manager::get_installed_plugins(temp_plugin_dir);
    EXPECT_EQ(installed_after.size(), 0) << "Should have no installed plugins after removal";
    
    // Verify directory was removed
    fs::path plugin_dir = fs::path(temp_plugin_dir) / plugin_name;
    EXPECT_FALSE(fs::exists(plugin_dir)) << "Plugin directory should be removed";
}

// Test: Remove non-existent plugin
TEST_F(PluginManagementTest, RemoveNonExistentPlugin) {
    // Try to remove a plugin that's not installed
    bool success = whatsmy::plugin_manager::remove_plugin("nonexistent", temp_plugin_dir);
    
    EXPECT_FALSE(success) << "Removing non-existent plugin should fail";
}

// Test: Update plugin (reinstall)
TEST_F(PluginManagementTest, UpdatePlugin) {
    if (!has_internet()) {
        GTEST_SKIP() << "Skipping test: No internet connection";
    }
    
    // Find a plugin compatible with current platform
    std::string plugin_name = find_compatible_plugin();
    if (plugin_name.empty()) {
        GTEST_SKIP() << "No plugins available for current platform";
    }
    
    // First install a plugin
    bool install_success = whatsmy::plugin_manager::install_plugin(plugin_name, temp_plugin_dir);
    ASSERT_TRUE(install_success) << "Plugin installation should succeed for test setup";
    
    // Get the binary modification time
    std::string binary_name = whatsmy::plugin_manager::get_platform_binary_name();
    fs::path binary_path = fs::path(temp_plugin_dir) / plugin_name / binary_name;
    auto mtime_before = fs::last_write_time(binary_path);
    
    // Wait a moment to ensure different timestamp
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Update the plugin
    bool update_success = whatsmy::plugin_manager::update_plugin(plugin_name, temp_plugin_dir);
    EXPECT_TRUE(update_success) << "Plugin update should succeed";
    
    // Verify plugin is still installed
    auto installed = whatsmy::plugin_manager::get_installed_plugins(temp_plugin_dir);
    EXPECT_EQ(installed.size(), 1) << "Should still have one installed plugin after update";
    EXPECT_EQ(installed[0], plugin_name) << "Installed plugin should still be '" << plugin_name << "'";
    
    // Binary should exist
    EXPECT_TRUE(fs::exists(binary_path)) << "Plugin binary should exist after update";
}

// Test: Update non-existent plugin
TEST_F(PluginManagementTest, UpdateNonExistentPlugin) {
    // Try to update a plugin that's not installed
    bool success = whatsmy::plugin_manager::update_plugin("nonexistent", temp_plugin_dir);
    
    EXPECT_FALSE(success) << "Updating non-installed plugin should fail";
}

// Test: Platform detection
TEST_F(PluginManagementTest, PlatformDetection) {
    std::string platform = whatsmy::plugin_manager::get_platform_name();
    std::string binary = whatsmy::plugin_manager::get_platform_binary_name();
    
    // Platform should be one of the supported ones
    EXPECT_TRUE(platform == "linux" || platform == "windows" || platform == "macos")
        << "Platform should be linux, windows, or macos, got: " << platform;
    
    // Binary name should match platform
    if (platform == "linux") {
        EXPECT_EQ(binary, "linux.so");
    } else if (platform == "windows") {
        EXPECT_EQ(binary, "windows.dll");
    } else if (platform == "macos") {
        EXPECT_EQ(binary, "macos.dylib");
    }
}

// Test: Multiple plugin installations
TEST_F(PluginManagementTest, MultiplePluginInstallations) {
    if (!has_internet()) {
        GTEST_SKIP() << "Skipping test: No internet connection";
    }
    
    // Get list of available plugins
    auto available = whatsmy::plugin_manager::fetch_plugin_list();
    ASSERT_GT(available.size(), 0) << "Need at least one plugin for this test";
    
    // Get current platform
    std::string current_platform = whatsmy::plugin_manager::get_platform_name();
    
    // Find plugins that support current platform
    std::vector<std::string> compatible_plugins;
    for (const auto& plugin : available) {
        if (std::find(plugin.platforms.begin(), plugin.platforms.end(), current_platform) != plugin.platforms.end()) {
            compatible_plugins.push_back(plugin.name);
        }
    }
    
    if (compatible_plugins.empty()) {
        GTEST_SKIP() << "No plugins available for platform: " << current_platform;
    }
    
    // Install first compatible plugin
    bool success = whatsmy::plugin_manager::install_plugin(compatible_plugins[0], temp_plugin_dir);
    EXPECT_TRUE(success) << "First plugin installation should succeed";
    
    // If there's a second compatible plugin, try installing it too
    if (compatible_plugins.size() > 1) {
        bool success2 = whatsmy::plugin_manager::install_plugin(compatible_plugins[1], temp_plugin_dir);
        EXPECT_TRUE(success2) << "Second plugin installation should succeed";
        
        // Verify both are installed
        auto installed = whatsmy::plugin_manager::get_installed_plugins(temp_plugin_dir);
        EXPECT_EQ(installed.size(), 2) << "Should have two installed plugins";
    }
}

// Test: Plugin directory creation
TEST_F(PluginManagementTest, PluginDirectoryCreation) {
    if (!has_internet()) {
        GTEST_SKIP() << "Skipping test: No internet connection";
    }
    
    // Find a plugin compatible with current platform
    std::string plugin_name = find_compatible_plugin();
    if (plugin_name.empty()) {
        GTEST_SKIP() << "No plugins available for current platform";
    }
    
    // Remove temp directory to test creation
    fs::remove_all(temp_plugin_dir);
    ASSERT_FALSE(fs::exists(temp_plugin_dir)) << "Temp directory should not exist before test";
    
    // Try to install plugin (should create directory)
    bool success = whatsmy::plugin_manager::install_plugin(plugin_name, temp_plugin_dir);
    EXPECT_TRUE(success) << "Plugin installation should succeed and create directories";
    
    // Verify directories were created
    EXPECT_TRUE(fs::exists(temp_plugin_dir)) << "Base plugin directory should be created";
    EXPECT_TRUE(fs::exists(fs::path(temp_plugin_dir) / plugin_name)) << "Plugin subdirectory should be created";
}

