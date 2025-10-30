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

## Quick Start

After installation, you can browse and install plugins:

```bash
# List available plugins
whatsmy plugin list

# Install a plugin
whatsmy plugin install gpu

# Use the plugin
whatsmy gpu

# See all installed plugins
whatsmy plugin installed
```

## Installation

### One-Line Installation (Recommended)

**Linux/macOS**:
```bash
curl -sSL https://raw.githubusercontent.com/whatsmycli/whatsmy/main/install.sh | bash
```

**Windows (PowerShell)**:
```powershell
irm https://raw.githubusercontent.com/whatsmycli/whatsmy/main/install.ps1 | iex
```

The installation script will:
- Detect your platform and architecture
- Download the latest release binary
- Install to the appropriate location
- Create the plugin directory
- Add to your PATH (if needed)
- Verify the installation

### Manual Installation

1. Download the latest release for your platform from [GitHub Releases](https://github.com/whatsmycli/whatsmy/releases)
2. Extract the binary
3. Move to a directory in your PATH:
   - **Linux/macOS**: `sudo mv whatsmy /usr/local/bin/`
   - **Windows**: Move `whatsmy.exe` to `C:\Program Files\whatsmy\` and add to PATH
4. Create plugin directory:
   - **Linux/macOS**: `sudo mkdir -p /usr/lib/whatsmy/plugins`
   - **Windows**: `mkdir C:\Program Files\whatsmy\plugins`
5. Verify installation: `whatsmy version`

### Building from Source

**Requirements**:
- C++17 compiler (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.15+
- Git

**Linux/macOS**:
```bash
git clone https://github.com/whatsmycli/whatsmy.git
cd whatsmy
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
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

For detailed architecture documentation, see [docs/architecture.md](docs/architecture.md).

## Plugin Management

**whatsmy** includes a built-in plugin manager for easy plugin discovery and installation.

### Browse Available Plugins

```bash
whatsmy plugin list
```

This fetches the latest plugin list from the repository and displays all available plugins with descriptions and supported platforms.

### Install Plugins

```bash
whatsmy plugin install <name>
```

Example:
```bash
whatsmy plugin install gpu
```

The installer will:
- Download the correct binary for your platform
- Install to the plugin directory
- Verify with SHA256 checksum (when available)
- Set proper file permissions

### Manage Installed Plugins

```bash
# View installed plugins
whatsmy plugin installed

# Remove a plugin
whatsmy plugin remove gpu

# Update a plugin to the latest version
whatsmy plugin update gpu

# Search for plugins
whatsmy plugin search graphics
```

### Plugin Directory

Plugins are installed to:
- **Linux/macOS**: `/usr/lib/whatsmy/plugins/`
- **Windows**: `C:\Program Files\whatsmy\plugins\`

You can override this with the `WHATSMY_PLUGIN_DIR` environment variable:
```bash
export WHATSMY_PLUGIN_DIR=~/.whatsmy/plugins
```

### Available Plugins

Check the [plugins repository](https://github.com/whatsmycli/plugins) for the current list of available plugins, or run:
```bash
whatsmy plugin list
```

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to contribute to this project.

### Quick Links
- [Code of Conduct](CODE_OF_CONDUCT.md)
- [Plugin API Documentation](https://github.com/whatsmycli/docs) - Complete plugin development guide
- [Troubleshooting Guide](https://github.com/whatsmycli/docs) - Common issues and solutions
- [Release Process](RELEASE.md) - How to create releases

## Related

- **[plugins](https://github.com/whatsmycli/plugins)** - Plugin repository
- **[plugin-template](https://github.com/whatsmycli/plugin-template)** - Create plugins

## License

GNU General Public License v3.0
