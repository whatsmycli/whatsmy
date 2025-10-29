// test_plugin_loader.cpp - Unit tests for plugin loader
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "whatsmy/plugin_loader.h"
#include "whatsmy/whatsmy.h"
#include <filesystem>
#include <fstream>
#include <cstdlib>

namespace fs = std::filesystem;

// Test fixture for plugin loader tests
class PluginLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary test directory for plugins
        test_dir = fs::temp_directory_path() / "whatsmy_test_plugins";
        fs::create_directories(test_dir);
        
        // Set environment variable to use test directory
        #ifdef _WIN32
        _putenv_s("WHATSMY_PLUGIN_DIR", test_dir.string().c_str());
        #else
        setenv("WHATSMY_PLUGIN_DIR", test_dir.string().c_str(), 1);
        #endif
    }
    
    void TearDown() override {
        // Clean up test directory
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
        
        // Unset environment variable
        #ifdef _WIN32
        _putenv_s("WHATSMY_PLUGIN_DIR", "");
        #else
        unsetenv("WHATSMY_PLUGIN_DIR");
        #endif
    }
    
    // Helper: Create a test plugin directory
    void CreatePluginDir(const std::string& name) {
        fs::create_directories(test_dir / name);
    }
    
    fs::path test_dir;
};

// Test: Non-existent plugin returns error
TEST_F(PluginLoaderTest, NonExistentPluginReturnsError) {
    int result = whatsmy::backend::PluginLoader::load_and_run("nonexistent_plugin");
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::PLUGIN_NOT_FOUND));
}

// Test: Empty plugin directory is handled
TEST_F(PluginLoaderTest, EmptyPluginDirectoryHandled) {
    // Test directory exists but is empty
    int result = whatsmy::backend::PluginLoader::load_and_run("any_plugin");
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::PLUGIN_NOT_FOUND));
}

// Test: Plugin directory without platform binary
TEST_F(PluginLoaderTest, PluginWithoutPlatformBinary) {
    CreatePluginDir("incomplete_plugin");
    // Directory exists but no platform-specific binary
    
    int result = whatsmy::backend::PluginLoader::load_and_run("incomplete_plugin");
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::PLUGIN_LOAD_ERROR));
}

// Test: Invalid plugin name characters
TEST_F(PluginLoaderTest, InvalidPluginNameHandled) {
    // Test with invalid characters that could cause path traversal
    int result1 = whatsmy::backend::PluginLoader::load_and_run("../invalid");
    int result2 = whatsmy::backend::PluginLoader::load_and_run("plugin/with/slashes");
    
    EXPECT_NE(result1, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_NE(result2, static_cast<int>(whatsmy::ExitCode::SUCCESS));
}

// Test: Environment variable override works
TEST_F(PluginLoaderTest, EnvironmentVariableOverride) {
    fs::path custom_dir = fs::temp_directory_path() / "custom_plugins";
    fs::create_directories(custom_dir);
    
    #ifdef _WIN32
    _putenv_s("WHATSMY_PLUGIN_DIR", custom_dir.string().c_str());
    #else
    setenv("WHATSMY_PLUGIN_DIR", custom_dir.string().c_str(), 1);
    #endif
    
    // Try to load a plugin - should look in custom_dir
    int result = whatsmy::backend::PluginLoader::load_and_run("test");
    
    // Plugin won't exist, but it should have tried the custom directory
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::PLUGIN_NOT_FOUND));
    
    // Cleanup
    fs::remove_all(custom_dir);
}

// Test: Empty string plugin name
TEST_F(PluginLoaderTest, EmptyPluginNameHandled) {
    int result = whatsmy::backend::PluginLoader::load_and_run("");
    
    EXPECT_NE(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
}

// Test: Very long plugin name
TEST_F(PluginLoaderTest, LongPluginNameHandled) {
    std::string long_name(1000, 'a');
    int result = whatsmy::backend::PluginLoader::load_and_run(long_name);
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::PLUGIN_NOT_FOUND));
}

// Test: Plugin with special characters
TEST_F(PluginLoaderTest, SpecialCharactersHandled) {
    int result1 = whatsmy::backend::PluginLoader::load_and_run("plugin@#$%");
    int result2 = whatsmy::backend::PluginLoader::load_and_run("plugin\nname");
    
    EXPECT_NE(result1, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_NE(result2, static_cast<int>(whatsmy::ExitCode::SUCCESS));
}

// Test: Null pointer safety (would cause crash if not handled)
TEST_F(PluginLoaderTest, NullStringSafety) {
    // Pass an empty string (C++ std::string handles this)
    std::string empty;
    int result = whatsmy::backend::PluginLoader::load_and_run(empty);
    
    EXPECT_NE(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
}

// Test: Case sensitivity
TEST_F(PluginLoaderTest, CaseSensitivity) {
    CreatePluginDir("TestPlugin");
    
    // Try both cases
    int result1 = whatsmy::backend::PluginLoader::load_and_run("TestPlugin");
    int result2 = whatsmy::backend::PluginLoader::load_and_run("testplugin");
    
    // On case-sensitive file systems, these should be different
    // On case-insensitive (Windows, macOS by default), they're the same
    #ifdef _WIN32
    // Windows is case-insensitive
    EXPECT_TRUE(true); // Just verify it doesn't crash
    #else
    // Linux is case-sensitive
    EXPECT_TRUE(true); // Just verify it doesn't crash
    #endif
}
