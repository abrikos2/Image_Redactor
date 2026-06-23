# PowerShell script to help install Qt6 and build Image Redactor

Write-Host "Image Redactor - Qt6 Installation Helper" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Green
Write-Host ""

# Check if we have Qt installed
$qtDir = $env:QTDIR
if ($qtDir) {
    Write-Host "Qt6 detected at: $qtDir" -ForegroundColor Yellow
} else {
    Write-Host "Qt6 not found in environment variables." -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install Qt6 from https://www.qt.io/download" -ForegroundColor Yellow
    Write-Host "After installation, make sure to set QTDIR or add Qt to PATH" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Common installation paths:" -ForegroundColor Yellow
    Write-Host "  C:\Qt\6.x.x\msvc2019_64" -ForegroundColor White
    Write-Host "  C:\Qt\Tools\QtCreator\bin" -ForegroundColor White
    Write-Host ""
    Read-Host "Press Enter to continue after installing Qt6"
}

# Build the project
Write-Host "Building Image Redactor..." -ForegroundColor Green

# Create build directory
if (!(Test-Path "build")) {
    New-Item -ItemType Directory -Name "build"
}
Set-Location "build"

# Try to configure with CMake
Write-Host "Configuring with CMake..." -ForegroundColor Cyan
cmake .. 

if ($LASTEXITCODE -eq 0) {
    Write-Host "CMake configuration successful!" -ForegroundColor Green
    
    # Build the project
    Write-Host "Building project..." -ForegroundColor Cyan
    cmake --build .
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Build successful!" -ForegroundColor Green
        Write-Host ""
        Write-Host "Running the application..." -ForegroundColor Cyan
        Set-Location ".."
        .\build\Image_Redactor.exe
    } else {
        Write-Host "Build failed!" -ForegroundColor Red
    }
} else {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Write-Host ""
    Write-Host "This might be due to missing Qt6 paths." -ForegroundColor Yellow
    Write-Host "Try running CMake manually with explicit path:" -ForegroundColor Yellow
    Write-Host "cmake .. -DCMAKE_PREFIX_PATH=""C:/Qt/6.x.x/msvc2019_64""" -ForegroundColor White
}

Write-Host ""
Read-Host "Press Enter to exit"