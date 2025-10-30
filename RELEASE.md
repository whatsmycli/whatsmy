# Release Process

This document describes the complete release process for **whatsmycli**, from version bumping to publishing a new release.

## Table of Contents

1. [Overview](#overview)
2. [Pre-Release Checklist](#pre-release-checklist)
3. [Version Bumping](#version-bumping)
4. [Changelog Generation](#changelog-generation)
5. [Testing Before Release](#testing-before-release)
6. [Creating the Release](#creating-the-release)
7. [Verifying CI Builds](#verifying-ci-builds)
8. [Publishing the Release](#publishing-the-release)
9. [Post-Release Tasks](#post-release-tasks)
10. [Rollback Procedure](#rollback-procedure)

---

## Overview

The whatsmycli project uses **GitHub Actions** for automated builds and releases. When a version tag is pushed, the CI/CD pipeline automatically:
- Builds binaries for all platforms (Linux, Windows, macOS)
- Generates checksums
- Creates a GitHub Release
- Uploads artifacts

### Release Types

- **Major Release** (x.0.0): Breaking changes, incompatible API changes
- **Minor Release** (1.x.0): New features, backward-compatible
- **Patch Release** (1.0.x): Bug fixes, backward-compatible

---

## Pre-Release Checklist

Before starting the release process, ensure:

- [ ] All planned features for this release are merged to `main`
- [ ] All CI builds are passing on `main` branch
- [ ] All tests pass locally and in CI
- [ ] Documentation is up to date
- [ ] Known issues are documented
- [ ] CHANGELOG.md is ready (or will be created as part of release)
- [ ] Release notes are prepared

---

## Version Bumping

### 1. Update CMakeLists.txt

Edit the `project()` version in `CMakeLists.txt`:

```cmake
project(whatsmy 
    VERSION 1.0.0
    DESCRIPTION "Universal system information tool"
    LANGUAGES CXX
)
```

Change `VERSION 1.0.0` to your target version (e.g., `1.1.0`, `2.0.0`).

### 2. Update Version Header (if exists)

If you have a `include/whatsmy/version.h` file, update it:

```cpp
#define WHATSMY_VERSION_MAJOR 1
#define WHATSMY_VERSION_MINOR 0
#define WHATSMY_VERSION_PATCH 0
#define WHATSMY_VERSION_STRING "1.0.0"
```

### 3. Update Plugin API Version (if changed)

If the plugin API has changed, update `include/whatsmy/plugin_api.h`:

```cpp
#define WHATSMY_PLUGIN_API_VERSION 1
```

### 4. Commit Version Changes

```bash
git add CMakeLists.txt include/whatsmy/version.h
git commit -m "Bump version to 1.0.0"
```

**Note**: Do NOT tag yet - that comes after changelog and testing.

---

## Changelog Generation

### Option 1: Manual CHANGELOG.md

Create or update `CHANGELOG.md` following [Keep a Changelog](https://keepachangelog.com/) format:

```markdown
# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-10-30

### Added
- Complete plugin system with dynamic library loading
- Cross-platform support (Linux, Windows, macOS)
- Automated CI/CD via GitHub Actions
- One-line installation scripts for all platforms
- Comprehensive documentation (CONTRIBUTING.md, CODE_OF_CONDUCT.md, architecture.md)
- Testing infrastructure with Google Test
- GPU detection plugin as reference implementation

### Changed
- (List any changes from previous version)

### Fixed
- (List any bug fixes)

### Deprecated
- (List any deprecated features)

### Removed
- (List any removed features)

### Security
- (List any security fixes)

## [0.1.0] - 2025-10-28

### Added
- Initial MVP release
- Basic command parser and plugin loader
```

### Option 2: Git Log Based

Generate changelog from git commits:

```bash
# Get commits since last tag
git log v0.1.0..HEAD --oneline --pretty=format:"- %s"

# Or more detailed
git log v0.1.0..HEAD --pretty=format:"- %s (%h)" --reverse
```

### Commit Changelog

```bash
git add CHANGELOG.md
git commit -m "Update CHANGELOG for v1.0.0"
git push origin main
```

---

## Testing Before Release

### 1. Build and Test Locally

**Linux**:
```bash
cd whatsmy
rm -rf build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
ctest --output-on-failure

# Test binary
./whatsmy help
./whatsmy version
```

**Verify output**:
- Version should show the new version number
- No errors or warnings during build
- All tests pass

### 2. Test Installation Scripts

**Test install.sh** (without actually installing):
```bash
# Review script for any issues
cat install.sh

# Test with dry-run by reading through the script
# (Full test requires pushing tag first)
```

### 3. Verify Documentation

Check that all documentation is accurate:
- [ ] README.md has correct installation instructions
- [ ] CONTRIBUTING.md is up to date
- [ ] docs/architecture.md reflects current design
- [ ] Plugin API documentation is current

---

## Creating the Release

### 1. Create Annotated Git Tag

Annotated tags include author, date, and message:

```bash
git tag -a v1.0.0 -m "Release 1.0.0: Initial stable release

Major features:
- Complete plugin system
- Cross-platform support (Linux, Windows, macOS)
- Automated CI/CD pipeline
- One-line installation
- Comprehensive documentation"
```

**Tag naming convention**: `v<major>.<minor>.<patch>` (e.g., `v1.0.0`, `v1.2.3`)

### 2. Push the Tag

```bash
# Push tag to GitHub (this triggers release workflow)
git push origin v1.0.0
```

**⚠️ Warning**: Pushing a tag immediately triggers the release workflow. Make sure everything is ready!

---

## Verifying CI Builds

### 1. Monitor GitHub Actions

After pushing the tag:

1. Go to: https://github.com/whatsmycli/whatsmy/actions
2. Find the "Release" workflow run
3. Watch the build progress

The workflow will:
- Build Linux (x64) binary
- Build Windows (x64) binary
- Build macOS (universal) binary
- Generate SHA256 checksums
- Create GitHub Release draft

### 2. Check Build Status

Each platform should show ✅ (success):
- **build-linux** - Ubuntu 22.04, GCC/Clang
- **build-windows** - Windows Server 2022, MSVC
- **build-macos** - macOS 13, Universal Binary

### 3. Review Build Logs

If any build fails:
1. Click on the failed job
2. Review the error logs
3. Fix the issue
4. Delete the tag: `git tag -d v1.0.0 && git push origin :refs/tags/v1.0.0`
5. Fix the code, commit, and retry

---

## Publishing the Release

### 1. Review the Draft Release

After CI completes successfully:

1. Go to: https://github.com/whatsmycli/whatsmy/releases
2. Find the draft release for your version
3. Review the auto-generated content:
   - Release title
   - Release notes (from commits or CHANGELOG)
   - Uploaded artifacts

### 2. Edit Release Notes

Add or enhance the release notes with:
- **Highlights**: Major features or changes
- **Breaking Changes**: If any (for major versions)
- **Installation Instructions**: Quick start
- **Known Issues**: If any
- **Thank You**: Credit contributors

Example release notes:

```markdown
## whatsmycli v1.0.0 - Initial Stable Release

We're excited to announce the first stable release of whatsmycli! 🎉

### Highlights

- **Unified Interface**: One command (`whatsmy <component>`) for all system information
- **Cross-Platform**: Works on Linux, Windows, and macOS
- **Plugin Architecture**: Extensible via dynamic plugins
- **One-Line Install**: Simple installation scripts for all platforms
- **Comprehensive Docs**: Full contributor guidelines, architecture docs, and API reference

### Installation

**Linux/macOS**:
```bash
curl -sSL https://raw.githubusercontent.com/whatsmycli/whatsmy/main/install.sh | bash
```

**Windows PowerShell**:
```powershell
irm https://raw.githubusercontent.com/whatsmycli/whatsmy/main/install.ps1 | iex
```

Or download binaries below and install manually.

### What's Included

- Core CLI application (`whatsmy`)
- Plugin loading system
- Complete documentation
- Example GPU plugin

### Getting Started

After installation:

```bash
whatsmy help      # Show available commands
whatsmy version   # Show version info
whatsmy gpu       # Display GPU information (requires plugin)
```

### Documentation

- [README](https://github.com/whatsmycli/whatsmy/blob/main/README.md) - Getting started
- [CONTRIBUTING](https://github.com/whatsmycli/whatsmy/blob/main/CONTRIBUTING.md) - How to contribute
- [Architecture](https://github.com/whatsmycli/whatsmy/blob/main/docs/architecture.md) - System design
- [Plugin API](https://github.com/whatsmycli/docs/blob/main/plugin-api.md) - Create plugins

### Known Issues

- Windows binaries require Windows 10 or later
- macOS binaries require macOS 11 (Big Sur) or later
- Some GPU drivers may not be detected on Linux

### What's Next

- Plugin management system (`whatsmy plugin install`)
- More core plugins (CPU, RAM, Network)
- Enhanced output formatting (JSON, YAML)

### Contributors

Thank you to everyone who helped make this release possible!

---

**Full Changelog**: https://github.com/whatsmycli/whatsmy/blob/main/CHANGELOG.md
```

### 3. Verify Artifacts

Check that all required files are attached:
- `whatsmy-linux-x64`
- `whatsmy-windows-x64.exe`
- `whatsmy-macos-universal`
- `checksums.txt`

### 4. Publish the Release

Click **"Publish release"** to make it public.

The release is now live! 🚀

---

## Post-Release Tasks

### 1. Verify Installation Scripts

Test that the installation scripts work with the new release:

**Linux/macOS**:
```bash
# In a clean environment or container
curl -sSL https://raw.githubusercontent.com/whatsmycli/whatsmy/main/install.sh | bash
whatsmy version
```

**Windows**:
```powershell
irm https://raw.githubusercontent.com/whatsmycli/whatsmy/main/install.ps1 | iex
whatsmy version
```

### 2. Update Documentation

Ensure documentation reflects the new version:
- [ ] README.md mentions latest version
- [ ] Installation instructions are correct
- [ ] Links point to correct releases

### 3. Announce the Release

Optionally announce on:
- GitHub Discussions
- Project website/blog
- Social media
- Relevant communities (Reddit, Discord, etc.)

Example announcement:

```
🚀 whatsmycli v1.0.0 is here!

whatsmycli provides a unified, intuitive interface for system information across Linux, Windows, and macOS.

One command to rule them all: `whatsmy <component>`

Install in seconds:
→ Linux/macOS: curl -sSL [url] | bash
→ Windows: irm [url] | iex

Features:
✅ Cross-platform support
✅ Plugin architecture
✅ One-line installation
✅ Open source (GPLv3)

Check it out: https://github.com/whatsmycli/whatsmy
```

### 4. Update Memory Bank (Internal)

Update progress tracking:
- Mark release phase as complete
- Document lessons learned
- Update active context for next phase

---

## Rollback Procedure

If critical issues are discovered after release:

### Option 1: Quick Patch Release

1. Fix the issue in a new branch
2. Merge to main
3. Follow release process for patch version (e.g., `v1.0.1`)
4. Mark previous release as having issues in release notes

### Option 2: Delete Release (Not Recommended)

Only in extreme cases:

```bash
# Delete remote tag
git push origin :refs/tags/v1.0.0

# Delete local tag
git tag -d v1.0.0

# Delete GitHub Release
# (Must be done via GitHub web interface)
```

**⚠️ Warning**: Deleting releases breaks trust with users. Always prefer a patch release.

---

## Troubleshooting

### Build Fails on Specific Platform

1. Check GitHub Actions logs for the specific platform
2. Common issues:
   - Missing dependencies
   - Compiler errors
   - Test failures
3. Fix locally, commit, delete tag, and re-release

### Tag Already Exists

```bash
# Delete local tag
git tag -d v1.0.0

# Delete remote tag (if pushed)
git push origin :refs/tags/v1.0.0

# Create new tag
git tag -a v1.0.0 -m "Release message"
```

### Release Workflow Didn't Trigger

- Verify tag follows pattern: `v*.*.*`
- Check GitHub Actions is enabled
- Review `.github/workflows/release.yml` triggers

### Artifacts Missing from Release

- Check workflow logs for upload errors
- Verify artifact paths in workflow
- Re-run failed jobs if needed

---

## Version History

| Version | Date       | Notes                          |
|---------|------------|--------------------------------|
| 1.0.0   | 2025-10-30 | Initial stable release         |
| 0.1.0   | 2025-10-28 | Internal MVP release           |

---

## See Also

- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
- [CHANGELOG.md](CHANGELOG.md) - Complete change history
- [README.md](README.md) - Project overview

---

**Last Updated**: October 30, 2025

