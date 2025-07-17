#include <iostream>
#include <fstream>

#include <cmath>
#include <boost/multiprecision/cpp_bin_float.hpp>

#include <argparse/argparse.hpp>
#include <TimekeepingConfig.h>

using quad = boost::multiprecision::cpp_bin_float_quad;

/*
 * Command-line interface for the Timer application.
 * This tool takes a CSV file as input, processes the data, and outputs a modified CSV file.
 * It computes time values from the input phase data
 * If the input file is not specified or is "stdin", it reads from standard input.
 * If the output file is not specified or is "stdout", it writes to standard output.
 *
 * Usage:
 * Timer <csv_in_file> <csv_out_file>
 * Example:
 * Timer input.csv output.csv
 *
 * Flags:
 * --help: Show this help message and exit.
 * --version: Show the version of the Timer application.
 * --error: Compute time errors rather than cumulative time.
 * --start: Specify the reference time for the phase data, default is 0.
 * --interval: Specify the time interval for time error calculations. Default is 0.1 seconds.
 * --io: read and write data from std io instead of files, overrides in_file and out_file.
 * --si_freq: Specify the frequency of the Si data.
 * --rb_freq: Specify the frequency of the Rb data.
 * --h_freq: Specify the frequency of the H data.
 * --z_freq: Specify the frequency of the Z data.
 * --si_error_freq: Specify the frequency of the Si data phase errors. Defaults to --si_freq.
 * --rb_error_freq: Specify the frequency of the Rb data phase errors. Defaults to --rb_freq.
 * --h_error_freq: Specify the frequency of the H data phase errors. Defaults to --h_freq.
 * --z_error_freq: Specify the frequency of the Z data phase errors. Defaults to --z_freq.
 */
void parse_args(argparse::ArgumentParser& program, int argc, char* argv[]) {

    program.add_argument("--io")
        .default_value(false)
        .implicit_value(true)
        .help("Read and write data from std io instead of files, overrides in_file and out_file.");

    program.add_argument("-e", "--error")
        .default_value(false)
        .implicit_value(true)
        .help("Compute time errors rather than cumulative time.");

    program.add_argument("--start")
        .nargs(1)
        .default_value("0")
        .help("Specify the reference time for the phase data, default is 0.");

    program.add_argument("-i", "--interval")
        .nargs(1)
        .default_value("0.1")
        .help("Specify the time interval for phase calculations. Default is 0.1 seconds.");

    program.add_argument("--si_freq")
        .nargs(1)
        .default_value("995532.6897452829")
        .help("Specify the frequency of the Si data. Default is 995532.6897452829.");

    program.add_argument("--rb_freq")
        .nargs(1)
        .default_value("10000000.00754296")
        .help("Specify the beginning of the Rb data. Default is 10000000.00754296.");

    program.add_argument("--h_freq")
        .nargs(1)
        .default_value("5000000.0000000065")
        .help("Specify the frequency of the H data. Default is 5000000.0000000065.");

    program.add_argument("--z_freq")
        .nargs(1)
        .default_value("10")
        .help("Specify the frequency of the Z data. Default is 10.");

    program.add_argument("--si_error_freq")
        .nargs(1)
        .default_value("")
        .help("Specify the frequency of the Si data phase errors. Defaults to --si_freq.");

    program.add_argument("--rb_error_freq")
        .nargs(1)
        .default_value("")
        .help("Specify the frequency of the Rb data phase errors. Defaults to --rb_freq.");

    program.add_argument("--h_error_freq")
        .nargs(1)
        .default_value("")
        .help("Specify the frequency of the H data phase errors. Defaults to --h_freq.");

    program.add_argument("--z_error_freq")
        .nargs(1)
        .default_value("")
        .help("Specify the frequency of the Z data phase errors. Defaults to --z_freq.");

    program.add_argument("in_file")
        .default_value("")
        .help("Input CSV file, required unless --io is used.");

    program.add_argument("out_file")
        .default_value("")
        .help("Output CSV file, required unless --io is used.");

    program.add_description("This tool takes a CSV file as input, processes the data, and outputs a modified CSV file.\n"
                            "It computes time values from the input phase data.");

    program.add_epilog("Example usage: Timer input.csv output.csv");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cerr << "Error: " << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }
}

/*
 * Main entry point for the Timer application.
 */
int main(int argc, char* argv[]) {
    // Initialize the argument parser with program name and version
    argparse::ArgumentParser program("Timer", 
        std::format("v{}.{}", Timekeeping_VERSION_MAJOR, Timekeeping_VERSION_MINOR), 
        argparse::default_arguments::all, true);

    // Parse command-line arguments
    parse_args(program, argc, argv);


    // Configure input and output files based on arguments
    bool read_stdio = program.get<bool>("--io");
    bool write_stdio = program.get<bool>("--io");

    std::string in_file = program.get<std::string>("in_file");
    std::string out_file = program.get<std::string>("out_file");

    // If the input or output file is not specified, use stdin/stdout
    if (in_file.empty() || in_file == "stdin") {
        read_stdio = true;
    } 

    if (out_file.empty() || out_file == "stdout") {
        write_stdio = true;
    }

    // Open input and output files if not using stdio

    std::ifstream csv_in_file;
    std::ofstream csv_out_file;

    if (!read_stdio) {
        csv_in_file.open(in_file);
        if (!csv_in_file.is_open()) {
            std::cerr << "Error: Could not open input file: " << in_file << std::endl;
            return 1;
        }
    }

    if (!write_stdio) {
        csv_out_file.open(out_file);
        if (!csv_out_file.is_open()) {
            std::cerr << "Error: Could not open output file: " << out_file << std::endl;
            return 1;
        }
    }

    // Set up input and output streams
    std::istream input_stream(read_stdio ? std::cin.rdbuf() : csv_in_file.rdbuf());
    std::ostream output_stream(write_stdio ? std::cout.rdbuf() : csv_out_file.rdbuf());

    // Parse start time and interval
    quad reference_time = 0; // Default reference time
    if (!program.get<std::string>("--start").empty()) {
        std::istringstream iss_start(program.get<std::string>("--start"));
        if (!(iss_start >> reference_time)) {
            std::cerr << "Error: Invalid start time value: " << program.get<std::string>("--start") << std::endl;
            return 1;
        }
    }

    quad interval = 0.1; // Default interval
    if (program.get<bool>("--error")) {
        if (!program.get<std::string>("--interval").empty()) {
            std::istringstream iss_int(program.get<std::string>("--interval"));
            if (!(iss_int >> interval)) {
                std::cerr << "Error: Invalid interval value: " << program.get<std::string>("--interval") << std::endl;
                return 1;
            }
        }
    }

    long long interval_n = 0;

    // Fields for input data
    int Year = 0;
    int Month = 0;
    int Day = 0;
    int Hour = 0;
    int Minute = 0;
    quad Second = 0.0;

    int S = 0;

    quad Si_Phase = 0;
    quad Rb_Phase = 0;
    quad H_Phase = 0;
    quad Z_Phase = 0;

    quad Si_Freq = 1;
    quad Rb_Freq = 1;
    quad H_Freq = 1;
    quad Z_Freq = 1;

    // Fields for output data
    quad Si_Mean_Freq = 1;
    quad Rb_Mean_Freq = 1;
    quad H_Mean_Freq = 1;
    quad Z_Mean_Freq = 1;
    quad Si_Error_Freq = 0;
    quad Rb_Error_Freq = 0;
    quad H_Error_Freq = 0;
    quad Z_Error_Freq = 0;

    quad Si_Time = 0;
    quad Rb_Time = 0;
    quad H_Time = 0;
    quad Z_Time = 0;

    // Read reference frequencies from command-line arguments
    if (!program.get<std::string>("--si_freq").empty()) {
        std::istringstream iss_si(program.get<std::string>("--si_freq"));
        if (!(iss_si >> Si_Mean_Freq)) {
            std::cerr << "Error: Invalid Si Frequency value: " << program.get<std::string>("--si_freq") << std::endl;
            return 1;
        }
    }
    if (!program.get<std::string>("--rb_freq").empty()) {
        std::istringstream iss_rb(program.get<std::string>("--rb_freq"));
        if (!(iss_rb >> Rb_Mean_Freq)) {
            std::cerr << "Error: Invalid Rb Frequency value: " << program.get<std::string>("--rb_freq") << std::endl;
            return 1;
        }
    }
    if (!program.get<std::string>("--h_freq").empty()) {
        std::istringstream iss_h(program.get<std::string>("--h_freq"));
        if (!(iss_h >> H_Mean_Freq)) {
            std::cerr << "Error: Invalid H Frequency value: " << program.get<std::string>("--h_freq") << std::endl;
            return 1;
        }
    }
    if (!program.get<std::string>("--z_freq").empty()) {
        std::istringstream iss_z(program.get<std::string>("--z_freq"));
        if (!(iss_z >> Z_Mean_Freq)) {
            std::cerr << "Error: Invalid Z Frequency value: " << program.get<std::string>("--z_freq") << std::endl;
            return 1;
        }
    }

    // Read error frequencies if --error is specified
    if (program.get<bool>("--error")) {
        if (!program.get<std::string>("--si_error_freq").empty()) {
            std::istringstream iss_si_err(program.get<std::string>("--si_error_freq"));
            if (!(iss_si_err >> Si_Error_Freq)) {
                std::cerr << "Error: Invalid Si Error Frequency value: " << program.get<std::string>("--si_error_freq") << std::endl;
                return 1;
            }
        } else {
            Si_Error_Freq = Si_Freq;
        }
        if (!program.get<std::string>("--rb_error_freq").empty()) {
            std::istringstream iss_rb_err(program.get<std::string>("--rb_error_freq"));
            if (!(iss_rb_err >> Rb_Error_Freq)) {
                std::cerr << "Error: Invalid Rb Error Frequency value: " << program.get<std::string>("--rb_error_freq") << std::endl;
                return 1;
            }
        } else {
            Rb_Error_Freq = Rb_Freq;
        }
        if (!program.get<std::string>("--h_error_freq").empty()) {
            std::istringstream iss_h_err(program.get<std::string>("--h_error_freq"));
            if (!(iss_h_err >> H_Error_Freq)) {
                std::cerr << "Error: Invalid H Error Frequency value: " << program.get<std::string>("--h_error_freq") << std::endl;
                return 1;
            }
        } else {
            H_Error_Freq = H_Freq;
        }
        if (!program.get<std::string>("--z_error_freq").empty()) {
            std::istringstream iss_z_err(program.get<std::string>("--z_error_freq"));
            if (!(iss_z_err >> Z_Error_Freq)) {
                std::cerr << "Error: Invalid Z Error Frequency value: " << program.get<std::string>("--z_error_freq") << std::endl;
                return 1;
            }
        } else {
            Z_Error_Freq = Z_Freq;
        }
    } 


    // Read the input file line by line

    std::string newline;
    bool first_line = true;

    while (std::getline(input_stream, newline)) {
        // Process each line of input
        // Input file has columns: Year, Month, Day, Hour, Minute, Second, S, Si_Phase, Rb_Phase, H_Phase, Z_Phase, Si_Freq, Rb_Freq, H_Freq, Z_Freq
        // Output file will add the computed columns: Si_Time, Rb_Time, H_Time, Z_Time
        // Comment lines starting with '#' are ignored

        if (newline.empty() || newline[0] == '#') {
            continue;
        }

        if (first_line) {
            // Print header for the output CSV
            output_stream.precision(std::numeric_limits<quad>::digits10);
            output_stream << "#Time data computed from Phase data by Timer tool." << std::endl;
            output_stream << "#Input file: " << program.get<std::string>("in_file") << std::endl;
            output_stream << "#Si Frequency: " << Si_Mean_Freq << " seconds" << std::endl;
            output_stream << "#Rb Frequency: " << Rb_Mean_Freq << " seconds" << std::endl;
            output_stream << "#H Frequency: " << H_Mean_Freq << " seconds" << std::endl;
            output_stream << "#Z Frequency: " << Z_Mean_Freq << " seconds" << std::endl;
            output_stream << "#Reference time: " << reference_time << " seconds" << std::endl;
            if (program.get<bool>("--error")) {
                output_stream << "#Interval: " << interval << " seconds" << std::endl;
                output_stream << "#Si Error Frequency: " << Si_Error_Freq << " seconds" << std::endl;
                output_stream << "#Rb Error Frequency: " << Rb_Error_Freq << " seconds" << std::endl;
                output_stream << "#H Error Frequency: " << H_Error_Freq << " seconds" << std::endl;
                output_stream << "#Z Error Frequency: " << Z_Error_Freq << " seconds" << std::endl;
                output_stream << "#Time errors computed from Phase data." << std::endl;
            }

            output_stream << "Year Month Day Hour Minute Second S Si_Phase Rb_Phase H_Phase Z_Phase Si_Freq Rb_Freq H_Freq Z_Freq "
                            << "Si_Time Rb_Time H_Time Z_Time" << std::endl;

            first_line = false;

            continue; // Skip the header line
        }

        // Parse the line into variables
        std::istringstream iss(newline);
        if(!(iss >> Year >> Month >> Day >> Hour >> Minute >> Second >> S 
                >> Si_Phase >> Rb_Phase >> H_Phase >> Z_Phase
            >> Si_Freq >> Rb_Freq >> H_Freq >> Z_Freq)) {
            std::cerr << "Error reading line: " << newline << std::endl;
            continue; // Skip this line if reading fails
        }

        // Compute the time values from the phase data
        if (!program.get<bool>("--error")) {

            // Cumulative time calculation
            Si_Time = Si_Phase / Si_Mean_Freq + reference_time;
            Rb_Time = Rb_Phase / Rb_Mean_Freq + reference_time;
            H_Time = H_Phase / H_Mean_Freq + reference_time;
            Z_Time = Z_Phase / Z_Mean_Freq + reference_time;

        } else {

            // Time error calculation
            interval_n ++;
            Si_Time = (Si_Phase - (interval_n * interval * Si_Mean_Freq)) / Si_Error_Freq;
            Rb_Time = (Rb_Phase - interval_n * interval * Rb_Mean_Freq) / Rb_Error_Freq;
            H_Time = (H_Phase - interval_n * interval * H_Mean_Freq) / H_Error_Freq;
            Z_Time = (Z_Phase - interval_n * interval * Z_Mean_Freq) / Z_Error_Freq;

        }

        // Write the output to the CSV file

        output_stream.precision(std::numeric_limits<double>::digits10);
        output_stream << Year << " " << Month << " " << Day << " "
                      << Hour << " " << Minute << " " << Second << " ";

        output_stream << S << " ";

        output_stream.precision(std::numeric_limits<quad>::digits10);
        output_stream << Si_Phase << " " << Rb_Phase << " " << H_Phase << " " << Z_Phase << " "
                      << Si_Freq << " " << Rb_Freq << " " << H_Freq << " " << Z_Freq << " "
                      << Si_Time << " " << Rb_Time << " " << H_Time << " " << Z_Time
                     << std::endl;
    }

    // Close the files
    output_stream.flush();
    if (!read_stdio) {
        csv_in_file.close();
    }
    if (!write_stdio) {
        csv_out_file.close();
    }
    return 0;
}