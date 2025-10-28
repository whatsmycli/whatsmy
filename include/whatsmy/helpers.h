// whatsmycli - Helper Functions Header
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#ifndef WHATSMY_HELPERS_H
#define WHATSMY_HELPERS_H

#include <string>
#include <vector>
#include <map>

namespace whatsmy {
namespace helpers {

/**
 * Output formatting functions
 */
namespace output {
    /**
     * Color codes for terminal output
     */
    enum class Color {
        RESET,
        BLACK,
        RED,
        GREEN,
        YELLOW,
        BLUE,
        MAGENTA,
        CYAN,
        WHITE,
        BRIGHT_BLACK,
        BRIGHT_RED,
        BRIGHT_GREEN,
        BRIGHT_YELLOW,
        BRIGHT_BLUE,
        BRIGHT_MAGENTA,
        BRIGHT_CYAN,
        BRIGHT_WHITE
    };

    /**
     * Text style modifiers
     */
    enum class Style {
        RESET,
        BOLD,
        DIM,
        ITALIC,
        UNDERLINE
    };

    /**
     * Column alignment options
     */
    enum class Alignment {
        LEFT,
        CENTER,
        RIGHT
    };

    /**
     * Table border style
     */
    enum class BorderStyle {
        NONE,       // No borders
        SIMPLE,     // Simple ASCII borders
        ROUNDED,    // Rounded Unicode borders
        DOUBLE      // Double-line Unicode borders
    };

    /**
     * Check if terminal supports colors
     */
    bool supports_color();

    /**
     * Enable/disable color output (default: auto-detect)
     */
    void set_color_enabled(bool enabled);

    /**
     * Get ANSI color code
     */
    std::string colorize(const std::string& text, Color color);

    /**
     * Get ANSI style code
     */
    std::string stylize(const std::string& text, Style style);

    /**
     * Apply both color and style
     */
    std::string format(const std::string& text, Color color, Style style = Style::RESET);

    /**
     * Print formatted message
     */
    void print(const std::string& message);
    
    /**
     * Print colored message
     */
    void print_colored(const std::string& message, Color color);

    /**
     * Print info message (cyan)
     */
    void print_info(const std::string& message);

    /**
     * Print success message (green)
     */
    void print_success(const std::string& message);

    /**
     * Print warning message (yellow)
     */
    void print_warning(const std::string& message);
    
    /**
     * Print error message to stderr (red)
     */
    void print_error(const std::string& message);

    /**
     * Align text within a specified width
     */
    std::string align_text(const std::string& text, size_t width, Alignment alignment = Alignment::LEFT);

    /**
     * Pad text to specified width
     */
    std::string pad_left(const std::string& text, size_t width, char fill = ' ');
    std::string pad_right(const std::string& text, size_t width, char fill = ' ');

    /**
     * Truncate text with ellipsis if too long
     */
    std::string truncate(const std::string& text, size_t max_width);

    /**
     * Table row structure
     */
    struct TableRow {
        std::vector<std::string> columns;
    };

    /**
     * Table formatting class
     */
    class Table {
    public:
        Table();

        /**
         * Set table headers
         */
        void set_headers(const std::vector<std::string>& headers);

        /**
         * Add a row to the table
         */
        void add_row(const std::vector<std::string>& row);

        /**
         * Set column alignment
         */
        void set_column_alignment(size_t column, Alignment alignment);

        /**
         * Set border style
         */
        void set_border_style(BorderStyle style);

        /**
         * Set minimum column widths
         */
        void set_column_widths(const std::vector<size_t>& widths);

        /**
         * Enable/disable headers
         */
        void set_headers_enabled(bool enabled);

        /**
         * Render the table as a string
         */
        std::string render() const;

        /**
         * Print the table to stdout
         */
        void print() const;

    private:
        std::vector<std::string> headers_;
        std::vector<TableRow> rows_;
        std::map<size_t, Alignment> column_alignments_;
        std::vector<size_t> min_column_widths_;
        BorderStyle border_style_;
        bool headers_enabled_;

        std::vector<size_t> calculate_column_widths() const;
        std::string render_separator(const std::vector<size_t>& widths, const char* left, const char* mid, const char* right, const char* fill) const;
        std::string render_row(const std::vector<std::string>& columns, const std::vector<size_t>& widths, const char* left, const char* mid, const char* right) const;
    };

}

/**
 * Error handling functions
 */
namespace error {
    /**
     * Error severity levels
     */
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    /**
     * Error codes matching ExitCode from whatsmy.h
     */
    enum class Code {
        SUCCESS = 0,
        INVALID_ARGS = 1,
        PLUGIN_NOT_FOUND = 2,
        PLUGIN_LOAD_ERROR = 3,
        PLUGIN_EXEC_ERROR = 4,
        UNKNOWN_ERROR = 255
    };

    /**
     * Set minimum log level (messages below this level won't be logged)
     */
    void set_log_level(Level level);

    /**
     * Get current log level
     */
    Level get_log_level();

    /**
     * Format error message with context
     */
    std::string format_error(const std::string& context, const std::string& details);

    /**
     * Format error message with error code
     */
    std::string format_error(Code code, const std::string& details);

    /**
     * Get human-readable error code description
     */
    std::string get_error_description(Code code);

    /**
     * Log message with specified level
     */
    void log(Level level, const std::string& message);

    /**
     * Log debug message
     */
    void debug_log(const std::string& message);

    /**
     * Log info message
     */
    void info_log(const std::string& message);

    /**
     * Log warning message
     */
    void warning_log(const std::string& message);

    /**
     * Log error message
     */
    void error_log(const std::string& message);

    /**
     * Log critical error message
     */
    void critical_log(const std::string& message);

    /**
     * Format exception message
     */
    std::string format_exception(const std::exception& e);
}

} // namespace helpers
} // namespace whatsmy

#endif // WHATSMY_HELPERS_H

