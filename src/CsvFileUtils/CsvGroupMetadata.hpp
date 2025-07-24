#ifndef __CSVFILEGROUPMETADATA_H__
#define __CSVFILEGROUPMETADATA_H__

#include "CsvFileMetadata.hpp"
#include <string>
#include <vector>

/**
 * @brief Metadata for a group of CSV data files.
 */
struct CsvGroupMetadata {

private:
  /**
   * @brief Path to the parent directory containing the CSV files.
   */
  std::string parentPath_;

  /**
   * @brief Template for the CSV data files in the group.
   */
  std::string dataTemplate_;

  /**
   * @brief List of data paths for each CSV file in the group.
   */
  std::vector<std::string> dataPaths_;

  /**
   * @brief Path to the JSON file for storing the group metadata.
   */
  std::string jsonFilePath_;

  /**
   * @brief Character used to denote comments in the CSV files.
   */
  std::string comment_;

  /**
   * @brief Delimiter used to separate values in the CSV files.
   */
  std::string delimiter_;

  /**
   * @brief If true, allows multiple delimiters in the CSV files.
   */
  bool multiDelimiter_;

  /**
   * @brief If true, the first line is treated as a header line containing
   * column names.
   */
  bool header_;

  /**
   * @brief Column names in the CSV files.
   */
  std::vector<std::string> colNames_;

  /**
   * @brief Total number of lines across all CSV files in the group.
   */
  long size_;

public:
  /**
   * @brief Default constructor for an empty CsvGroupMetadata object.
   * @warning The object will not have any valid paths or metadata until set
   * explicitly.
   */
  CsvGroupMetadata() = default;

  /**
   * @brief Parameterized constructor for CsvGroupMetadata.
   * @param parentPath Path to the parent directory containing the CSV files.
   * @param dataTemplate Template for the CSV data files in the group.
   * @param dataPaths List of data paths for each CSV file in the group.
   * @param jsonFilePath Path to the JSON file for storing the group metadata.
   * @param comment Character used to denote comments in the CSV files.
   * @param delimiter Delimiter used to separate values in the CSV files.
   * @param multiDelimiter If true, allows multiple delimiters in the CSV files.
   * @param header If true, the first line is treated as a header line
   * containing column names.
   * @param colNames Optional vector of column names to use instead of reading
   * from the file.
   * @param size Total number of lines across all CSV files in the group
   * (default is -1, indicating unknown).
   */
  CsvGroupMetadata(std::string parentPath, std::string dataTemplate,
                   std::vector<std::string> dataPaths = {},
                   std::string jsonFilePath = "", std::string comment = "#",
                   std::string delimiter = ",", bool multiDelimiter = false,
                   bool header = true, std::vector<std::string> colNames = {},
                   long size = -1);

  /**
   * @brief Reads metadata from a JSON file and returns a CsvGroupMetadata
   * object.
   * @param jsonFilePath Path to the JSON file to read.
   * @return A CsvGroupMetadata object populated with the data from the JSON
   * file.
   */
  static CsvGroupMetadata readMetadata(std::string jsonFilePath);

  /**
   * @brief Writes the current metadata to a JSON file.
   * @throws std::runtime_error if the file cannot be opened or written.
   */
  void writeToJsonFile();

  /**
   * @brief Returns a string representation of the CsvGroupMetadata object.
   * @return A string containing the parent path, data template, and other
   * metadata.
   */
  std::string toString() const;

  /**
   * @brief Overloaded output stream operator for CsvGroupMetadata.
   * @param os The output stream to write to.
   * @param metadata The CsvGroupMetadata object to print.
   * @return The output stream after writing the metadata.
   */
  friend std::ostream &operator<<(std::ostream &os,
                                  const CsvGroupMetadata &metadata) {
    os << metadata.toString();
    return os;
  }

  // Getters for the metadata fields

  /**
   * @brief Gets the path to the parent directory containing the CSV files.
   * @return The parent path as a string.
   */
  const std::string parentPath() const { return parentPath_; }

  /**
   * @brief Gets the template for the CSV data files in the group.
   * @return The data template as a string.
   */
  const std::string dataTemplate() const { return dataTemplate_; }

  /**
   * @brief Gets the list of metadata for each CSV file in the group.
   * @return A vector of CsvFileMetadata objects.
   */
  // const std::vector<CsvFileMetadata>& csvFiles() const { return csvFiles_; }

  /**
   * @brief Gets the path to the JSON file for storing the group metadata.
   * @return The JSON file path as a string.
   */
  const std::string &jsonFilePath() const { return jsonFilePath_; }

  /**
   * @brief Gets the character used to denote comments in the CSV files.
   * @return The comment character as a string.
   */
  const std::string &comment() const { return comment_; }

  /**
   * @brief Gets the delimiter used to separate values in the CSV files.
   * @return The delimiter character as a string.
   */
  const std::string &delimiter() const { return delimiter_; }

  /**
   * @brief Checks if multiple delimiters are allowed in the CSV files.
   * @return True if multiple delimiters are allowed, false otherwise.
   */
  bool multiDelimiter() const { return multiDelimiter_; }

  /**
   * @brief Checks if the first line is treated as a header line containing
   * column names.
   * @return True if the first line is a header, false otherwise.
   */
  bool header() const { return header_; }

  /**
   * @brief Gets the column names in the CSV files.
   * @return A vector of column names.
   */
  const std::vector<std::string> &colNames() const { return colNames_; }

  /**
   * @brief Gets the total number of lines across all CSV files in the group.
   * @return The total number of lines, or -1 if unknown.
   */
  long size() const { return size_; }

  // Setters for the metadata fields
  /**
   * @brief Sets the path to the parent directory containing the CSV files.
   * @param path The new parent path to set.
   */
  // void setParentPath(const std::string& path) { parentPath_ = path; }

  /**
   * @brief Sets the template for the CSV data files in the group.
   * @param templatePath The new data template to set.
   */
  // void setDataTemplate(const std::string& templatePath) { dataTemplate_ =
  // templatePath; }

  /**
   * @brief Sets the list of data paths for each CSV file in the group.
   * @param dataPaths A vector of strings containing the data paths to set.
   */
  void setDataPaths(const std::vector<std::string> &dataPaths) {
    dataPaths_ = dataPaths;
  }

  /**
   * @brief Sets the path to the JSON file for storing the group metadata.
   * @param path The new JSON file path to set.
   */
  // void setJsonFilePath(const std::string& path) { jsonFilePath_ = path; }

  /**
   * @brief Sets the character used to denote comments in the CSV files.
   * @param comment The new comment character to set.
   */
  // void setComment(const std::string& comment) { comment_ = comment; }

  /**
   * @brief Sets the delimiter used to separate values in the CSV files.
   * @param delimiter The new delimiter character to set.
   */
  // void setDelimiter(const std::string& delimiter) { delimiter_ = delimiter; }

  /**
   * @brief Sets whether multiple delimiters are allowed in the CSV files.
   * @param multiDelimiter True to allow multiple delimiters, false otherwise.
   */
  // void setMultiDelimiter(bool multiDelimiter) { multiDelimiter_ =
  // multiDelimiter; }

  /**
   * @brief Sets whether the first line is treated as a header line containing
   * column names.
   * @param header True if the first line is a header, false otherwise.
   */
  // void setHeader(bool header) { header_ = header; }

  /**
   * @brief Sets the column names in the CSV files.
   * @param colNames A vector of column names to set.
   */
  void setColNames(const std::vector<std::string> &colNames) {
    colNames_ = colNames;
  }

  /**
   * @brief Sets the total number of lines across all CSV files in the group.
   * @param size The new total number of lines to set.
   */
  void setSize(long size) { size_ = size; }
};

#endif // __CSVFILEGROUPMETADATA_H__