#include "CsvGroup.hpp"
#include "CsvFile.hpp"
#include "CsvFileMetadata.hpp"

#include <algorithm>
#include <iostream>
#include <regex>
#include <sstream>

CsvGroup::CsvGroup(CsvGroupMetadata metadata, bool ignoreCache) {

  this->metadata_ = metadata;
  this->files_ = std::vector<CsvFile>();
  this->startingLineNumbers_ = std::vector<long>();

  update(ignoreCache);
}

CsvGroup::CsvGroup(const std::string jsonFilePath, bool ignoreCache) {
  // Read metadata from the JSON file
  metadata_ = CsvGroupMetadata::readMetadata(jsonFilePath);

  CsvGroup(metadata_, ignoreCache);
}

CsvGroup::~CsvGroup() {
  // Destructor does not need to do anything special
}

bool CsvGroup::update(bool ignoreCache) {

  bool file_updated = false;

  std::vector<std::filesystem::directory_entry> matched_files;
  std::regex file_template_regex(metadata_.dataTemplate());

  // Load all files matching the template into the files vector
  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(metadata_.parentPath())) {
    std::string file_subpath =
        entry.path().lexically_relative(metadata_.parentPath()).string();

    if (std::regex_match(file_subpath, file_template_regex) &&
        entry.is_regular_file()) {
      matched_files.push_back(entry);
    }
  }

  int total_files = matched_files.size();
  int current_file = -1;

  // Update the files vector with the matched files
  for (const auto &entry : matched_files) {
    std::string dataPath = entry.path().string();

    // Check if the file already exists in the files vector
    auto it = std::find_if(files_.begin(), files_.end(),
                           [&dataPath](const CsvFile &file) {
                             return file.metadata().dataFilePath() == dataPath;
                           });

    // If the file is found, update it; otherwise, create a new CsvFile object
    if (it != files_.end()) {
      file_updated |= it->update();
    } else {
      // If the file is not in the list, create a new CsvFile object
      CsvFileMetadata metadata(
          dataPath, "", "", metadata_.comment(), metadata_.delimiter(),
          metadata_.multiDelimiter(), metadata_.header(), metadata_.colNames(),
          -1 // Total lines will be determined later
      );
      CsvFile csv_file(metadata, ignoreCache);
      files_.push_back(csv_file);

      file_updated = true; // Mark that the file list was updated
    }
  }

  // Sort the files vector based on the file path
  std::sort(files_.begin(), files_.end());

  // If col_names is empty, use the first file's column names
  if (metadata_.colNames().empty() && !files_.empty()) {
    metadata_.setColNames(files_[0].metadata().colNames());
  }

  // Ensure all files have the same column names
  if (!files_.empty()) {
    // Ensure all files have the same column names
    for (const auto &file : files_) {
      if (file.metadata().colNames() != metadata_.colNames()) {
        throw std::runtime_error("All files must have the same column names");
      }
    }
  }

  // Update the metadata with the files
  std::vector<std::string> file_path_list;
  for (const auto &file : files_) {
    file_path_list.push_back(file.metadata().dataFilePath());
  }
  metadata_.setDataPaths(file_path_list);

  // Compile list of starting line numbers
  startingLineNumbers_.clear();
  long lines = 0;
  for (int i = 0; i < files_.size(); i++) {
    const auto &file = files_[i];
    startingLineNumbers_.push_back(lines);
    lines += file.metadata().size(); // Get the size from the file metadata
  }

  // Update total lines count
  metadata_.setSize(lines);

  if (file_updated) {
    // Write the updated metadata to the JSON file
    metadata_.writeToJsonFile();
  }

  return file_updated; // Return true if the file list was updated
}

std::pair<long, long> CsvGroup::getFileIndexAndRow(long row) const {
  if (row < 0 || row >= metadata_.size()) {
    throw std::out_of_range("Row index out of range");
  }

  // Find the file index for the given row
  long file_index = 0;
  while (file_index < startingLineNumbers_.size() - 1 &&
         row >= startingLineNumbers_[file_index + 1]) {
    file_index++;
  }

  return {file_index, row - startingLineNumbers_[file_index]};
}

std::string CsvGroup::getRawLine(long row) {
  // Get the file index and row number within that file
  auto [file_index, row_in_file] = getFileIndexAndRow(row);

  // Read the specified row from the determined file
  return files_[file_index].getRawLine(row_in_file);
}

std::map<std::string, std::string> CsvGroup::getRow(long row) {
  // Get the file index and row number within that file
  auto [file_index, row_in_file] = getFileIndexAndRow(row);

  // Read the specified row from the determined file
  CsvFile file = files_[file_index];
  return files_[file_index].getRow(row_in_file);
}

std::string CsvGroup::toString() const {
  std::ostringstream oss;
  oss << metadata_;

  for (const auto &file : files_) {
    oss << "\n" << file.toString();
  }
  return oss.str();
}