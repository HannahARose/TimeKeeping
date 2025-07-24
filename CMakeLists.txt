cmake_minimum_required(VERSION 4.0)

# Timekeeping Version
project(Timekeeping VERSION 0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set the C++ standard
add_library(timekeeping_compiler_flags INTERFACE)
target_compile_features(timekeeping_compiler_flags INTERFACE cxx_std_23)

# Add a Configuration Header
configure_file(TimekeepingConfig.h.in TimekeepingConfig.h)
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