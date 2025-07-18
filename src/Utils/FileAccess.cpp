#include "FileAccess.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <boost/tokenizer.hpp>


#include <vector>
#include <map>

// Constructor to initialize the CSV data file with the given parameters
// filePath: Path to the CSV file
// delimiter: Character used to separate values in the CSV file (default is ',')
// comment: Character used to denote comments in the CSV file (default is '#')
// header: If true, the first line is treated as a header line containing column names
// col_names: Optional vector of column names to use instead of reading from the file
// check_lines: If true, checks if each line has the same number of columns as the header
// 
// Performance Benchmark: 7.7 seconds for 1.5 GB file, not checking lines on a MacBook Pro M4 Max
CsvDataFile::CsvDataFile(std::string filePath, 
            std::string delimiter, 
            bool multi_delimiter,
            bool check_lines,
            std::string comment, 
            bool header,
            std::vector<std::string> col_names
        ) : tokenizer(std::string(""), boost::escaped_list_separator<char>()) {

    // Open the CSV file for reading
    this->filePath = filePath;
    inputFile.open(filePath);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Failed to open input file");
    }

    this->delimiter = delimiter;
    this->multi_delimiter = multi_delimiter;
    this->separator = boost::escaped_list_separator<char>("\\", delimiter, "\"");
    this->comment = comment;
    this->checkLines = check_lines;
    if (!col_names.empty()) {
        this->col_names = col_names;
    }
    this->header = header;

    update(); // Read the file to populate lineMap and col_names
}

bool CsvDataFile::update() {
    // Read the file to locate line locations
    bool header_line = header && lineMap.empty();
    std::streampos pos;
    std::string line;
    bool file_updated = false;

    if (!lineMap.empty()) {
        // Go to the start of the last line read
        inputFile.seekg(lineMap.back());
        std::getline(inputFile, line); // Clear the input buffer
    } else {
        inputFile.seekg(0); // Start from the beginning if lineMap is empty
    }

    while (! inputFile.eof() && !inputFile.fail() && !inputFile.bad()) {
        pos = inputFile.tellg();
        std::getline(inputFile, line);
        
        // Skip comment lines or empty lines
        if (line.empty() || comment.find(line[0]) != std::string::npos) {
            continue;
        }

        tokenizer = boost::tokenizer<boost::escaped_list_separator<char> >(line, separator);

        if (header_line) {
            // If header is specified, read the first line as column names
            this->col_names.clear();
            for (const auto& token : tokenizer) {
                this->col_names.push_back(token);
            }
            header_line = false;
            file_updated = true; // Mark that the file was updated
            continue;
        }

        // Store the position of the line in the file
        lineMap.push_back(pos);
        file_updated = true; // Mark that the file was updated

        // Check if the line has the same number of columns as the header
        if (checkLines) {
            if (this->col_names.empty()) {
                throw std::runtime_error("Header is not defined, cannot check line consistency");
            }
            int col_count = 0;
            for (const auto& token : tokenizer) {
                // Do nothing, just iterate to count tokens
                col_count++;
            }
            if (col_count != this->col_names.size()) {
                throw std::runtime_error("Line has different number of columns than header");
            }
        }
    }

    return file_updated; // Return true if the file was updated
}


// Close the file when the object is destroyed
CsvDataFile::~CsvDataFile() {
    if (inputFile.is_open()) {
            inputFile.close();
    }
}


// Read a specific row from the CSV file
// row: The row number to read (0-based index)
// Returns a map of column names to values for the specified row
std::map<std::string, std::string> CsvDataFile::readRow(long row) {

    // check if the row index is valid
    if (row < 0 || row >= lineMap.size()) {
        throw std::out_of_range("Row index out of range");
    }

    // Get the line corresponding to the row index
    std::map<std::string, std::string> rowData;
    inputFile.clear(); // Clear any EOF or error flags
    inputFile.seekg(lineMap[row]); // Move to the start of the specified line
    std::getline(inputFile, currentLine);

    // Tokenize the current line and populate the rowData map
    tokenizer = boost::tokenizer<boost::escaped_list_separator<char> >(currentLine, separator);
    auto col_it = col_names.begin();
    for (const auto& token : tokenizer) {
        if (multi_delimiter & token.empty()) {
            continue; // Skip empty tokens
        }
        if (col_it != col_names.end()) {
            rowData[*col_it] = token;
            ++col_it;
        }
    }
    return rowData;
}

// Compatability with vector of CsvDataFile objects
// Default constructor initializes with default values
CsvDataFile::CsvDataFile() : tokenizer(std::string(""), boost::escaped_list_separator<char>()) {
    // Default constructor initializes with default values
}
// Copy constructor
CsvDataFile::CsvDataFile(const CsvDataFile& other) 
    : filePath(other.filePath),
    col_names(other.col_names), lineMap(other.lineMap), 
    delimiter(other.delimiter), multi_delimiter(other.multi_delimiter), 
    comment(other.comment), header(other.header), checkLines(other.checkLines),
    currentLine(other.currentLine), separator(other.separator), tokenizer(other.tokenizer) {
    inputFile.open(other.filePath); // copy the file stream
    if (!inputFile.is_open()) {
        throw std::runtime_error("Failed to open input file in copy constructor");
    }
}
CsvDataFile& CsvDataFile::operator=(const CsvDataFile& other) {
    if (this != &other) {
        inputFile.open(other.filePath); // copy the file stream
        if (!inputFile.is_open()) {
            throw std::runtime_error("Failed to open input file in copy constructor");
        }
        filePath = other.filePath;
        col_names = other.col_names;
        lineMap = other.lineMap;
        delimiter = other.delimiter;
        multi_delimiter = other.multi_delimiter;
        comment = other.comment;
        header = other.header;
        checkLines = other.checkLines;
        currentLine = other.currentLine;
        separator = other.separator;
        tokenizer = other.tokenizer;
    }
    return *this;
}
// Move constructor
CsvDataFile::CsvDataFile(CsvDataFile&& other)
    : filePath(other.filePath), inputFile(std::move(other.inputFile)), col_names(std::move(other.col_names)), 
      lineMap(std::move(other.lineMap)), delimiter(other.delimiter), multi_delimiter(other.multi_delimiter), 
      comment(other.comment), header(other.header), checkLines(other.checkLines), currentLine(std::move(other.currentLine)),
      separator(std::move(other.separator)), tokenizer(std::move(other.tokenizer)) {
    // Move constructor transfers ownership of resources
}
CsvDataFile& CsvDataFile::operator=(CsvDataFile&& other) {
    if (this != &other) {
        filePath = std::move(other.filePath);
        inputFile = std::move(other.inputFile);
        col_names = std::move(other.col_names);
        lineMap = std::move(other.lineMap);
        delimiter = other.delimiter;
        multi_delimiter = other.multi_delimiter;
        comment = other.comment;
        header = other.header;
        checkLines = other.checkLines;
        currentLine = std::move(other.currentLine);
        separator = std::move(other.separator);
        tokenizer = std::move(other.tokenizer);
    }
    return *this;
}

// Function to compare two directory entries for sorting
bool CsvDataFile::operator<(const CsvDataFile& other) const {
    return filePath < other.filePath; // Compare based on file path
}

// Constructor to initialize the time group with a vector of CsvDataFile objects
// files: Vector of CsvDataFile objects to be grouped together
// 
// Performance Benchmark: 7.2 seconds for 1 GB across 9 files, not checking lines on a MacBook Pro M4 Max
CsvDataFileTimeGroup::CsvDataFileTimeGroup(std::string parentPath, 
    std::string fileTemplate,
    std::string delimiter, 
    bool multi_delimiter,
    bool check_lines,
    std::string comment, 
    bool header,
    std::vector<std::string> col_names) {

    this->parentPath = parentPath;
    this->fileTemplate = fileTemplate;

    this->delimiter = delimiter;
    this->multi_delimeter = multi_delimiter;
    this->comment = comment;
    this->check_lines = check_lines;
    this->header = header;
    this->col_names = col_names;
    this->files.clear();
    this->starting_line_numbers.clear();

    update();
}

// Update the file list based on the current directory and file template
// only refreshes the last file and any new files that match the template
// Returns true if the file list was updated, false otherwise
bool CsvDataFileTimeGroup::update() {
    bool file_updated = false;
    std::vector<std::filesystem::directory_entry> matched_files;
    std::regex file_template_regex(fileTemplate);

    // Load all files matching the template into the files vector
    for (const auto& entry : std::filesystem::recursive_directory_iterator(parentPath)) {
        std::string file_subpath = entry.path().lexically_relative(parentPath).string();

        if (std::regex_match(file_subpath, file_template_regex) && entry.is_regular_file()) {
            matched_files.push_back(entry);
        }
    }

    for (const auto& entry : matched_files) {
        std::string file_path = entry.path().string();

        // Check if the file already exists in the files vector
        auto it = std::find_if(files.begin(), files.end(),
            [&file_path](const CsvDataFile& file) {
                return file.filePath == file_path;
            });

        if (it != files.end()) {
            file_updated |= it->update(); // Update the existing file
        } else {
            // If the file is not in the list, create a new CsvDataFile object
            try {
                CsvDataFile csv_file(file_path, delimiter, multi_delimeter, check_lines, comment, header, col_names);
                files.push_back(csv_file);
            } catch (const std::exception& e) {
                std::cerr << "Error reading file " << file_path << ": " << e.what() << std::endl;
            }
            file_updated = true; // Mark that the file list was updated
        }
    }

    // Sort the files vector based on the file path
    std::sort(files.begin(), files.end());

    // If col_names is empty, use the first file's column names
    if (this->col_names.empty() && !files.empty()) {
        this->col_names = files[0].col_names;
    } 
    if (!files.empty()) {
        // Ensure all files have the same column names
        for (const auto& file : files) {
            if (file.col_names != this->col_names) {
                throw std::runtime_error("All files must have the same column names");
            }
        }
    }

    // Compile list of starting line numbers
    starting_line_numbers.clear();
    long total_lines = 0;
    for (int i = 0; i < files.size(); i++) {
        const auto& file = files[i];
        starting_line_numbers.push_back(total_lines);
        total_lines += file.lineMap.size();
    }

    return file_updated; // Return true if the file list was updated
}

// Deconstructor to clean up the resources
CsvDataFileTimeGroup::~CsvDataFileTimeGroup() {
    // No explicit cleanup needed as CsvDataFile's destructor will handle file closing
}

// Read a specific row from the grouped CSV files
// row: The row number to read (0-based index)
// Returns a map of column names to values for the specified row across all files
std::map<std::string, std::string> CsvDataFileTimeGroup::readRow(long row) {
    std::map<std::string, std::string> rowData;

    // Determine which file the row belongs to
    long file_index = starting_line_numbers.size() - 1; // Default to the last file
    for (size_t i = 0; i < starting_line_numbers.size(); i++) {
        if (row < starting_line_numbers[i]) {
            file_index = i - 1;
            break;
        }
    }

    if (file_index < 0 || file_index >= files.size()) {
        throw std::out_of_range("Row index out of range");
    }

    // Read the specified row from the determined file
    rowData = files[file_index].readRow(row - starting_line_numbers[file_index]);

    return rowData;
}