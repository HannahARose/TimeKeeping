#ifndef __CSVFILEMETADATA_H__
#define __CSVFILEMETADATA_H__

#include <cstddef>
#include <string>
#include <vector>

/**
 * @brief Metadata for a CSV data file.
 */
struct CsvFileMetadata {

private:
  /**
   * @brief Path to the CSV file.
   */
  std::string dataFilePath_;

  /**
   * @brief Path to the cached line map file.
   */
  std::string cacheFilePath_;

  /**
   * @brief Path to the JSON file for storing the data.
   */
  std::string jsonFilePath_;

  /**
   * @brief Character used to denote comments in the CSV file.
   */
  std::string comment_;

  /**
   * @brief Delimiter used to separate values in the CSV file.
   */
  std::string delimiter_;

  /**
   * @brief If true, allows multiple delimiters in the CSV file.
   */
  bool multiDelimiter_;

  /**
   * @brief If true, the first line is treated as a header line containing
   * column names.
   */
  bool header_;

  /**
   * @brief Column names in the CSV file.
   */
  std::vector<std::string> colNames_;

  /**
   * @brief Total number of lines in the CSV file.
   */
  long size_;

  /**
   */
  std::size_t cache_size_;

public:
  /**
   * @brief Default constructor for an empty CsvFileMetadata object.
   * @warning The object will not have any valid paths or metadata until set
   * explicitly.
   */
  CsvFileMetadata() = default;

  /**
   * @brief Constructor to initialize the CsvFileMetadata with the given
   * parameters.
   * @param dataFilePath Path to the CSV file.
   * @param cacheFilePath Path to the cached line map file (defaults to
   * dataFilePath+'.cache').
   * @param jsonFilePath Path to the JSON file for storing the data (defaults to
   * dataFilePath+'.json').
   * @param comment Character used to denote comments in the CSV file (default
   * is '#').
   * @param delimiter Delimiter used to separate values in the CSV file (default
   * is ',').
   * @param multiDelimiter If true, allows multiple delimiters in the CSV file
   * (default is false).
   * @param header If true, the first line is treated as a header line
   * containing column names (default is true).
   * @param colNames Optional vector of column names to use instead of reading
   * from the file (default is empty).
   * @param size Total number of lines in the CSV file (default is -1, meaning
   * unknown).
   */
  CsvFileMetadata(std::string dataFilePath, std::string cacheFilePath = "",
                  std::string jsonFilePath = "", std::string comment = "#",
                  std::string delimiter = ",", bool multiDelimiter = false,
                  bool header = true, std::vector<std::string> colNames = {},
                  long size = -1, std::size_t cache_size = 0);

  /**
   * @brief Static method to read the metadata from an existing JSON file.
   * @param jsonFilePath Path to the JSON file containing the metadata.
   * @note This method will read the JSON file and populate the metadata fields
   * accordingly.
   */
  static CsvFileMetadata readMetadata(std::string jsonFilePath);

  /**
   * @brief Writes the metadata to the JSON file.
   */
  void writeToJsonFile();

  /**
   * @brief Converts the metadata to a string representation.
   * @return A string containing the metadata information.
   */
  std::string toString() const;

  /**
   * @brief Overloaded output stream operator for easy printing of
   * CsvFileMetadata.
   * @param os The output stream to write to.
   * @param metadata The CsvFileMetadata object to print.
   * @return The output stream after writing the metadata.
   */
  friend std::ostream &operator<<(std::ostream &os,
                                  const CsvFileMetadata &metadata);

  // Getters for the metadata fields
  /**
   * @brief Gets the path to the CSV data file.
   * @return The path to the CSV data file.
   */
  const std::string &dataFilePath() const { return dataFilePath_; }

  /**
   * @brief Gets the path to the cached line map file.
   * @return The path to the cached line map file.
   */
  const std::string &cacheFilePath() const { return cacheFilePath_; }

  /**
   * @brief Gets the path to the JSON file for storing the data.
   * @return The path to the JSON file.
   */
  const std::string &jsonFilePath() const { return jsonFilePath_; }

  /**
   * @brief Gets the character used to denote comments in the CSV file.
   * @return The comment character.
   */
  const std::string &comment() const { return comment_; }

  /**
   * @brief Gets the delimiter used to separate values in the CSV file.
   * @return The delimiter character.
   */
  const std::string &delimiter() const { return delimiter_; }

  /**
   * @brief Checks if multiple delimiters are allowed in the CSV file.
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
   * @brief Gets the column names in the CSV file.
   * @return A vector of column names.
   */
  const std::vector<std::string> &colNames() const { return colNames_; }

  /**
   * @brief Gets the total number of lines in the CSV file.
   * @return The total number of lines, or -1 if unknown.
   */
  long size() const { return size_; }

  /**
   * @brief Gets the size of the cache.
   * @return The size of the cache.
   */
  std::size_t cacheSize() const { return cache_size_; }

  // Setters for the metadata fields
  /**
   * @brief Sets the path to the CSV data file.
   * @param path The new path to the CSV data file.
   */
  // void setDataFilePath(const std::string& path) { dataFilePath_ = path; }

  /**
   * @brief Sets the path to the cached line map file.
   * @param path The new path to the cached line map file.
   */
  // void setCacheFilePath(const std::string& path) { cacheFilePath_ = path; }

  /**
   * @brief Sets the path to the JSON file for storing the data.
   * @param path The new path to the JSON file.
   */
  // void setJsonFilePath(const std::string& path) { jsonFilePath_ = path; }

  /**
   * @brief Sets the character used to denote comments in the CSV file.
   * @param comment The new comment character.
   */
  // void setComment(const std::string& comment) { comment_ = comment; }

  /**
   * @brief Sets the delimiter used to separate values in the CSV file.
   * @param delimiter The new delimiter character.
   */
  // void setDelimiter(const std::string& delimiter) { delimiter_ = delimiter; }

  /**
   * @brief Sets whether multiple delimiters are allowed in the CSV file.
   * @param multiDelimiter True if multiple delimiters are allowed, false
   * otherwise.
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
   * @brief Sets the column names in the CSV file.
   * @param colNames A vector of column names to set.
   */
  // void setColNames(const std::vector<std::string>& colNames) { colNames_ =
  // colNames; }

  /**
   * @brief Append the given column name to the list of column names.
   * @param colName The column name to append.
   */
  void appendColName(const std::string &colName) {
    colNames_.push_back(colName);
  }

  /**
   * @brief Sets the total number of lines in the CSV file.
   * @param size The total number of lines to set, or -1 if unknown.
   */
  void setSize(long size) { size_ = size; }

  /**
   * @brief Sets the size of the cache.
   * @param cache_size The new size of the cache.
   */
  void setCacheSize(std::size_t cache_size) { cache_size_ = cache_size; }
};

#endif // __CSVFILEMETADATA_H__