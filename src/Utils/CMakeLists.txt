# Attach Library
add_library(Utils STATIC 
    "ProgressBar.cpp"
    )

# Link Dependencies
target_link_libraries(Utils PRIVATE timekeeping_compiler_flags)
target_link_libraries(Utils PUBLIC Boost::date_time)

target_link_directories(Utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})