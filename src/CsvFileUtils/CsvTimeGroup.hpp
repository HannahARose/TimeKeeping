#ifndef __CSVTIMEGROUP_H__
#define __CSVTIMEGROUP_H__

#include "CsvGroup.hpp"
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>

using date_time = boost::posix_time::ptime;
using time_delt = boost::posix_time::time_duration;

using quad = boost::multiprecision::cpp_bin_float_quad;

#include <map>
#include <tuple>

/**
 * @brief Formats for parsing time strings into date_time objects.
 */
enum TimeFormat {
  /**
   * @brief "YYYY-MM-DD HH:MM:SS.fffffffff"
   */
  standard,

  /**
   * @brief "YYYYMMDDTHHMMSS.fffffffff"
   */
  iso,

  /**
   * @brief "YYYY-MM-DDTHH:MM:SS.fffffffff"
   */
  isoExtended
};

/**
 * @brief Parses a time string into a boost::posix_time::ptime object.
 * @param format: The format of the time string
 * @param time_str: The time string to parse
 * @return: A boost::posix_time::ptime object representing the parsed time
 */
date_time parseTime(TimeFormat format, const std::string &time_str);

/**
 * @brief Represents the format of time data in the CSV files.
 */
enum class CsvTimeFormat {
  /**
   * @brief Time Data stored in a single column, "Time", in standard format.
   */
  oneColStandard,

  /**
   * @brief Time Data stored in two columns, "Day" and "Time", with
   * "Day" as "YYMMDD" and "Time" as "HHMMSS.fffffffff".
   */
  twoColShort,
};

struct CsvTimeGroup {
private:
  /**
   * @brief The group of CSV files containing time data.
   */
  CsvGroup csvGroup_;

  /**
   * @brief The format of the time data in the CSV files.
   */
  CsvTimeFormat timeFormat_;

  /**
   * @brief A cache for time values to avoid repeated parsing.
   * Maps from index in the CSV file to the corresponding date_time.
   */
  std::map<size_t, date_time> timeCache_;

  /**
   * @brief A cache for extrapolation parameters, on the low side.
   * @details Maps from column name to a tuple of (reference_time, constant,
   * slope).
   */
  std::map<std::string, std::tuple<date_time, quad, quad>>
      extrapolationCacheLow_;

  /**
   * @brief A cache for extrapolation parameters, on the high side.
   * @details Maps from column name to a tuple of (reference_time, constant,
   * slope).
   */
  std::map<std::string, std::tuple<date_time, quad, quad>>
      extrapolationCacheHigh_;

public:
  CsvTimeGroup(CsvGroupMetadata metadata, CsvTimeFormat timeFormat,
               bool ignoreCache = false)
      : csvGroup_(metadata, ignoreCache), timeFormat_(timeFormat) {}

  date_time timeOfRow(size_t index);

  date_time startTime() { return timeOfRow(0); }

  date_time endTime() { return timeOfRow(csvGroup_.metadata().size() - 1); }

  std::pair<size_t, size_t> bounds(date_time time);

  size_t closestIndex(date_time time);

  quad colAtTime(date_time time, const std::string &colName);

  std::map<std::string, std::string> operator[](size_t index) {
    return csvGroup_[index];
  }
};
#endif // __CSVTIMEGROUP_H__