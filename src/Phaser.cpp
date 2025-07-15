#include <iostream>
#include <fstream>

#include <cmath>
#include <boost/multiprecision/cpp_bin_float.hpp>

#include <argparse/argparse.hpp>
#include <TimekeepingConfig.h>

using quad = boost::multiprecision::cpp_bin_float_quad;

/*
 * Command-line interface for the Phaser application.
 * This tool takes a CSV file as input, processes the data, and outputs a modified CSV file.
 * It computes phase values from the input frequency data
 * It can also compute phase errors based on provided phase data.
 * If the input file is not specified or is "stdin", it reads from standard input.
 * If the output file is not specified or is "stdout", it writes to standard output.
 * 
 * Usage:
 * ./Phaser <csv_in_file> <csv_out_file>
 * Example:
 * ./Phaser input.csv output.csv
 * 
 * Flags:
 * --help: Show this help message and exit.
 * --version: Show the version of the Phaser application.
 * --check: Compute phase errors from given phase data, must be used when phase data provided.
 * --io: read and write data from std io instead of files, overrides in_file and out_file.
 * --interval: Specify the time interval for phase calculations. Default is 0.1 seconds.
 * --si_start: Specify the begining of the Si phase data.
 * --rb_start: Specify the begining of the Rb phase data.
 * --h_start: Specify the begining of the H phase data.
 * --z_start: Specify the begining of the Z phase data.
 */
void parse_args(argparse::ArgumentParser& program, int argc, char* argv[]) {

    program.add_argument("-c", "--check")
        .default_value(false)
        .implicit_value(true)
        .help("Compute phase errors from given phase data, must be used when phase data provided.");

    program.add_argument("--io")
        .default_value(false)
        .implicit_value(true)
        .help("Read and write data from std io instead of files, overrides in_file and out_file.");

    program.add_argument("-i", "--interval")
        .nargs(1)
        .default_value("0.1")
        .help("Specify the time interval for phase calculations. Default is 0.1 seconds.");

    program.add_argument("--si_start")
        .nargs(1)
        .default_value("")
        .help("Specify the beginning of the Si phase data. Default is 0.");

    program.add_argument("--rb_start")
        .nargs(1)
        .default_value("")
        .help("Specify the beginning of the Rb phase data. Default is 0.");

    program.add_argument("--h_start")
        .nargs(1)
        .default_value("")
        .help("Specify the beginning of the H phase data. Default is 0.");

    program.add_argument("--z_start")
        .nargs(1)
        .default_value("")
        .help("Specify the beginning of the Z phase data. Default is 0.");

    program.add_argument("in_file")
        .default_value("stdin")
        .help("Input CSV file, defaultes to stdin.");

    program.add_argument("out_file")
        .default_value("stdout")
        .help("Output CSV file, defaults to stdout.");

    program.add_description("This tool takes a CSV file as input, processes the data, and outputs a modified CSV file.\n"
                            "It computes phase values from the input frequency data.\n"
                            "It can also compute phase errors based on provided phase data.");
    program.add_epilog("Example usage: ./Phaser input.csv output.csv");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cerr << "Error: " << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }
}

/*
 * Main entry point for the Phaser application.
 */
int main(int argc, char* argv[]) {
    // Initialize the argument parser with program name and version
    argparse::ArgumentParser program("Phaser", 
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

    // Setup input and output streams
    std::istream input_stream(read_stdio ? std::cin.rdbuf() : csv_in_file.rdbuf());
    std::ostream output_stream(write_stdio ? std::cout.rdbuf() : csv_out_file.rdbuf());

    // Parse time delta between measurements
    quad interval;
    try {
        std::istringstream iss(program.get<std::string>("--interval"));
        iss >> interval;
    } catch (const std::exception &e) {
        std::cerr << "Error parsing --interval: " << e.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    // Fields for input data
    int Day;
    double Time;
    int S;
    quad Si_Freq = 0;
    quad Rb_Freq = 0;
    quad H_Freq = 0;
    quad Z_Freq = 0;
    quad Si_Phase = 0;
    quad Rb_Phase = 0;
    quad H_Phase = 0;
    quad Z_Phase = 0;

    // Fields for output data
    quad Si_Phase_From_Freq = 0;
    quad Rb_Phase_From_Freq = 0;
    quad H_Phase_From_Freq = 0;
    quad Z_Phase_From_Freq = 0;
    quad Si_Phase_Error = 0;
    quad Rb_Phase_Error = 0;
    quad H_Phase_Error = 0;
    quad Z_Phase_Error = 0;

    // Read the input file line by line

    std::string newline;
    bool first_line = true;

    while (std::getline(input_stream, newline)){
        // Process each line of input
        // Input file has columns: Day, Time, S, Si_Phase, Rb_Phase, H_Phase, Z_Phase, Si_Freq, Rb_Freq, H_Freq, Z_Freq
        // Output file will add the computed columns: Si_Phase_From_Freq, Rb_Phase_From_Freq, H_Phase_From_Freq, Z_Phase_From_Freq,
        // Also break down Day and Time into separate columns: Year, Month, Day, Hour, Minute, Second
        // Si_Phase_Error, Rb_Phase_Error, H_Phase_Error, Z_Phase_Error

        // Comment lines starting with '#' are ignored
        if (newline.empty() || newline[0] == '#') {
            continue;
        }

        std::istringstream iss(newline);

        // If only converting frequencies to phases, we can skip the phase data
        if (program.get<bool>("--check")) {
            if(!(iss >> Day >> Time >> S >> Si_Phase >> Rb_Phase >> H_Phase >> Z_Phase 
                >> Si_Freq >> Rb_Freq >> H_Freq >> Z_Freq)) {
                std::cerr << "Error reading line: " << newline << std::endl;
                continue; // Skip this line if reading fails
            }
        } else {
            // If not checking, we read frequencies only
            if(!(iss >> Day >> Time >> S >> Si_Freq >> Rb_Freq >> H_Freq >> Z_Freq)) {
                std::cerr << "Error reading line: " << newline << std::endl;
                continue; // Skip this line if reading fails
            }
        }

        if (first_line) {
            // Print header for the output CSV
            output_stream << "#Phase data computed from frequency data by Phaser tool." << std::endl
                        << "#Input file: " << program.get<std::string>("in_file") << std::endl
                        << "#Interval: " << interval << " seconds" << std::endl;

            if (program.get<bool>("--check")) {
                output_stream << "Year Month Day Hour Minute Second S Si_Phase Rb_Phase H_Phase Z_Phase Si_Freq Rb_Freq H_Freq Z_Freq "
                             << "Si_Phase_From_Freq Rb_Phase_From_Freq H_Phase_From_Freq Z_Phase_From_Freq "
                             << "Si_Phase_Error Rb_Phase_Error H_Phase_Error Z_Phase_Error" << std::endl;
            } else {
                output_stream << "Year Month Day Hour Minute Second S Si_Phase Rb_Phase H_Phase Z_Phase"
                             << "Si_Freq Rb_Freq H_Freq Z_Freq" << std::endl;
            }

            // Initialize phase values from the first line
            if (program.get<bool>("--check")) {
                Si_Phase_From_Freq = Si_Phase;
                Rb_Phase_From_Freq = Rb_Phase;
                H_Phase_From_Freq = H_Phase;
                Z_Phase_From_Freq = Z_Phase;
            } else {
                if (!program.get<std::string>("--si_start").empty()) {
                    std::istringstream iss_si(program.get<std::string>("--si_start"));
                    if(!(iss_si >> Si_Phase_From_Freq)) {
                        std::cerr << "Error reading --si_start value." << std::endl;
                        return 1;
                    }
                }
                if (!program.get<std::string>("--rb_start").empty()) {
                    std::istringstream iss_rb(program.get<std::string>("--rb_start"));
                    iss_rb >> Rb_Phase_From_Freq;
                }
                if (!program.get<std::string>("--h_start").empty()) {
                    std::istringstream iss_h(program.get<std::string>("--h_start"));
                    iss_h >> H_Phase_From_Freq;
                }
                if (!program.get<std::string>("--z_start").empty()) {
                    std::istringstream iss_z(program.get<std::string>("--z_start"));
                    iss_z >> Z_Phase_From_Freq;
                }

                // Accumulate phase values from the first line
                Si_Phase_From_Freq += Si_Freq * interval;
                Rb_Phase_From_Freq += Rb_Freq * interval;
                H_Phase_From_Freq += H_Freq * interval;
                Z_Phase_From_Freq += Z_Freq * interval;
            }

            first_line = false;
        } else {
            // Accumulate phase values from frequencies
            Si_Phase_From_Freq += Si_Freq * interval;
            Rb_Phase_From_Freq += Rb_Freq * interval;
            H_Phase_From_Freq += H_Freq * interval;
            Z_Phase_From_Freq += Z_Freq * interval; 
        }

        if (program.get<bool>("--check")) {
            // If checking, compute phase errors
            Si_Phase_Error = Si_Phase - Si_Phase_From_Freq;
            Rb_Phase_Error = Rb_Phase - Rb_Phase_From_Freq;
            H_Phase_Error = H_Phase - H_Phase_From_Freq;
            Z_Phase_Error = Z_Phase - Z_Phase_From_Freq;
        } 

        // Convert Day and Time to Year, Month, Day, Hour, Minute, Second
        int Year = Day / 10000;
        Year += 2000; // Assuming the year is in the 21st century
        int Month = (Day % 10000) / 100;
        int DayOfMonth = Day % 100;
        int Hour = Time / 10000;
        int Minute = (fmod(Time, 10000) / 100);
        double Second = fmod(Time, 100);

        // Write the output to the CSV file
        output_stream.precision(std::numeric_limits<double>::digits10);
        output_stream << Year << " " << Month << " " << DayOfMonth << " "
                      << Hour << " " << Minute << " " << Second << " ";

        output_stream << S << " ";

        output_stream.precision(std::numeric_limits<quad>::digits10);
        if (program.get<bool>("--check")) {
            // Write the output with phase errors
            output_stream << Si_Phase << " " << Rb_Phase << " " << H_Phase << " " << Z_Phase << " "
                         << Si_Freq << " " << Rb_Freq << " " << H_Freq << " " << Z_Freq << " "
                         << Si_Phase_From_Freq << " " << Rb_Phase_From_Freq 
                         << " " << H_Phase_From_Freq << " " << Z_Phase_From_Freq 
                         << " " << Si_Phase_Error << " " 
                         << Rb_Phase_Error << " "
                         << H_Phase_Error << " "
                         << Z_Phase_Error 
                         << std::endl;
        } else {
            // Write the output without phase errors
            output_stream << Si_Phase_From_Freq << " "
                         << Rb_Phase_From_Freq << " "
                         << H_Phase_From_Freq << " "
                         << Z_Phase_From_Freq << " "
                         << Si_Freq << " " << Rb_Freq << " " << H_Freq << " " << Z_Freq
                         << std::endl;
        }
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