// #include "csv.hpp"

// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include <vector>
// #include <stdexcept>
// #include <string>
// #include <DataFrame/DataFrame.h>

// typedef hmdf::StdDataFrame<int> DataFrame;

// DataFrame read_csv(const std::string& filename, char delimiter, char comment_char) {
//     std::ifstream file(filename);
//     if (!file.is_open()) {
//         throw std::runtime_error("Could not open file: " + filename);
//     }

//     std::vector<std::vector<std::string>> rows;
//     std::string line;
//     while (std::getline(file, line)) {
//         if (line.empty() || line[0] == comment_char) {
//             continue; // Skip empty and comment lines
//         }
//         std::stringstream ss(line);
//         std::string cell;
//         std::vector<std::string> row;
//         while (std::getline(ss, cell, delimiter)) {
//             row.push_back(cell);
//         }
//         rows.push_back(row);
//     }

//     file.close();


