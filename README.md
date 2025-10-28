# whatsmy

Fast, minimal, and extensible cross-platform system information CLI with plugin support.

[![Build Status](https://github.com/whatsmycli/whatsmy/workflows/build/badge.svg)](https://github.com/whatsmycli/whatsmy/actions)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey)](https://github.com/whatsmycli/whatsmy)

One command. All your system info. Any platform.

```bash
whatsmy gpu      # Show everything about your graphics card
whatsmy cpu      # Show everything about your processor
whatsmy ram      # Show everything about your memory
```

## Features

- **Unified Interface**: One command pattern for all system information
- **Cross-Platform**: Works identically on Linux, Windows, and macOS
- **Lightning Fast**: Optimized C++ implementation with minimal overhead
- **Extensible**: Simple plugin system for unlimited functionality
- **Clean Codebase**: Well-documented, easy to contribute to
- **Open Source**: GPLv3 licensed, community-driven development

### Build from Source

**Requirements**:
- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.15 or later
- Git

**Linux/macOS**:
```bash
git clone https://github.com/whatsmycli/whatsmy.git
cd whatsmy
mkdir build && cd build
cmake ..
make
sudo make install
```

**Windows**:
```powershell
git clone https://github.com/whatsmycli/whatsmy.git
cd whatsmy
mkdir build
cd build
cmake ..
cmake --build . --config Release
cmake --install .
```

## Architecture

**whatsmycli** follows a clean, modular architecture designed for easy contribution:

```
whatsmy/
├── main.cpp                    # Entry point only
├── src/
│   ├── whatsmy.cpp            # Application logic & command routing
│   └── backend/               # All implementation details
│       ├── plugin_loader.cpp  # Dynamic library loading
│       ├── gpu_detector.cpp   # GPU detection implementation
│       ├── platform/          # Platform-specific code
│       │   ├── linux.cpp
│       │   ├── windows.cpp
│       │   └── macos.cpp
│       └── utils/             # Utility functions
│           ├── output.cpp
│           └── error.cpp
├── include/whatsmy/           # Public headers
│   ├── plugin_api.h          # Plugin interface
│   ├── types.h
│   └── version.h
└── CMakeLists.txt            # Build configuration
```

### Design Principles

1. **Clarity First**: Code should be immediately understandable
2. **Separation of Concerns**: Entry point → Logic → Implementation
3. **Platform Isolation**: Platform-specific code in dedicated files
4. **Minimal Dependencies**: Standard library only (no bloat)
5. **Fast Compilation**: Quick iteration for developers

### Getting Started

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Make your changes
4. Build and test locally
5. Commit with clear messages: `git commit -m "Add feature X"`
6. Push to your fork: `git push origin feature/my-feature`
7. Open a Pull Request

## License

This project is licensed under the **GNU General Public License v3.0**.

See [LICENSE](LICENSE) for full details.

## Acknowledgments

Built with the goal of making system information accessible to everyone. Special thanks to all contributors who help make this project better!
