# Architecture Documentation

## Overview

**whatsmycli** is a minimal, extensible cross-platform CLI tool built around a plugin architecture. The core application is intentionally lightweight, with all functionality provided through dynamically loaded plugins.

**Design Philosophy**: Everything is a plugin. The core application's sole responsibility is to load and execute plugins based on user commands.

## Table of Contents

- [System Architecture](#system-architecture)
- [Core Components](#core-components)
- [Plugin System](#plugin-system)
- [Data Flow](#data-flow)
- [Platform Abstraction](#platform-abstraction)
- [Build System](#build-system)
- [Testing Architecture](#testing-architecture)
- [Distribution Model](#distribution-model)
- [Security Considerations](#security-considerations)
- [Future Roadmap](#future-roadmap)

## System Architecture

### High-Level Overview

```
┌────────────────────────────────────────────────────────────────┐
│                        User Interface                          │
│                     (Command Line)                            │
└─────────────────────────┬──────────────────────────────────────┘
                          │
                          ▼
┌────────────────────────────────────────────────────────────────┐
│                      whatsmy Core                              │
│                                                                │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │   Command    │  │   Plugin     │  │   Helper     │       │
│  │   Parser     │─▶│   Loader     │  │  Functions   │       │
│  └──────────────┘  └──────┬───────┘  └──────────────┘       │
│                            │                                   │
└────────────────────────────┼───────────────────────────────────┘
                             │
                             ▼
                 ┌───────────────────────┐
                 │  Platform Abstraction │
                 │   (Linux/Win/macOS)   │
                 └───────────┬───────────┘
                             │
            ┌────────────────┼────────────────┐
            │                │                │
            ▼                ▼                ▼
      ┌─────────┐      ┌─────────┐    ┌─────────┐
      │  GPU    │      │  CPU    │    │ Custom  │
      │ Plugin  │      │ Plugin  │    │ Plugins │
      └─────────┘      └─────────┘    └─────────┘
```

### Architectural Layers

1. **User Interface Layer**: Command-line interface accepting commands
2. **Application Layer**: Command parsing and routing
3. **Plugin Management Layer**: Dynamic plugin loading and execution
4. **Platform Abstraction Layer**: OS-specific implementations
5. **Plugin Layer**: Individual plugins providing functionality

## Core Components

### 1. Main Entry Point (`main.cpp`)

**Responsibility**: Minimal entry point that delegates to application logic

```cpp
int main(int argc, char* argv[]) {
    try {
        return whatsmy::run(argc, argv);
    } catch (const std::exception& e) {
        // Handle critical errors
        return ExitCode::CRITICAL_ERROR;
    }
}
```

**Design Decision**: Keep entry point minimal to simplify testing and maintenance.

### 2. Command Parser (`src/whatsmy.cpp`)

**Responsibilities**:
- Parse command-line arguments
- Handle built-in commands (`help`, `version`)
- Route plugin commands to plugin loader
- Provide command suggestions using Levenshtein distance

**Flow**:
```
User Command
    │
    ├─▶ "help" or "version" ─▶ Display info ─▶ Exit
    │
    ├─▶ "--debug" flag ─▶ Enable debug mode
    │
    └─▶ Other commands ─▶ Load and execute plugin
```

**Key Features**:
- No double-dash (`--`) prefix for commands (simpler UX)
- Smart suggestions for typos (e.g., "gpo" → suggests "gpu")
- Debug mode support via `--debug` or `WHATSMY_DEBUG=1`

### 3. Plugin Loader (`src/backend/plugin_loader.cpp`)

**Responsibilities**:
- Discover available plugins
- Load platform-specific plugin binaries
- Validate plugin structure and symbols
- Execute plugin functions
- Handle plugin errors

**Discovery Process**:
```
1. Read WHATSMY_PLUGIN_DIR environment variable (if set)
   OR
   Use default: /usr/lib/whatsmy/plugins/

2. Scan plugin directory for subdirectories

3. For each subdirectory:
   - Detect current platform (Linux/Windows/macOS)
   - Look for platform-specific binary:
     • linux.so (Linux)
     • windows.dll (Windows)
     • macos.dylib (macOS)

4. Cache available plugins (future optimization)
```

**Loading Process**:
```
1. Validate plugin file:
   - Exists and readable
   - Correct binary format (ELF/PE/Mach-O)
   - Reasonable size (1KB - 100MB)
   - Proper permissions

2. Load dynamic library:
   - dlopen() on Linux/macOS
   - LoadLibrary() on Windows

3. Resolve symbols:
   - Find plugin_run() function
   - Validate function signature

4. Execute:
   - Call plugin_run()
   - Capture return code
   - Handle exceptions

5. Cleanup:
   - dlclose() / FreeLibrary()
   - Report errors if any
```

### 4. Plugin Validator (`src/backend/plugin_validator.cpp`)

**Responsibilities**:
- Verify binary format (ELF, PE, Mach-O)
- Check architecture compatibility (32-bit vs 64-bit)
- Validate required symbols exist
- Detect security issues (permissions, world-writable)

**Validation Layers**:

1. **File Access Validation**:
   - File exists
   - Readable permissions
   - Regular file (not directory or symlink)
   - Size within acceptable range

2. **Binary Format Validation**:
   - Linux: ELF magic number (`0x7F 'E' 'L' 'F'`)
   - Windows: PE header (`MZ` signature)
   - macOS: Mach-O magic numbers (multiple variants)

3. **Symbol Validation**:
   - `plugin_run` function exists
   - Correct function signature
   - C linkage (extern "C")

4. **Return Code Validation**:
   - Exit code in valid range (0-255)
   - Standard codes documented (0-7)

### 5. Helper Functions (`src/helpers/`)

#### Output Formatting (`output.cpp`)

**Features**:
- ANSI color support with auto-detection
- Text styling (bold, italic, underline)
- Table rendering with 3 border styles:
  - Simple (ASCII)
  - Rounded (Unicode box-drawing)
  - Double (Unicode double-line)
- Column alignment (left, center, right)
- Utility functions for padding, truncation, centering

**Usage by Plugins**:
```cpp
#include "whatsmy/helpers.h"

int plugin_run() {
    using namespace whatsmy::helpers;
    
    print_info("GPU Information:");
    
    Table table;
    table.set_border_style(BorderStyle::ROUNDED);
    table.add_row({"Property", "Value"});
    table.add_row({"Name", "NVIDIA GTX 1050"});
    table.render();
    
    return 0;
}
```

#### Error Handling (`error.cpp`)

**Features**:
- Five log levels (DEBUG, INFO, WARNING, ERROR, CRITICAL)
- Colored output per severity
- Automatic timestamps for errors
- Environment variable control (`WHATSMY_DEBUG`)
- Error code enumeration with descriptions

### 6. Platform Abstraction (`src/backend/platform/`)

**Purpose**: Isolate platform-specific code to maximize portability

**Structure**:
```
platform/
├── linux.cpp      # Linux-specific implementations
├── windows.cpp    # Windows-specific implementations
└── macos.cpp      # macOS-specific implementations
```

**Abstracted Operations**:
- Dynamic library loading
- Symbol resolution
- File system access
- Binary format detection
- Error message retrieval

**Example** (Dynamic Library Loading):
```cpp
// Linux/macOS (linux.cpp, macos.cpp)
void* handle = dlopen(path.c_str(), RTLD_LAZY);
if (!handle) {
    std::string error = dlerror();
    // Handle error
}

// Windows (windows.cpp)
HMODULE handle = LoadLibraryA(path.c_str());
if (!handle) {
    DWORD error = GetLastError();
    // Handle error
}
```

## Plugin System

### Plugin Architecture

```
Plugin Directory Structure:
/usr/lib/whatsmy/plugins/
├── gpu/
│   ├── linux.so
│   ├── windows.dll
│   └── macos.dylib
├── cpu/
│   ├── linux.so
│   ├── windows.dll
│   └── macos.dylib
└── custom-plugin/
    ├── linux.so
    └── windows.dll
```

**Naming Convention**:
- **Folder name** = command name (e.g., `gpu` → `whatsmy gpu`)
- **Binary name** = platform (e.g., `linux.so`, `windows.dll`, `macos.dylib`)

### Plugin API

**Interface** (`include/whatsmy/plugin_api.h`):

```cpp
extern "C" {
    // Required: Main entry point
    int plugin_run(void);
    
    // Future expansion (optional):
    // const char* plugin_version(void);
    // const char* plugin_description(void);
    // int plugin_init(void);
    // void plugin_cleanup(void);
}
```

**Return Codes**:
- `0`: Success
- `1`: General error
- `2`: Invalid input
- `3`: Resource not found
- `4`: Permission denied
- `5`: Not supported
- `6`: Network error
- `7`: Timeout
- `8-255`: Custom error codes

**Design Decisions**:
1. **C-compatible interface**: Ensures cross-compiler compatibility
2. **Minimal API**: Start simple, expand based on real needs
3. **Extern "C" linkage**: Prevents C++ name mangling issues
4. **Return codes**: Simple, reliable error communication

### Plugin Lifecycle

```
┌─────────────┐
│   Request   │
│   Plugin    │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  Discovery  │  Scan plugin directory
│             │  Find plugin folder
└──────┬──────┘
       │
       ▼
┌─────────────┐
│ Validation  │  Check file exists
│             │  Verify binary format
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   Loading   │  dlopen / LoadLibrary
│             │  Resolve plugin_run symbol
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  Execution  │  Call plugin_run()
│             │  Capture return code
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   Cleanup   │  dlclose / FreeLibrary
│             │  Report any errors
└─────────────┘
```

### Plugin Development

**Typical Plugin Structure**:
```cpp
#include "whatsmy/plugin_api.h"
#include "whatsmy/helpers.h"
#include <iostream>

extern "C" {
    int plugin_run() {
        try {
            // Detect platform
            #ifdef __linux__
                // Linux implementation
            #elif _WIN32
                // Windows implementation
            #elif __APPLE__
                // macOS implementation
            #else
                return 5;  // Not supported
            #endif
            
            return 0;  // Success
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;  // General error
        }
    }
}
```

**Build Process** (CMake):
```cmake
add_library(plugin SHARED plugin.cpp)
set_target_properties(plugin PROPERTIES OUTPUT_NAME "linux")
```

## Data Flow

### Command Execution Flow

```
User Types: whatsmy gpu
    │
    ▼
main.cpp receives argc, argv
    │
    ▼
whatsmy::run() parses arguments
    │
    ├─▶ "help" → display_help() → exit(0)
    ├─▶ "version" → display_version() → exit(0)
    │
    └─▶ "gpu" → PluginLoader::load_and_run("gpu")
            │
            ▼
        Find plugin directory: /usr/lib/whatsmy/plugins/gpu/
            │
            ▼
        Detect platform: Linux → look for linux.so
            │
            ▼
        Validate plugin: check ELF format, symbols
            │
            ▼
        Load library: dlopen("linux.so")
            │
            ▼
        Resolve symbol: dlsym("plugin_run")
            │
            ▼
        Execute: plugin_run()
            │
            ├─▶ Plugin displays GPU info
            │
            └─▶ Returns exit code (0 = success)
            │
            ▼
        Cleanup: dlclose()
            │
            ▼
        Return exit code to user
```

### Error Handling Flow

```
Error Detected
    │
    ▼
Determine Error Category
    │
    ├─▶ Plugin Not Found
    │       └─▶ Display available plugins
    │       └─▶ Suggest similar names
    │       └─▶ Link to troubleshooting
    │
    ├─▶ Load Failure
    │       └─▶ Check architecture mismatch
    │       └─▶ Check missing dependencies
    │       └─▶ Check symbol issues
    │       └─▶ Provide specific solution
    │
    ├─▶ Runtime Error
    │       └─▶ Catch exception
    │       └─▶ Display error message
    │       └─▶ Return error code
    │
    └─▶ Unknown Error
            └─▶ Display generic message
            └─▶ Enable debug mode suggestion
```

## Platform Abstraction

### Strategy

1. **Conditional Compilation**: Use preprocessor directives for platform detection
2. **Platform-Specific Files**: Separate implementations in `platform/` directory
3. **Abstract Interfaces**: Define common interfaces, implement per-platform
4. **CMake Integration**: Automatically select correct source files

### Platform Detection

```cpp
#ifdef __linux__
    // Linux code
#elif _WIN32
    // Windows code
#elif __APPLE__
    // macOS code
#else
    #error "Unsupported platform"
#endif
```

### Platform-Specific Considerations

#### Linux
- **Dynamic loading**: `dlopen`, `dlsym`, `dlclose` from `<dlfcn.h>`
- **Binary format**: ELF (Executable and Linkable Format)
- **GPU detection**: `/sys/class/drm/`, `/proc/driver/nvidia/version`
- **Package naming**: `.so` extension

#### Windows
- **Dynamic loading**: `LoadLibrary`, `GetProcAddress`, `FreeLibrary` from `<windows.h>`
- **Binary format**: PE (Portable Executable)
- **GPU detection**: Setup API, WMI, DXGI
- **Package naming**: `.dll` extension
- **Special considerations**: Requires COM initialization for some APIs

#### macOS
- **Dynamic loading**: `dlopen`, `dlsym`, `dlclose` (similar to Linux)
- **Binary format**: Mach-O (Mach Object)
- **GPU detection**: IOKit framework, Metal API
- **Package naming**: `.dylib` extension
- **Special considerations**: Code signing, SIP (System Integrity Protection)

## Build System

### CMake Structure

```cmake
cmake_minimum_required(VERSION 3.15)
project(whatsmy VERSION 0.1.0)

# Platform detection
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Source files
set(CORE_SOURCES
    main.cpp
    src/whatsmy.cpp
    src/backend/plugin_loader.cpp
    src/backend/plugin_validator.cpp
    src/helpers/output.cpp
    src/helpers/error.cpp
)

# Platform-specific sources
if(LINUX)
    list(APPEND CORE_SOURCES src/backend/platform/linux.cpp)
elseif(WIN32)
    list(APPEND CORE_SOURCES src/backend/platform/windows.cpp)
elseif(APPLE)
    list(APPEND CORE_SOURCES src/backend/platform/macos.cpp)
endif()

# Executable
add_executable(whatsmy ${CORE_SOURCES})

# Compiler flags
if(MSVC)
    target_compile_options(whatsmy PRIVATE /W4 /O2)
else()
    target_compile_options(whatsmy PRIVATE -Wall -Wextra -O3)
endif()

# Installation
install(TARGETS whatsmy DESTINATION bin)
install(DIRECTORY DESTINATION lib/whatsmy/plugins)
```

### Build Configurations

**Debug**:
- Debug symbols (`-g`)
- No optimization (`-O0`)
- All warnings (`-Wall -Wextra`)
- Optional sanitizers

**Release**:
- Maximum optimization (`-O3`)
- Link-Time Optimization (`-flto`)
- Strip symbols (`-s`)
- Static linking (where appropriate)
- NDEBUG defined

## Testing Architecture

### Test Structure

```
tests/
├── CMakeLists.txt           # Test configuration
├── unit/                    # Unit tests
│   ├── test_command_parser.cpp
│   ├── test_plugin_loader.cpp
│   └── test_helpers.cpp
└── integration/             # Integration tests
    ├── test_e2e.cpp
    └── test_plugin_execution.cpp
```

### Testing Framework

- **Framework**: Google Test (integrated via FetchContent)
- **Runner**: CTest (CMake's testing system)
- **Coverage**: 75+ test cases covering core functionality

### Test Categories

1. **Unit Tests**:
   - Command parser logic
   - Plugin loader functions
   - Helper utilities
   - Error handling

2. **Integration Tests**:
   - End-to-end command execution
   - Plugin loading and execution
   - Error scenarios
   - Flag handling

### CI Testing

GitHub Actions runs tests on:
- **Linux**: Ubuntu 22.04 (GCC and Clang)
- **Windows**: Windows Server 2022 (MSVC)
- **macOS**: macOS 13 (Universal binary)

## Distribution Model

### Primary Distribution: GitHub Releases

```
User
  │
  ├─▶ One-line install:
  │   curl ... | bash
  │       │
  │       ├─▶ Detect platform/arch
  │       ├─▶ Download latest release from GitHub API
  │       ├─▶ Verify checksum
  │       ├─▶ Extract binary
  │       ├─▶ Install to /usr/local/bin
  │       ├─▶ Create plugin directory
  │       └─▶ Verify installation
  │
  └─▶ Manual install:
      │
      ├─▶ Download from GitHub Releases page
      ├─▶ Extract archive
      └─▶ Copy to PATH manually
```

### Release Process

1. **Version bump**: Update version in CMakeLists.txt and version.h
2. **Create tag**: `git tag -a v1.0.0 -m "Release 1.0.0"`
3. **Push tag**: `git push origin v1.0.0`
4. **GitHub Actions**:
   - Builds binaries for all platforms
   - Creates GitHub Release
   - Uploads artifacts with checksums
   - Generates changelog

### Future Distribution (Post v1.0.0)

- Package managers (Homebrew, AUR, Chocolatey, Scoop)
- Linux distributions (.deb, .rpm)
- Container images (Docker, Snap)

## Security Considerations

### Plugin Security Model

**Current**: Trust-based model
- Plugins run with same privileges as main application
- All plugins must be open-source (GPLv3)
- Community verification via plugins repository

**Risks**:
- Malicious plugins can access system resources
- No sandboxing or isolation
- Plugins can crash or hang

**Mitigations**:
- Plugin validation (binary format, symbols)
- Exception handling for plugin crashes
- Community review process
- Source code verification

**Future Enhancements**:
- Process isolation (separate processes for plugins)
- Capability-based security
- Syscall filtering (seccomp on Linux)
- Code signing requirements
- Plugin permissions model

### Input Validation

- Sanitize plugin names before loading
- Validate file paths (prevent traversal attacks)
- Limit resource usage (file size limits)
- Check permissions (warn on world-writable)

### Dependency Security

- Minimal dependencies (standard library only)
- Static linking where possible
- Regular security audits
- Automated vulnerability scanning (future)

## Future Roadmap

### Phase 4: Plugin Management System (Future)

**Built-in Plugin Manager**:
```bash
whatsmy plugin list         # List available plugins
whatsmy plugin install gpu  # Install plugin
whatsmy plugin installed    # Show installed plugins
whatsmy plugin update gpu   # Update plugin
whatsmy plugin remove gpu   # Uninstall plugin
```

**Features**:
- GitHub API integration
- Automatic updates
- Dependency management
- Plugin versioning

### Phase 5: Core Plugins (Future)

Essential system information plugins:
- `cpu` - CPU information
- `ram` - Memory information
- `disk` - Storage information
- `network` - Network interfaces
- `os` - Operating system details
- `battery` - Battery status (laptops)

### Phase 6: Enhanced Features (Future)

**Output Formats**:
- JSON output (`whatsmy --json gpu`)
- YAML output (`whatsmy --yaml gpu`)
- Plain text (default)

**Configuration**:
- User configuration file (`~/.config/whatsmy/config.yaml`)
- Plugin-specific settings
- Output preferences

**Performance**:
- Plugin caching (keep loaded between invocations)
- Query result caching
- Parallel plugin execution

### Phase 7: Community Growth (Future)

- Documentation website
- Plugin marketplace
- Tutorial videos
- Community showcase
- Package manager distributions

## Design Principles

### 1. Simplicity First

- Minimal core with maximum extensibility
- Clear, understandable code
- No over-engineering

### 2. Plugin-Only Architecture

- Everything is a plugin (no built-in commands)
- Core is just a plugin loader
- Consistent user experience

### 3. Cross-Platform by Design

- Platform abstraction from the start
- Test on all platforms automatically
- Consistent behavior everywhere

### 4. Contributor-Friendly

- Clear code organization
- Comprehensive documentation
- Welcoming contribution process
- Fast review turnaround

### 5. Performance Matters

- Fast startup time (< 50ms)
- Minimal memory footprint (< 10MB)
- Small binary size (< 2MB)
- Efficient plugin loading

## Conclusion

The **whatsmycli** architecture is designed for simplicity, extensibility, and cross-platform compatibility. By keeping the core minimal and delegating all functionality to plugins, we create a system that's easy to understand, maintain, and extend.

The plugin system allows unlimited growth while maintaining a clean, focused core. Platform abstraction ensures consistent behavior across operating systems, and the automated CI/CD pipeline guarantees quality on every commit.

This architecture is built to grow with the community while remaining true to its core principle: **everything is a plugin**.

---

**Document Version**: 1.0  
**Last Updated**: October 30, 2025  
**Project Version**: 0.1.0

