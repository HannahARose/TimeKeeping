#include "CsvTimeGroup.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/math/statistics/linear_regression.hpp>
#include <cstddef>
#include <string>

date_time parseTime(TimeFormat format, const std::string &time_str) {
  switch (format) {
  case TimeFormat::standard:
    return boost::posix_time::time_from_string(time_str);
  case TimeFormat::iso:
    return boost::posix_time::from_iso_string(time_str);
  case TimeFormat::isoExtended:
    return boost::posix_time::from_iso_extended_string(time_str);
  default:
    throw std::invalid_argument("Unsupported time format: " +
                                std::to_string(static_cast<int>(format)));
  }
}

date_time CsvTimeGroup::timeOfRow(size_t index) {
  // Check if the index is already cached
  if (timeCache_.find(index) != timeCache_.end()) {
    return timeCache_[index];
  }

  // If not cached, parse the time from the CSV row
  auto row = csvGroup_[index];
  date_time time;

  switch (timeFormat_) {
  case CsvTimeFormat::oneColStandard:
    return parseTime(TimeFormat::standard, row["Time"]);
  case CsvTimeFormat::twoColShort:
    return parseTime(TimeFormat::iso,
                     "20" + row["Day"] + "T" + row["Time"].replace(6, 1, ","));

  default:
    throw std::invalid_argument("Unsupported CSV time format: " +
                                std::to_string(static_cast<int>(timeFormat_)));
  }

  // Cache the parsed time
  timeCache_[index] = time;
  return time;
}

std::pair<size_t, size_t> CsvTimeGroup::bounds(date_time time) {
  long start_index = 0;
  long end_index = csvGroup_.metadata().size() - 1;

  date_time start_time = timeOfRow(start_index);
  date_time end_time = timeOfRow(end_index);

  if (time < start_time) {
    return {-1, start_index};
  }
  if (time > end_time) {
    return {end_index, -1};
  }

  while (end_index - start_index > 1) {
    long middle_index = (start_index + end_index) / 2;
    date_time middle_time = timeOfRow(middle_index);

    if (middle_time < time) {
      start_index = middle_index;
      start_time = middle_time;
    } else {
      end_index = middle_index;
      end_time = middle_time;
    }
  }
  return {start_index, end_index};
}

size_t CsvTimeGroup::closestIndex(date_time time) {
  auto [start_index, end_index] = bounds(time);
  if (start_index == -1) {
    return end_index;
  }
  if (end_index == -1) {
    return start_index;
  }

  date_time start_time = timeOfRow(start_index);
  date_time end_time = timeOfRow(end_index);

  return (time - start_time < end_time - time) ? start_index : end_index;
}

quad CsvTimeGroup::colAtTime(date_time time, const std::string &colName) {
  // Handle extrapolation for times outside the range of the CSV data
  if (time < startTime()) {
    // Generate extrapolation parameters if not already cached
    if (!extrapolationCacheLow_.contains(colName)) {
      date_time ref_time = startTime();

      std::vector<quad> times;
      std::vector<quad> values;

      for (int i = 0; i < 10; ++i) {
        times.push_back((timeOfRow(i) - ref_time).total_microseconds());
        values.push_back(boost::lexical_cast<quad>(csvGroup_[i][colName]));
      }

      // Perform linear regression to estimate the value at the given time
      auto [constant, slope] =
          boost::math::statistics::simple_ordinary_least_squares(times, values);
      extrapolationCacheLow_[colName] = {ref_time, constant, slope};
    }
    // Use the cached extrapolation parameters to calculate the value
    auto &[ref_time, constant, slope] = extrapolationCacheLow_[colName];
    return constant + slope * (time - ref_time).total_microseconds();
  } else if (time > endTime()) {
    // Generate extrapolation parameters for the high side if not already cached
    if (!extrapolationCacheHigh_.contains(colName)) {
      date_time ref_time = endTime();

      std::vector<quad> times;
      std::vector<quad> values;

      for (int i = csvGroup_.metadata().size() - 10;
           i < csvGroup_.metadata().size(); ++i) {
        times.push_back((timeOfRow(i) - ref_time).total_microseconds());
        values.push_back(boost::lexical_cast<quad>(csvGroup_[i][colName]));
      }

      // Perform linear regression to estimate the value at the given time
      auto [constant, slope] =
          boost::math::statistics::simple_ordinary_least_squares(times, values);
      extrapolationCacheHigh_[colName] = {ref_time, constant, slope};
    }
    // Use the cached extrapolation parameters to calculate the value
    auto &[ref_time, constant, slope] = extrapolationCacheHigh_[colName];
    return constant + slope * (time - ref_time).total_microseconds();
  }

  // If the time is within the range of the CSV data, find the two rows that
  // enclose the time
  auto [start_index, end_index] = bounds(time);

  // Interpolate the value at the given time
  quad start_value = boost::lexical_cast<quad>(csvGroup_[start_index][colName]);
  quad end_value = boost::lexical_cast<quad>(csvGroup_[end_index][colName]);

  date_time start_time = timeOfRow(start_index);
  date_time end_time = timeOfRow(end_index);

  return start_value + (end_value - start_value) *
                           (time - start_time).total_microseconds() /
                           (end_time - start_time).total_microseconds();
}