// whatsmycli - Main Application Header
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#ifndef WHATSMY_H
#define WHATSMY_H

#include <string>
#include <vector>

namespace whatsmy {

/**
 * Application version information
 */
constexpr const char* VERSION = "1.2.1";
constexpr const char* APP_NAME = "whatsmy";

/**
 * Exit codes
 */
enum class ExitCode {
    SUCCESS = 0,
    INVALID_ARGS = 1,
    PLUGIN_NOT_FOUND = 2,
    PLUGIN_LOAD_ERROR = 3,
    PLUGIN_EXEC_ERROR = 4
};

/**
 * Parse and execute command
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit code
 */
int run(int argc, char* argv[]);

/**
 * Display help message
 */
void show_help();

/**
 * Display version information
 */
void show_version();

} // namespace whatsmy

#endif // WHATSMY_H

