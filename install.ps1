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
    Write-Host "[INFO] " -ForegroundColor Blue -NoNewline
    Write-Host $Message
}

function Write-Success {
    param($Message)
    Write-Host "[OK] " -ForegroundColor Green -NoNewline
    Write-Host $Message
}

function Write-Warning {
    param($Message)
    Write-Host "[WARN] " -ForegroundColor Yellow -NoNewline
    Write-Host $Message
}

function Write-Error {
    param($Message)
    Write-Host "[ERROR] " -ForegroundColor Red -NoNewline
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
    # Always install to user directory (no admin required)
    $script:InstallMode = "user"
    $script:InstallDir = $InstallDirUser
    $script:PluginDir = $PluginDirUser
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
        
        # Download and verify checksum silently
        try {
            $checksumPath = Join-Path $tempDir "checksum.txt"
            Invoke-WebRequest -Uri $checksumUrl -OutFile $checksumPath
            
            $expectedHash = (Get-Content $checksumPath).Split()[0]
            $actualHash = (Get-FileHash -Path $archivePath -Algorithm SHA256).Hash
            
            if ($expectedHash.ToLower() -ne $actualHash.ToLower()) {
                Write-Error "Checksum verification failed"
                exit 1
            }
        }
        catch {
            # Skip checksum verification if not available
        }
        
        # Extract archive silently
        Expand-Archive -Path $archivePath -DestinationPath $tempDir -Force
        
        # Create installation directory
        if (-not (Test-Path $InstallDir)) {
            New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
        }
        
        # Find the binary in the extracted archive
        $binaryDest = Join-Path $InstallDir $BinaryName
        
        # Try multiple possible locations for the binary
        $possiblePaths = @(
            (Join-Path $tempDir "bin\$BinaryName"),                                    # Standard location
            (Join-Path $tempDir $BinaryName),                                          # Root of archive
            (Join-Path $tempDir "Release\$BinaryName"),                                # CMake Release build dir
            (Get-ChildItem -Path $tempDir -Recurse -Filter $BinaryName -File | Select-Object -First 1 -ExpandProperty FullName)  # Recursive search
        )
        
        $binaryFound = $false
        foreach ($path in $possiblePaths) {
            if ($path -and (Test-Path $path)) {
                Copy-Item -Path $path -Destination $binaryDest -Force
                $binaryFound = $true
                break
            }
        }
        
        if (-not $binaryFound) {
            Write-Error "Could not find $BinaryName in archive"
            Write-Info "Archive contents:"
            Get-ChildItem -Path $tempDir -Recurse | ForEach-Object { Write-Host "  $($_.FullName)" }
            exit 1
        }
        
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
        $currentPath = [Environment]::GetEnvironmentVariable("Path", "User")
        $newPath = "$currentPath;$InstallDir"
        
        try {
            [Environment]::SetEnvironmentVariable("Path", $newPath, "User")
            
            # Update current session PATH
            $env:Path = "$env:Path;$InstallDir"
        }
        catch {
            Write-Warning "Could not update PATH automatically"
            Write-Info "Please add $InstallDir to your PATH manually"
        }
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

