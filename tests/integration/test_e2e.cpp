// test_e2e.cpp - End-to-end integration tests
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "whatsmy/whatsmy.h"
#include <sstream>
#include <string>
#include <cstdlib>

// Test fixture for end-to-end tests
class E2ETest : public ::testing::Test {
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
    
    // Simulate command-line execution
    int ExecuteCommand(const std::vector<std::string>& args) {
        ClearOutput();
        
        std::vector<char*> argv;
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        
        return whatsmy::run(static_cast<int>(argv.size()), argv.data());
    }
    
private:
    std::streambuf* old_stdout;
    std::streambuf* old_stderr;
    std::stringstream captured_stdout;
    std::stringstream captured_stderr;
};

// ===== Basic Command Execution Tests =====

// Test: Help command workflow
TEST_F(E2ETest, HelpCommandWorkflow) {
    int result = ExecuteCommand({"whatsmy", "help"});
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_THAT(output, ::testing::HasSubstr("Usage:"));
    EXPECT_THAT(output, ::testing::HasSubstr("whatsmy"));
    EXPECT_THAT(output, ::testing::HasSubstr("COMMANDS"));
    EXPECT_THAT(output, ::testing::HasSubstr("help"));
    EXPECT_THAT(output, ::testing::HasSubstr("version"));
}

// Test: Version command workflow
TEST_F(E2ETest, VersionCommandWorkflow) {
    int result = ExecuteCommand({"whatsmy", "version"});
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_THAT(output, ::testing::HasSubstr("whatsmy"));
    EXPECT_THAT(output, ::testing::HasSubstr("1.1.0"));
    EXPECT_THAT(output, ::testing::HasSubstr("Copyright"));
    EXPECT_THAT(output, ::testing::HasSubstr("enXov"));
}

// Test: No arguments workflow
TEST_F(E2ETest, NoArgumentsWorkflow) {
    int result = ExecuteCommand({"whatsmy"});
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::INVALID_ARGS));
    EXPECT_THAT(output, ::testing::HasSubstr("Usage:"));
}

// ===== Flag Variations Tests =====

// Test: All help flag variations work
TEST_F(E2ETest, HelpFlagVariations) {
    std::vector<std::vector<std::string>> help_commands = {
        {"whatsmy", "help"},
        {"whatsmy", "--help"},
        {"whatsmy", "-h"}
    };
    
    for (const auto& cmd : help_commands) {
        int result = ExecuteCommand(cmd);
        std::string output = GetStdout();
        
        EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
        EXPECT_THAT(output, ::testing::HasSubstr("Usage:"));
    }
}

// Test: All version flag variations work
TEST_F(E2ETest, VersionFlagVariations) {
    std::vector<std::vector<std::string>> version_commands = {
        {"whatsmy", "version"},
        {"whatsmy", "--version"},
        {"whatsmy", "-v"}
    };
    
    for (const auto& cmd : version_commands) {
        int result = ExecuteCommand(cmd);
        std::string output = GetStdout();
        
        EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
        EXPECT_THAT(output, ::testing::HasSubstr("whatsmy"));
        EXPECT_THAT(output, ::testing::HasSubstr("1.1.0"));
    }
}

// ===== Error Handling Tests =====

// Test: Unknown command error handling
TEST_F(E2ETest, UnknownCommandErrorHandling) {
    int result = ExecuteCommand({"whatsmy", "unknown_command_xyz"});
    std::string error_output = GetStderr();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::PLUGIN_NOT_FOUND));
    EXPECT_THAT(error_output, ::testing::HasSubstr("not found"));
}

// Test: Typo detection and suggestion
TEST_F(E2ETest, TypoDetectionSuggestion) {
    int result = ExecuteCommand({"whatsmy", "gpo"}); // Typo for "gpu"
    std::string error_output = GetStderr();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::PLUGIN_NOT_FOUND));
    EXPECT_THAT(error_output, ::testing::HasSubstr("not found"));
    EXPECT_THAT(error_output, ::testing::HasSubstr("gpu")); // Should suggest "gpu"
}

// Test: Multiple similar commands suggestion
TEST_F(E2ETest, MultipleSimilarCommandsSuggestion) {
    int result = ExecuteCommand({"whatsmy", "cp"}); // Could be "cpu"
    std::string error_output = GetStderr();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::PLUGIN_NOT_FOUND));
    EXPECT_THAT(error_output, ::testing::HasSubstr("not found"));
    EXPECT_THAT(error_output, ::testing::HasSubstr("cpu")); // Should suggest "cpu"
}

// ===== Debug Mode Tests =====

// Test: Debug mode activation
TEST_F(E2ETest, DebugModeActivation) {
    int result = ExecuteCommand({"whatsmy", "--debug", "nonexistent"});
    std::string output = GetStderr();
    
    // Should show debug output (exact format may vary)
    EXPECT_NE(result, static_cast<int>(whatsmy::ExitCode::INVALID_ARGS));
}

// Test: Short debug flag
TEST_F(E2ETest, ShortDebugFlag) {
    int result = ExecuteCommand({"whatsmy", "-d", "nonexistent"});
    
    // Should process debug flag
    EXPECT_NE(result, static_cast<int>(whatsmy::ExitCode::INVALID_ARGS));
}

// Test: Debug mode with help command
TEST_F(E2ETest, DebugModeWithHelp) {
    int result = ExecuteCommand({"whatsmy", "--debug", "help"});
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_THAT(output, ::testing::HasSubstr("Usage:"));
}

// ===== Environment Variable Tests =====

// Test: WHATSMY_DEBUG environment variable
TEST_F(E2ETest, DebugEnvironmentVariable) {
    #ifdef _WIN32
    _putenv_s("WHATSMY_DEBUG", "1");
    #else
    setenv("WHATSMY_DEBUG", "1", 1);
    #endif
    
    int result = ExecuteCommand({"whatsmy", "help"});
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    
    // Cleanup
    #ifdef _WIN32
    _putenv_s("WHATSMY_DEBUG", "");
    #else
    unsetenv("WHATSMY_DEBUG");
    #endif
}

// ===== Output Format Tests =====

// Test: Help output structure
TEST_F(E2ETest, HelpOutputStructure) {
    int result = ExecuteCommand({"whatsmy", "help"});
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    
    // Should have essential sections (simplified help)
    EXPECT_THAT(output, ::testing::HasSubstr("Usage:"));
    EXPECT_THAT(output, ::testing::HasSubstr("COMMANDS"));
    EXPECT_THAT(output, ::testing::HasSubstr("PLUGIN MANAGEMENT"));
}

// Test: Version output structure
TEST_F(E2ETest, VersionOutputStructure) {
    int result = ExecuteCommand({"whatsmy", "version"});
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    
    // Should include version number and copyright
    EXPECT_THAT(output, ::testing::HasSubstr("1.1.0"));
    EXPECT_THAT(output, ::testing::HasSubstr("Copyright"));
    EXPECT_THAT(output, ::testing::HasSubstr("2025"));
}

// ===== Command Precedence Tests =====

// Test: First command takes precedence
TEST_F(E2ETest, FirstCommandPrecedence) {
    int result = ExecuteCommand({"whatsmy", "help", "version"});
    std::string output = GetStdout();
    
    // Should show help, not version
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_THAT(output, ::testing::HasSubstr("Usage:"));
}

// Test: Debug flag before command
TEST_F(E2ETest, DebugFlagBeforeCommand) {
    int result = ExecuteCommand({"whatsmy", "--debug", "help"});
    std::string output = GetStdout();
    
    // Should still show help
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_THAT(output, ::testing::HasSubstr("Usage:"));
}

// ===== Stress Tests =====

// Test: Very long command name
TEST_F(E2ETest, VeryLongCommandName) {
    std::string long_command(1000, 'a');
    int result = ExecuteCommand({"whatsmy", long_command});
    
    // Should handle gracefully
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::PLUGIN_NOT_FOUND));
}

// Test: Special characters in command
TEST_F(E2ETest, SpecialCharactersInCommand) {
    int result1 = ExecuteCommand({"whatsmy", "test@#$%"});
    int result2 = ExecuteCommand({"whatsmy", "test\ncommand"});
    int result3 = ExecuteCommand({"whatsmy", "test\tcommand"});
    
    // All should fail safely
    EXPECT_NE(result1, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_NE(result2, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    EXPECT_NE(result3, static_cast<int>(whatsmy::ExitCode::SUCCESS));
}

// Test: Empty command argument
TEST_F(E2ETest, EmptyCommandArgument) {
    int result = ExecuteCommand({"whatsmy", ""});
    
    // Should handle empty string
    EXPECT_NE(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
}

// ===== Exit Code Verification =====

// Test: All exit codes are used correctly
TEST_F(E2ETest, ExitCodeCorrectness) {
    // SUCCESS
    int help_result = ExecuteCommand({"whatsmy", "help"});
    EXPECT_EQ(help_result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    
    // INVALID_ARGS
    int no_args_result = ExecuteCommand({"whatsmy"});
    EXPECT_EQ(no_args_result, static_cast<int>(whatsmy::ExitCode::INVALID_ARGS));
    
    // PLUGIN_NOT_FOUND
    int not_found_result = ExecuteCommand({"whatsmy", "nonexistent"});
    EXPECT_EQ(not_found_result, static_cast<int>(whatsmy::ExitCode::PLUGIN_NOT_FOUND));
}

// ===== User Experience Tests =====

// Test: Help message is helpful
TEST_F(E2ETest, HelpMessageIsHelpful) {
    int result = ExecuteCommand({"whatsmy", "help"});
    std::string output = GetStdout();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::SUCCESS));
    
    // Should contain essential information (simplified help)
    EXPECT_THAT(output, ::testing::HasSubstr("whatsmy <plugin name>"));
    EXPECT_THAT(output, ::testing::HasSubstr("help"));
    EXPECT_THAT(output, ::testing::HasSubstr("version"));
    EXPECT_THAT(output, ::testing::HasSubstr("plugin list"));
    EXPECT_THAT(output, ::testing::HasSubstr("plugin install"));
}

// Test: Error messages are clear
TEST_F(E2ETest, ErrorMessagesAreClear) {
    int result = ExecuteCommand({"whatsmy", "nonexistent"});
    std::string error_output = GetStderr();
    
    EXPECT_EQ(result, static_cast<int>(whatsmy::ExitCode::PLUGIN_NOT_FOUND));
    
    // Error should be clear about what went wrong
    EXPECT_THAT(error_output, ::testing::HasSubstr("not found"));
    EXPECT_THAT(error_output, ::testing::HasSubstr("nonexistent"));
}

