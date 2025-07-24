#include "LineMapFile.hpp"

#include <iostream>
#include <sstream>

LineMapFile::LineMapFile(const std::string &filePath) { setFilePath(filePath); }

LineMapFile::~LineMapFile() {
  if (lineMapReader_.is_open()) {
    lineMapReader_.close();
  }
  if (lineMapWriter_.is_open()) {
    lineMapWriter_.close();
  }
}

size_t LineMapFile::size() {
  if (!lineMapReader_.is_open()) {
    throw std::runtime_error(
        "Line map file is not open for reading while getting size");
  }

  lineMapReader_.seekg(0, lineMapReader_.end);
  std::streamoff fileSize = lineMapReader_.tellg();
  return fileSize / sizeof(std::streamoff);
}

bool LineMapFile::empty() { return size() == 0; }

bool LineMapFile::isEqualSpaced(int segments) {
  if (!lineMapReader_.is_open()) {
    return false; // Cannot check if not open
  }

  size_t lineCount = size();

  std::streamoff firstPosition = getLinePosition(0);
  std::streamoff secondPosition = getLinePosition(1);
  std::streamoff spacing = secondPosition - firstPosition;

  size_t segmentLength = lineCount / segments;
  if (segmentLength == 0) {
    segmentLength = 1;
  }
  for (size_t i = 0; i < lineCount; i += segmentLength) {
    if (getLinePosition(i) - firstPosition != i * spacing) {
      return false; // Found a line that is not evenly spaced
    }
  }

  firstLineLoc_ = firstPosition;
  spacing_ = spacing;
  return true; // All checked lines are equally spaced
}

std::streamoff LineMapFile::getLinePosition(size_t lineNumber) {
  if (equalSpaced_) {
    // If the line map is evenly spaced, calculate the position directly
    return firstLineLoc_ + lineNumber * spacing_;
  }

  if (lineMapCache_.find(lineNumber) != lineMapCache_.end()) {
    return lineMapCache_[lineNumber];
  }

  if (!lineMapReader_.is_open()) {
    throw std::runtime_error(
        "Line map file is not open for reading while getting line position");
  }

  size_t lineCount = size();

  // Calculate the position of the specified line
  if (lineNumber >= lineCount) {
    throw std::out_of_range("Line number is out of range");
  }
  std::streamoff linePosition = lineNumber * sizeof(std::streamoff);

  lineMapReader_.seekg(linePosition);
  std::streamoff position;
  lineMapReader_.read(reinterpret_cast<char *>(&position),
                      sizeof(std::streamoff));

  // Add the position to the cache
  lineMapCache_[lineNumber] = position;
  return position;
}

std::streamoff LineMapFile::back() {
  size_t lineCount = size();
  if (lineCount == 0) {
    throw std::out_of_range(
        "No lines in the line map file when accessing back");
  }
  return getLinePosition(lineCount - 1);
}

void LineMapFile::writeLinePosition(size_t lineNumber,
                                    std::streamoff position) {
  if (!lineMapWriter_.is_open()) {
    throw std::runtime_error(
        "Line map file is not open for writing while writing line position");
  }

  size_t lineCount = size();

  // Calculate the position of the specified line
  if (lineNumber > lineCount) {
    throw std::out_of_range(
        "Line number is out of range while writing position");
  }
  std::streamoff linePosition = lineNumber * sizeof(std::streamoff);

  lineMapWriter_.seekp(linePosition);
  lineMapWriter_.write(reinterpret_cast<const char *>(&position),
                       sizeof(std::streamoff));
}

void LineMapFile::push_back(std::streamoff position) {
  if (!lineMapWriter_.is_open()) {
    throw std::runtime_error(
        "Line map file is not open for writing while pushing back");
  }

  // Write the position to the end of the file
  lineMapWriter_.seekp(0, std::ios::end);
  lineMapWriter_.write(reinterpret_cast<const char *>(&position),
                       sizeof(std::streamoff));
}

void LineMapFile::clear() {
  if (lineMapWriter_.is_open()) {
    lineMapWriter_.close();
  }
  lineMapWriter_.open(filePath_, std::ios::binary | std::ios::trunc);
  if (!lineMapWriter_.is_open()) {
    throw std::runtime_error(
        "Failed to open line map file for writing while clearing");
  }
  // Clear the cache
  lineMapCache_.clear();
}

std::string LineMapFile::toString() const {
  std::ostringstream oss;
  oss << "Line Map File: " << filePath_ << "\n";
  oss << lineMapCache_.size() << " cached positions:\n";
  return oss.str();
}

void LineMapFile::setFilePath(const std::string &filePath) {
  filePath_ = filePath;
  if (!filePath.empty()) {
    if (lineMapWriter_.is_open()) {
      lineMapWriter_.close();
    }
    lineMapWriter_.open(filePath_, std::ios::binary | std::ios::app);
    if (!lineMapWriter_.is_open()) {
      throw std::runtime_error("Failed to open line map file for writing while "
                               "setting file path to " +
                               filePath_);
    }

    if (lineMapReader_.is_open()) {
      lineMapReader_.close();
    }
    lineMapReader_.open(filePath_, std::ios::binary);
    if (!lineMapReader_.is_open()) {
      throw std::runtime_error("Failed to open line map file for reading while "
                               "setting file path to " +
                               filePath_);
    }
  }
  lineMapCache_.clear();
  equalSpaced_ = isEqualSpaced(100);
}

LineMapFile::LineMapFile(const LineMapFile &other)
    : filePath_(other.filePath_), lineMapCache_(other.lineMapCache_),
      equalSpaced_(other.equalSpaced_), firstLineLoc_(other.firstLineLoc_),
      spacing_(other.spacing_) {
  // Open the line map file for reading
  lineMapReader_.open(filePath_, std::ios::binary);
  if (!lineMapReader_.is_open()) {
    throw std::runtime_error("Failed to open line map file for reading in copy "
                             "constructor");
  }

  // Open the line map file for writing
  lineMapWriter_.open(filePath_, std::ios::binary | std::ios::app);
  if (!lineMapWriter_.is_open()) {
    throw std::runtime_error("Failed to open line map file for writing in copy "
                             "constructor");
  }
}

LineMapFile::LineMapFile(LineMapFile &&other)
    : filePath_(std::move(other.filePath_)),
      lineMapCache_(std::move(other.lineMapCache_)),
      equalSpaced_(other.equalSpaced_), firstLineLoc_(other.firstLineLoc_),
      spacing_(other.spacing_) {
  // Move the file streams
  lineMapReader_ = std::move(other.lineMapReader_);
  lineMapWriter_ = std::move(other.lineMapWriter_);
}

LineMapFile &LineMapFile::operator=(const LineMapFile &other) {
  if (this != &other) {
    filePath_ = other.filePath_;
    lineMapCache_ = other.lineMapCache_;
    equalSpaced_ = other.equalSpaced_;
    firstLineLoc_ = other.firstLineLoc_;
    spacing_ = other.spacing_;

    // Reopen the file streams
    lineMapReader_.close();
    lineMapReader_.open(filePath_, std::ios::binary);
    if (!lineMapReader_.is_open()) {
      throw std::runtime_error("Failed to open line map file for reading in "
                               "assignment operator");
    }

    lineMapWriter_.close();
    lineMapWriter_.open(filePath_, std::ios::binary | std::ios::app);
    if (!lineMapWriter_.is_open()) {
      throw std::runtime_error("Failed to open line map file for writing in "
                               "assignment operator");
    }
  }
  return *this;
}

LineMapFile &LineMapFile::operator=(LineMapFile &&other) {
  if (this != &other) {
    filePath_ = std::move(other.filePath_);
    lineMapCache_ = std::move(other.lineMapCache_);
    equalSpaced_ = other.equalSpaced_;
    firstLineLoc_ = other.firstLineLoc_;
    spacing_ = other.spacing_;

    // Move the file streams
    lineMapReader_ = std::move(other.lineMapReader_);
    lineMapWriter_ = std::move(other.lineMapWriter_);

    // Ensure the moved object is in a valid state
    other.lineMapReader_.close();
    other.lineMapWriter_.close();
  }
  return *this;
}