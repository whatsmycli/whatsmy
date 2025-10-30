// test_command_parser.cpp - Unit tests for command parser
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "whatsmy/whatsmy.h"
#include <sstream>
#include <string>

// Test fixture for command parser tests
class CommandParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Redirect stdout to capture output
        old_stdout = std::cout.rdbuf();
        std::cout.rdbuf(captured_stdout.rdbuf());
        
        // Redirect stderr to capture errors
        old_stderr = std::cerr.rdbuf();
        std::cerr.rdbuf(captured_stderr.rdbuf());
    }
    
    void TearDown() override {
        // Restore stdout and stderr
        std::cout.rdbuf(old_stdout);
        std::cerr.rdbuf(old_stderr);
    }
    
    std::string GetStdout() {
        return captured_stdout.str();
    }
    
    std::string GetStderr() {
        return captured_stderr.str();
    }
    
    void ClearOutput() {
        captured_stdout.str("");
        captured_stdout.clear();
        captured_stderr.str("");
        captured_stderr.clear();
    }
    
private:
    std::streambuf* old_stdout;
    std::streambuf* old_stderr;
    std::stringstream captured_stdout;
    std::stringstream captured_stderr;
};

// Test: No arguments should show help and return error code
TEST_F(CommandParserTest, NoArgumentsShowsHelp) {
    const char* argv[] = {"whatsmy"};
    int result = whatsmy::run(1, const_cast<char**>(argv));
    
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::INVALID_ARGS));
    EXPECT_THAT(output, ::testing::HasSubstr("Usage:"));
    EXPECT_THAT(output, ::testing::HasSubstr("whatsmy"));
}

// Test: Help command should display usage
TEST_F(CommandParserTest, HelpCommandDisplaysUsage) {
    const char* argv[] = {"whatsmy", "help"};
    int result = whatsmy::run(2, const_cast<char**>(argv));
    
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_THAT(output, ::testing::HasSubstr("Usage:"));
    EXPECT_THAT(output, ::testing::HasSubstr("whatsmy"));
}

// Test: --help flag should work
TEST_F(CommandParserTest, HelpFlagWorks) {
    const char* argv[] = {"whatsmy", "--help"};
    int result = whatsmy::run(2, const_cast<char**>(argv));
    
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_THAT(output, ::testing::HasSubstr("Usage:"));
}

// Test: -h flag should work
TEST_F(CommandParserTest, ShortHelpFlagWorks) {
    const char* argv[] = {"whatsmy", "-h"};
    int result = whatsmy::run(2, const_cast<char**>(argv));
    
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_THAT(output, ::testing::HasSubstr("Usage:"));
}

// Test: Version command should display version info
TEST_F(CommandParserTest, VersionCommandDisplaysVersion) {
    const char* argv[] = {"whatsmy", "version"};
    int result = whatsmy::run(2, const_cast<char**>(argv));
    
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_THAT(output, ::testing::HasSubstr("whatsmy"));
    EXPECT_THAT(output, ::testing::HasSubstr("1.0.0"));
    EXPECT_THAT(output, ::testing::HasSubstr("enXov"));
}

// Test: --version flag should work
TEST_F(CommandParserTest, VersionFlagWorks) {
    const char* argv[] = {"whatsmy", "--version"};
    int result = whatsmy::run(2, const_cast<char**>(argv));
    
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_THAT(output, ::testing::HasSubstr("whatsmy"));
    EXPECT_THAT(output, ::testing::HasSubstr("1.0.0"));
}

// Test: -v flag should work
TEST_F(CommandParserTest, ShortVersionFlagWorks) {
    const char* argv[] = {"whatsmy", "-v"};
    int result = whatsmy::run(2, const_cast<char**>(argv));
    
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_THAT(output, ::testing::HasSubstr("whatsmy"));
}

// Test: Unknown command should suggest similar commands
TEST_F(CommandParserTest, UnknownCommandSuggestsSimilar) {
    const char* argv[] = {"whatsmy", "gpo"}; // Typo for "gpu"
    int result = whatsmy::run(2, const_cast<char**>(argv));
    
    std::string output = GetStderr();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::PLUGIN_NOT_FOUND));
    EXPECT_THAT(output, ::testing::HasSubstr("not found"));
    // Should suggest "gpu" as a similar command
    EXPECT_THAT(output, ::testing::HasSubstr("gpu"));
}

// Test: Completely invalid command should show error
TEST_F(CommandParserTest, InvalidCommandShowsError) {
    const char* argv[] = {"whatsmy", "xyz123notacommand"};
    int result = whatsmy::run(2, const_cast<char**>(argv));
    
    std::string output = GetStderr();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::PLUGIN_NOT_FOUND));
    EXPECT_THAT(output, ::testing::HasSubstr("not found"));
}

// Test: Debug flag should be recognized
TEST_F(CommandParserTest, DebugFlagRecognized) {
    const char* argv[] = {"whatsmy", "--debug", "test"};
    int result = whatsmy::run(3, const_cast<char**>(argv));
    
    std::string output = GetStderr();
    
    // Debug mode should be enabled (plugin won't be found, but debug should work)
    // Just check that it doesn't fail with INVALID_ARGS
    EXPECT_NE(result, static_cast<int>(whatsmy::ExitCode::INVALID_ARGS));
}

// Test: Short debug flag (-d) should work
TEST_F(CommandParserTest, ShortDebugFlagWorks) {
    const char* argv[] = {"whatsmy", "-d", "test"};
    int result = whatsmy::run(3, const_cast<char**>(argv));
    
    // Should enable debug mode (same as --debug)
    // Plugin won't be found, but flag should be processed
    EXPECT_NE(result, static_cast<int>(whatsmy::ExitCode::INVALID_ARGS));
}

// Test: Multiple commands should only process the first
TEST_F(CommandParserTest, OnlyFirstCommandProcessed) {
    const char* argv[] = {"whatsmy", "help", "version"};
    int result = whatsmy::run(3, const_cast<char**>(argv));
    
    std::string output = GetStdout();
    
    // Should show help, not version
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_THAT(output, ::testing::HasSubstr("Usage:"));
}

