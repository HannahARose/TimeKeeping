#ifndef __CSVFILEGROUP_H__
#define __CSVFILEGROUP_H__

#include "CsvFile.hpp"
#include "CsvGroupMetadata.hpp"

struct CsvGroup {

private:
  /**
   * @brief Metadata for the group of CSV data files.
   */
  CsvGroupMetadata metadata_;

  /**
   * @brief List of CSV data files in the group.
   */
  std::vector<CsvFile> files_;

  /**
   * @brief List of starting line numbers for each file in the group.
   */
  std::vector<long> startingLineNumbers_;

public:
  /**
   * @brief Construct a new CsvGroup object.
   * @param metadata Metadata for the group of CSV data files.
   * @param ignoreCache Whether to ignore cached data.
   */
  CsvGroup(CsvGroupMetadata metadata, bool ignoreCache = false);

  /**
   * @brief Construct a new CsvGroup object from a JSON file.
   * @param jsonFilePath Path to the JSON file containing metadata.
   * @param ignoreCache Whether to ignore cached data.
   */
  CsvGroup(const std::string jsonFilePath, bool ignoreCache = false);

  /**
   * @brief Destructor for the CsvGroup object.
   */
  ~CsvGroup();

  /**
   * @brief Update the group metadata and files.
   * @param updateCache If true, updates the cached line map.
   * @return true if the group was updated, false otherwise.
   */
  bool update(bool ignoreCache = false);

  /**
   * @brief Get the file index and row number for a specific row in the group.
   * @param row The row number to get (0-based index).
   * @return A pair containing the file index and the row number within that
   * file.
   * @throws std::out_of_range if the row index is out of range.
   */
  std::pair<long, long> getFileIndexAndRow(long row) const;

  /**
   * @brief Reads a specific row from the group of CSV files.
   * @param row The row number to read (0-based index).
   * @return A string containing the raw data of the specified row.
   * @throws std::out_of_range if the row index is out of range.
   * @throws std::runtime_error if there is an error reading the line from the
   * file
   */
  std::string getRawLine(long row);

  /**
   * @brief Reads a specific row from the group of CSV files and returns it as a
   * map of column names to values.
   * @param row The row number to read (0-based index).
   * @return A map where keys are column names and values are the corresponding
   * data for that row.
   * @throws std::out_of_range if the row index is out of range.
   * @throws std::runtime_error if there is an error reading the line from the
   * file
   */
  std::map<std::string, std::string> getRow(long row);

  /**
   * @brief Overloaded operator to access a specific row by index.
   * @param row The row number to access (0-based index).
   * @return A map containing the data for the specified row.
   */
  std::map<std::string, std::string> operator[](long row) {
    return getRow(row);
  }

  /**
   * @brief Converts the group of CSV files to a string representation.
   * @return A string containing a summary of the group contents.
   */
  std::string toString() const;

  /**
   * @brief Overloaded output stream operator for easy printing of CsvGroup.
   * @param os The output stream to write to.
   * @param csvGroup The CsvGroup object to print.
   * @return The output stream after writing the group contents.
   */
  friend std::ostream &operator<<(std::ostream &os, const CsvGroup &csvGroup) {
    os << csvGroup.toString();
    return os;
  }

  // Getters for metadata and files
  /**
   * @brief Get the metadata of the CSV group.
   * @return The metadata of the CSV group.
   */
  const CsvGroupMetadata metadata() const { return metadata_; }

  /**
   * @brief Get the list of CSV files in the group.
   * @return A vector containing the CSV files in the group.
   */
  // const std::vector<CsvFile> &files() const { return files_; }

  /**
   * @brief Get the starting line numbers for each file in the group.
   * @return A vector containing the starting line numbers for each file.
   */
  // const std::vector<long> &startingLineNumbers() const {
  //   return startingLineNumbers_;
  // }
};

#endif // __CSVFILEGROUP_H__