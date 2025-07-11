# Executables
add_executable(Phaser Phaser.cpp)
target_link_libraries(Phaser PRIVATE timekeeping_compiler_flags)

install(TARGETS Phaser
    DESTINATION bin
)

link_directories("/opt/homebrew/lib")
include_directories("/opt/homebrew/include")

# Set the output directory for the executable
# set_target_properties(Phaser PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
#     RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
#     RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
# )