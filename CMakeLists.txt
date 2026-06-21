cmake_minimum_required(VERSION 3.16)
project(Image_Redactor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV 4.x
find_package(OpenCV REQUIRED COMPONENTS core highgui imgproc imgcodecs)
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. Install it with: apt-get install libopencv-dev")
endif()

# Collect source files
set(SOURCES
    main.cpp
    src/Controller.cpp
    src/GUI.cpp
    src/Image.cpp
)

# Collect header files (for IDE integration)
set(HEADERS
    include/Controller.h
    include/GUI.h
    include/Image.h
)

# Create executable
add_executable(Image_Redactor ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(Image_Redactor PRIVATE include)

# Link OpenCV libraries
target_link_libraries(Image_Redactor PRIVATE ${OpenCV_LIBS})

# Compiler warnings
target_compile_options(Image_Redactor PRIVATE -Wall -Wextra -Wpedantic)

# Install target (optional)
install(TARGETS Image_Redactor DESTINATION bin)
