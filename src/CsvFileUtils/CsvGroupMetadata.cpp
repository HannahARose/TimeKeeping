#include "CsvGroupMetadata.hpp"

#include <boost/json.hpp>
#include <fstream>
#include <sstream>

CsvGroupMetadata::CsvGroupMetadata(
    std::string parentPath,
    std::string dataTemplate,
    std::vector<std::string> dataPaths,
    std::string jsonFilePath,
    std::string comment,
    std::string delimiter,
    bool multiDelimiter,
    bool header,
    const std::vector<std::string> colNames,
    long size
) : parentPath_(parentPath),
    dataTemplate_(dataTemplate),
    dataPaths_(std::move(dataPaths)),
    jsonFilePath_(jsonFilePath),
    comment_(comment),
    delimiter_(delimiter),
    multiDelimiter_(multiDelimiter),
    header_(header),
    colNames_(colNames),
    size_(size) 
{
    // Default for jsonFilePath if not provided
    if (jsonFilePath_.empty()) {
        jsonFilePath_ = parentPath_ + "/group_metadata.json";
    }
}


CsvGroupMetadata CsvGroupMetadata::readMetadata(std::string jsonFilePath) {
    // Read the JSON file and populate the metadata fields

    std::ifstream reader(jsonFilePath);
    if (!reader.is_open()) {
        throw std::runtime_error("Could not open JSON file: " + jsonFilePath);
    }

    boost::json::value jsonValue;
    reader >> jsonValue;
    reader.close();

    auto obj = jsonValue.as_object();

    CsvGroupMetadata metadata(
        obj["parentPath"].as_string().c_str(),
        obj["dataTemplate"].as_string().c_str(),
        boost::json::value_to<std::vector<std::string>>(obj["dataPaths"]),
        obj["jsonFilePath"].as_string().c_str(),
        obj["comment"].as_string().c_str(),
        obj["delimiter"].as_string().c_str(),
        obj["multi_delimiter"].as_bool(),
        obj["header"].as_bool(),
        boost::json::value_to<std::vector<std::string>>(obj["colNames"]),
        obj["total_lines"].as_int64()
    );

    return metadata;
}


void CsvGroupMetadata::writeToJsonFile() {
    // Write the metadata to a JSON file
    boost::json::object obj;

    obj["parentPath"] = parentPath_;
    obj["dataTemplate"] = dataTemplate_;
    obj["dataPaths"] = boost::json::value_from(dataPaths_);
    obj["jsonFilePath"] = jsonFilePath_;

    obj["comment"] = comment_;
    obj["delimiter"] = delimiter_;
    obj["multi_delimiter"] = multiDelimiter_;

    obj["header"] = header_;
    obj["colNames"] = boost::json::value_from(colNames_);

    obj["total_lines"] = size_;

    std::ofstream writer;
    writer.open(jsonFilePath_);
    writer << boost::json::serialize(obj);
    writer.close();
}


std::string CsvGroupMetadata::toString() const {
    std::ostringstream oss;
    oss << "Data Template: " << dataTemplate_ << "\n"
        << "Files Found: " << dataPaths_.size() << "\n"
        << parentPath_ << "\n";
    for (const auto& path : dataPaths_) {
        oss << "$/" << path << "\n";
    }

    oss << "\nJSON File Path: " << jsonFilePath_ << "\n"
        << "Comment: " << comment_ << "\n"
        << "Delimiter: " << delimiter_ << "\n"
        << "Multi Delimiter: " << (multiDelimiter_ ? "true" : "false") << "\n"
        << "Header: " << (header_ ? "true" : "false") << "\n"
        << "Column Names: ";
    for (const auto& colName : colNames_) {
        oss << colName << " ";
    }
    oss << "\nTotal Lines: " << size_;
    return oss.str();
}