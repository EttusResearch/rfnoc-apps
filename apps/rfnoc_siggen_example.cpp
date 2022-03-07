//
// Copyright 2022 Ettus Research, a National Instruments Brand
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

//
// Example application to show how to instantiate and test the RFNoC SigGen block.
//

#include <uhd/exception.hpp>
#include <uhd/rfnoc/block_control.hpp>
#include <uhd/rfnoc/duc_block_control.hpp>
#include <uhd/rfnoc/siggen_block_control.hpp>
#include <uhd/rfnoc/radio_control.hpp>
#include <uhd/rfnoc_graph.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/thread.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <complex>
#include <csignal>
#include <fstream>
#include <iostream>
#include <thread>
#include <iostream>

// Convenient namespacing for options parsing
namespace po = boost::program_options ;

int UHD_SAFE_MAIN(int argc, char *argv[]) {
    uhd::set_thread_priority_safe() ;

    // Options
    float rate ;
    size_t duration ;
    float freq ;
    float gain ;
    size_t radio_id ;
    size_t radio_chan ;
    std::string waveform ;
    std::string usrp_args ;

    po::options_description desc("Allowed options") ;
    desc.add_options()
        ("help",                                                                                                                "Help message")
        ("rate",        po::value<float>(&rate)->default_value(200.0e6),                                                        "Sample rate that the block is running")
        ("duration",    po::value<size_t>(&duration)->default_value(10),                                                        "Duration in seconds")
        ("freq",        po::value<float>(&freq)->default_value(2.45e9),                                                         "RF Center Frequency")
        ("radio_id",    po::value<size_t>(&radio_id)->default_value(0),                                                         "Radio ID")
        ("radio_chan",  po::value<size_t>(&radio_chan)->default_value(0),                                                       "Radio Channel")
        ("gain",        po::value<float>(&gain)->default_value(10.0),                                                           "TX Gain")
        ("waveform",    po::value<std::string>(&waveform)->default_value("noise"),                                              "Waveform [constant, sine, noise]")
        ("args",        po::value<std::string>(&usrp_args)->default_value("addr=192.168.30.2,second_addr=192.168.40.2"),        "UHD Device Arguments")
    ;

    po::variables_map vm ;
    po::store(po::parse_command_line(argc, argv, desc), vm) ;
    po::notify(vm) ;
    if( vm.count("help") ) {
        std::cout << "RFNoC SigGen Block " << desc << std::endl ;
        std::cout << std::endl
                  << "Example application that tests the RFNoC SigGen block"
                  << std::endl ;
        return EXIT_SUCCESS ;
    }

    uhd::rfnoc::siggen_waveform wf ;

    if( waveform == "noise" ) {
        wf = uhd::rfnoc::siggen_waveform::NOISE ;
    } else if( waveform == "constant" ) {
        wf = uhd::rfnoc::siggen_waveform::CONSTANT ;
    } else if( waveform == "sine" ) {
        wf = uhd::rfnoc::siggen_waveform::SINE_WAVE ;
    } else {
        printf("%s is not [noise, constant, sine]\n", waveform.c_str() ) ;
        return -1 ;
    }

    std::cout << "Creating the RFNoC graph with args: " << usrp_args << "..." << std::endl ;
    auto graph = uhd::rfnoc::rfnoc_graph::make(usrp_args) ;

    // Radio Block Controller
    uhd::rfnoc::block_id_t radio_control_id(0, "Radio", radio_id) ;
    uhd::rfnoc::radio_control::sptr radio_control ;
    radio_control = graph->get_block<uhd::rfnoc::radio_control>(radio_control_id) ;
    if( !radio_control ) {
        std::cout << "ERROR: Failed to find Radio Block Controller!" << std::endl ;
        return EXIT_FAILURE ;
    }
    std::cout << "Using radio " << radio_control_id << ", channel " << radio_chan << std::endl ;

    // DUC Block Controller
    uhd::rfnoc::block_id_t duc_control_id(0, "DUC", radio_id) ;
    uhd::rfnoc::duc_block_control::sptr duc_control ;
    duc_control = graph->get_block<uhd::rfnoc::duc_block_control>(duc_control_id) ;
    if( !duc_control ) {
        std::cout << "ERROR: Failed to find DUC Block Controller!" << std::endl ;
        return EXIT_FAILURE ;
    }
    std::cout << "Using duc " << duc_control_id << std::endl ;

    // SigGen Block Controller
    uhd::rfnoc::block_id_t siggen_control_id(0, "SigGen", 0) ;
    uhd::rfnoc::siggen_block_control::sptr siggen_control ;
    siggen_control = graph->get_block<uhd::rfnoc::siggen_block_control>(siggen_control_id) ;
    if( !siggen_control ) {
        std::cout << "ERROR: Failed to find SigGen Block Controller!" << std::endl ;
        return EXIT_FAILURE ;
    }
    std::cout << "Using siggen " << siggen_control_id << std::endl ;

    // Connecting the graph SigGen -> DUC -> Radio
    graph->connect(siggen_control->get_block_id(),  0, duc_control->get_block_id(),   0);
    graph->connect(duc_control->get_block_id(),     0, radio_control->get_block_id(), 0);
    graph->commit() ;

    // Radio Frequency
    radio_control->set_tx_frequency(freq, radio_chan) ;
    radio_control->set_tx_gain(gain, radio_chan) ;

    // DSP Interpolation
    duc_control->set_output_rate(200e6, radio_chan) ;
    duc_control->set_input_rate(rate, radio_chan) ;

    // SigGen Settings
    siggen_control->set_waveform(wf, 0) ;
    if( wf == uhd::rfnoc::siggen_waveform::CONSTANT ) {
        siggen_control->set_amplitude(0.99, 0) ;
    } else if( wf == uhd::rfnoc::siggen_waveform::SINE_WAVE ) {
        siggen_control->set_sine_frequency(1e3, rate, 0) ;
    }

    // Enable the SigGen
    siggen_control->set_enable(true, 0) ;

    // Sleep while signals are generated
    std::this_thread::sleep_for(std::chrono::seconds(duration)) ;

    // Disable the SigGen
    siggen_control->set_enable(false, 0) ;

    return EXIT_SUCCESS ;
}
