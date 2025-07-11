# Attach Library

add_library(Timekeeping_Utilities "csv.cpp")

target_link_directories(Timekeeping_Utilities INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})