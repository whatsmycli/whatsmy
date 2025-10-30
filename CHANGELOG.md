# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-10-30

### Added

**Core Features**:
- Complete plugin system with dynamic library loading (dlopen/LoadLibrary)
- Cross-platform support for Linux, Windows, and macOS
- Unified command interface: `whatsmy <component>`
- Built-in commands: `help`, `version`
- Smart command suggestions using Levenshtein distance algorithm
- Debug mode with `--debug` flag for troubleshooting
- Plugin validation system with binary format verification
- Comprehensive error handling with actionable error messages

**Build System**:
- CMake 3.15+ build configuration with cross-platform support
- Platform detection and conditional compilation
- Debug and Release build configurations
- Link-Time Optimization (LTO) for optimized binaries
- Static linking of runtime libraries for portability (Linux)
- Symbol stripping in Release mode for smaller binaries
- CPack configuration for automated packaging

**CI/CD Infrastructure**:
- GitHub Actions workflows for automated builds and releases
- Build workflow: Tests compilation on Linux (GCC/Clang), Windows (MSVC), macOS (Universal binary)
- Release workflow: Automated release creation on version tags
- Cross-platform binary artifacts with SHA256 checksums
- Artifact uploads with 7-day retention for testing

**Testing**:
- Complete testing infrastructure with Google Test v1.14.0
- 75 test cases covering unit and integration testing
- Unit tests for command parser, plugin loader, and helpers
- Integration tests for end-to-end workflows
- Tests run automatically in CI on all platforms

**Installation**:
- One-line installation scripts for all platforms
- `install.sh` for Linux/macOS with platform/architecture detection
- `install.ps1` for Windows PowerShell with automatic PATH configuration
- Checksum verification for secure installation
- Support for both system-wide and user-local installation

**Documentation**:
- Comprehensive README with installation instructions and usage examples
- CONTRIBUTING.md with detailed contribution guidelines
- CODE_OF_CONDUCT.md based on Contributor Covenant 2.1
- docs/architecture.md with complete system architecture documentation
- RELEASE.md with detailed release process documentation
- Plugin API documentation (separate repository)
- Troubleshooting guide (separate repository)

**Helper Functions**:
- Complete ANSI color support (16 colors with bright variants)
- Auto-detection of terminal color capabilities
- Text styling (Bold, Dim, Italic, Underline)
- Advanced table rendering with 3 border styles (Simple, Rounded, Double)
- Column alignment (left, center, right)
- Automatic column width calculation
- Colored print functions (info, success, warning, error)
- Five severity levels for logging (DEBUG, INFO, WARNING, ERROR, CRITICAL)
- Colored output per log level
- Timestamp support for ERROR and CRITICAL levels
- Log level filtering system
- Environment variable support (WHATSMY_DEBUG)

**Plugin System**:
- Plugin directory structure: `/usr/lib/whatsmy/plugins/<name>/<platform>.<ext>`
- Automatic platform binary selection (linux.so, windows.dll, macos.dylib)
- Environment variable override (WHATSMY_PLUGIN_DIR) for development
- Symbol resolution and validation
- Exception safety in plugin execution
- Clean resource management (dlclose/FreeLibrary)
- Plugin validation with binary format verification (ELF/PE/Mach-O)
- Architecture compatibility checking (32-bit vs 64-bit)
- File size validation (min 1KB, max 100MB)
- Permission checks with security warnings
- Return code validation (0-255 range)

**Platform Abstraction**:
- Linux implementation using dlopen/dlsym/dlclose
- Windows implementation using LoadLibrary/GetProcAddress/FreeLibrary
- macOS implementation using dlopen/dlsym/dlclose with platform-specific flags
- Platform-specific error reporting
- Architecture mismatch detection
- Missing dependency identification

**Error Handling**:
- Enhanced plugin not found error messages with diagnostics
- Plugin directory availability checking
- List of available plugins when one is not found
- Platform-specific binary availability information
- Load failure diagnostics with contextual explanations
- Runtime error reporting with exception handling
- Debug mode for verbose troubleshooting output

**Example Content**:
- GPU plugin (plugin-gpu) as reference implementation
  - Linux GPU detection via `/sys/class/drm/`
  - Windows GPU detection using Setup API (pending testing)
  - Vendor detection (NVIDIA, AMD, Intel)
  - PCI ID display (vendor:device)
  - Driver version extraction
- Plugin template (plugin-template) for developers
  - Minimal implementation example
  - Cross-platform platform detection
  - Complete build system with CMake
  - Comprehensive documentation

### Changed
- Improved command-line argument parsing for better user experience
- Enhanced help message with examples and environment variables
- Optimized plugin loading for faster execution
- Updated all file headers with copyright attribution (enXov)

### Fixed
- Memory leaks verified with Valgrind (zero leaks detected)
- Compiler warnings resolved (clean builds with -Wall -Wextra -Wpedantic)
- CMake configuration for proper plugin directory creation
- Installation rules for headers and binaries

### Security
- Plugin validation before execution
- Binary format verification to prevent loading invalid files
- File permission checks with warnings for world-writable plugins
- Symbol verification to ensure plugin_run exists
- Input sanitization for plugin names
- Safe exception handling across plugin boundaries

## [0.1.0] - 2025-10-28

### Added
- Initial MVP release
- Basic project structure with src/ and include/ directories
- Main application entry point (main.cpp)
- Command parser and router (src/whatsmy.cpp)
- Plugin loader backend (src/backend/plugin_loader.cpp)
- Platform-specific implementations (linux.cpp, windows.cpp, macos.cpp)
- Plugin API definition (include/whatsmy/plugin_api.h)
- Helper functions for output and error handling
- CMake build system with Debug/Release configurations
- Plugin directory creation during installation

### Internal
- Memory bank documentation structure created
- Project brief and technical context documented
- System architecture patterns defined
- Development workflow established

---

## Release History

| Version | Date       | Highlights                                    |
|---------|------------|-----------------------------------------------|
| 1.0.0   | 2025-10-30 | First stable release with complete CI/CD      |
| 0.1.0   | 2025-10-28 | Initial MVP with core plugin system           |

---

## Future Roadmap

### Planned for Future Releases

**Phase 4: Plugin Management System** (v1.1.0)
- Built-in plugin manager (`whatsmy plugin list`, `whatsmy plugin install`)
- GitHub API integration for plugin discovery
- Automatic plugin downloads and installation
- Plugin update management

**Phase 5: Core Plugins** (v1.2.0)
- CPU detection plugin
- RAM/Memory information plugin
- Disk/Storage information plugin
- Network information plugin
- OS information plugin
- Battery status plugin (laptops)

**Phase 6: Enhanced Features** (v1.3.0)
- Output format options (JSON, YAML, plain text)
- Configuration file support
- Plugin configuration system
- Caching for expensive queries
- Plugin versioning and compatibility checks

**Phase 7: Community Growth** (v2.0.0+)
- Package manager distributions (Homebrew, AUR, Chocolatey, Snap, winget)
- Plugin ecosystem expansion
- Documentation website
- Community plugin showcase

---

## Notes

- This is the first stable release of whatsmycli
- The project follows [Semantic Versioning](https://semver.org/)
- Breaking changes will be clearly marked in future releases
- Plugin API version 1 is considered stable

---

**Full Documentation**: https://github.com/whatsmycli/whatsmy

**Report Issues**: https://github.com/whatsmycli/whatsmy/issues

**Contribute**: See [CONTRIBUTING.md](CONTRIBUTING.md)

