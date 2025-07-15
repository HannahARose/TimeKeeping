cmake_minimum_required(VERSION 4.0)

# Timekeeping Version
project(Timekeeping VERSION 0.1)

# Set the C++ standard
add_library(timekeeping_compiler_flags INTERFACE)
target_compile_features(timekeeping_compiler_flags INTERFACE cxx_std_20)

# Add a Configuration Header
configure_file(TimekeepingConfig.h.in TimekeepingConfig.h)
install(FILES "${CMAKE_BINARY_DIR}/TimekeepingConfig.h"
    DESTINATION include
)


# External dependencies
include(FetchContent)

# Include directories for Boost
set(BOOST_ENABLE_CMAKE ON) # Enable CMake build system for Boost
FetchContent_Declare(
  Boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.88.0/boost-1.88.0-cmake.tar.xz
  URL_MD5 3edffaacd2cfe63c240ef1b99497c74f #  Optional: Verify the downloaded file
)

# Make Boost available to your project
FetchContent_MakeAvailable(Boost)

# ArgParse
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

# Include source directory
add_subdirectory(src)

# Include tests directory if tests are enabled
option(ENABLE_TESTS "Enable tests" ON)
if(ENABLE_TESTS)
    add_subdirectory(tests)
endif()