cmake_minimum_required(VERSION 3.15)

# Timekeeping Version
project(Timekeeping VERSION 0.1)

# Set the C++ standard
add_library(timekeeping_compiler_flags INTERFACE)
target_compile_features(timekeeping_compiler_flags INTERFACE cxx_std_17)

# Add a Configuration Header
configure_file(TimekeepingConfig.h.in TimekeepingConfig.h)
install(FILES "${CMAKE_BINARY_DIR}/TimekeepingConfig.h"
    DESTINATION include
)

# Include source directory
add_subdirectory(src)

# Include tests directory if tests are enabled
option(ENABLE_TESTS "Enable tests" ON)
if(ENABLE_TESTS)
    add_subdirectory(tests)
endif()