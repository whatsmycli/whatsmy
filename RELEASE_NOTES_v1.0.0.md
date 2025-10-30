# whatsmycli v1.0.0 - First Stable Release 🎉

We're thrilled to announce the first stable release of **whatsmycli**! This project provides a unified, intuitive interface for system information across Linux, Windows, and macOS through an extensible plugin architecture.

## 🌟 Highlights

### Unified Command Interface
- **One command to rule them all**: `whatsmy <component>`
- No need to remember different commands across platforms
- Simple, intuitive syntax for all system information queries

### Cross-Platform Support
- ✅ **Linux** (x86_64) - Full support
- ✅ **Windows** (x64) - Full support with MSVC builds
- ✅ **macOS** (Universal binary) - Intel and Apple Silicon

### Plugin Architecture
- Dynamic plugin loading system
- Platform-specific binaries (`.so`, `.dll`, `.dylib`)
- Robust validation and error handling
- Safe exception handling across plugin boundaries

### One-Line Installation
- Simple installation scripts for all platforms
- Automatic platform and architecture detection
- Checksum verification for security
- Automatic PATH configuration

### Complete CI/CD Pipeline
- Automated builds on every commit
- Cross-platform testing (Linux, Windows, macOS)
- Automated release creation on version tags
- 75 unit and integration tests (100% passing)

### Comprehensive Documentation
- Detailed architecture documentation
- Contributor guidelines with code of conduct
- Complete release process documentation
- Plugin API documentation (separate repository)

## 📦 Installation

### Quick Install

**Linux/macOS**:
```bash
curl -sSL https://raw.githubusercontent.com/whatsmycli/whatsmy/main/install.sh | bash
```

**Windows (PowerShell)**:
```powershell
irm https://raw.githubusercontent.com/whatsmycli/whatsmy/main/install.ps1 | iex
```

### Manual Installation

1. Download the appropriate binary for your platform from the Assets section below
2. Extract and move to a directory in your PATH
3. Create plugin directory (`/usr/lib/whatsmy/plugins` on Unix, `C:\Program Files\whatsmy\plugins` on Windows)
4. Verify installation with `whatsmy version`

For detailed installation instructions, see the [README](https://github.com/whatsmycli/whatsmy#readme).

## 🎯 What's Included

### Core Application
- **whatsmy** - Main CLI executable
- Plugin loading and validation system
- Command parser with smart suggestions
- Debug mode for troubleshooting
- Comprehensive error handling

### Features
- ✅ Dynamic plugin loading (dlopen/LoadLibrary)
- ✅ Plugin validation (binary format, architecture, symbols)
- ✅ Smart command suggestions (Levenshtein distance)
- ✅ Debug mode (`--debug` flag and `WHATSMY_DEBUG` env var)
- ✅ Environment variable overrides (`WHATSMY_PLUGIN_DIR`)
- ✅ Colored output with auto-detection
- ✅ Advanced table rendering (3 border styles)
- ✅ Comprehensive logging system (5 severity levels)

### Build System
- ✅ CMake 3.15+ configuration
- ✅ Cross-platform compilation
- ✅ Debug and Release configurations
- ✅ Link-Time Optimization (LTO)
- ✅ Static linking for portability (Linux)
- ✅ CPack packaging support

### Testing
- ✅ Google Test framework integration
- ✅ 75 test cases (unit + integration)
- ✅ CI testing on all platforms
- ✅ 100% test pass rate

### Documentation
- ✅ README with quick start
- ✅ CONTRIBUTING.md (contribution guidelines)
- ✅ CODE_OF_CONDUCT.md (community standards)
- ✅ CHANGELOG.md (version history)
- ✅ RELEASE.md (release process)
- ✅ docs/architecture.md (system design)

## 🚀 Getting Started

After installation:

```bash
# Show help
whatsmy help

# Show version
whatsmy version

# Run a plugin (requires plugin installation)
whatsmy gpu

# Debug mode
whatsmy --debug gpu
```

## 🔌 Plugin Development

Create your own plugins using our template:

```bash
git clone https://github.com/whatsmycli/plugin-template.git
cd plugin-template
# Follow README to customize and build
```

See the [Plugin API Documentation](https://github.com/whatsmycli/docs) for complete details.

Example plugins:
- **[plugin-gpu](https://github.com/whatsmycli/plugin-gpu)** - GPU detection (reference implementation)
- **[plugin-template](https://github.com/whatsmycli/plugin-template)** - Minimal example

## 📚 Documentation

### Core Documentation
- [README](https://github.com/whatsmycli/whatsmy/blob/main/README.md) - Project overview and installation
- [CONTRIBUTING](https://github.com/whatsmycli/whatsmy/blob/main/CONTRIBUTING.md) - How to contribute
- [CODE_OF_CONDUCT](https://github.com/whatsmycli/whatsmy/blob/main/CODE_OF_CONDUCT.md) - Community standards
- [CHANGELOG](https://github.com/whatsmycli/whatsmy/blob/main/CHANGELOG.md) - Complete version history
- [RELEASE](https://github.com/whatsmycli/whatsmy/blob/main/RELEASE.md) - Release process guide

### Technical Documentation
- [Architecture Guide](https://github.com/whatsmycli/whatsmy/blob/main/docs/architecture.md) - System design and internals
- [Plugin API](https://github.com/whatsmycli/docs) - Create plugins
- [Troubleshooting](https://github.com/whatsmycli/docs) - Common issues and solutions

## ⚠️ Known Issues

### Platform-Specific
- **Windows**: Requires Windows 10 or later for ANSI color support
- **macOS**: Requires macOS 11 (Big Sur) or later
- **Linux**: Some GPU drivers may not be detected automatically

### Limitations
- Plugin management system not yet implemented (planned for v1.1.0)
- Limited core plugins available (GPU only as reference)
- No configuration file support yet (planned for v1.3.0)

### Workarounds
- For plugin directory issues, use `WHATSMY_PLUGIN_DIR` environment variable
- For debugging, use `--debug` flag or `WHATSMY_DEBUG=1`
- See [Troubleshooting Guide](https://github.com/whatsmycli/docs) for more solutions

## 🗺️ Roadmap

### Upcoming Releases

**v1.1.0 - Plugin Management** (Target: Q1 2026)
- Built-in plugin manager (`whatsmy plugin install`, `whatsmy plugin list`)
- GitHub API integration for plugin discovery
- Automatic plugin updates
- Plugin versioning support

**v1.2.0 - Core Plugins** (Target: Q2 2026)
- CPU detection plugin
- RAM/Memory information plugin
- Disk/Storage information plugin
- Network information plugin
- OS information plugin
- Battery status plugin

**v1.3.0 - Enhanced Features** (Target: Q3 2026)
- Output format options (JSON, YAML)
- Configuration file support
- Plugin configuration system
- Query result caching

**v2.0.0 - Community Growth** (Target: Q4 2026+)
- Package manager distributions (Homebrew, AUR, Chocolatey, Snap, winget)
- Plugin ecosystem expansion
- Documentation website
- Community plugin showcase

See the [full roadmap](https://github.com/whatsmycli/whatsmy/blob/main/CHANGELOG.md#future-roadmap) in CHANGELOG.md.

## 🤝 Contributing

We welcome contributions! Whether you're:
- 🐛 Reporting bugs
- 💡 Suggesting features
- 📝 Improving documentation
- 🔌 Creating plugins
- 💻 Contributing code

Please read our [Contributing Guide](https://github.com/whatsmycli/whatsmy/blob/main/CONTRIBUTING.md) to get started.

## 🙏 Acknowledgments

Thank you to everyone who contributed to making this release possible!

Special thanks to:
- The C++ and CMake communities for excellent tools and documentation
- GitHub Actions for free CI/CD
- Google Test for the testing framework
- All future contributors and plugin developers

## 📝 Release Notes

This release includes:
- Complete plugin system with dynamic library loading
- Cross-platform support for Linux, Windows, and macOS
- Automated CI/CD pipeline via GitHub Actions
- One-line installation scripts for all platforms
- Comprehensive documentation and contributor guidelines
- Complete testing infrastructure with 75 test cases
- GPU detection plugin as reference implementation

For a complete list of changes, see [CHANGELOG.md](https://github.com/whatsmycli/whatsmy/blob/main/CHANGELOG.md).

## 🔗 Links

- **Homepage**: https://github.com/whatsmycli/whatsmy
- **Documentation**: https://github.com/whatsmycli/docs
- **Plugins**: https://github.com/whatsmycli/plugins
- **Plugin Template**: https://github.com/whatsmycli/plugin-template
- **Issues**: https://github.com/whatsmycli/whatsmy/issues
- **Discussions**: https://github.com/whatsmycli/whatsmy/discussions

## 📄 License

GNU General Public License v3.0 (GPLv3)

See [LICENSE](https://github.com/whatsmycli/whatsmy/blob/main/LICENSE) for details.

---

**Full Changelog**: https://github.com/whatsmycli/whatsmy/blob/main/CHANGELOG.md

**Verify Installation**: Run `whatsmy version` to confirm successful installation.

**Get Help**: Visit our [documentation](https://github.com/whatsmycli/docs) or open an [issue](https://github.com/whatsmycli/whatsmy/issues).

---

*Built with ❤️ using C++17 and CMake*

