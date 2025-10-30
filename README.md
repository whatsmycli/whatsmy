# whatsmy

Core CLI application for whatsmycli. Provides a unified command interface across all platforms through a plugin system.

```bash
whatsmy gpu      # Run GPU plugin
whatsmy cpu      # Run CPU plugin
whatsmy help     # Show help
whatsmy version  # Show version
```

## Building from Source

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

See [architecture.md](https://github.com/whatsmycli/docs) for detailed documentation.

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
