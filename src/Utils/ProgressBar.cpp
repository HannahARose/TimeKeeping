#include "ProgressBar.hpp"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <iostream>
using time_delt = boost::posix_time::time_duration;

void ProgressBar::updateProgress(double progress, double total,
                                 std::string message) {
  int bar_width = 50; // Width of the progress bar
  double percentage = progress / total;

  if (percentage > 1.0) {
    percentage = 1.0; // Cap at 100%
  }

  int filled_width = static_cast<int>(bar_width * percentage);
  std::cout.precision(4);

  // Print the progress bar
  std::cout << "\r[";
  for (int i = 0; i < filled_width; ++i) {
    std::cout << "#";
  }
  for (int i = filled_width; i < bar_width; ++i) {
    std::cout << "-";
  }
  std::cout << "] ";

  // Print the message
  std::cout << message;

  time_delt elapsed_time =
      boost::posix_time::second_clock::universal_time() - startTime_;
  time_delt estimated_time = elapsed_time * (1.0 / percentage - 1.0);

  std::cout << " (" << elapsed_time << "/" << estimated_time;

  std::cout << ", " << std::setw(5) << std::fixed << std::setprecision(2)
            << static_cast<int>(percentage * 10000) / 100.0 << "%)";

  std::cout.flush(); // Ensure immediate output
  if (percentage >= 1.0) {
    std::cout << std::endl; // New line when complete
  }
}