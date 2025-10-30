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
PLUGIN_DIR_USER="$HOME/.local/lib/whatsmy/plugins"

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
    
    print_info "Detected platform: $PLATFORM-$ARCH"
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
    
    print_success "Download tool found: ${DOWNLOAD_CMD%% *}"
}

determine_install_mode() {
    # Check if we can write to system directories
    if [ -w "$INSTALL_DIR_SYSTEM" ] || [ "$(id -u)" -eq 0 ]; then
        INSTALL_MODE="system"
        INSTALL_DIR="$INSTALL_DIR_SYSTEM"
        PLUGIN_DIR="$PLUGIN_DIR_SYSTEM"
        print_info "Installing system-wide to $INSTALL_DIR"
    else
        # Check if running in non-interactive mode (piped from curl/wget)
        if [ ! -t 0 ]; then
            # Non-interactive mode: default to user installation
            print_warning "No write permission to $INSTALL_DIR_SYSTEM"
            print_info "Running in non-interactive mode, defaulting to user installation"
            INSTALL_MODE="user"
            INSTALL_DIR="$INSTALL_DIR_USER"
            PLUGIN_DIR="$PLUGIN_DIR_USER"
            mkdir -p "$INSTALL_DIR"
            print_info "Installing for current user to $INSTALL_DIR"
        else
            # Interactive mode: ask user preference
            print_warning "No write permission to $INSTALL_DIR_SYSTEM"
            echo ""
            echo "Installation options:"
            echo "  1) Install for current user only (no sudo required)"
            echo "  2) Install system-wide (requires sudo)"
            echo ""
            read -p "Choose installation mode [1/2]: " choice
            
            case "$choice" in
                1)
                    INSTALL_MODE="user"
                    INSTALL_DIR="$INSTALL_DIR_USER"
                    PLUGIN_DIR="$PLUGIN_DIR_USER"
                    mkdir -p "$INSTALL_DIR"
                    print_info "Installing for current user to $INSTALL_DIR"
                    ;;
                2)
                    INSTALL_MODE="system"
                    INSTALL_DIR="$INSTALL_DIR_SYSTEM"
                    PLUGIN_DIR="$PLUGIN_DIR_SYSTEM"
                    print_info "Installing system-wide to $INSTALL_DIR (will use sudo)"
                    ;;
                *)
                    print_error "Invalid choice"
                    exit 1
                    ;;
            esac
        fi
    fi
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
    
    # Download and verify checksum
    print_info "Verifying checksum..."
    if ! $DOWNLOAD_CMD "$CHECKSUM_URL" $DOWNLOAD_OUTPUT "$TMP_DIR/checksum.txt"; then
        print_warning "Could not download checksum file, skipping verification"
    else
        cd "$TMP_DIR"
        if command -v sha256sum &> /dev/null; then
            if ! echo "$(cat checksum.txt)" | sha256sum -c -; then
                print_error "Checksum verification failed"
                exit 1
            fi
        elif command -v shasum &> /dev/null; then
            if ! shasum -a 256 -c checksum.txt; then
                print_error "Checksum verification failed"
                exit 1
            fi
        else
            print_warning "No checksum tool found, skipping verification"
        fi
        print_success "Checksum verified"
    fi
    
    # Extract archive
    print_info "Extracting archive..."
    tar -xzf "$TMP_DIR/$ARCHIVE_NAME" -C "$TMP_DIR"
    
    # Install binary
    print_info "Installing binary to $INSTALL_DIR..."
    if [ "$INSTALL_MODE" = "system" ] && [ "$(id -u)" -ne 0 ]; then
        sudo install -m 755 "$TMP_DIR/bin/$BINARY_NAME" "$INSTALL_DIR/"
    else
        install -m 755 "$TMP_DIR/bin/$BINARY_NAME" "$INSTALL_DIR/"
    fi
    print_success "Binary installed to $INSTALL_DIR/$BINARY_NAME"
    
    # Create plugin directory
    print_info "Creating plugin directory..."
    if [ "$INSTALL_MODE" = "system" ] && [ "$(id -u)" -ne 0 ]; then
        sudo mkdir -p "$PLUGIN_DIR"
        sudo chmod 755 "$PLUGIN_DIR"
    else
        mkdir -p "$PLUGIN_DIR"
        chmod 755 "$PLUGIN_DIR"
    fi
    print_success "Plugin directory created at $PLUGIN_DIR"
}

update_path() {
    # Check if install directory is in PATH
    if echo "$PATH" | grep -q "$INSTALL_DIR"; then
        print_success "Installation directory is already in PATH"
        return
    fi
    
    if [ "$INSTALL_MODE" = "user" ]; then
        print_warning "$INSTALL_DIR is not in your PATH"
        echo ""
        echo "Add the following line to your shell configuration file:"
        echo "  (~/.bashrc, ~/.zshrc, ~/.profile, etc.)"
        echo ""
        echo "  export PATH=\"$INSTALL_DIR:\$PATH\""
        echo ""
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
        print_error "Installation verification failed"
        print_info "Binary installed but not found in PATH"
        print_info "You may need to restart your shell or add $INSTALL_DIR to your PATH"
        exit 1
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

