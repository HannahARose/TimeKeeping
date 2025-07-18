#include <iostream>
#include <ranges>
#include "Utils/FileAccess.hpp"


void printFileData(const CsvDataFileTimeGroup &csvFile) {
    for (const auto& colName : csvFile.col_names) {
        std::cout << colName << " ";
    }
    std::cout << std::endl;
    std::cout << csvFile.files.size() << " Files Found:" << std::endl;
    for (int i = 0; i < csvFile.files.size(); i++) {
        const auto& file = csvFile.files[i];
        std::cout << file.filePath << " with " << file.lineMap.size() << " lines, starting at " << csvFile.starting_line_numbers[i] << std::endl;
    }
    std::cout << "Total lines across all files: " << csvFile.starting_line_numbers.back() + csvFile.files.back().lineMap.size() << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <csv_file_path> <file_template>" << std::endl;
        return 1;
    }

    CsvDataFileTimeGroup csvFile(argv[1], argv[2], " ", true, false, "#", false, 
        {"Day", "Time", "S", "Si_Phase", "Rb_Phase", "H_Phase", "Z_Phase", "Si_Freq", "Rb_Freq", "H_Freq", "Z_Freq"});

    printFileData(csvFile);

    while (true) {
        std::cout << "Enter row number to read (or -1 to exit, -2 to update): ";
        int row;
        std::cin >> row;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the input buffer

        if (row == -1) {
            break;
        }

        if (row == -2) {
            bool updated = csvFile.update();

            if (updated) {
                std::cout << "File list updated." << std::endl;
                printFileData(csvFile);
            } else {
                std::cout << "No changes detected." << std::endl;
            }

            continue;
        }

        try {
            auto rowData = csvFile.readRow(row);
            for (const auto& [colName, value] : rowData) {
                std::cout << colName << ": " << value << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error reading row: " << e.what() << std::endl;
        }
    }

    return 0;
}