// whatsmycli - Output Helper Implementation
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#include "whatsmy/helpers.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #define ISATTY _isatty
    #define FILENO _fileno
#else
    #include <unistd.h>
    #define ISATTY isatty
    #define FILENO fileno
#endif

namespace whatsmy {
namespace helpers {
namespace output {

namespace {
    // Global state for color support
    bool color_enabled = true;
    bool color_support_checked = false;
    bool color_supported = false;

    /**
     * Detect if the terminal supports colors
     */
    bool detect_color_support() {
        // Check if stdout is a terminal
        if (!ISATTY(FILENO(stdout))) {
            return false;
        }

        // Check environment variables
        const char* term = std::getenv("TERM");
        if (!term || std::string(term) == "dumb") {
            return false;
        }

        // Check for common color support indicators
        const char* colorterm = std::getenv("COLORTERM");
        if (colorterm) {
            return true;
        }

        // Windows 10+ supports ANSI colors
        #ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD mode = 0;
            if (GetConsoleMode(hOut, &mode)) {
                // Enable ANSI escape sequences on Windows 10+
                mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, mode);
                return true;
            }
        }
        #endif

        return true; // Most modern terminals support colors
    }

    /**
     * Get ANSI color code string
     */
    std::string get_color_code(Color color) {
        switch (color) {
            case Color::RESET:          return "\033[0m";
            case Color::BLACK:          return "\033[30m";
            case Color::RED:            return "\033[31m";
            case Color::GREEN:          return "\033[32m";
            case Color::YELLOW:         return "\033[33m";
            case Color::BLUE:           return "\033[34m";
            case Color::MAGENTA:        return "\033[35m";
            case Color::CYAN:           return "\033[36m";
            case Color::WHITE:          return "\033[37m";
            case Color::BRIGHT_BLACK:   return "\033[90m";
            case Color::BRIGHT_RED:     return "\033[91m";
            case Color::BRIGHT_GREEN:   return "\033[92m";
            case Color::BRIGHT_YELLOW:  return "\033[93m";
            case Color::BRIGHT_BLUE:    return "\033[94m";
            case Color::BRIGHT_MAGENTA: return "\033[95m";
            case Color::BRIGHT_CYAN:    return "\033[96m";
            case Color::BRIGHT_WHITE:   return "\033[97m";
            default:                    return "\033[0m";
        }
    }

    /**
     * Get ANSI style code string
     */
    std::string get_style_code(Style style) {
        switch (style) {
            case Style::RESET:      return "\033[0m";
            case Style::BOLD:       return "\033[1m";
            case Style::DIM:        return "\033[2m";
            case Style::ITALIC:     return "\033[3m";
            case Style::UNDERLINE:  return "\033[4m";
            default:                return "\033[0m";
        }
    }
} // anonymous namespace

bool supports_color() {
    if (!color_support_checked) {
        color_supported = detect_color_support();
        color_support_checked = true;
    }
    return color_supported;
}

void set_color_enabled(bool enabled) {
    color_enabled = enabled;
}

std::string colorize(const std::string& text, Color color) {
    if (!color_enabled || !supports_color() || color == Color::RESET) {
        return text;
    }
    return get_color_code(color) + text + get_color_code(Color::RESET);
}

std::string stylize(const std::string& text, Style style) {
    if (!color_enabled || !supports_color() || style == Style::RESET) {
        return text;
    }
    return get_style_code(style) + text + get_style_code(Style::RESET);
}

std::string format(const std::string& text, Color color, Style style) {
    if (!color_enabled || !supports_color()) {
        return text;
    }
    std::string result = text;
    if (style != Style::RESET) {
        result = get_style_code(style) + result;
    }
    if (color != Color::RESET) {
        result = get_color_code(color) + result;
    }
    if (style != Style::RESET || color != Color::RESET) {
        result += get_color_code(Color::RESET);
    }
    return result;
}

void print(const std::string& message) {
    std::cout << message << std::endl;
}

void print_colored(const std::string& message, Color color) {
    std::cout << colorize(message, color) << std::endl;
}

void print_info(const std::string& message) {
    std::cout << colorize("Info: ", Color::CYAN) << message << std::endl;
}

void print_success(const std::string& message) {
    std::cout << colorize("✓ ", Color::GREEN) << message << std::endl;
}

void print_warning(const std::string& message) {
    std::cout << colorize("Warning: ", Color::YELLOW) << message << std::endl;
}

void print_error(const std::string& message) {
    std::cerr << colorize("Error: ", Color::RED) << message << std::endl;
}

std::string align_text(const std::string& text, size_t width, Alignment alignment) {
    if (text.length() >= width) {
        return text;
    }

    size_t padding = width - text.length();

    switch (alignment) {
        case Alignment::LEFT:
            return text + std::string(padding, ' ');
        
        case Alignment::RIGHT:
            return std::string(padding, ' ') + text;
        
        case Alignment::CENTER: {
            size_t left_pad = padding / 2;
            size_t right_pad = padding - left_pad;
            return std::string(left_pad, ' ') + text + std::string(right_pad, ' ');
        }
        
        default:
            return text;
    }
}

std::string pad_left(const std::string& text, size_t width, char fill) {
    if (text.length() >= width) {
        return text;
    }
    return std::string(width - text.length(), fill) + text;
}

std::string pad_right(const std::string& text, size_t width, char fill) {
    if (text.length() >= width) {
        return text;
    }
    return text + std::string(width - text.length(), fill);
}

std::string truncate(const std::string& text, size_t max_width) {
    if (text.length() <= max_width) {
        return text;
    }
    if (max_width <= 3) {
        return text.substr(0, max_width);
    }
    return text.substr(0, max_width - 3) + "...";
}

// ============================================================================
// Table Implementation
// ============================================================================

Table::Table() 
    : border_style_(BorderStyle::SIMPLE)
    , headers_enabled_(true) 
{
}

void Table::set_headers(const std::vector<std::string>& headers) {
    headers_ = headers;
}

void Table::add_row(const std::vector<std::string>& row) {
    TableRow table_row;
    table_row.columns = row;
    rows_.push_back(table_row);
}

void Table::set_column_alignment(size_t column, Alignment alignment) {
    column_alignments_[column] = alignment;
}

void Table::set_border_style(BorderStyle style) {
    border_style_ = style;
}

void Table::set_column_widths(const std::vector<size_t>& widths) {
    min_column_widths_ = widths;
}

void Table::set_headers_enabled(bool enabled) {
    headers_enabled_ = enabled;
}

std::vector<size_t> Table::calculate_column_widths() const {
    size_t num_columns = 0;
    
    // Determine number of columns
    if (headers_enabled_ && !headers_.empty()) {
        num_columns = headers_.size();
    }
    for (const auto& row : rows_) {
        num_columns = std::max(num_columns, row.columns.size());
    }
    
    if (num_columns == 0) {
        return {};
    }

    std::vector<size_t> widths(num_columns, 0);

    // Consider minimum widths
    for (size_t i = 0; i < num_columns && i < min_column_widths_.size(); ++i) {
        widths[i] = min_column_widths_[i];
    }

    // Calculate from headers
    if (headers_enabled_) {
        for (size_t i = 0; i < headers_.size(); ++i) {
            widths[i] = std::max(widths[i], headers_[i].length());
        }
    }

    // Calculate from rows
    for (const auto& row : rows_) {
        for (size_t i = 0; i < row.columns.size(); ++i) {
            widths[i] = std::max(widths[i], row.columns[i].length());
        }
    }

    return widths;
}

std::string Table::render_separator(const std::vector<size_t>& widths, 
                                     const char* left, const char* mid, const char* right, const char* fill) const {
    if (border_style_ == BorderStyle::NONE) {
        return "";
    }

    std::ostringstream oss;
    oss << left;
    
    for (size_t i = 0; i < widths.size(); ++i) {
        for (size_t j = 0; j < widths[i] + 2; ++j) { // +2 for padding
            oss << fill;
        }
        if (i < widths.size() - 1) {
            oss << mid;
        }
    }
    
    oss << right << "\n";
    return oss.str();
}

std::string Table::render_row(const std::vector<std::string>& columns, 
                               const std::vector<size_t>& widths,
                               const char* left, const char* mid, const char* right) const {
    std::ostringstream oss;
    
    if (border_style_ != BorderStyle::NONE) {
        oss << left << " ";
    }

    for (size_t i = 0; i < widths.size(); ++i) {
        std::string cell = (i < columns.size()) ? columns[i] : "";
        
        // Apply alignment
        Alignment align = Alignment::LEFT;
        auto it = column_alignments_.find(i);
        if (it != column_alignments_.end()) {
            align = it->second;
        }
        
        oss << align_text(cell, widths[i], align);
        
        if (border_style_ != BorderStyle::NONE) {
            if (i < widths.size() - 1) {
                oss << " " << mid << " ";
            } else {
                oss << " " << right;
            }
        } else {
            if (i < widths.size() - 1) {
                oss << "  "; // Two spaces between columns
            }
        }
    }
    
    oss << "\n";
    return oss.str();
}

std::string Table::render() const {
    auto widths = calculate_column_widths();
    if (widths.empty()) {
        return "";
    }

    std::ostringstream result;

    // Determine border characters based on style
    // Using const char* for Unicode characters to avoid char overflow warnings
    struct BorderChars {
        const char* top_left;
        const char* top_mid;
        const char* top_right;
        const char* mid_left;
        const char* mid_mid;
        const char* mid_right;
        const char* bot_left;
        const char* bot_mid;
        const char* bot_right;
        const char* vert;
        const char* horiz;
    };

    BorderChars chars;

    switch (border_style_) {
        case BorderStyle::SIMPLE:
            chars = {"+", "+", "+", "+", "+", "+", "+", "+", "+", "|", "-"};
            break;

        case BorderStyle::ROUNDED:
            chars = {"╭", "┬", "╮", "├", "┼", "┤", "╰", "┴", "╯", "│", "─"};
            break;

        case BorderStyle::DOUBLE:
            chars = {"╔", "╦", "╗", "╠", "╬", "╣", "╚", "╩", "╝", "║", "═"};
            break;

        case BorderStyle::NONE:
        default:
            chars = {" ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " "};
            break;
    }

    // Top border
    if (border_style_ != BorderStyle::NONE) {
        result << render_separator(widths, chars.top_left, chars.top_mid, chars.top_right, chars.horiz);
    }

    // Headers
    if (headers_enabled_ && !headers_.empty()) {
        result << render_row(headers_, widths, chars.vert, chars.vert, chars.vert);
        
        // Header separator
        if (border_style_ != BorderStyle::NONE && !rows_.empty()) {
            result << render_separator(widths, chars.mid_left, chars.mid_mid, chars.mid_right, chars.horiz);
        }
    }

    // Data rows
    for (const auto& row : rows_) {
        result << render_row(row.columns, widths, chars.vert, chars.vert, chars.vert);
    }

    // Bottom border
    if (border_style_ != BorderStyle::NONE) {
        result << render_separator(widths, chars.bot_left, chars.bot_mid, chars.bot_right, chars.horiz);
    }

    return result.str();
}

void Table::print() const {
    std::cout << render();
}

} // namespace output
} // namespace helpers
} // namespace whatsmy

