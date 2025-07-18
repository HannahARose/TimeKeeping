# Attach Library
add_library(Timekeeping_Utilities "FileAccess.cpp")

# Link Dependencies
target_link_libraries(Timekeeping_Utilities PRIVATE timekeeping_compiler_flags)
target_link_libraries(Timekeeping_Utilities PRIVATE Boost::filesystem)
target_link_libraries(Timekeeping_Utilities PUBLIC Boost::tokenizer)
target_link_libraries(Timekeeping_Utilities PRIVATE argparse)

target_link_directories(Timekeeping_Utilities INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})