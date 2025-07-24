/*
 * SrTime.cpp
 * Calculates the current time deviation between the NIST Hydrogen Maser and a
 * generated Silicon3 / Strontium clock.
 *
 * This file is part of the TimeKeeping project.
 *
 */

#include "CsvFileUtils/CsvGroupMetadata.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/json.hpp>
#include <boost/json/object.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/math/statistics/linear_regression.hpp>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

#include "CsvFileUtils/CsvTimeGroup.hpp"
#include "Utils/ProgressBar.hpp"

#include <boost/multiprecision/cpp_bin_float.hpp>

using quad = boost::multiprecision::cpp_bin_float_quad;

#include <TimekeepingConfig.h>
#include <argparse/argparse.hpp>

/* Argument parsing and command line interface setup
 */
void setupArgParser(argparse::ArgumentParser &parser, int argc, char *argv[]) {
  parser.add_description("SrTime - Calculate time deviation between NIST "
                         "Hydrogen Maser and Silicon3 / Strontium clock.");

  parser.add_argument("-c", "--config")
      .nargs(1)
      .default_value("{}")
      .help("JSON configuration file with parameters for the calculation.");

  parser.add_epilog("Example usage: SrTime -c \"config.json\" ");

  try {
    parser.parse_args(argc, argv);
  } catch (const std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    std::exit(1);
  }
}

quad get_si_offset(boost::json::object config) {
  quad si_offset;

  std::string si_major_offset_str =
      config["Si3_Major_Offset"].as_string().c_str();
  std::string si_minor_offset_str =
      config["Si3_Minor_Offset"].as_string().c_str();

  // Read the first line to get the offset
  si_offset = boost::lexical_cast<quad>(si_major_offset_str) +
              boost::lexical_cast<quad>(si_minor_offset_str);

  return si_offset;
}

int main(int argc, char *argv[]) {
  // Setup the argument parser
  argparse::ArgumentParser parser("SrTime",
                                  std::format("v{}.{}",
                                              Timekeeping_VERSION_MAJOR,
                                              Timekeeping_VERSION_MINOR),
                                  argparse::default_arguments::all, true);
  setupArgParser(parser, argc, argv);

  // Parse the configuration file
  boost::json::object config;
  try {
    std::ifstream config_file;
    config_file.open(parser.get<std::string>("--config"));
    if (config_file) {
      boost::json::value json_value;
      config_file >> json_value;
      config = json_value.as_object();
      config_file.close();
    } else {
      std::cerr << "Error opening config file: "
                << parser.get<std::string>("--config") << std::endl;
      return 1;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error parsing config file: " << e.what() << std::endl;
    return 1;
  }

  // Load the data files
  std::cout << "Loading Si3 vs Sr Frequency data files" << std::endl;
  std::string si_freq_path = config["Si3_Data_Path"].as_string().c_str();
  std::string si_freq_template =
      config["Si3_Data_Template"].as_string().c_str();
  CsvGroupMetadata si_freq_metadata(si_freq_path, si_freq_template, {}, "", "#",
                                    ",\r", false, true, {"Time", "Si_Freq"},
                                    -1);
  CsvTimeGroup si_freq_files(si_freq_metadata, CsvTimeFormat::oneColStandard,
                             false);

  std::cout << std::endl << "Loading Si3 vs Maser data files" << std::endl;
  CsvGroupMetadata phase_freq_metadata(
      config["Si3_Maser_Data_Path"].as_string().c_str(),
      config["Si3_Maser_Data_Template"].as_string().c_str(), {}, "", "#", " ",
      true, false,
      {"Day", "Time", "S", "Si_Phase", "Rb_Phase", "H_Phase", "Z_Phase",
       "Si_Freq", "Rb_Freq", "H_Freq", "Z_Freq"});
  CsvTimeGroup phase_freq_files(phase_freq_metadata, CsvTimeFormat::twoColShort,
                                false);

  quad si_offset = get_si_offset(config);
  std::cout << "Si3 Frequency Offset: " << si_offset << " Hz" << std::endl;

  long si_division =
      boost::lexical_cast<long>(config["Si3_Division"].as_string().c_str());
  std::cout << "Si3 Frequency Division: " << si_division << std::endl;

  quad h_freq =
      boost::lexical_cast<quad>(
          config["Maser_Nominal_Frequency"].as_string().c_str()) *
      (1 + boost::lexical_cast<quad>(
               config["Maser_Starting_Fractional_Offset"].as_string().c_str()));
  quad h_drift = boost::lexical_cast<quad>(
      config["Maser_Fractional_Drift_Rate"].as_string().c_str());

  std::cout << "Hydrogen Maser Frequency: " << h_freq << " Hz" << std::endl;
  std::cout << "Hydrogen Maser Drift Rate: " << h_drift << "/s" << std::endl;

  // Parse the time range
  date_time epoch_time = parseTime(TimeFormat::isoExtended,
                                   config["Epoch_Time"].as_string().c_str());
  std::cout << "Epoch Time: "
            << boost::posix_time::to_iso_extended_string(epoch_time)
            << std::endl;

  date_time start_time = parseTime(TimeFormat::isoExtended,
                                   config["Start_Time"].as_string().c_str());
  std::cout << "Start Time: "
            << boost::posix_time::to_iso_extended_string(start_time)
            << std::endl;
  date_time end_time = parseTime(TimeFormat::isoExtended,
                                 config["End_Time"].as_string().c_str());
  std::cout << "End Time: "
            << boost::posix_time::to_iso_extended_string(end_time) << std::endl;

  quad time_step =
      boost::lexical_cast<quad>(config["Time_Step"].as_string().c_str());
  std::cout << "Time Step: " << time_step << " seconds" << std::endl;

  quad half_time = time_step / 2;

  date_time current_time = epoch_time;
  date_time mean_time;
  long interval_count = 0;

  quad si_frequency;
  quad acc_phase = 0;

  std::cout << "Finding location of epoch time in data files" << std::endl;
  size_t epoch_data_index = phase_freq_files.closestIndex(epoch_time);
  if (epoch_data_index < 0) {
    std::cerr << "Error: Epoch time not found in data files." << std::endl;
    return 1;
  }
  std::cout << "Epoch data index: " << epoch_data_index << std::endl;
  std::map<std::string, std::string> data_row =
      phase_freq_files[epoch_data_index];
  date_time data_time = phase_freq_files.timeOfRow(epoch_data_index);
  std::cout << "Epoch data time: "
            << boost::posix_time::to_iso_extended_string(data_time)
            << std::endl;
  quad data_phase = boost::lexical_cast<quad>(data_row["Si_Phase"]);
  quad data_freq = boost::lexical_cast<quad>(data_row["Si_Freq"]);
  std::cout << "Epoch data phase: " << data_phase << std::endl;
  quad epoch_data_phase = data_phase;

  std::map<std::string, std::string> new_data_row;
  quad new_data_phase;
  quad new_data_freq;
  time_delt half_time_gap = boost::posix_time::seconds(long(half_time)) +
                            boost::posix_time::microseconds(
                                long((half_time - long(half_time)) * 1e6));
  time_delt gap_limit = boost::posix_time::seconds(long(5 * time_step)) +
                        boost::posix_time::microseconds(
                            long((5 * time_step - long(2 * time_step)) * 1e6));

  // Copy config to output file
  std::string config_output_path =
      config["Output_File"].as_string().c_str() + std::string(".config");
  std::ofstream config_output_file(config_output_path);
  if (config_output_file.is_open()) {
    config_output_file << boost::json::serialize(config) << std::endl;
    config_output_file.close();
    std::cout << "Configuration written to: " << config_output_path
              << std::endl;
  } else {
    std::cerr << "Error: Could not open config output file." << std::endl;
    return 1;
  }

  // Create output file
  std::string output_file_path = config["Output_File"].as_string().c_str();
  std::cout << "Writing output to: " << output_file_path << std::endl;
  // join the data path and output file name
  std::ofstream output_file(output_file_path);
  if (!output_file.is_open()) {
    std::cerr << "Error: Could not open output file: " << output_file_path
              << std::endl;
    return 1;
  }
  output_file << "Index,Time,Time Deviation,Si Freq,H Freq,Diff Freq,Data "
                 "Logged Time"
              << std::endl;
  output_file.precision(std::numeric_limits<quad>::digits10);

  std::cout << "Starting time calculation from epoch time" << std::endl;
  ProgressBar progress_bar;
  while (current_time <= end_time) {
    // Calculate the mean time for the current interval
    mean_time = current_time;

    si_frequency = si_freq_files.colAtTime(mean_time, "Si_Freq");
    si_frequency = (si_frequency + si_offset) / si_division;

    // Calculate the time deviation
    quad time_deviation = (data_phase - acc_phase - epoch_data_phase) / h_freq;

    if (current_time >= start_time) {
      output_file << interval_count << ","
                  << boost::posix_time::to_iso_extended_string(current_time)
                  << "," << time_deviation << "," << si_frequency << ","
                  << h_freq << "," << data_freq << ","
                  << boost::posix_time::to_iso_extended_string(data_time)
                  << "\n";
    }

    // Update the progress bar every minute
    if (interval_count % 600 == 0) {
      time_delt progress = (current_time - epoch_time);
      time_delt totaltime = (end_time - epoch_time);
      std::stringstream msg;
      msg << "Time " << boost::posix_time::to_iso_extended_string(current_time)
          << " Desync Gap: " << std::setw(4)
          << (current_time - data_time).total_milliseconds()
          << "ms Deviation: " << std::setw(8) << std::fixed
          << std::setprecision(3) << time_deviation * 1e9 << " ns";
      progress_bar.updateProgress(progress.total_microseconds(),
                                  totaltime.total_microseconds(), msg.str());
    }

    // Calculate the accumulated phase
    acc_phase += (h_freq - si_frequency) * time_step;
    h_freq *= 1 + h_drift * time_step;

    new_data_row = phase_freq_files[++epoch_data_index];
    new_data_phase = boost::lexical_cast<quad>(new_data_row["Si_Phase"]);
    new_data_freq = boost::lexical_cast<quad>(new_data_row["Si_Freq"]);
    if (fabs((new_data_phase - data_phase) - (new_data_freq * time_step)) >
        1e-6) {
      std::cerr << std::endl;
      std::cerr << "Warning: Large gap detected in data files." << std::endl;
      std::cerr << "Gap: " << (new_data_phase - data_phase) / new_data_freq
                << " s" << std::endl;
      quad gap_size = (new_data_phase - data_phase) / new_data_freq;
      long gap_interval = long(round(gap_size / time_step));
      std::cerr << "Estimated " << gap_interval - 1 << " data points missing."
                << std::endl;
      quad gap_error = (gap_size - gap_interval * time_step) / time_step;
      std::cerr << "Fractional error " << gap_error << std::endl;
      if (gap_error > 0.1) {
        std::cerr << "Error: Gap too large, exiting." << std::endl;
        output_file.close();
        return 1;
      }

      for (long i = 0; i < gap_interval - 1; ++i) {
        interval_count++;
        current_time =
            epoch_time +
            boost::posix_time::seconds(long(time_step * interval_count)) +
            boost::posix_time::microseconds(
                long((time_step * interval_count -
                      long(time_step * interval_count)) *
                     1e6));
        mean_time = current_time + half_time_gap;
        si_frequency =
            (si_freq_files.colAtTime(mean_time, "Si_Freq") + si_offset) /
            si_division;
        acc_phase += (h_freq - si_frequency) * time_step;

        h_freq *= 1 + h_drift * time_step;
      }
    }

    data_row = new_data_row;
    data_time = phase_freq_files.timeOfRow(epoch_data_index);
    data_phase = boost::lexical_cast<quad>(data_row["Si_Phase"]);
    data_freq = boost::lexical_cast<quad>(data_row["Si_Freq"]);

    interval_count++;
    current_time =
        epoch_time +
        boost::posix_time::seconds(long(time_step * interval_count)) +
        boost::posix_time::microseconds(long(
            (time_step * interval_count - long(time_step * interval_count)) *
            1e6));
  }

  return 0;
}