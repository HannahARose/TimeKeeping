#ifndef FILE_ACCESS_HPP
#define FILE_ACCESS_HPP

#include <iostream>
#include <fstream>
#include <filesystem>
#include <boost/tokenizer.hpp>


#include <vector>
#include <map>

/*
 * Coordinates random access to a dataset in a CSV file.
 * Read only, accesses the data without ever loading the entire file into memory.
 * This is useful for large datasets where loading the entire file is impractical.
 * The class provides methods to read specific rows and columns from the CSV file.
*/

struct CsvDataFile {

    std::string filePath;
    std::ifstream inputFile;

    std::vector<std::string> col_names;
    std::vector<std::streampos> lineMap;

    std::string delimiter;
    bool multi_delimiter;
    std::string comment;
    bool header;
    bool checkLines;

    std::string currentLine;
    boost::escaped_list_separator<char> separator;
    boost::tokenizer<boost::escaped_list_separator<char> > tokenizer;

    // Constructor to initialize the CSV data file with the given parameters
    // filePath: Path to the CSV file
    // delimiter: Character used to separate values in the CSV file (default is ',')
    // comment: Character used to denote comments in the CSV file (default is '#')
    // header: If true, the first line is treated as a header line containing column names
    // col_names: Optional vector of column names to use instead of reading from the file
    // check_lines: If true, checks if each line has the same number of columns as the header
    CsvDataFile(std::string filePath, 
                std::string delimiter = ",", 
                bool multi_delimiter = false,
                bool check_lines = false,
                std::string comment = "#", 
                bool header = true,
                std::vector<std::string> col_names = {} 
            );
    
    // Update the line map based on any new lines added to the file
    // Returns true if the file was updated, false otherwise
    bool update();

    // Close the file when the object is destroyed
    ~CsvDataFile();

    // Read a specific row from the CSV file
    // row: The row number to read (0-based index)
    // Returns a map of column names to values for the specified row
    std::map<std::string, std::string> readRow(long row);


    // Compatability with vector of CsvDataFile objects
    CsvDataFile();
    // Copy constructor
    CsvDataFile(const CsvDataFile& other);
    CsvDataFile& operator=(const CsvDataFile& other);
    // Coppy assignment
    CsvDataFile(CsvDataFile&& other);
    CsvDataFile& operator=(CsvDataFile&& other);

    // Function to compare two directory entries for sorting
    bool operator<(const CsvDataFile& other) const;
};

struct CsvDataFileTimeGroup {
    std::string parentPath;
    std::string fileTemplate;
    std::vector<CsvDataFile> files;
    std::vector<std::string> col_names;
    std::vector<long> starting_line_numbers;

    std::string delimiter;
    bool multi_delimeter;
    std::string comment;
    bool header;
    bool check_lines;
    

    // Constructor to initialize the time group with a vector of CsvDataFile objects
    // files: Vector of CsvDataFile objects to be grouped together
    CsvDataFileTimeGroup(std::string parentPath,
                         std::string fileTemplate, 
                         std::string delimiter = ",", 
                         bool multi_delimiter = false,
                         bool check_lines = false,
                         std::string comment = "#", 
                         bool header = true,
                         std::vector<std::string> col_names = {});
    
    // Update the file list based on the current directory and file template
    // only refreshes the last file and any new files that match the template
    // Returns true if the file list was updated, false otherwise
    bool update();

    // Deconstructor to clean up the resources
    ~CsvDataFileTimeGroup();

    // Read a specific row from the grouped CSV files
    // row: The row number to read (0-based index)
    // Returns a map of column names to values for the specified row across all files
    std::map<std::string, std::string> readRow(long row);
};

#endif // FILE_ACCESS_HPP