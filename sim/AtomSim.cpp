#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <string>
#include <vector>


// Definitions
const unsigned int Sec = 1000000;	// microseconds in 1s
const unsigned int mSec = 1000;		// microseconds in 1ms

// Simulation Parameters
unsigned int delay_amt = 1000 * mSec; //default 

// This is used to display reason for simulation termination
std::string end_simulation_reason;

// Global flags
bool verbose_flag = false;
bool debug_mode = false;
bool trace_enabled = false;

const unsigned int default_mem_size = 131072;	// 128KB
const unsigned int default_entry_point = 0x00000000;

#include "defs.hpp"
#include "backend.hpp"

// Input file
std::string ifile = "";

// Trace directory
std::string trace_dir;

/**
 * @brief parses command line arguments given to the assembler and 
 * accordingly sets appropriate internal flags/variables and/or displays 
 * info messages.
 * 
 * @param argc argument count
 * @param argv argument vector
 * @return true if we need to exit after this step
 * @return false otherwise
 */
bool parse_commandline_args(const int argc, char**argv)
{
    // ============= STEP-2: PARSE COMMAND-LINE ARGUMENTS ================
    if(argc < 2)
    {
		throwError("CLI0", "Too few arguments\n For help, try : atomsim --help\n", true);
    }
    int i = 1;
    while(i < argc)
    {
        std::string argument = argv[i];

        // check if it is a flag
        if(argument[0] == '-')
        {
            if(argument == "-v")
            {
				// turn on verbose
                verbose_flag = true;
                i++;
            }
			else if(argument == "-d")
            {
				// run in debug mode
                debug_mode = true;
                i++;
            }
            else if(argument == "-h")
            {
				// show short help message
                std::cout << Info_short_help_msg;
                return true;
            }
			else if(argument == "--help")
            {
				// print long help message
                std::cout << Info_long_help_msg;
                return true;
            }
            else if(argument == "--version")
            {
				// print charon version info
                std::cout << Info_version << std::endl << Info_copyright;
            }
			else if(argument == "--trace-dir")
            {
				// print long help message
                if(i == argc-1)
				{
					throwError("CLI1", "Trace directory not provided\n", true);
				}
				i++;
				trace_dir = argv[i];
				i++;
            }
            
            else
            {
				throwError("CLI2", "Unknown argument: " + argument + "\n", true);
            }
        }
        else
        {
            // specify input files
            if(ifile != "")
            {
				throwError("CLI3", "Multiple Input files povided\n", true);
            }
            else
                ifile = argument;
            i++;
        }
    }

	if (ifile == "")
	{
		// No input file povided
		throwError("CLI4", "No input file povided\n", true);
	}
    return false;
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
			if(b->mem->fetchByte(0x0001fffe) == 1)
			{
				std::cout << (char)b->mem->fetchByte(0x0001ffff);
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
	// Parse commandline arguments
	if(parse_commandline_args(argc, argv))
		return 0;

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


