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

# The executable code is here
add_subdirectory(apps)
add_subdirectory(include)