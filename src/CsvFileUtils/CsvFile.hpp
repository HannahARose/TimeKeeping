#ifndef __CSVFILE_H__
#define __CSVFILE_H__

#include "CsvFileMetadata.hpp"
#include "LineMapFile.hpp"

#include <fstream>
#include <ios>
#include <map>

/**
 * @brief Random access to a dataset in a single CSV file.
 *
 * Read only, accesses the data without ever loading the entire file into
 * memory. This is useful for large datasets where loading the entire file is
 * impractical. The class provides methods to read specific rows and columns
 * from the CSV file.
 */
struct CsvFile {

private:
  /**
   * @brief Metadata for reading the CSV data file.
   */
  CsvFileMetadata metadata_;

  /**
   * @brief Data file stream for reading the CSV file.
   */
  std::ifstream dataFile_;

  /**
   * @brief Map of line numbers to their positions in the file, relative to the
   * first line.
   */
  LineMapFile lineMap_;

public:
  /**
   * @brief Constructor to initialize the CSV data file with the given metadata.
   * @param metadata The metadata specifying the CSV data file.
   * @param overwriteCache If true, ignores the cache file if it exists.
   */
  CsvFile(CsvFileMetadata metadata, bool overwriteCache = false);

  /**
   * @brief Constructor to initialize reading metadata from the specified json
   * file.
   * @param jsonFilePath The path to the json file containing the metadata.
   * @param overwriteCache If true, ignores the cache file if it exists.
   * @throws std::runtime_error if the file cannot be opened or read.
   */
  CsvFile(const std::string jsonFilePath, bool overwriteCache = false);

  /**
   * @brief Close the file when the object is destroyed.
   */
  ~CsvFile();

  /**
   * @brief Update the line map based on any new lines added to the file.
   * @param overwriteCache If true, deletes the cached line map and rebuilds.
   * @return true if the file was updated, false otherwise.
   * lines on a MacBook Pro M4 Max
   */
  bool update(bool overwriteCache = false);

  /**
   * @brief Reads a specific row from the CSV file.
   * @param row The row number to read (0-based index).
   * @return A string containing the raw data of the specified row.
   * @throws std::out_of_range if the row index is out of range.
   * @throws std::runtime_error if there is an error reading the line from the
   * file
   */
  std::string getRawLine(long row);

  /**
   * @brief Reads a specific row from the CSV file and returns it as a map of
   * column names to values.
   * @param row The row number to read (0-based index).
   * @return A map where keys are column names and values are the corresponding
   * data for that row.
   * @throws std::out_of_range if the row index is out of range.
   * @throws std::runtime_error if there is an error reading the line from the
   * file
   */
  std::map<std::string, std::string> getRow(long row);

  // Compatability with vector of CsvDataFile objects

  /**
   * @brief Default constructor for an empty CsvFile object.
   * @note This constructor initializes an empty CsvFile object without any
   * metadata or file paths.
   */
  CsvFile() = default;

  /**
   * @brief Copy constructor to create a new CsvFile object from an existing
   * one.
   * @param other The CsvFile object to copy from.
   */
  CsvFile(const CsvFile &other);

  /**
   * @brief Copy assignment operator to assign one CsvFile object to another.
   * @param other The CsvFile object to copy from.
   * @return A reference to the current CsvFile object after assignment.
   */
  CsvFile &operator=(const CsvFile &other);

  /**
   * @brief Move constructor to create a new CsvFile object by transferring
   * ownership from another.
   * @param other The CsvFile object to move from.
   */
  CsvFile(CsvFile &&other);

  /**
   * @brief Move assignment operator to assign one CsvFile object to another by
   * transferring ownership.
   * @param other The CsvFile object to move from.
   * @return A reference to the current CsvFile object after assignment.
   */
  CsvFile &operator=(CsvFile &&other);

  /**
   * @brief Comparison operator to compare two CsvFile objects, done
   * alphabetically by file path.
   * @param other The CsvFile object to compare with.
   * @return true if the current object is less than the other, false otherwise.
   */
  bool operator<(const CsvFile &other) const;

  /**
   * @brief Converts the CSV file contents to a string representation.
   * @return A string containing a summary of the CSV file contents.
   */
  std::string toString() const;

  /**
   * @brief Overloaded output stream operator for easy printing of CsvFile.
   * @param os The output stream to write to.
   * @param csvFile The CsvFile object to print.
   * @return The output stream after writing the CSV file contents.
   */
  friend std::ostream &operator<<(std::ostream &os, const CsvFile &csvFile);

  // Getters for the metadata fields
  /**
   * @brief Gets the metadata associated with this CSV file.
   * @return The CsvFileMetadata object containing metadata information.
   */
  const CsvFileMetadata &metadata() const { return metadata_; }
};

#endif // __CSVFILE_H__