# Executables
add_executable(Phaser Phaser.cpp)
add_executable(Timer Timer.cpp)
add_executable(Testing testing.cpp)

# Add subdirectories for other components
add_subdirectory(Utils)

# Link Dependencies
target_link_libraries(Phaser PRIVATE timekeeping_compiler_flags)
target_link_libraries(Phaser PRIVATE Boost::multiprecision)
target_link_libraries(Phaser PRIVATE argparse)

target_include_directories(
  Phaser PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(Timer PRIVATE timekeeping_compiler_flags)
target_link_libraries(Timer PRIVATE Boost::multiprecision)
target_link_libraries(Timer PRIVATE argparse)

target_include_directories(
  Timer PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(Testing PRIVATE timekeeping_compiler_flags)
target_link_libraries(Testing PRIVATE argparse)
target_link_libraries(Testing PRIVATE Timekeeping_Utilities)

target_include_directories(
  Testing PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Install the executables
install(TARGETS Phaser 
    DESTINATION bin
)
install(TARGETS Timer 
    DESTINATION bin
)


# Set the output directory for the executables
set_target_properties(Phaser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
)
set_target_properties(Timer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
)
set_target_properties(Testing PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
)
