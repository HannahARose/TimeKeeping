#include "CsvFileMetadata.hpp"

#include <boost/json.hpp>

#include <fstream>
#include <sstream>

CsvFileMetadata::CsvFileMetadata(std::string dataFilePath,
                                 std::string cacheFilePath,
                                 std::string jsonFilePath, std::string comment,
                                 std::string delimiter, bool multi_delimiter,
                                 bool header,
                                 std::vector<std::string> col_names,
                                 long total_lines, std::size_t cache_size)
    : dataFilePath_(dataFilePath), cacheFilePath_(cacheFilePath),
      jsonFilePath_(jsonFilePath), comment_(comment), delimiter_(delimiter),
      multiDelimiter_(multi_delimiter), header_(header), colNames_(col_names),
      size_(total_lines), cache_size_(cache_size) {
  // Default for cacheFilePath if not provided
  if (cacheFilePath_.empty()) {
    cacheFilePath_ = dataFilePath_ + ".cache";
  }

  // Default for jsonFilePath if not provided
  if (jsonFilePath_.empty()) {
    jsonFilePath_ = dataFilePath_ + ".json";
  }
}

CsvFileMetadata CsvFileMetadata::readMetadata(std::string jsonFilePath) {
  // Read the JSON file and populate the metadata fields

  std::ifstream reader(jsonFilePath);
  if (!reader.is_open()) {
    throw std::runtime_error("Could not open JSON file: " + jsonFilePath);
  }

  boost::json::value jsonValue;
  reader >> jsonValue;
  reader.close();

  auto obj = jsonValue.as_object();

  CsvFileMetadata metadata(
      obj["dataFilePath"].as_string().c_str(),
      obj["cacheFilePath"].as_string().c_str(),
      obj["jsonFilePath"].as_string().c_str(),
      obj["comment"].as_string().c_str(), obj["delimiter"].as_string().c_str(),
      obj["multi_delimiter"].as_bool(), obj["header"].as_bool(),
      boost::json::value_to<std::vector<std::string>>(obj["col_names"]),
      obj["total_lines"].as_int64(), obj["cache_size"].as_int64());

  return metadata;
}

void CsvFileMetadata::writeToJsonFile() {
  // Write the metadata to the JSON file
  boost::json::object obj;

  obj["dataFilePath"] = dataFilePath_;
  obj["cacheFilePath"] = cacheFilePath_;
  obj["jsonFilePath"] = jsonFilePath_;

  obj["comment"] = comment_;
  obj["delimiter"] = delimiter_;
  obj["multi_delimiter"] = multiDelimiter_;

  obj["header"] = header_;
  obj["col_names"] = boost::json::value_from(colNames_);

  obj["total_lines"] = size_;
  obj["cache_size"] = cache_size_;

  std::ofstream writer;
  writer.open(jsonFilePath_);
  writer << boost::json::serialize(obj);
  writer.close();
}

std::string CsvFileMetadata::toString() const {
  std::ostringstream oss;
  oss << "Metadata for CSV File: '" << dataFilePath_ << "'\n"
      << "Cache stored at: '" << cacheFilePath_ << "'\n"
      << "Metadata stored at: '" << jsonFilePath_ << "'\n"
      << "Comment Character(s): '" << comment_ << "'\n"
      << "Delimiter Character(s): '" << delimiter_ << "'\n"
      << "Allows Multiple Delimiters: " << (multiDelimiter_ ? "true" : "false")
      << "\n"
      << "Header Processed: " << (header_ ? "true" : "false") << "\n"
      << "Column Names: ";

  for (const auto &col : colNames_) {
    oss << col << ", ";
  }

  oss << "\nTotal Lines: " << size_;
  oss << "\nCache Size: " << cache_size_ << " bytes\n";

  return oss.str();
}

std::ostream &operator<<(std::ostream &os, const CsvFileMetadata &metadata) {
  os << metadata.toString();
  return os;
}
