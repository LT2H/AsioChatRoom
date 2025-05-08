# Works with 3.15 and tested through 4.0
cmake_minimum_required(VERSION 3.15...4.0)

# Project name and a few useful settings. Other commands can pick up the results
project(
  AsioFramework
  VERSION 0.1
  DESCRIPTION "An example project with CMake"
  LANGUAGES CXX)

# FetchContent added in CMake 3.11, downloads during the configure step
# FetchContent_MakeAvailable was added in CMake 3.14; simpler usage
include(FetchContent)

# Asio library
FetchContent_Declare(
    asio
    URL https://downloads.sourceforge.net/project/asio/asio/1.30.2%20%28Stable%29/asio-1.30.2.tar.gz
    URL_HASH SHA256=12e7bb4dada8bc1191de9d550a59ee658ce4e645ffc97c911c099ab4e8699d55
)
FetchContent_MakeAvailable(asio)
# Adds asio

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG        3.3.8  # or latest stable
)
FetchContent_MakeAvailable(glfw)

# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG        master
)
FetchContent_MakeAvailable(imgui)

# Create ImGui backend target
add_library(imgui_backend
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_backend PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_backend PUBLIC glfw OpenGL::GL)
find_package(OpenGL REQUIRED)

# The executable code is here
add_subdirectory(apps)
add_subdirectory(include)

# Detect platform-specific libraries
if(WIN32)
    set(PLATFORM_LIBS
        ws2_32
        mswsock
        kernel32
        user32
        gdi32
        winspool
        shell32
        ole32
        oleaut32
        uuid
        comdlg32
        advapi32)
elseif(UNIX)
    set(PLATFORM_LIBS
        pthread)  # Needed if use std::thread or Asio's io_context multithreading
endif()