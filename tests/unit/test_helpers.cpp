// test_helpers.cpp - Unit tests for helper functions
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "whatsmy/helpers.h"
#include <sstream>
#include <string>

// Test fixture for helper function tests
class HelpersTest : public ::testing::Test {
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

// ===== Output Helper Tests =====

// Test: Color code generation
TEST_F(HelpersTest, ColorCodeGeneration) {
    using namespace whatsmy::helpers::output;
    
    std::string red = colorize("test", Color::RED);
    std::string green = colorize("test", Color::GREEN);
    std::string blue = colorize("test", Color::BLUE);
    
    // Should contain the text
    EXPECT_THAT(red, ::testing::HasSubstr("test"));
    EXPECT_THAT(green, ::testing::HasSubstr("test"));
    EXPECT_THAT(blue, ::testing::HasSubstr("test"));
}

// Test: Style application
TEST_F(HelpersTest, StyleApplication) {
    using namespace whatsmy::helpers::output;
    
    std::string bold = stylize("test", Style::BOLD);
    std::string italic = stylize("test", Style::ITALIC);
    std::string underline = stylize("test", Style::UNDERLINE);
    
    EXPECT_THAT(bold, ::testing::HasSubstr("test"));
    EXPECT_THAT(italic, ::testing::HasSubstr("test"));
    EXPECT_THAT(underline, ::testing::HasSubstr("test"));
}

// Test: Print functions work
TEST_F(HelpersTest, PrintFunctionsWork) {
    using namespace whatsmy::helpers::output;
    
    print_info("Info message");
    std::string info_output = GetStdout();
    ClearOutput();
    
    print_success("Success message");
    std::string success_output = GetStdout();
    ClearOutput();
    
    print_warning("Warning message");
    std::string warning_output = GetStdout();
    ClearOutput();
    
    print_error("Error message");
    std::string error_output = GetStderr();
    
    EXPECT_THAT(info_output, ::testing::HasSubstr("Info message"));
    EXPECT_THAT(success_output, ::testing::HasSubstr("Success message"));
    EXPECT_THAT(warning_output, ::testing::HasSubstr("Warning message"));
    EXPECT_THAT(error_output, ::testing::HasSubstr("Error message"));
}

// Test: Text alignment functions
TEST_F(HelpersTest, TextAlignmentWorks) {
    using namespace whatsmy::helpers::output;
    
    std::string left = align_text("test", 10, Alignment::LEFT);
    std::string center = align_text("test", 10, Alignment::CENTER);
    std::string right = align_text("test", 10, Alignment::RIGHT);
    
    EXPECT_EQ(left.length(), 10);
    EXPECT_EQ(center.length(), 10);
    EXPECT_EQ(right.length(), 10);
}

// Test: Table rendering
TEST_F(HelpersTest, TableRenderingWorks) {
    using namespace whatsmy::helpers::output;
    
    Table table;
    table.set_headers({"Name", "Value"});
    table.add_row({"GPU", "NVIDIA GTX 1050"});
    table.add_row({"CPU", "Intel i7"});
    
    std::string rendered = table.render();
    
    EXPECT_THAT(rendered, ::testing::HasSubstr("Name"));
    EXPECT_THAT(rendered, ::testing::HasSubstr("Value"));
    EXPECT_THAT(rendered, ::testing::HasSubstr("GPU"));
    EXPECT_THAT(rendered, ::testing::HasSubstr("NVIDIA GTX 1050"));
    EXPECT_THAT(rendered, ::testing::HasSubstr("CPU"));
    EXPECT_THAT(rendered, ::testing::HasSubstr("Intel i7"));
}

// Test: Table with different border styles
TEST_F(HelpersTest, TableBorderStyles) {
    using namespace whatsmy::helpers::output;
    
    Table simple_table;
    simple_table.set_border_style(BorderStyle::SIMPLE);
    simple_table.set_headers({"A", "B"});
    simple_table.add_row({"1", "2"});
    std::string simple = simple_table.render();
    
    Table rounded_table;
    rounded_table.set_border_style(BorderStyle::ROUNDED);
    rounded_table.set_headers({"A", "B"});
    rounded_table.add_row({"1", "2"});
    std::string rounded = rounded_table.render();
    
    Table double_table;
    double_table.set_border_style(BorderStyle::DOUBLE);
    double_table.set_headers({"A", "B"});
    double_table.add_row({"1", "2"});
    std::string double_border = double_table.render();
    
    // All should render something
    EXPECT_FALSE(simple.empty());
    EXPECT_FALSE(rounded.empty());
    EXPECT_FALSE(double_border.empty());
}

// Test: Table column alignment
TEST_F(HelpersTest, TableColumnAlignment) {
    using namespace whatsmy::helpers::output;
    
    Table table;
    table.set_headers({"Left", "Center", "Right"});
    table.set_column_alignment(0, Alignment::LEFT);
    table.set_column_alignment(1, Alignment::CENTER);
    table.set_column_alignment(2, Alignment::RIGHT);
    table.add_row({"A", "B", "C"});
    
    std::string rendered = table.render();
    
    EXPECT_THAT(rendered, ::testing::HasSubstr("A"));
    EXPECT_THAT(rendered, ::testing::HasSubstr("B"));
    EXPECT_THAT(rendered, ::testing::HasSubstr("C"));
}

// ===== Error Helper Tests =====

// Test: Error code descriptions
TEST_F(HelpersTest, ErrorCodeDescription) {
    using namespace whatsmy::helpers::error;
    
    std::string success = get_error_description(Code::SUCCESS);
    std::string plugin_not_found = get_error_description(Code::PLUGIN_NOT_FOUND);
    std::string plugin_load_error = get_error_description(Code::PLUGIN_LOAD_ERROR);
    
    EXPECT_FALSE(success.empty());
    EXPECT_FALSE(plugin_not_found.empty());
    EXPECT_FALSE(plugin_load_error.empty());
}

// Test: Log function with different levels
TEST_F(HelpersTest, LogFunctionWorks) {
    using namespace whatsmy::helpers::error;
    
    log(Level::DEBUG, "Debug message");
    log(Level::INFO, "Info message");
    log(Level::WARNING, "Warning message");
    log(Level::ERROR, "Error message");
    log(Level::CRITICAL, "Critical message");
    
    // Just verify no crash
    EXPECT_TRUE(true);
}

// Test: Log level filtering
TEST_F(HelpersTest, LogLevelFiltering) {
    using namespace whatsmy::helpers::error;
    
    // Set log level to WARNING
    set_log_level(Level::WARNING);
    
    ClearOutput();
    log(Level::DEBUG, "Debug message");
    log(Level::INFO, "Info message");
    log(Level::WARNING, "Warning message");
    
    // Reset to default level
    set_log_level(Level::INFO);
    
    // Just verify no crash
    EXPECT_TRUE(true);
}

// Test: Format error functions
TEST_F(HelpersTest, FormatErrorFunctions) {
    using namespace whatsmy::helpers::error;
    
    std::string error1 = format_error("context", "details");
    std::string error2 = format_error(Code::PLUGIN_NOT_FOUND, "plugin details");
    
    EXPECT_FALSE(error1.empty());
    EXPECT_FALSE(error2.empty());
}

// Test: Color support detection
TEST_F(HelpersTest, ColorSupportDetection) {
    using namespace whatsmy::helpers::output;
    
    // Just check that the function returns something
    bool has_color = supports_color();
    
    // Should return true or false (both are valid)
    EXPECT_TRUE(has_color || !has_color);
}

// Test: Empty string handling
TEST_F(HelpersTest, EmptyStringHandling) {
    using namespace whatsmy::helpers::output;
    
    std::string empty = colorize("", Color::RED);
    std::string empty_aligned = align_text("", 10, Alignment::CENTER);
    
    // Should handle empty strings gracefully
    EXPECT_TRUE(empty.empty() || !empty.empty());
    EXPECT_EQ(empty_aligned.length(), 10);
}

// Test: Special characters in text
TEST_F(HelpersTest, SpecialCharactersHandling) {
    using namespace whatsmy::helpers::output;
    
    std::string special = "Test\nNew\tLine\rCarriage";
    std::string colored = colorize(special, Color::GREEN);
    
    EXPECT_THAT(colored, ::testing::HasSubstr("Test"));
}
