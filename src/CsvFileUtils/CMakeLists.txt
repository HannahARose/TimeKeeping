# Attach Library
add_library(CsvFileUtils STATIC 
    "CsvFile.cpp" 
    "CsvGroup.cpp" 
    "CsvFileMetadata.cpp" 
    "CsvGroupMetadata.cpp" 
    "LineMapFile.cpp" 
    "CsvTimeGroup.cpp"
    )

# Link Dependencies
target_link_libraries(CsvFileUtils PRIVATE timekeeping_compiler_flags)
target_link_libraries(CsvFileUtils PUBLIC Boost::tokenizer Boost::json Boost::algorithm Boost::multiprecision Boost::date_time)

target_link_directories(CsvFileUtils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})