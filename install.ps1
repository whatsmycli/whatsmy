# whatsmycli installation script for Windows
# Copyright (C) 2025 enXov
# Licensed under GPLv3

$ErrorActionPreference = "Stop"

# Configuration
$Repo = "whatsmycli/whatsmy"
$BinaryName = "whatsmy.exe"
$InstallDirSystem = "$env:ProgramFiles\whatsmy"
$InstallDirUser = "$env:LOCALAPPDATA\whatsmy"
$PluginDirSystem = "$env:ProgramFiles\whatsmy\plugins"
$PluginDirUser = "$env:LOCALAPPDATA\whatsmy\plugins"

# Functions
function Write-Info {
    param($Message)
    Write-Host "ℹ " -ForegroundColor Blue -NoNewline
    Write-Host $Message
}

function Write-Success {
    param($Message)
    Write-Host "✓ " -ForegroundColor Green -NoNewline
    Write-Host $Message
}

function Write-Warning {
    param($Message)
    Write-Host "⚠ " -ForegroundColor Yellow -NoNewline
    Write-Host $Message
}

function Write-Error {
    param($Message)
    Write-Host "✗ " -ForegroundColor Red -NoNewline
    Write-Host $Message
}

function Test-Administrator {
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Get-Architecture {
    $arch = (Get-WmiObject Win32_OperatingSystem).OSArchitecture
    if ($arch -match "64") {
        return "x64"
    } else {
        Write-Error "32-bit Windows is not supported"
        exit 1
    }
}

function Get-LatestVersion {
    Write-Info "Fetching latest release information..."
    
    try {
        $apiUrl = "https://api.github.com/repos/$Repo/releases/latest"
        $response = Invoke-RestMethod -Uri $apiUrl -Method Get
        $script:Version = $response.tag_name -replace '^v', ''
        
        Write-Success "Latest version: v$script:Version"
        return $script:Version
    }
    catch {
        Write-Error "Failed to fetch release information: $_"
        exit 1
    }
}

function Select-InstallMode {
    $isAdmin = Test-Administrator
    
    if ($isAdmin) {
        $script:InstallMode = "system"
        $script:InstallDir = $InstallDirSystem
        $script:PluginDir = $PluginDirSystem
        Write-Info "Installing system-wide to $InstallDir"
    }
    else {
        Write-Warning "Not running as Administrator"
        Write-Host ""
        Write-Host "Installation options:"
        Write-Host "  1) Install for current user only (no admin required)"
        Write-Host "  2) Exit and restart as Administrator"
        Write-Host ""
        
        $choice = Read-Host "Choose installation mode [1/2]"
        
        switch ($choice) {
            "1" {
                $script:InstallMode = "user"
                $script:InstallDir = $InstallDirUser
                $script:PluginDir = $PluginDirUser
                Write-Info "Installing for current user to $InstallDir"
            }
            "2" {
                Write-Info "Please restart this script as Administrator"
                exit 0
            }
            default {
                Write-Error "Invalid choice"
                exit 1
            }
        }
    }
}

function Download-AndInstall {
    param($Version)
    
    $arch = Get-Architecture
    $archiveName = "whatsmy-windows-$arch.zip"
    $downloadUrl = "https://github.com/$Repo/releases/download/v$Version/$archiveName"
    $checksumUrl = "$downloadUrl.sha256"
    
    # Create temporary directory
    $tempDir = Join-Path $env:TEMP "whatsmy-install-$(Get-Random)"
    New-Item -ItemType Directory -Path $tempDir | Out-Null
    
    try {
        # Download archive
        Write-Info "Downloading $archiveName..."
        $archivePath = Join-Path $tempDir $archiveName
        Invoke-WebRequest -Uri $downloadUrl -OutFile $archivePath
        Write-Success "Downloaded $archiveName"
        
        # Download and verify checksum
        Write-Info "Verifying checksum..."
        try {
            $checksumPath = Join-Path $tempDir "checksum.txt"
            Invoke-WebRequest -Uri $checksumUrl -OutFile $checksumPath
            
            $expectedHash = (Get-Content $checksumPath).Split()[0]
            $actualHash = (Get-FileHash -Path $archivePath -Algorithm SHA256).Hash
            
            if ($expectedHash.ToLower() -ne $actualHash.ToLower()) {
                Write-Error "Checksum verification failed"
                exit 1
            }
            
            Write-Success "Checksum verified"
        }
        catch {
            Write-Warning "Could not verify checksum: $_"
        }
        
        # Extract archive
        Write-Info "Extracting archive..."
        Expand-Archive -Path $archivePath -DestinationPath $tempDir -Force
        
        # Create installation directory
        Write-Info "Creating installation directory..."
        if (-not (Test-Path $InstallDir)) {
            New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
        }
        
        # Install binary
        Write-Info "Installing binary to $InstallDir..."
        $binarySource = Join-Path $tempDir "bin\$BinaryName"
        $binaryDest = Join-Path $InstallDir $BinaryName
        
        if (Test-Path $binarySource) {
            Copy-Item -Path $binarySource -Destination $binaryDest -Force
        }
        else {
            # Binary might be directly in the archive
            $binarySource = Join-Path $tempDir $BinaryName
            if (Test-Path $binarySource) {
                Copy-Item -Path $binarySource -Destination $binaryDest -Force
            }
            else {
                Write-Error "Could not find $BinaryName in archive"
                exit 1
            }
        }
        
        Write-Success "Binary installed to $binaryDest"
        
        # Create plugin directory
        Write-Info "Creating plugin directory..."
        if (-not (Test-Path $PluginDir)) {
            New-Item -ItemType Directory -Path $PluginDir -Force | Out-Null
        }
        Write-Success "Plugin directory created at $PluginDir"
    }
    finally {
        # Cleanup
        if (Test-Path $tempDir) {
            Remove-Item -Path $tempDir -Recurse -Force
        }
    }
}

function Update-Path {
    # Check if install directory is in PATH
    $currentPath = [Environment]::GetEnvironmentVariable("Path", "User")
    
    if ($currentPath -notlike "*$InstallDir*") {
        Write-Info "Adding $InstallDir to PATH..."
        
        $target = if ($InstallMode -eq "system") { "Machine" } else { "User" }
        $currentPath = [Environment]::GetEnvironmentVariable("Path", $target)
        $newPath = "$currentPath;$InstallDir"
        
        try {
            [Environment]::SetEnvironmentVariable("Path", $newPath, $target)
            Write-Success "Added to PATH"
            
            # Update current session PATH
            $env:Path = "$env:Path;$InstallDir"
        }
        catch {
            Write-Warning "Could not update PATH automatically"
            Write-Info "Please add $InstallDir to your PATH manually"
        }
    }
    else {
        Write-Success "Installation directory is already in PATH"
    }
}

function Test-Installation {
    Write-Info "Verifying installation..."
    
    $binaryPath = Join-Path $InstallDir $BinaryName
    
    if (Test-Path $binaryPath) {
        Write-Success "Installation verified!"
        Write-Host ""
        Write-Host "Run 'whatsmy version' to check the installed version"
        Write-Host "Run 'whatsmy help' to see available commands"
        Write-Host ""
        
        # Show version
        try {
            & $binaryPath version
        }
        catch {
            Write-Warning "Could not run binary. You may need to restart your terminal."
        }
    }
    else {
        Write-Error "Installation verification failed"
        exit 1
    }
}

function Main {
    Write-Host ""
    Write-Host "=================================================="
    Write-Host "  whatsmycli - Installation Script"
    Write-Host "=================================================="
    Write-Host ""
    
    Select-InstallMode
    $version = Get-LatestVersion
    Download-AndInstall -Version $version
    Update-Path
    Test-Installation
    
    Write-Host ""
    Write-Success "Installation complete!"
    Write-Host ""
    Write-Host "Next steps:"
    Write-Host "  • Install plugins from: https://github.com/whatsmycli/plugins"
    Write-Host "  • Documentation: https://github.com/whatsmycli/whatsmy"
    Write-Host ""
    Write-Host "Note: You may need to restart your terminal for PATH changes to take effect."
    Write-Host ""
}

Main

