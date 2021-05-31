#include "include/cxxopts/cxxopts.hpp"
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <string>
#include <vector>

// Definitions
const char default_tace_dir[] = "build/trace";

const unsigned int default_mem_size = 131072;	// 128KB
const unsigned int default_entry_point = 0x00000000;

const unsigned int TX_ADDRESS = 0x00012001;
const unsigned int TX_ACK_ADDRESS = 0x00012002;

// Global flags
bool verbose_flag = false;
bool debug_mode = false;
bool trace_enabled = false;

// Global vars

// This is used to display reason for simulation termination
std::string end_simulation_reason;


#include "defs.hpp"
#include "backend.hpp"


/**
 * @brief parses command line arguments given to the assembler and 
 * accordingly sets appropriate internal flags/variables and/or displays 
 * info messages.
 * 
 * @param argc argument count
 * @param argv argument vector
 * @param ifile input file name (pointer)
 * @param tdir trace_dir (pointer)
 */
void parse_commandline_args(int argc, char**argv, std::string &ifile, std::string &tdir)
{
	try
	{
		// Usage Message Header
		cxxopts::Options options(argv[0], std::string(Info_version)+"\nSimulator for AtomRVSoC");
		
		options.positional_help("input").show_positional_help();


		// Adding CLI options
		options.add_options("General")
		("h,help", "Show this message")
		("version", "Show version information")
		("i, input", "Specify an input file", cxxopts::value<std::string>(ifile));

		options.add_options("Debug")
		("v,verbose", "Turn on verbose output", cxxopts::value<bool>(verbose_flag)->default_value("false"))
		("d,debug", "Start in debug mode", cxxopts::value<bool>(debug_mode)->default_value("false"))
		("t,trace", "Enable VCD tracing ", cxxopts::value<bool>(trace_enabled)->default_value("false"))
		("trace-dir", "Specify a trace directory", cxxopts::value<std::string>(tdir)->default_value(tdir));


	    options.parse_positional({"input"});

		//options.allow_unrecognised_options();
		
		// parse CLI options
		auto result = options.parse(argc, argv);

		if(result.unmatched().size() != 0)
		{
			std::string unknown_args;
			for(int i=0; i<result.unmatched().size(); i++)
				unknown_args = unknown_args + result.unmatched()[i] + (i==result.unmatched().size()-1 ? "" :", ");
			throwError("ARGPARSE~", "Unrecognized aguments [" + unknown_args + "]", true);
		}


		if (result.count("help"))
		{
			std::cout << options.help() << std::endl;
			exit(0);
		}
		if (result.count("version"))
		{
			std::cout << Info_version << std::endl << Info_copyright << std::endl;
			exit(0);
		}
		if (result.count("input")>1)
		{
			throwError("CLIPARSE~", "Multiple input files specified", true);
			exit(0);
		}
		if (result.count("input")==0)
		{
			throwError("CLIPARSE~", "No input files specified", true);
			exit(0);
		}

		std::cout << "Input File:" << ifile << "\n";

	}
	catch(const cxxopts::OptionException& e)
	{
		throwError("CLIPARSE~", "Error while parsing command line arguments...\n" + (std::string)e.what(), true);
	}	
}

/**
 * @brief Run specified cycles of simulation
 * 
 * @param cycles no to cycles to run for
 * @param b pointer to backend object
 */
void tick(long unsigned int cycles, Backend * b, const bool show_data = true)
{
	for(long unsigned int i=0; i<cycles && !b->done(); i++)
	{
		if(b->done())
		{
			break;
		}
		if(show_data)
		{
			b->tick();
			b->refreshData();
			b->displayData();
		}
		else
		{
			b->refreshData();
			b->tick();
			
			// Rx Listener
			if(b->mem->fetchByte(TX_ACK_ADDRESS) == 1)
			{
				std::cout << (char)b->mem->fetchByte(TX_ADDRESS);
			}
		}
	}
}


/**
 * @brief Main function
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @return int exit code
 */
int main(int argc, char **argv)
{
	std::string ifile;
	std::string trace_dir = default_tace_dir;

	// Parse commandline arguments
	parse_commandline_args(argc, argv, ifile, trace_dir);
	
	// Initialize verilator
	Verilated::commandArgs(argc, argv);

	// Create a new backend instance
	Backend bkend(ifile, default_mem_size);

	// Run simulation
	if(debug_mode)
	{
		std::string input;
		while(true)
		{
			if(bkend.done())	// if $finish encountered by verilator
			{
				end_simulation_reason = "Backend encountered a $finish";
				break;
			}

			// Parse Input
			std::cout << ": ";
			getline(std::cin, input);
			
			// Tokenize
			std::vector<std::string> token;
			tokenize(input, token, ' ');

			// Parse Command
			if(token[0] == "q" | token[0] == "quit")
			{
				// Quit simulator
				end_simulation_reason = "User interruption";
				break;
			}
			else if(token[0] == "r")
			{
				// Run indefinitely
				tick(-1, &bkend);
			}
			else if(token[0] == "rst")
			{
				// Reset Simulator
				bkend.reset();
			}
			else if(token[0] == "")
			{
				// Run for 1 cycles
				tick(1, &bkend);
			}
			else if(token[0] == "verbose-on")
			{
				// turn on verbose
				verbose_flag = true;
			}
			else if(token[0] == "verbose-off")
			{
				// turn on verbose
				verbose_flag = false;
			}
			else if(token[0] == "mem")
			{
				if(token.size()<2)
					throwError("DBG~", "\"mem\" command expects address as argument\n");
				unsigned int addr = std::stoi(token[1]);
				printf("%08X : %02X %02X %02X %02X\n", addr, bkend.mem->fetchByte(addr),
				 bkend.mem->fetchByte(addr+1),bkend.mem->fetchByte(addr+2), bkend.mem->fetchByte(addr+3));
			}
			else if(token[0] == "dumpmem")
			{
				if(token.size()<2)
					throwError("DBG~", "\"dumpmem\" command expects filename as argument\n");
				
				// turn on verbose
				std::vector<std::string> fcontents;
				for(int i=0; i<bkend.mem->size-4; i+=4)
				{	
					char hex [30];
					sprintf(hex, "0x%08X\t:\t0x%08X", i, bkend.mem->fetchWord(i));
					fcontents.push_back(hex);
				}
				fWrite(fcontents, token[1]);
			}
			else if(token[0] == "for")
			{
				// run for specified cycles
				if(token.size()<2)
					throwError("DBG0", "\"for\" command expects one argument\n");
				else
					tick(std::stoi(token[1]), &bkend);
			}
			else if(token[0] == "trace")
			{
				// Enable trace
				if(token.size()<2)
					throwError("DBG1", "Trace command expects a filename\n");
				else
				{
					if(trace_enabled == false)
					{
						std::string tracefile = trace_dir+"/"+token[1];
						bkend.tb->openTrace(tracefile.c_str());
						std::cout << "Trace enabled : \"" << tracefile << "\" opened for output.\n";
						trace_enabled = true;
					}
					else
						std::cout << "Trace was already enabled\n";
				}
			}
			else if(token[0] == "notrace")
			{
				// Disable trace
				if(trace_enabled == true)
				{
					bkend.tb->closeTrace();
					std::cout << "Trace disabled\n";
					trace_enabled = false;
				}
				else
					std::cout << "Trace was not enabled \n";
			}
			else
			{
				throwError("DBG2", "Unknown command \"" + token[0] + "\"\n");
			}
			input.clear();
		}
	}
	else
	{
		std::cout << "_________________________________________________________________\n";
		tick(-1, &bkend, false);
	}

	if(trace_enabled) // if trace file is open, close it before exiting
		bkend.tb->closeTrace();
	
	std::cout << "Simulation ended @ tick " << bkend.tb->m_tickcount_total << " due to : " << end_simulation_reason << std::endl;
	exit(EXIT_SUCCESS);    
}


