#ifndef __LINEMAPFILE_H__
#define __LINEMAPFILE_H__

#include <fstream>
#include <map>
#include <string>

/**
 * @brief Class to manage a line map file for a CSV file.
 *
 * This class provides functionality to read and write line positions in a
 * binary file, allowing for efficient access to specific lines in a CSV file.
 */
struct LineMapFile {
private:
  /**
   * @brief Path to the line map file.
   */
  std::string filePath_;

  /**
   * @brief File stream for reading the line map file.
   */
  std::ifstream lineMapReader_;

  /**
   * @brief File stream for writing the line map file.
   */
  std::ofstream lineMapWriter_;

  /**
   * @brief Indicates whether the line map is evenly spaced.
   */
  bool equalSpaced_;

  /**
   * @brief The first line location in the line map.
   */
  std::streamoff firstLineLoc_;

  /**
   * @brief The spacing between line locations in the line map.
   */
  std::streamoff spacing_;

  /**
   * @brief Cache for line positions to avoid repeated file access.
   */
  std::map<size_t, std::streamoff> lineMapCache_;

public:
  /**
   * @brief Default constructor to initialize the line map file with an empty
   * path.
   */
  // LineMapFile() = default;

  /**
   * @brief Constructor to initialize the LineMapFile with the specified file
   * path.
   * @param filePath The path to the line map file.
   */
  LineMapFile(const std::string &filePath = "");

  /**
   * @brief Destructor to close the file streams.
   */
  ~LineMapFile();

  /**
   * @brief Gets the total number of lines in the line map file.
   */
  size_t size();

  /**
   * @brief Checks if the line map file is empty.
   * @return True if the line map file is empty, false otherwise.
   */
  bool empty();

  /**
   * @brief Checks if the line map is evenly spaced.
   * @param depth The depth to check for equal spacing.
   * @return True if the line map is evenly spaced, false otherwise.
   */
  bool isEqualSpaced(int depth = 10);

  /**
   * @brief Reads the line position for a specific line number.
   * @param lineNumber The line number to read (0-based index).
   * @return The position of the specified line in the file.
   * @throws std::out_of_range if the line number is out of range.
   * @throws std::runtime_error if there is an error reading the file.
   */
  std::streamoff getLinePosition(size_t lineNumber);

  std::streamoff operator[](size_t lineNumber) {
    return getLinePosition(lineNumber);
  }

  /**
   * @brief Retrieves the position of the last line read.
   * @return The position of the last line read.
   */
  std::streamoff back();

  /**
   * @brief Writes the position of a specific line number to the file.
   * @param lineNumber The line number to write (0-based index).
   * @param position The position of the line in the file.
   * @throws std::runtime_error if there is an error writing to the file.
   */
  void writeLinePosition(size_t lineNumber, std::streamoff position);

  /**
   * @brief Appends a new line position to the end of the line map file.
   * @param position The position of the new line in the file.
   * @throws std::runtime_error if there is an error writing to the file.
   */
  void push_back(std::streamoff position);

  /**
   * @brief Clears the line map file, removing all entries.
   */
  void clear();

  std::string toString() const;

  /**
   * @brief Sets the file path for the line map file.
   * @param filePath The new file path to set.
   */
  friend std::ostream &operator<<(std::ostream &os,
                                  const LineMapFile &lineMapFile) {
    os << lineMapFile.toString();
    return os;
  }

  // compatibility with vector of LineMapFile objects

  /**
   * @brief Copy constructor to create a new LineMapFile object from an existing
   * one.
   */
  LineMapFile(const LineMapFile &other);

  /**
   * @brief Move constructor to create a new LineMapFile object by transferring
   * ownership from another LineMapFile object.
   * @param other The LineMapFile object to move from.
   * @return A new LineMapFile object with the moved data.
   */
  LineMapFile(LineMapFile &&other);

  /**
   * @brief Copy assignment operator to assign one LineMapFile object to
   * another.
   * @param other The LineMapFile object to copy from.
   * @return A reference to the current LineMapFile object after assignment.
   */
  LineMapFile &operator=(const LineMapFile &other);

  /**
   * @brief Move assignment operator to assign one LineMapFile object to
   * another by transferring ownership.
   * @param other The LineMapFile object to move from.
   * @return A reference to the current LineMapFile object after assignment.
   */
  LineMapFile &operator=(LineMapFile &&other);

  // getters
  /**
   * @brief Gets the file path of the line map file.
   * @return The file path of the line map file.
   */
  std::string filePath() const { return filePath_; }

  // setters
  /**
   * @brief Sets the file path for the line map file.
   * @param filePath The new file path to set.
   */
  void setFilePath(const std::string &filePath);
};

#endif // __LINEMAPFILE_H__