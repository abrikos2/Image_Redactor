@echo off
echo Image Redactor - Qt6 Installation and Build Script
echo.

echo This script will:
echo 1. Check if Qt6 is installed
echo 2. Provide instructions for installing Qt6 if needed
echo 3. Build the project with CMake
echo.

echo Checking for Qt6 installation...
echo.

REM Check if Qt environment variables are set
if "%QTDIR%"=="" (
    echo Qt6 not detected in environment variables.
    echo Please install Qt6 from https://www.qt.io/download
    echo After installing, make sure to add Qt to your PATH or set QTDIR variable.
    echo.
    echo Common installation paths:
    echo - C:\Qt\6.x.x\msvc2019_64
    echo - C:\Qt\Tools\QtCreator\bin
    echo.
    echo Then run this script again or build manually with:
    echo cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64"
    echo.
    pause
    exit /b 1
) else (
    echo Qt6 detected at: %QTDIR%
)

echo.
echo Building project with CMake...
echo.

mkdir build
cd build

REM Try to build with default settings first
cmake .. 
if %errorlevel% neq 0 (
    echo CMake configuration failed.
    echo This might be due to missing Qt6 paths.
    echo Please try manually running:
    echo cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64"
    echo.
    pause
    exit /b 1
)

cmake --build .
if %errorlevel% neq 0 (
    echo Build failed.
    pause
    exit /b 1
)

echo.
echo Build successful!
echo Running the application...
cd ..
build\Image_Redactor.exe
pause