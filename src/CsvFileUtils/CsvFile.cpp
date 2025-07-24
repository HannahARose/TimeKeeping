#include "CsvFile.hpp"
#include "CsvFileMetadata.hpp"
#include "LineMapFile.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include <fstream>
#include <sstream>
#include <string>

#include <iostream>

CsvFile::CsvFile(CsvFileMetadata metadata, bool overwriteCache) {
  metadata_ = std::move(metadata);

  // Open the CSV file for reading
  dataFile_.open(metadata_.dataFilePath(), std::ios::in);
  if (!dataFile_.is_open()) {
    throw std::runtime_error("Failed to open input file");
  }

  // Initialize the line map file
  lineMap_.setFilePath(metadata_.cacheFilePath());

  update(overwriteCache);
}

CsvFile::CsvFile(const std::string jsonFilePath, bool overwriteCache) {
  // Read metadata from the JSON file
  metadata_ = CsvFileMetadata::readMetadata(jsonFilePath);

  CsvFile(metadata_, overwriteCache);
}

CsvFile::~CsvFile() {
  if (dataFile_.is_open()) {
    dataFile_.close();
  }
}

bool CsvFile::update(bool overwriteCache) {
  boost::escaped_list_separator<char> separator("\\", metadata_.delimiter(),
                                                "\"");
  boost::tokenizer<boost::escaped_list_separator<char>> tokenizer(
      std::string(""), separator);

  std::streampos pos;
  std::string line;

  std::ofstream lineMapWriter;

  if (overwriteCache) {
    // If we are overwriting the cache, clear the existing line map
    lineMap_.clear();
  }

  // Read the file to locate line locations
  bool header_line = metadata_.header() && lineMap_.size() == 0;

  bool file_updated = false;

  if (!lineMap_.empty()) {
    // Go to the start of the last line read
    dataFile_.seekg(lineMap_.back(), std::ios::beg);
    // Read the next line to continue from after we left off
    std::getline(dataFile_, line);
  } else {
    dataFile_.seekg(
        0, std::ios::beg); // Start from the beginning if lineMap is empty

    // Handle potential Byte Order Mark (BOM) at the start of the file
    if (dataFile_.peek() == 0xEF) {
      dataFile_.ignore(3); // Skip the BOM
    }
  }

  // Read the file line by line
  while (!dataFile_.eof() && !dataFile_.fail() && !dataFile_.bad()) {
    // Get the current position in the file and read the line
    pos = dataFile_.tellg();
    std::getline(dataFile_, line);

    // trim whitespace from the line
    boost::algorithm::trim(line);

    // Skip comment lines or empty lines
    if (line.empty() ||
        metadata_.comment().find(line.front()) != std::string::npos) {
      continue;
    }

    // Tokenize the line using the specified delimiter
    tokenizer =
        boost::tokenizer<boost::escaped_list_separator<char>>(line, separator);

    // Handle the header line if specified
    if (header_line) {
      // If header is specified, read the first line as column names
      if (metadata_.colNames().empty()) {
        for (const auto &token : tokenizer) {
          metadata_.appendColName(token);
        }
      }

      // Update the header line flag
      header_line = false;
      // Mark that the file was updated
      file_updated = true;

      // Skip further processing for the header line
      continue;
    }

    // Now we have a valid line, we can store its position

    // Store the position of the line in the file
    lineMap_.push_back(pos);
    // Update the file_updated flag
    file_updated = true;
  }

  metadata_.setSize(lineMap_.size()); // Update the total lines count

  // Write the metadata to the JSON file
  metadata_.writeToJsonFile();

  return file_updated;
}

std::string CsvFile::getRawLine(long row) {
  // check if the row index is valid
  if (row < 0 || row >= lineMap_.size()) {
    throw std::out_of_range("Row index out of range");
  }

  // Clear any EOF or error flags
  dataFile_.clear();
  // move input file stream to the start of the specified row

  dataFile_.seekg(lineMap_[row], std::ios::beg);

  // Read the line from the file
  std::string line;
  std::getline(dataFile_, line);

  if (dataFile_.fail()) {
    throw std::runtime_error("Failed to read line from file");
  }

  return line;
}

std::map<std::string, std::string> CsvFile::getRow(long row) {
  std::map<std::string, std::string> rowData;

  // get the line from the specified row
  std::string line = getRawLine(row);

  // Set up the tokenizer with the specified delimiter
  boost::escaped_list_separator<char> separator("\\", metadata_.delimiter(),
                                                "\"");
  boost::tokenizer<boost::escaped_list_separator<char>> tokenizer(line,
                                                                  separator);

  auto col_it = metadata_.colNames().begin();
  for (const auto &token : tokenizer) {
    std::string trimmed_token =
        boost::algorithm::trim_copy(token); // Trim whitespace from the token

    // If multi_delimiter is enabled and the token is empty, skip it
    if (metadata_.multiDelimiter() & trimmed_token.empty()) {
      continue;
    }

    // Add the token to the row data map
    if (col_it != metadata_.colNames().end()) {
      rowData[*col_it] = trimmed_token;
      ++col_it;
    }
  }
  return rowData;
}

CsvFile::CsvFile(const CsvFile &other)
    : metadata_(other.metadata_), lineMap_(other.lineMap_) {

  // Open the data file from the other CsvFile object
  dataFile_.open(other.metadata_.dataFilePath());
  if (!dataFile_.is_open()) {
    throw std::runtime_error("Failed to open data file in copy constructor");
  }
}

CsvFile &CsvFile::operator=(const CsvFile &other) {
  if (this != &other) {
    dataFile_.open(other.metadata_.dataFilePath()); // copy the file stream
    if (!dataFile_.is_open()) {
      throw std::runtime_error(
          "Failed to open data file in copy assignment operator");
    }

    metadata_ = other.metadata_;
    lineMap_ = other.lineMap_;
  }

  return *this;
}

CsvFile::CsvFile(CsvFile &&other)
    : metadata_(std::move(other.metadata_)),
      dataFile_(std::move(other.dataFile_)),
      lineMap_(std::move(other.lineMap_)) {}

CsvFile &CsvFile::operator=(CsvFile &&other) {
  if (this != &other) {
    dataFile_ = std::move(other.dataFile_); // Move the file stream
    metadata_ = std::move(other.metadata_);
    lineMap_ = std::move(other.lineMap_);

    // Ensure the moved object is in a valid state
    other.dataFile_.close();
  }
  return *this;
}

bool CsvFile::operator<(const CsvFile &other) const {
  // Compare the file paths of the two CsvFile objects
  return metadata_.dataFilePath() < other.metadata_.dataFilePath();
}

std::string CsvFile::toString() const {
  std::ostringstream oss;
  oss << "CSV File Metadata:\n";
  oss << metadata_ << "\n";
  oss << "\n";
  oss << "Line Map:\n";
  oss << lineMap_ << "\n";
  return oss.str();
}

std::ostream &operator<<(std::ostream &os, const CsvFile &csvFile) {
  os << csvFile.toString();
  return os;
}