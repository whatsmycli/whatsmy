# Contributing to whatsmycli

Thank you for your interest in contributing to **whatsmycli**! We welcome contributions from the community to help make this project better.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How to Report Issues](#how-to-report-issues)
- [How to Submit Pull Requests](#how-to-submit-pull-requests)
- [Code Style Guidelines](#code-style-guidelines)
- [Testing Requirements](#testing-requirements)
- [Review Process](#review-process)
- [Development Setup](#development-setup)
- [Project Structure](#project-structure)

## Code of Conduct

This project adheres to a [Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to the project maintainers.

## How to Report Issues

### Before Reporting

1. **Search existing issues**: Check if your issue has already been reported
2. **Check documentation**: Review the [README](README.md), [troubleshooting guide](docs/troubleshooting.md), and other docs
3. **Test with latest version**: Ensure you're using the most recent release

### Reporting Bugs

When reporting a bug, please include:

- **Description**: Clear description of the problem
- **Steps to reproduce**: Detailed steps to trigger the bug
- **Expected behavior**: What you expected to happen
- **Actual behavior**: What actually happened
- **Environment**:
  - OS and version (e.g., Ubuntu 22.04, Windows 11, macOS 13)
  - Architecture (x86_64, arm64)
  - whatsmy version (`whatsmy version`)
  - Compiler and version (if building from source)
- **Logs**: Include relevant error messages or debug output (`whatsmy --debug <command>`)
- **Screenshots**: If applicable

**Example Bug Report**:
```markdown
**Bug**: whatsmy gpu crashes on Windows 11

**Steps to Reproduce**:
1. Install whatsmy 0.1.0 on Windows 11
2. Run `whatsmy gpu`
3. Application crashes with error code 1

**Expected**: Display GPU information
**Actual**: Application crashes

**Environment**:
- OS: Windows 11 Pro (22H2)
- Architecture: x86_64
- whatsmy version: 0.1.0
- Compiler: MSVC 2022

**Error Output**:
```
Error: Failed to load plugin 'gpu'
...
```
```

### Requesting Features

For feature requests, please include:

- **Description**: Clear explanation of the feature
- **Use case**: Why this feature would be useful
- **Examples**: How it would work (commands, output, etc.)
- **Alternatives**: Any alternative solutions you've considered

## How to Submit Pull Requests

### Before Submitting

1. **Check existing PRs**: Ensure no one else is working on the same thing
2. **Create an issue**: For major changes, discuss in an issue first
3. **Fork the repository**: Work on your own fork
4. **Create a feature branch**: Use a descriptive name (e.g., `feature/add-cpu-plugin`, `fix/memory-leak`)

### PR Guidelines

1. **One feature/fix per PR**: Keep PRs focused and manageable
2. **Write clear commit messages**: Follow conventional commits format
   - `feat: add CPU detection for Linux`
   - `fix: resolve memory leak in plugin loader`
   - `docs: update troubleshooting guide`
   - `test: add unit tests for command parser`
   - `refactor: simplify plugin validation logic`
3. **Update documentation**: If your change affects user-facing behavior
4. **Add tests**: For new features and bug fixes
5. **Keep changes minimal**: Don't include unrelated changes or formatting-only modifications

### PR Checklist

Before submitting, ensure:

- [ ] Code follows the [code style guidelines](#code-style-guidelines)
- [ ] All tests pass (`ctest` or `cmake --build build --target test`)
- [ ] New code has appropriate test coverage
- [ ] Documentation is updated (README, docs/, comments)
- [ ] Commit messages are clear and descriptive
- [ ] No compiler warnings on `-Wall -Wextra -Wpedantic`
- [ ] No memory leaks (test with valgrind or sanitizers)
- [ ] CI passes (GitHub Actions will test all platforms)

### Submitting the PR

1. **Push to your fork**: `git push origin feature/your-feature-name`
2. **Create PR on GitHub**: Use the web interface
3. **Fill out PR template**: Provide clear description, motivation, and testing details
4. **Link related issues**: Use "Closes #123" or "Fixes #456" in description
5. **Request review**: Maintainers will review your PR

**Example PR Description**:
```markdown
## Description
Add CPU detection for Linux systems using /proc/cpuinfo

## Motivation
Users need CPU information alongside GPU information

## Changes
- Added src/backend/platform/linux_cpu.cpp
- Implemented cpuinfo parsing
- Added unit tests for CPU detection
- Updated documentation

## Testing
- Tested on Ubuntu 22.04, Fedora 38, Arch Linux
- All unit tests pass
- No memory leaks detected with valgrind
- CI passes on all platforms

Closes #42
```

## Code Style Guidelines

### General Principles

1. **Clarity over cleverness**: Write code that's easy to understand
2. **Consistent formatting**: Follow existing code style
3. **Meaningful names**: Use descriptive variable and function names
4. **Comments**: Explain *why*, not *what*
5. **Modularity**: Keep functions small and focused

### C++ Style

#### Naming Conventions

- **Files**: Snake case (e.g., `plugin_loader.cpp`, `error.h`)
- **Classes**: PascalCase (e.g., `PluginLoader`, `HardwareDetector`)
- **Functions**: Snake case (e.g., `load_plugin()`, `get_version()`)
- **Variables**: Snake case (e.g., `plugin_name`, `error_code`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `MAX_PLUGINS`, `DEFAULT_PATH`)
- **Namespaces**: Snake case (e.g., `whatsmy::backend`, `whatsmy::helpers`)

#### Formatting

- **Indentation**: 4 spaces (no tabs)
- **Braces**: Opening brace on same line (K&R style)
  ```cpp
  if (condition) {
      // code
  } else {
      // code
  }
  ```
- **Line length**: 100-120 characters (soft limit)
- **Spacing**: 
  - Space after control keywords: `if (`, `for (`, `while (`
  - No space for function calls: `function(arg)`
  - Space around operators: `a + b`, `x = 5`

#### Code Organization

```cpp
// Copyright notice
// SPDX-License-Identifier: GPL-3.0-or-later

#include "header.h"  // Own header first
#include <system>    // System headers
#include "project"   // Project headers

namespace whatsmy {

// Constants

// Types and classes

// Function implementations

}  // namespace whatsmy
```

#### Best Practices

- **Use C++17 features**: `std::filesystem`, `std::optional`, `if constexpr`, etc.
- **Prefer RAII**: Use smart pointers (`std::unique_ptr`, `std::shared_ptr`)
- **Avoid raw pointers**: Unless interfacing with C APIs
- **Use `const` liberally**: Make intentions clear
- **Initialize variables**: Always initialize at declaration
- **Use `auto` when obvious**: `auto ptr = std::make_unique<Type>()`
- **Avoid macros**: Use `constexpr` or `inline` functions instead

#### Error Handling

- **Use exceptions for exceptional cases**: Memory allocation failures, I/O errors
- **Return codes for expected failures**: Plugin not found, invalid input
- **Always validate input**: Never trust external data
- **Provide meaningful error messages**: Help users understand what went wrong

Example:
```cpp
std::optional<Plugin> load_plugin(const std::string& name) {
    if (name.empty()) {
        helpers::log(Level::ERROR, "Plugin name cannot be empty");
        return std::nullopt;
    }
    
    // Try to load plugin
    // Return plugin or std::nullopt on failure
}
```

### Documentation Style

#### Code Comments

```cpp
// Brief description of what this does
void function_name() {
    // Why we're doing this (not what)
    complex_operation();
}
```

#### Header Documentation

```cpp
/**
 * @brief Load and execute a plugin by name
 * 
 * Searches for the plugin in the plugin directory, loads the platform-specific
 * binary, and executes the plugin_run() function.
 * 
 * @param name Plugin name (folder name in plugin directory)
 * @return Exit code from plugin (0 on success, non-zero on error)
 * @throws std::runtime_error if plugin loading fails critically
 */
int load_and_run(const std::string& name);
```

### CMake Style

- **Indentation**: 4 spaces
- **Commands**: Lowercase (e.g., `add_executable`, `target_link_libraries`)
- **Variables**: UPPER_CASE for CMake variables, respect existing naming
- **Comments**: Explain non-obvious configuration choices

## Testing Requirements

### Unit Tests

- **Coverage**: Aim for >80% code coverage
- **Framework**: Google Test (already integrated)
- **Location**: `tests/unit/`
- **Naming**: `test_<component>.cpp` (e.g., `test_plugin_loader.cpp`)

Example:
```cpp
#include <gtest/gtest.h>
#include "whatsmy/plugin_loader.h"

TEST(PluginLoader, LoadValidPlugin) {
    // Arrange
    PluginLoader loader;
    
    // Act
    auto result = loader.load("test_plugin");
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "test_plugin");
}
```

### Integration Tests

- **Purpose**: Test complete workflows
- **Location**: `tests/integration/`
- **Coverage**: Command-line interface, plugin execution, error handling

### Running Tests

```bash
# Build with tests
cd build
cmake -DBUILD_TESTING=ON ..
make

# Run all tests
ctest

# Run specific test
./tests/unit/test_plugin_loader

# Run with verbose output
ctest --verbose
```

### Platform Testing

All changes are automatically tested on:
- **Linux**: Ubuntu 22.04 (GCC and Clang)
- **Windows**: Windows Server 2022 (MSVC)
- **macOS**: macOS 13 (Universal binary)

GitHub Actions will run tests automatically on every PR.

## Review Process

### What to Expect

1. **Initial review**: Within 1-2 weeks (we're a small team!)
2. **Feedback**: Maintainers may request changes
3. **Discussion**: We may discuss design decisions
4. **Approval**: PR needs approval from at least one maintainer
5. **Merge**: Once approved and CI passes, we'll merge

### Review Criteria

- **Code quality**: Clean, readable, maintainable
- **Testing**: Adequate test coverage
- **Documentation**: Changes are documented
- **Compatibility**: Works across all platforms
- **Performance**: No unnecessary performance regressions
- **Security**: No security vulnerabilities introduced

### Addressing Feedback

- **Be responsive**: Reply to review comments in a timely manner
- **Ask questions**: If feedback is unclear, ask for clarification
- **Make changes**: Push new commits to your branch
- **Mark resolved**: Use GitHub's "resolve conversation" feature
- **Be patient**: Reviews take time, especially for large changes

## Development Setup

### Prerequisites

- **C++17 compiler**: GCC 9+, Clang 10+, or MSVC 2019+
- **CMake**: Version 3.15 or later
- **Git**: For version control
- **Optional**: valgrind (Linux), Address Sanitizer (for leak detection)

### Clone and Build

```bash
# Fork the repository on GitHub first

# Clone your fork
git clone https://github.com/YOUR_USERNAME/whatsmy.git
cd whatsmy

# Add upstream remote
git remote add upstream https://github.com/whatsmycli/whatsmy.git

# Create build directory
mkdir build && cd build

# Configure (Debug mode for development)
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON ..

# Build
make

# Run tests
ctest
```

### Development Workflow

1. **Sync with upstream**: `git fetch upstream && git merge upstream/main`
2. **Create feature branch**: `git checkout -b feature/my-feature`
3. **Make changes**: Edit code, add tests, update docs
4. **Test locally**: Build and run tests
5. **Commit**: `git commit -m "feat: descriptive message"`
6. **Push**: `git push origin feature/my-feature`
7. **Create PR**: Use GitHub web interface

### Debugging

**Linux (GDB)**:
```bash
gdb ./whatsmy
(gdb) run gpu
(gdb) backtrace  # if crash
```

**Linux (Valgrind)**:
```bash
valgrind --leak-check=full ./whatsmy gpu
```

**Windows (Visual Studio)**:
- Open project in Visual Studio
- Set breakpoints
- Press F5 to debug

## Project Structure

```
whatsmy/
├── .github/
│   └── workflows/          # CI/CD (GitHub Actions)
├── docs/                   # Documentation
│   ├── architecture.md     # System architecture
│   ├── plugin-api.md       # Plugin development guide
│   └── troubleshooting.md  # User troubleshooting guide
├── include/
│   └── whatsmy/            # Public headers
│       ├── plugin_api.h    # Plugin interface
│       ├── plugin_loader.h # Plugin loading
│       ├── helpers.h       # Helper functions
│       └── *.h             # Other headers
├── src/
│   ├── whatsmy.cpp         # Main application logic
│   ├── backend/            # Implementation details
│   │   ├── plugin_loader.cpp
│   │   ├── plugin_validator.cpp
│   │   └── platform/       # Platform-specific code
│   │       ├── linux.cpp
│   │       ├── windows.cpp
│   │       └── macos.cpp
│   └── helpers/            # Helper implementations
│       ├── output.cpp      # Output formatting
│       └── error.cpp       # Error handling
├── tests/
│   ├── unit/               # Unit tests
│   └── integration/        # Integration tests
├── main.cpp                # Entry point
├── CMakeLists.txt          # Build configuration
├── install.sh              # Linux/macOS installer
├── install.ps1             # Windows installer
├── LICENSE                 # GPLv3
├── README.md               # Project overview
└── CONTRIBUTING.md         # This file
```

## Questions?

If you have questions about contributing:

1. Check the [documentation](docs/)
2. Search [existing issues](https://github.com/whatsmycli/whatsmy/issues)
3. Ask in a new issue with the "question" label
4. Email the maintainers (see [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md))

## Thank You!

Thank you for contributing to **whatsmycli**! Your contributions help make system information accessible to everyone.

## License

By contributing to this project, you agree that your contributions will be licensed under the GNU General Public License v3.0.

