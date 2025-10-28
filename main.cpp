// whatsmycli - Unified system information CLI tool
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include "whatsmy/whatsmy.h"
#include "whatsmy/helpers.h"
#include <iostream>
#include <exception>

int main(int argc, char* argv[]) {
    try {
        // Call into application logic
        return whatsmy::run(argc, argv);
    }
    catch (const std::exception& e) {
        // Handle standard exceptions
        whatsmy::helpers::output::print_error(
            whatsmy::helpers::error::format_error("Exception", e.what())
        );
        return static_cast<int>(whatsmy::ExitCode::PLUGIN_EXEC_ERROR);
    }
    catch (...) {
        // Handle unknown exceptions
        whatsmy::helpers::output::print_error("An unknown error occurred");
        return static_cast<int>(whatsmy::ExitCode::PLUGIN_EXEC_ERROR);
    }
}

