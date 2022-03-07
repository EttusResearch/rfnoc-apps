//
// Copyright 2022 Ettus Research, a National Instruments Brand
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

//
// Example application to show how to use RFNoC FIR Filter Block.
// Set coefficients, send samples from file to FIR Filter block,
// and writes received samples to file.
//

#include <uhd/utils/safe_main.hpp>
#include <uhd/rfnoc_graph.hpp>
#include <uhd/rfnoc/fir_filter_block_control.hpp>
#include <boost/program_options.hpp>
#include <vector>
#include <complex>
#include <type_traits>
#include <fstream>
#include <iostream>
#include <thread>
#include <cmath>

namespace po = boost::program_options;

static bool stop_signal_called = false;

void sig_int_handler(int)
{
    stop_signal_called = true;
}

template <typename samp_type>
void send_from_file(
    uhd::tx_streamer::sptr tx_stream,
    const std::string& file)
{
    uhd::tx_metadata_t md;
    md.start_of_burst = false;
    md.end_of_burst   = false;
    std::vector<samp_type> buff(tx_stream->get_max_num_samps());
    std::ifstream infile(file.c_str(), std::ifstream::binary);
    if (not infile.is_open()) {
        std::string error = "Input samples file '" + file + "' "
                            "failed to open";
        std::cout << "ERROR: " << error << std::endl;
        throw std::runtime_error(error);
    }

    // Loop until the entire file has been read or ctrl-c is pressed
    while (not md.end_of_burst and not stop_signal_called) {
        infile.read((char*)&buff.front(), buff.size() * sizeof(samp_type));
        size_t num_tx_samps = size_t(infile.gcount() / sizeof(samp_type));

        md.end_of_burst = infile.eof();

        const size_t samples_sent = tx_stream->send(&buff.front(), num_tx_samps, md);
        if (samples_sent != num_tx_samps) {
            std::string error = "TX streamer timed out while streaming";
            std::cout << "ERROR: " << error << std::endl;
            throw std::runtime_error(error);
        }
    }

    if (infile.is_open()) {
        infile.close();
    }
}

template <typename samp_type>
void recv_to_file(
    uhd::rx_streamer::sptr rx_stream,
    const std::string& file)
{
    uhd::rx_metadata_t md;
    std::vector<samp_type> buff(rx_stream->get_max_num_samps());
    std::ofstream outfile;
    if (not file.empty()) {
        outfile.open(file.c_str(), std::ofstream::binary);
    }

    // Run this loop until receiving EOB or ctrl-c was pressed
    while (not md.end_of_burst and not stop_signal_called) {
        auto num_rx_samps =
            rx_stream->recv(&buff.front(), buff.size(), md, 5.0);

        if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_TIMEOUT) {
            std::string error = "RX streamer timed out while receiving";
            std::cout << "ERROR: " << error << std::endl;
            throw std::runtime_error(error);
        }
        if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_OVERFLOW) {
            if (md.out_of_sequence) {
                std::string error = "RX streamer detected dropped or "
                                    "out of sequence packet";
                std::cout << "ERROR: " << error << std::endl;
                throw std::runtime_error(error);
            } else {
                std::string error = "RX streamer detected overflow, but "
                                    "without a radio block this should be "
                                    "impossible.";
                std::cout << "ERROR: " << error << std::endl;
                throw std::runtime_error(error);
            }
        }
        if (md.error_code != uhd::rx_metadata_t::ERROR_CODE_NONE) {
            std::string error = "RX streamer detected an error: " + md.strerror();
            std::cout << "ERROR: " << error << std::endl;
            throw std::runtime_error(error);
        }

        if (outfile.is_open()) {
            outfile.write((const char*)&buff.front(), num_rx_samps * sizeof(samp_type));
        }

    }
    if (outfile.is_open()) {
        outfile.close();
    }
}

int UHD_SAFE_MAIN(int argc, char* argv[])
{
    std::string args, input_file, output_file, coeffs_file, type;

    // setup the program options
    po::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
        ("help",       "help message")
        ("args",        po::value<std::string>(&args)->default_value(""), "USRP device address args")
        ("input_file",  po::value<std::string>(&input_file)->default_value("input.dat"), "binary file to read input samples")
        ("output_file", po::value<std::string>(&output_file)->default_value("output.dat"), "binary file to write output samples")
        ("coeffs_file", po::value<std::string>(&coeffs_file)->default_value("coeffs.dat"), "binary file (short integer) of coefficients")
        ("type",        po::value<std::string>(&type)->default_value("float"), "Input and output binary file type: double, float, or short")
    ;
    // clang-format on
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // print the help message
    if (vm.count("help")) {
        std::cout << "RFNoC FIR Filter Block samples from file" << desc << std::endl;
        std::cout << std::endl
                  << "Application that sends samples from a file to a "
                  << "RFNoC FIR Filter block and writes received samples to a file.\n"
                  << "Note: Coefficient binary file format must be short integers!"
                  << std::endl;
        return EXIT_SUCCESS;
    }

    // Parse sample type
    std::string cpu_format;
    if (type == "double") {
        cpu_format = "fc64";
    } else if (type == "float") {
        cpu_format = "fc32";
    } else if (type == "short") {
        cpu_format = "sc16";
    } else {
        std::cout << "ERROR: Invalid type '" << type << "'!" << std::endl;
        return EXIT_FAILURE;
    }

    // Create RFNoC graph object
    auto graph = uhd::rfnoc::rfnoc_graph::make(args);

    // Verify we have a fir filter block and instantiate block controller
    auto fir_filter_blocks = graph->find_blocks<uhd::rfnoc::fir_filter_block_control>("");
    if (fir_filter_blocks.empty()) {
        std::cout << "ERROR: No fir filter block found." << std::endl;
        return EXIT_FAILURE;
    }
    auto fir_filter_block =
        graph->get_block<uhd::rfnoc::fir_filter_block_control>(fir_filter_blocks.front());
    if (!fir_filter_block) {
        std::cout << "ERROR: Failed to instantiate block controller!" << std::endl;
        return EXIT_FAILURE;
    }

    // Setup RX and TX streamers
    uhd::stream_args_t stream_args(cpu_format, "sc16");
    auto tx_stream = graph->create_tx_streamer(1, stream_args);
    auto rx_stream = graph->create_rx_streamer(1, stream_args);

    // Connect blocks to create a Host -> FIR Filter Block -> Host flowgraph
    graph->connect(tx_stream, 0, fir_filter_block->get_block_id(), 0);
    graph->connect(fir_filter_block->get_block_id(), 0, rx_stream, 0);
    graph->commit();

    // Load coefficients from file
    auto max_num_coeffs = fir_filter_block->get_max_num_coefficients(0);
    std::ifstream coeffs_ifs(coeffs_file.c_str(), std::ifstream::binary);
    if (not coeffs_ifs.is_open()) {
        std::cout << "ERROR: Coefficients file '" << coeffs_file << "' "
                  << "failed to open."
                  << std::endl;
        return EXIT_FAILURE;
    }

    // Check if too many coefficients
    coeffs_ifs.seekg(0, std::ios::end);
    auto num_coeffs = size_t(coeffs_ifs.tellg() / sizeof(int16_t));
    coeffs_ifs.seekg(0, std::ios::beg);
    if (num_coeffs == 0) {
        coeffs_ifs.close();
        std::cout << "ERROR: Coefficients file '" << coeffs_file << "' "
                  << "cannot be empty."
                  << std::endl;
        return EXIT_FAILURE;
    }
    if (num_coeffs > max_num_coeffs) {
        coeffs_ifs.close();
        std::cout << "ERROR: Provided " << num_coeffs << " coefficients "
                  << "but the fir filter block supports a maximum of "
                  << max_num_coeffs << " coefficients."
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<int16_t> coeffs_buff(num_coeffs);
    coeffs_ifs.read((char*)&coeffs_buff.front(), num_coeffs * sizeof(int16_t));
    fir_filter_block->set_coefficients(coeffs_buff, 0);
    coeffs_ifs.close();

    // Threads for sending samples to and receiving samples from Window block
    std::vector<std::thread> threads;
    if (type == "double") {
        threads.push_back(std::thread(
            &send_from_file<std::complex<double> >, tx_stream, input_file));
        threads.push_back(std::thread(
            &recv_to_file<std::complex<double> >, rx_stream, output_file));
    } else if (type == "float") {
        threads.push_back(std::thread(
            &send_from_file<std::complex<float> >, tx_stream, input_file));
        threads.push_back(std::thread(
            &recv_to_file<std::complex<float> >, rx_stream, output_file));
    } else if (type == "short") {
        threads.push_back(std::thread(
            &send_from_file<std::complex<int16_t> >, tx_stream, input_file));
        threads.push_back(std::thread(
            &recv_to_file<std::complex<int16_t> >, rx_stream, output_file));
    }
    for(auto &t : threads)
    {
        t.join();
    }

    return EXIT_SUCCESS;
}
