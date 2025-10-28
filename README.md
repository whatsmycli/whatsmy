# whatsmy

Fast, minimal, and extensible cross-platform system information CLI with plugin support.

## Overview

**whatsmy** is a CLI tool that provides a unified command interface across all platforms. All functionality comes from plugins.

```bash
whatsmy gpu      # Run GPU plugin
whatsmy cpu      # Run CPU plugin
whatsmy help     # Show help
whatsmy version  # Show version
```

## Architecture

```
whatsmy/
├── main.cpp              # Entry point
├── src/
│   ├── whatsmy.cpp      # Command routing
│   ├── backend/         # Plugin loader
│   └── helpers/         # Helper functions
├── include/whatsmy/     # Public headers
└── CMakeLists.txt       # Build config
```

## Building

**Requirements**:
- C++17 compiler
- CMake 3.15+

**Linux/macOS**:
```bash
mkdir build && cd build
cmake ..
make
```

**Windows**:
```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Contributing

Contributions welcome! See the memory bank for architectural decisions and coding standards.

## Related

- **[plugins](https://github.com/whatsmycli/plugins)** - Plugin repository
- **[plugin-template](https://github.com/whatsmycli/plugin-template)** - Create plugins

## License

GNU General Public License v3.0
