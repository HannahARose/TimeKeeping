// void print_progress_bar(int progress, int total, int lines) {
//     int bar_width = 50;
//     float percentage = static_cast<float>(progress) / total;
//     int filled_width = static_cast<int>(bar_width * percentage);
//     std::cout.precision(4);

//     std::cout << "\r["; // Carriage return and start of bar
//     for (int i = 0; i < filled_width; ++i) {
//         std::cout << "#";
//     }
//     for (int i = filled_width; i < bar_width; ++i) {
//         std::cout << "-";
//     }
//     std::cout << "] " << progress << " out of " << total << " files, " 
//               << lines << " lines processed.";
//     std::cout.flush(); // Ensure immediate output
// }

