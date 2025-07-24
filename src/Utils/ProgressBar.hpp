#ifndef __PROGRESSBAR_H__
#define __PROGRESSBAR_H__

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <string>

using date_time = boost::posix_time::ptime;

struct ProgressBar {
private:
  date_time startTime_;

public:
  ProgressBar()
      : startTime_(boost::posix_time::second_clock::universal_time()) {}

  void updateProgress(double progress, double total, std::string message = "");
};

#endif // __PROGRESSBAR_H__