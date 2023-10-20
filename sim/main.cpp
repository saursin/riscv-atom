#include <iostream>
#include <csignal>
#include "include/cxxopts/cxxopts.hpp"
#include "verilated.h"

#include "util.hpp"
#include "atomsim.hpp"

#ifndef TARGET_HEADER
#error TARGET_HEADER macro not defined
#endif

// Include appropriate backend-header
#include TARGET_HEADER

// define atomsim version
#define ATOMSIM_VERSION "v2.2"

#define ATOMSIM_BANNER \
"   ___  __             _____     \n" 		\
"  / _ |/ /____  __ _  / __(_)_ _ \n" 		\
" / __ / __/ _ \\/  ' \\_\\ \\/ /  ' \\\n" 	\
"/_/ |_\\__/\\___/_/_/_/___/_/_/_/_/ "

// Global Variables
// (use extern in other files)

volatile bool CTRL_C_PRESSED = false;
bool NO_COLOR_OUTPUT = 0;


/**
 * @brief Handle SIGINT (Ctrl+C)
 * @param signal_num 
 */
static void sigint_handler(int signal_num)
{
	if(CTRL_C_PRESSED)	// variable already set
		exit(-1);
	
	std::cerr << "\nRecieved SIGINT [" << signal_num << "]" << std::endl;
	CTRL_C_PRESSED = true;
	signal(signal_num, &sigint_handler);
}


/**
 * @brief Parse command line arguments
 * @details parses command line arguments given to the atomsim executable and 
 * accordingly sets appropriate internal flags/variables and/or displays 
 * info messages.
 * @param argc argument count
 * @param argv argument vector
 * @param infile input file name (pointer)
 */
void parse_commandline_args(int argc, char**argv, Atomsim_config &sim_config, Backend_config &backend_config)
{
	// default values are directly defined in respective headers during definition
	const Atomsim_config default_sim_config;
	const Backend_config default_backend_config;

	try
	{
		// Usage Message Header
		cxxopts::Options options(argv[0], "AtomSim " ATOMSIM_VERSION "\nInteractive RTL Simulator for Atom based systems [ " ATOMSIM_TARGETNAME " ]");
		
		options.positional_help("input").show_positional_help();

		// Adding CLI options
		options.add_options("General")
		("h,help", "Show this message")
		("version", "Show version information")
		("soctarget", "Show current AtomSim SoC target")
		("no-color", "Don't show colored output", cxxopts::value<bool>(sim_config.no_color_flag)->default_value(default_sim_config.no_color_flag?"true":"false"))
		("no-banner", "Don't show banner", cxxopts::value<bool>(sim_config.no_banner_flag)->default_value(default_sim_config.no_banner_flag?"true":"false"))
		("i,input", "Specify an input file", cxxopts::value<std::string>(sim_config.ifile))
		;
		
		options.add_options("Sim Config")
		("maxitr", "Specify maximum simulation iterations", cxxopts::value<unsigned long int>(sim_config.maxitr)->default_value(std::to_string(default_sim_config.maxitr)))
		;

		options.add_options("Backend Config")
		("p,vuart-port", "serial port for virtual UART", cxxopts::value<std::string>(backend_config.vuart_portname)->default_value(default_backend_config.vuart_portname))
		("b,vuart-baud", "serial baud rate for virtual UART", cxxopts::value<uint32_t>(backend_config.vuart_baudrate)->default_value(std::to_string(default_backend_config.vuart_baudrate)))
		("u,enable-uart-dump", "Enable dumping UART data (from soc) to stdout", cxxopts::value<bool>(backend_config.enable_uart_dump)->default_value(default_backend_config.enable_uart_dump?"true":"false"))
		
		#ifdef TARGET_ATOMBONES
		("bootrom-size", "Specify size of bootrom to simulate (in KB)", cxxopts::value<uint32_t>(backend_config.bootrom_size_kb)->default_value(std::to_string(default_backend_config.bootrom_size_kb)))
		("bootrom-image", "Specify bootrom hex image", cxxopts::value<std::string>(backend_config.bootrom_img)->default_value(default_backend_config.bootrom_img))
		("ram-size", "Specify size of RAM memory to simulate (in KB)", cxxopts::value<uint32_t>(backend_config.ram_size_kb)->default_value(std::to_string(default_backend_config.ram_size_kb)))
		#endif

		#ifdef TARGET_HYDROGENSOC
		("bootmode", "Specify bootmode signal", cxxopts::value<int>(backend_config.bootmode)->default_value(std::to_string(default_backend_config.bootmode)))
		#endif
		;

		options.add_options("Debug")
		("v,verbose", "Turn on verbose output", cxxopts::value<bool>(sim_config.verbose_flag)->default_value(default_sim_config.verbose_flag?"true":"false"))
		("d,debug", "Start in debug mode", cxxopts::value<bool>(sim_config.debug_flag)->default_value(default_sim_config.debug_flag?"true":"false"))
		("t,trace", "Enable VCD tracing ", cxxopts::value<bool>(sim_config.trace_flag)->default_value(default_sim_config.trace_flag?"true":"false"))
		("trace-file", "Specify trace file", cxxopts::value<std::string>(sim_config.trace_file)->default_value(default_sim_config.trace_file))
		("dump-file", "Specify dump file", cxxopts::value<std::string>(sim_config.dump_file)->default_value(default_sim_config.dump_file))
		("ebreak-dump", "Enable processor state dump at hault", cxxopts::value<bool>(sim_config.dump_on_ebreak_flag)->default_value(default_sim_config.dump_on_ebreak_flag?"true":"false"))
		("signature", "Enable signature dump at hault (Used for riscv compliance tests)", cxxopts::value<std::string>(sim_config.signature_file)->default_value(default_sim_config.signature_file))
		;

	    options.parse_positional({"input"});

		//options.allow_unrecognised_options();
		
		// parse CLI options
		auto result = options.parse(argc, argv);

		if(result.unmatched().size() != 0)
		{
			std::string unknown_args;
			for(unsigned int i=0; i<result.unmatched().size(); i++)
				unknown_args = unknown_args + result.unmatched()[i] + (i==result.unmatched().size()-1 ? "" :", ");
			throwError("CLI0", "Unrecognized arguments [" + unknown_args + "]", true);
		}

		if (result.count("help"))
		{
			std::cout << options.help() << std::endl;
			exit(EXIT_SUCCESS);
		}
		if (result.count("version"))
		{
			std::cout << ATOMSIM_VERSION " [ " ATOMSIM_TARGETNAME " ] " <<  std::endl;
			exit(EXIT_SUCCESS);
		}
		if (result.count("soctarget"))
		{
			std::cout << ATOMSIM_TARGETNAME << std::endl;
			exit(EXIT_SUCCESS);
		}
		if (result.count("input")>1)
		{
			throwError("CLI1", "Multiple input files specified", true);
		}
		if (result.count("input")==0)
		{
			throwError("CLI2", "No input files specified", true);
		}
	}
	catch(const cxxopts::OptionException& e)
	{
		throwError("CLI3", "Error while parsing command line arguments: " + (std::string)e.what(), true);
	}	
}



/**
 * @brief Main
 * 
 * @param argc argument count 
 * @param argv argument vector
 * @return int return code
 */
int main(int argc, char ** argv)
{
	// Define SIGINT handler
	signal(SIGINT, &sigint_handler);

	// Configure Atomsim
    Atomsim_config sim_config;		// Sim config parameters
	Backend_config backend_config;	// Backend config parameters

    // Initialize verilator
	Verilated::commandArgs(argc, argv);
	
	// Parse commandline arguments
	parse_commandline_args(argc, argv, sim_config, backend_config);

	// Print banner
	if(!sim_config.no_banner_flag)
		std::cout << ATOMSIM_BANNER << " " << ATOMSIM_VERSION << std::endl;
	
	// Print soctarget
	if (sim_config.verbose_flag)
		std::cout << "soctarget: " << ATOMSIM_TARGETNAME << std::endl;
	
	int exitcode=0;
	try
	{
		// Initialize Sim
		Atomsim sim(sim_config, backend_config);

		// Run sim
		exitcode = sim.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
    
	return exitcode;
}