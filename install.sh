#!/usr/bin/env bash
# whatsmycli installation script for Linux and macOS
# Copyright (C) 2025 enXov
# Licensed under GPLv3

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
REPO="whatsmycli/whatsmy"
BINARY_NAME="whatsmy"
INSTALL_DIR_SYSTEM="/usr/local/bin"
INSTALL_DIR_USER="$HOME/.local/bin"
PLUGIN_DIR_SYSTEM="/usr/local/lib/whatsmy/plugins"
PLUGIN_DIR_USER="$HOME/.local/share/whatsmy/plugins"

# Functions
print_info() {
    echo -e "${BLUE}ℹ${NC} $1"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

detect_platform() {
    local os=$(uname -s | tr '[:upper:]' '[:lower:]')
    local arch=$(uname -m)
    
    case "$os" in
        linux*)
            PLATFORM="linux"
            ;;
        darwin*)
            PLATFORM="macos"
            ;;
        *)
            print_error "Unsupported operating system: $os"
            exit 1
            ;;
    esac
    
    case "$arch" in
        x86_64|amd64)
            ARCH="x64"
            ;;
        aarch64|arm64)
            if [ "$PLATFORM" = "macos" ]; then
                # macOS universal binary supports both architectures
                ARCH="universal"
            else
                ARCH="arm64"
            fi
            ;;
        *)
            print_error "Unsupported architecture: $arch"
            exit 1
            ;;
    esac
}

check_requirements() {
    # Check for curl or wget
    if command -v curl &> /dev/null; then
        DOWNLOAD_CMD="curl -sSL"
        DOWNLOAD_OUTPUT="-o"
    elif command -v wget &> /dev/null; then
        DOWNLOAD_CMD="wget -q"
        DOWNLOAD_OUTPUT="-O"
    else
        print_error "Neither curl nor wget found. Please install one of them."
        exit 1
    fi
}

determine_install_mode() {
    # Always install to user directory (no root required)
    INSTALL_MODE="user"
    INSTALL_DIR="$INSTALL_DIR_USER"
    PLUGIN_DIR="$PLUGIN_DIR_USER"
    mkdir -p "$INSTALL_DIR"
}

get_latest_version() {
    print_info "Fetching latest release information..."
    
    local api_url="https://api.github.com/repos/$REPO/releases/latest"
    local response
    
    if ! response=$($DOWNLOAD_CMD "$api_url"); then
        print_error "Failed to fetch release information"
        exit 1
    fi
    
    VERSION=$(echo "$response" | grep '"tag_name":' | sed -E 's/.*"v([^"]+)".*/\1/')
    
    if [ -z "$VERSION" ]; then
        print_error "Could not determine latest version"
        exit 1
    fi
    
    print_success "Latest version: v$VERSION"
}

download_and_install() {
    # Construct download URL
    if [ "$PLATFORM" = "macos" ]; then
        ARCHIVE_NAME="whatsmy-macos-universal.tar.gz"
    else
        ARCHIVE_NAME="whatsmy-$PLATFORM-$ARCH.tar.gz"
    fi
    
    DOWNLOAD_URL="https://github.com/$REPO/releases/download/v$VERSION/$ARCHIVE_NAME"
    CHECKSUM_URL="$DOWNLOAD_URL.sha256"
    
    # Create temporary directory
    TMP_DIR=$(mktemp -d)
    trap "rm -rf $TMP_DIR" EXIT
    
    print_info "Downloading $ARCHIVE_NAME..."
    if ! $DOWNLOAD_CMD "$DOWNLOAD_URL" $DOWNLOAD_OUTPUT "$TMP_DIR/$ARCHIVE_NAME"; then
        print_error "Failed to download binary"
        exit 1
    fi
    print_success "Downloaded $ARCHIVE_NAME"
    
    # Download and verify checksum silently
    if ! $DOWNLOAD_CMD "$CHECKSUM_URL" $DOWNLOAD_OUTPUT "$TMP_DIR/checksum.txt"; then
        : # Skip checksum verification if not available
    else
        cd "$TMP_DIR"
        if command -v sha256sum &> /dev/null; then
            if ! echo "$(cat checksum.txt)" | sha256sum -c - &> /dev/null; then
                print_error "Checksum verification failed"
                exit 1
            fi
        elif command -v shasum &> /dev/null; then
            if ! shasum -a 256 -c checksum.txt &> /dev/null; then
                print_error "Checksum verification failed"
                exit 1
            fi
        fi
    fi
    
    # Extract archive silently
    tar -xzf "$TMP_DIR/$ARCHIVE_NAME" -C "$TMP_DIR"
    
    # Find the binary in the extracted archive
    
    # Try multiple possible locations for the binary
    BINARY_SOURCE=""
    
    # Location 1: Standard bin/ directory
    if [ -f "$TMP_DIR/bin/$BINARY_NAME" ]; then
        BINARY_SOURCE="$TMP_DIR/bin/$BINARY_NAME"
    # Location 2: Inside version-named subdirectory (e.g., whatsmy-1.0.0-linux-x64/bin/whatsmy)
    elif [ -f "$TMP_DIR"/whatsmy-*-*/bin/$BINARY_NAME ]; then
        BINARY_SOURCE=$(find "$TMP_DIR" -type f -name "$BINARY_NAME" -path "*/bin/*" | head -n 1)
    # Location 3: Root of archive
    elif [ -f "$TMP_DIR/$BINARY_NAME" ]; then
        BINARY_SOURCE="$TMP_DIR/$BINARY_NAME"
    # Location 4: Recursive search as fallback
    else
        BINARY_SOURCE=$(find "$TMP_DIR" -type f -name "$BINARY_NAME" | head -n 1)
    fi
    
    if [ -z "$BINARY_SOURCE" ] || [ ! -f "$BINARY_SOURCE" ]; then
        print_error "Could not find $BINARY_NAME in archive"
        print_info "Archive contents:"
        find "$TMP_DIR" -type f
        exit 1
    fi
    
    # Install the binary
    install -m 755 "$BINARY_SOURCE" "$INSTALL_DIR/"
    
    # Create plugin directory
    print_info "Creating plugin directory..."
    mkdir -p "$PLUGIN_DIR"
    chmod 755 "$PLUGIN_DIR"
    print_success "Plugin directory created at $PLUGIN_DIR"
}

update_path() {
    # Check if install directory is in PATH
    if echo "$PATH" | grep -q "$INSTALL_DIR"; then
        return
    fi
    
    # Detect which shell config file to update
    local shell_config=""
    local shell_name=$(basename "$SHELL")
    
    case "$shell_name" in
        zsh)
            shell_config="$HOME/.zshrc"
            ;;
        bash)
            shell_config="$HOME/.bashrc"
            ;;
        fish)
            shell_config="$HOME/.config/fish/config.fish"
            ;;
        *)
            shell_config="$HOME/.profile"
            ;;
    esac
    
    # Check if PATH is already in the config file
    if [ -f "$shell_config" ] && grep -q "export PATH=\"$INSTALL_DIR:\$PATH\"" "$shell_config" 2>/dev/null; then
        print_info "You may need to restart your shell or run: source $shell_config"
    else
        # Add PATH to shell config silently
        echo "" >> "$shell_config"
        echo "# Added by whatsmycli installer" >> "$shell_config"
        echo "export PATH=\"$INSTALL_DIR:\$PATH\"" >> "$shell_config"
        print_info "Run 'source $shell_config' or restart your shell to apply changes"
    fi
}

verify_installation() {
    print_info "Verifying installation..."
    
    if command -v "$BINARY_NAME" &> /dev/null; then
        print_success "Installation verified!"
        echo ""
        echo "Run '$BINARY_NAME version' to check the installed version"
        echo "Run '$BINARY_NAME help' to see available commands"
        
        # Show version
        "$BINARY_NAME" version
    else
        print_warning "Binary installed but not found in PATH"
        echo ""
        echo "The installation was successful, but '$BINARY_NAME' is not in your PATH yet."
        echo ""
        if [ "$INSTALL_MODE" = "user" ]; then
            echo "To use whatsmy, either:"
            echo "  1. Restart your shell (recommended)"
            echo "  2. Run: source ~/.bashrc  (or ~/.zshrc, depending on your shell)"
            echo "  3. Or run with full path: $INSTALL_DIR/$BINARY_NAME"
        else
            echo "You may need to restart your shell for PATH changes to take effect."
        fi
        echo ""
        # Exit with success since installation completed successfully
        # The PATH not being updated yet is not an installation failure
    fi
}

main() {
    echo ""
    echo "=================================================="
    echo "  whatsmycli - Installation Script"
    echo "=================================================="
    echo ""
    
    detect_platform
    check_requirements
    determine_install_mode
    get_latest_version
    download_and_install
    update_path
    verify_installation
    
    echo ""
    print_success "Installation complete!"
    echo ""
    echo "Next steps:"
    echo "  • Install plugins from: https://github.com/whatsmycli/plugins"
    echo "  • Documentation: https://github.com/whatsmycli/whatsmy"
    echo ""
}

main "$@"

