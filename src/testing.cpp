#include "CsvFileUtils/CsvGroup.hpp"

#include <ctime>
#include <iostream>
#include <time.h>

int main(int argc, char *argv[]) {
  // CsvGroupMetadata metadata(
  //     "./data/PhaseFreq_B_4/", "PhaseFreq_B_4_[0-9]{6}_[0-9].txt", {}, "",
  //     "#", " ", true, false,
  //     {"Day", "Time", "S", "Si_Phase", "Rb_Phase", "H_Phase", "Z_Phase",
  //      "Si_Freq", "Rb_Freq", "H_Freq", "Z_Freq"},
  //     -1);

  // CsvGroupMetadata metadata("./data/Si3/", "Si3_[0-9]{2}.csv", {}, "", "#",
  // ",",
  //                           false, true, {"Time", "Si_Freq"}, -1);

  CsvGroupMetadata metadata(
      "./data/Freq_B_3", "Freq_B_3_[0-9]{6}_[0-9].txt", {}, "", "#", " ", true,
      false, {"Day", "Time", "S", "Si_Freq", "Rb_Freq", "H_Freq", "Z_Freq"},
      -1);

  std::cout << "Created a metadata object:\n";
  std::cout << metadata << std::endl;

  time_t start = time(NULL);
  std::cout << "Attempting to read the csv file using the metadata starting at "
            << ctime(&start) << "..." << std::endl;
  CsvGroup csvGroup(metadata, true); // true to ignore cache
  time_t end = time(NULL);
  std::cout << "CSV group read successfully in " << difftime(end, start)
            << " seconds.\n";

  std::cout << "CSV group contents:\n";
  std::cout << csvGroup << std::endl;

  std::cout << "Line 25 of the CSV file:\n";
  std::cout << csvGroup.getRawLine(25) << std::endl;

  std::cout << "Now attempting to read the group, using the new cache we just "
               "created...\n";
  start = time(NULL);
  CsvGroup csvGroupWithCache(metadata);
  end = time(NULL);
  std::cout << "CSV file updated successfully in " << difftime(end, start)
            << " seconds.\n";

  std::cout << "CSV group contents after update:\n";
  std::cout << csvGroupWithCache << std::endl;
  std::cout << "Line 2005 of the updated CSV file:\n";
  std::cout << csvGroupWithCache.getRawLine(25) << std::endl;
  return 0;
}