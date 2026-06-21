#!/bin/bash

echo "Building Image Redactor..."
mkdir -p build
cd build
cmake ..
if [ $? -ne 0 ]; then
    echo "CMake failed. Please make sure you have OpenCV and Qt installed."
    read -p "Press Enter to continue..."
    exit 1
fi
make
if [ $? -ne 0 ]; then
    echo "Build failed."
    read -p "Press Enter to continue..."
    exit 1
fi

echo "Build successful! Running the application..."
cd ..
./build/Image_Redactor
read -p "Press Enter to continue..."