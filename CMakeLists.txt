cmake_minimum_required(VERSION 4.0)

# Timekeeping Version
project(Timekeeping VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set the C++ standard
add_library(timekeeping_compiler_flags INTERFACE)
target_compile_features(timekeeping_compiler_flags INTERFACE cxx_std_23)

# Get Git commit hash
execute_process(COMMAND git rev-parse HEAD OUTPUT_VARIABLE GIT_COMMIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get Git branch name
execute_process(COMMAND git rev-parse --abbrev-ref HEAD OUTPUT_VARIABLE GIT_BRANCH_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)

# Add a Configuration Header
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/TimekeepingConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/TimekeepingConfig.h)
# Add the generated header to include directories
include_directories(${CMAKE_CURRENT_BINARY_DIR}) 
install(FILES "${CMAKE_BINARY_DIR}/TimekeepingConfig.h"
    DESTINATION include
)


# External dependencies
include(cmake/CPM.cmake)
include(GNUInstallDirs)
option(BUILD_SHARED_LIBS "Build shared libraries" NO)

# Include directories for Boost
set(BOOST_URL https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.tar.xz)
set(BOOST_URL_SHA256 7da75f171837577a52bbf217e17f8ea576c7c246e4594d617bfde7fafd408be5)

set(BOOST_INCLUDE_LIBRARIES date_time headers multiprecision tokenizer json algorithm interprocess)
CPMAddPackage("gh:HannahARose/boost-cmake#v1.87.0-rc5")

# ArgParse
CPMAddPackage("gh:p-ranav/argparse#v3.2")


# Include source directory
add_subdirectory(src)

# Include tests directory if tests are enabled
option(ENABLE_TESTS "Enable tests" ON)
if(ENABLE_TESTS)
    add_subdirectory(tests)
endif()