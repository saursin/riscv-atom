/**
 * @file AtomSim.cpp
 * @author Saurabh Singh (saurabh.s99100@gmail.com)
 * @brief This is the main C++ file for the AtomSim.
 * 
 * @copyright 
 * MIT License
 *
 * Copyright (c) 2021 Saurabh Singh
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <string>
#include <vector>

#include "include/cxxopts/cxxopts.hpp"

// ===== Prototypes =====
void ExitAtomSim(std::string message, bool exit_with_error=false);


// ===== Global flags =====
bool verbose_flag 			= false;
bool debug_mode 			= false;
bool trace_enabled 			= false;
bool dump_regs_on_ebreak 	= false;	// Used by SCAR framework


// ===== Global Variables =====
// Input File
std::string ifile;

// Max Iteration
const unsigned long int default_maxitr = 10000000;
unsigned long int maxitr = default_maxitr;

// Trace Directory
const char default_trace_dir[] 		= "build/trace";
std::string trace_dir = default_trace_dir;

// Dump Directory
const char default_dump_dir[] 		= "build/dump";
std::string dump_dir = default_dump_dir;


#include "defs.hpp"

// ===== Other Configurations =====
#define DEBUG_PRINT_T2B	// print registers in top to bottom fashion 
//#define DEBUG_PRINT_L2R	// print registers in left to right fashion 


// ====== Backend specific definitions =====
#ifdef TARGET_ATOMBONES
// Signature file
std::string signature_file 	= "";

// Include Backend
#include "Backend_AtomBones.hpp"

// Backend name
const std::string AtomSimBackend = "AtomBones";

// Backend Object
Backend_AtomBones *bkend;		

// Default mem size for atomBones
const unsigned long int default_mem_size = 134217728 + 3 + 1;	// 128MB (Code & Data) + 3 Bytes (Serial IO) + 1 (To make word access possible)
unsigned long int mem_size = default_mem_size;
#endif

/**
 * @brief Exit AtomSim
 * 
 * @param message Exit message
 * @param exit_with_error 
 */
void ExitAtomSim(std::string message, bool exit_with_error)
{
	// ===== Pre-Exit Procedure =====
	// if trace file is open, close it before exiting
	if(trace_enabled)
		bkend->tb->closeTrace();

	// ===== Exit Message =====
	if(verbose_flag && message.length() != 0)
		std::cout << message << "\n";

	// Destroy backend
	bkend->~Backend_AtomBones();

	// ===== Exit =====
	if(exit_with_error)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}


/**
 * @brief parses command line arguments given to the assembler and 
 * accordingly sets appropriate internal flags/variables and/or displays 
 * info messages.
 * 
 * @param argc argument count
 * @param argv argument vector
 * @param infile input file name (pointer)
 */
void parse_commandline_args(int argc, char**argv, std::string &infile)
{
	try
	{
		// Usage Message Header
		cxxopts::Options options(argv[0], std::string(Info_version)+"\nRTL simulator for Atom based systems");
		
		options.positional_help("input").show_positional_help();

		// Adding CLI options
		options.add_options("General")
		("h,help", "Show this message")
		("version", "Show version information")
		("i,input", "Specify an input file", cxxopts::value<std::string>(infile));
		
		options.add_options("Config")
		("maxitr", "Specify maximum simulation iterations", cxxopts::value<unsigned long int>(maxitr)->default_value(std::to_string(default_maxitr)))
		#ifdef TARGET_ATOMBONES
		("memsize", "Specify size of memory to simulate", cxxopts::value<unsigned long int>(mem_size)->default_value(std::to_string(default_mem_size)))
		("uart-broadcast", "enable uart broadcasting over", cxxopts::value<unsigned long int>(mem_size)->default_value(std::to_string(default_mem_size)))
		#endif
		;

		options.add_options("Debug")
		("v,verbose", "Turn on verbose output", cxxopts::value<bool>(verbose_flag)->default_value("false"))
		("d,debug", "Start in debug mode", cxxopts::value<bool>(debug_mode)->default_value("false"))
		("t,trace", "Enable VCD tracing ", cxxopts::value<bool>(trace_enabled)->default_value("false"))
		("trace-dir", "Specify trace directory", cxxopts::value<std::string>(trace_dir)->default_value(default_trace_dir))
		("dump-dir", "Specify dump directory", cxxopts::value<std::string>(dump_dir)->default_value(default_trace_dir))
		("ebreak-dump", "Enable processor state dump at hault", cxxopts::value<bool>(dump_regs_on_ebreak)->default_value("false"))
		#ifdef TARGET_ATOMBONES
		("signature", "Enable signature sump at hault (Used for riscv compliance tests)", cxxopts::value<std::string>(signature_file)->default_value(""))
		#endif
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
			std::cout << Info_version << std::endl << Info_copyright << std::endl;
			exit(EXIT_SUCCESS);
		}
		if (result.count("input")>1)
		{
			throwError("CLI1", "Multiple input files specified", true);
			exit(EXIT_SUCCESS);
		}
		if (result.count("input")==0)
		{
			throwError("CLI2", "No input files specified", true);
			exit(EXIT_SUCCESS);
		}

		if (verbose_flag)
			std::cout << "Input File: " << infile << "\n";

	}
	catch(const cxxopts::OptionException& e)
	{
		throwError("CLI3", "Error while parsing command line arguments...\n" + (std::string)e.what(), true);
	}	
}


/**
 * @brief Run specified cycles of simulation
 * 
 * @param cycles no to cycles to run for
 */
void run(long unsigned int cycles)
{	
	// Run for specified cycles
	for(long unsigned int i=0; i<cycles; i++)
	{
		bkend->tick();
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Main function
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @return int exit code
 */
int main(int argc, char **argv)
{
    if (verbose_flag)
		std::cout << "AtomSim [" << AtomSimBackend << "]\n";

	// Initialize verilator
	Verilated::commandArgs(argc, argv);
	
	// Parse commandline arguments
	parse_commandline_args(argc, argv, ifile);

	// Create a new backend instance
	#ifdef TARGET_ATOMBONES
	bkend = new Backend_AtomBones(ifile, default_mem_size);
	#endif

	if(trace_enabled == true)
	{
		std::string tracefile = trace_dir+"/trace.vcd";
		bkend->tb->openTrace(tracefile.c_str());
		std::cout << "Trace enabled : \"" << tracefile << "\" opened for output.\n";
		trace_enabled = true;
	}

	// Run simulation
	if(debug_mode)
	{
		std::string input;
		while(true)
		{
			// Parse Input
			std::cout << ": ";
			getline(std::cin, input);
			
			// Tokenize
			std::vector<std::string> token;
			tokenize(input, token, ' ');

			// Parse Command
			if((token[0] == "q") | (token[0] == "quit"))
			{
				// Quit simulator
				ExitAtomSim("Exiting due to user interuption");
			}
			else if(token[0] == "r")
			{
				// Run indefinitely
				run(-1);
			}
			else if(token[0] == "rst")
			{
				// Reset Simulator
				bkend->reset();
			}
			else if(token[0] == "")
			{
				// Run for 1 cycles
				run(1);
			}
			else if(token[0] == "for")
			{
				// run for specified cycles
				if(token.size()<2)
					throwError("CMD2", "\"for\" command expects one argument\n");
				else
					run(std::stoi(token[1]));
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
			else if(token[0] == "trace-on")
			{
				// Enable trace
				if(token.size()<2)
					throwError("CMD3", "Trace command expects a filename\n");
				else
				{
					if(trace_enabled == false)
					{
						std::string tracefile = trace_dir + "/"+token[1];
						bkend->tb->openTrace(tracefile.c_str());
						std::cout << "Trace enabled : \"" << tracefile << "\" opened for output.\n";
						trace_enabled = true;
					}
					else
						std::cout << "Trace was already enabled\n";
				}
			}
			else if(token[0] == "trace-off")
			{
				// Disable trace
				if(trace_enabled == true)
				{
					bkend->tb->closeTrace();
					std::cout << "Trace disabled\n";
					trace_enabled = false;
				}
				else
					std::cout << "Trace was not enabled \n";
			}
			// ============== BACKEND SPECIFIC COMMANDS ==================
			#ifdef TARGET_ATOMBONES
			else if(token[0] == "mem")
			{
				if(token.size()<2)
					throwError("CMD0", "\"mem\" command expects address as argument\n", false);
				else
				{
					uint32_t addr;
					if(token[1].substr(0, 2) == "0x")			// Hex Number
						addr = std::stoi(token[1], nullptr, 16);
					else if(token[1].substr(0, 2) == "0b")		// Binary Number
						addr = std::stoi(token[1], nullptr , 2);
					else										// Decimal Number
						addr = std::stoi(token[1], nullptr, 10);
					
					printf("%08x : %02x %02x %02x %02x\n", addr, bkend->mem->fetchByte(addr),
						bkend->mem->fetchByte(addr+1),bkend->mem->fetchByte(addr+2), bkend->mem->fetchByte(addr+3));
				}
			}
			else if(token[0] == "dumpmem")
			{
				if(token.size()<2)
					throwError("CMD1", "\"dumpmem\" command expects filename as argument\n");
				
				std::vector<std::string> fcontents;
				for(unsigned int i=0; i<bkend->mem->size-4; i+=4)
				{	
					char hex [30];
					sprintf(hex, "0x%08x\t:\t0x%08x", i, bkend->mem->fetchWord(i));
					fcontents.push_back(hex);
				}
				fWrite(fcontents, token[1]);
			}
			#endif

			else
			{
				throwError("CMD4", "Unknown command \"" + token[0] + "\"\n");
			}
			input.clear();
		}
	}
	else
	{
		run(-1);
	}

	// Control must never Reach Here //
	ExitAtomSim("PROGRAM CONTROL FAULT", true);
	return 0;
}
