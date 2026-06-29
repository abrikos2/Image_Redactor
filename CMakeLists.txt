cmake_minimum_required(VERSION 3.16)
project(Image_Redactor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED COMPONENTS core highgui imgproc imgcodecs)
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. Install it with: apt-get install libopencv-dev")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(SOURCES
    main.cpp
    src/Controller.cpp
    src/GUI.cpp
    src/Image.cpp
)

set(HEADERS
    include/Controller.h
    include/GUI.h
    include/Image.h
)

add_executable(Image_Redactor ${SOURCES} ${HEADERS})

target_include_directories(Image_Redactor PRIVATE include)

target_link_libraries(Image_Redactor PRIVATE 
    ${OpenCV_LIBS}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

target_compile_options(Image_Redactor PRIVATE -Wall -Wextra -Wpedantic)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(Image_Redactor_Tests
    tests/main_test.cpp
    src/Image.cpp
)

target_include_directories(Image_Redactor_Tests PRIVATE include)

target_link_libraries(Image_Redactor_Tests PRIVATE 
    ${OpenCV_LIBS}
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(Image_Redactor_Tests)

install(TARGETS Image_Redactor DESTINATION bin)
