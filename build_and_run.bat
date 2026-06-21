@echo off
echo Building Image Redactor...
mkdir build
cd build
cmake ..
if %errorlevel% neq 0 (
    echo CMake failed. Please make sure you have OpenCV and Qt installed.
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
echo Build successful! Running the application...
cd ..
build\Image_Redactor.exe
pause