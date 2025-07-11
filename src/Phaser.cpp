#include <iostream>
#include <fstream>
#include <format>
#include <boost/multiprecision/cpp_bin_float.hpp>

#include <argparse/argparse.hpp>
#include <TimekeepingConfig.h>

using quad = boost::multiprecision::cpp_bin_float_100;

/*
 * Main entry point for the Phaser application.
 * This tool takes a CSV file as input, processes the data, and outputs a modified CSV file.
 * It computes phase values from the input frequency data
 * It can also compute phase errors based on provided phase data.
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
 * --io: read and write data from std io instead of files.
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
        .help("Read and write data from std io instead of files.");
    
    program.add_argument("-i", "--interval")
        .nargs(1)
        .default_value("0.1")
        .help("Specify the time interval for phase calculations. Default is 0.1 seconds.");

    program.add_argument("--si_start")
        .nargs(1)
        .default_value(0)
        .help("Specify the beginning of the Si phase data. Default is 0.");

    program.add_argument("--rb_start")
        .nargs(1)
        .default_value(0)
        .help("Specify the beginning of the Rb phase data. Default is 0.");

    program.add_argument("--h_start")
        .nargs(1)
        .default_value(0)
        .help("Specify the beginning of the H phase data. Default is 0.");

    program.add_argument("--z_start")
        .nargs(1)
        .default_value(0)
        .help("Specify the beginning of the Z phase data. Default is 0.");

    program.add_argument("in_file")
        .default_value("")
        .help("Input CSV file, required unless --io is used.");

    program.add_argument("out_file")
        .default_value("")
        .help("Output CSV file, required unless --io is used.");

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

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("Phaser", 
        std::format("v{}.{}", Timekeeping_VERSION_MAJOR, Timekeeping_VERSION_MINOR), 
        argparse::default_arguments::all, true);

    parse_args(program, argc, argv);

    // Configure input and output files based on arguments
    std::ifstream csv_in_file;
    std::ofstream csv_out_file;
    if (program.get<bool>("--io")) {
        csv_in_file.open("stdin");
        csv_out_file.open("stdout");
    } else {
        try {
            csv_in_file.open(program.get<std::string>("in_file"));
            csv_out_file.open(program.get<std::string>("out_file"));
            if (!csv_in_file.is_open()) {
                throw std::runtime_error("Could not open input file: " + program.get<std::string>("in_file"));
            }
            if (!csv_out_file.is_open()) {
                throw std::runtime_error("Could not open output file: " + program.get<std::string>("out_file"));
            }
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            std::cerr << program;
            return 1;
        }
    }

    std::string newline;
    bool first_line = true;

    quad interval;
    try {
        std::istringstream iss(program.get<std::string>("--interval"));
        iss >> interval;
    } catch (const std::exception &e) {
        std::cerr << "Error parsing --interval: " << e.what() << std::endl;
        std::cerr << program;
        return 1;
    }
    int Day;
    float Time;
    int S;
    quad Si_Freq = 0;
    quad Rb_Freq = 0;
    quad H_Freq = 0;
    quad Z_Freq = 0;
    quad Si_Phase = 0;
    quad Rb_Phase = 0;
    quad H_Phase = 0;
    quad Z_Phase = 0;
    quad Si_Phase_From_Freq = 0;
    quad Rb_Phase_From_Freq = 0;
    quad H_Phase_From_Freq = 0;
    quad Z_Phase_From_Freq = 0;
    quad Si_Phase_Error = 0;
    quad Rb_Phase_Error = 0;
    quad H_Phase_Error = 0;
    quad Z_Phase_Error = 0;

    // Read the input file line by line

    while (std::getline(csv_in_file, newline)) {
        // Process each line of input
        // Input file has columns: Day, Time, S, Si_Phase, Rb_Phase, H_Phase, Z_Phase, Si_Freq, Rb_Freq, H_Freq, Z_Freq
        // Output file will add the computed columns: Si_Phase_From_Freq, Rb_Phase_From_Freq, H_Phase_From_Freq, Z_Phase_From_Freq,
        // Si_Phase_Error, Rb_Phase_Error, H_Phase_Error, Z_Phase_Error

        // Comment lines starting with '#' are ignored
        if (newline.empty() || newline[0] == '#') {
            continue;
        }

        std::istringstream iss(newline);
        // If only converting frequencies to phases, we can skip the phase data
        if (!(iss >> Day >> Time >> S >> Si_Phase >> Rb_Phase >> H_Phase >> Z_Phase >> Si_Freq >> Rb_Freq >> H_Freq >> Z_Freq)) {
            std::cerr << "Error reading line: " << newline << std::endl;
            continue; // Skip this line if reading fails
        }


        if (first_line) {
            // Print header for the output CSV
            output_file << "Day Time S Si_Phase Rb_Phase H_Phase Z_Phase Si_Freq Rb_Freq H_Freq Z_Freq "
                      << "Si_Phase_From_Freq Rb_Phase_From_Freq H_Phase_From_Freq Z_Phase_From_Freq "
                      << "Si_Phase_Error Rb_Phase_Error H_Phase_Error Z_Phase_Error" << std::endl;

            Si_Phase_From_Freq = Si_Phase;
            Rb_Phase_From_Freq = Rb_Phase;
            H_Phase_From_Freq = H_Phase;
            Z_Phase_From_Freq = Z_Phase;
            first_line = false;
        } else {

                Si_Phase_From_Freq += Si_Freq * interval;
                Rb_Phase_From_Freq += Rb_Freq * interval;
                H_Phase_From_Freq += H_Freq * interval;
                Z_Phase_From_Freq += Z_Freq * interval; 

                Si_Phase_Error = Si_Phase - Si_Phase_From_Freq;
                Rb_Phase_Error = Rb_Phase - Rb_Phase_From_Freq;
                H_Phase_Error = H_Phase - H_Phase_From_Freq;
                Z_Phase_Error = Z_Phase - Z_Phase_From_Freq;
        }

        output_file << Day << " " << Time << " " << S << " " << std::setprecision(std::numeric_limits<quad>::digits10) 
                    << Si_Phase << " " << Rb_Phase << " " << H_Phase << " " << Z_Phase << " "
                    << Si_Freq << " " << Rb_Freq << " " << H_Freq << " " << Z_Freq << " "
                    << Si_Phase_From_Freq << " " << Rb_Phase_From_Freq << " " << H_Phase_From_Freq << " " << Z_Phase_From_Freq << " "
                    << Si_Phase_Error << " " << Rb_Phase_Error << " " << H_Phase_Error << " " << Z_Phase_Error << std::endl;

    }


    return 0;
}