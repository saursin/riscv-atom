#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <string>
#include <vector>

#include "include/cxxopts/cxxopts.hpp"

// ===== Definitions =====
const char default_trace_dir[] 		= "build/trace";
const char default_dump_dir[] 		= "build/dump";

// ===== Global flags =====
bool verbose_flag 			= false;
bool debug_mode 			= false;
bool trace_enabled 			= false;
bool dump_regs_on_ebreak 	= false;

// ===== Global vars =====
std::string ifile;
#ifdef TARGET_ATOMBONES
std::string signature_file 		= "";
const unsigned long int default_mem_size 	= 134217731;	// 128MB (Code & Data) + 3 Bytes (Serial IO)
unsigned long int mem_size 	= default_mem_size;
#endif

const unsigned int default_entry_point = 0x00000000;
const unsigned long int default_maxitr = 10000000;
unsigned long int maxitr = default_maxitr;

std::string trace_dir = default_trace_dir;
std::string dump_dir = default_dump_dir;

std::string end_simulation_reason; // This is used to display reason for simulation termination

// ===== Configurations =====
#define DEBUG_PRINT_T2B	// print registers in top to bottom fashion 
//#define DEBUG_PRINT_L2R	// print registers in left to right fashion 

#include "defs.hpp"

// ===== Backend selection =====
// These macros are defined in command line during compiling.
#ifdef TARGET_ATOMBONES
#include "Backend_AtomBones.hpp"
const std::string AtomSimBackend = "AtomBones";
#endif



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
		("maxitr", "Specify maximum simulation iterations", cxxopts::value<unsigned long int>(maxitr))
		#ifdef TARGET_ATOMBONES
		("memsize", "Specify size of memory to simulate", cxxopts::value<unsigned long int>(mem_size))
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
			throwError("CLI0", "Unrecognized aguments [" + unknown_args + "]", true);
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
			throwError("CLI1", "Multiple input files specified", true);
			exit(0);
		}
		if (result.count("input")==0)
		{
			throwError("CLI2", "No input files specified", true);
			exit(0);
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
 * @param b pointer to backend object
 * @param show_data if true, show execution data.
 */
void tick(long unsigned int cycles, Backend * b, const bool show_data = true)
{	
	// Run for specified cycles
	for(long unsigned int i=0; i<cycles && !b->done(); i++)
	{
		if(b->done())	// Encountered $finish() in RTL
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
			if(dump_regs_on_ebreak) 
				b->refreshData();
			b->tick();
		}

		if (b->tb->m_core->AtomBones->atom_core->InstructionRegister == 0x100073)	// Hault condition
		{
			if (verbose_flag)
				std::cout << "Haulting @ tick " << b->tb->m_tickcount << "\n";

			if(dump_regs_on_ebreak)	// dump contents of registers into a text file, to be used by scar framework
			{
				std::vector<std::string> fcontents;
				
				for(int i=0; i<34; i++)
				{
					char temp [50];
					unsigned int tmpval;

					switch(i-2)
					{
						case -2: tmpval = b->pc_e; sprintf(temp, "pc 0x%08x", tmpval); break;
						case -1: tmpval = b->ins_e; sprintf(temp, "ir 0x%08x", tmpval); break;
						default: tmpval = b->rf[i-2]; sprintf(temp, "x%d 0x%08x",i-2, tmpval); break;
					}
					fcontents.push_back(std::string(temp));
				}
				fWrite(fcontents, std::string(trace_dir)+"/dump.txt");
			}
			
			#ifdef TARGET_ATOMBONES
			// Dunmp signature on hault condition, to be used by riscv-arch-test (official riscv compliance testing framework)
			if(signature_file.length()!=0)	
			{
				// ============= Get start and end address of signature. =============
				long int begin_signature_at = -1;
				long int end_signature_at = -1;

				ELFIO::elfio reader;

				if (!reader.load(ifile)) 
				{
					throwError("SIG", "Can't find or process ELF file : " + ifile + "\n", true);
				}

				ELFIO::Elf_Half n = reader.sections.size();
				for ( ELFIO::Elf_Half i = 0; i < n; ++i )  // For all sections
				{
					ELFIO::section* sec = reader.sections[i];
					if ( SHT_SYMTAB == sec->get_type() || SHT_DYNSYM == sec->get_type() ) 
					{
						ELFIO::symbol_section_accessor symbols( reader, sec );
						ELFIO::Elf_Xword sym_no = symbols.get_symbols_num();
						if ( sym_no > 0 ) 
						{	
							for ( ELFIO::Elf_Xword i = 0; i < sym_no; ++i ) {
								std::string   name;
								ELFIO::Elf64_Addr value = 0;
								ELFIO::Elf_Xword size	= 0;
								unsigned char bind    	= 0;
								unsigned char type    	= 0;
								ELFIO::Elf_Half section = 0;
								unsigned char other   	= 0;
								symbols.get_symbol( i, name, value, size, bind, type,
													section, other );

								if (name == "begin_signature")
									begin_signature_at = value;
								if (name == "end_signature")
									end_signature_at = value;
							}
						}
					}
				}

				if(begin_signature_at == -1 || end_signature_at == -1)
				{
					throwError("SIG", "One or both of 'begin_signature' & 'end_signature' symbols missing from ELF symbol table: " + ifile + "\n", true);
				}

				// ========= dump data to signature file ==============
				std::vector<std::string> fcontents;
				printf("Dumping signature region [0x%08lx-0x%08lx]\n", begin_signature_at, end_signature_at);

				for(long int i=begin_signature_at; i<end_signature_at; i=i+4)
				{
					char temp [50];
					sprintf(temp, "%08x", b->mem->fetchWord(i));
					fcontents.push_back(temp);
				}
				fWrite(fcontents, signature_file);
			}
			#endif

			exit(EXIT_SUCCESS);
		}
		if(b->tb->m_tickcount > maxitr)
		{
			throwWarning("SIM0", "Simulation iterations exceeded maxitr("+std::to_string(maxitr)+")\n");
			exit(EXIT_SUCCESS);
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
    if (verbose_flag)
		std::cout << "AtomSim [" << AtomSimBackend << "]\n";

	// Initialize verilator
	Verilated::commandArgs(argc, argv);
	
	// Parse commandline arguments
	parse_commandline_args(argc, argv, ifile);

	// Create a new backend instance
	Backend bkend(ifile, default_mem_size);

	if(trace_enabled == true)
	{
		std::string tracefile = trace_dir;
		bkend.tb->openTrace(tracefile.c_str());
		std::cout << "Trace enabled : \"" << tracefile << "\" opened for output.\n";
		trace_enabled = true;
	}

	// Run simulation
	if(debug_mode)
	{
		std::string input;
		while(true)
		{
			if(bkend.done())	// if $finish encountered by verilator
			{
				end_simulation_reason = "$finish encountered";
				break;
			}

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
			#ifdef TARGET_ATOMBONES		// Atombones specific commands
			else if(token[0] == "mem")
			{
				if(token.size()<2)
					throwError("CMD0", "\"mem\" command expects address as argument\n");
				unsigned int addr = std::stoi(token[1]);
				printf("%08x : %02x %02x %02x %02x\n", addr, bkend.mem->fetchByte(addr),
				 bkend.mem->fetchByte(addr+1),bkend.mem->fetchByte(addr+2), bkend.mem->fetchByte(addr+3));
			}
			else if(token[0] == "dumpmem")
			{
				if(token.size()<2)
					throwError("CMD1", "\"dumpmem\" command expects filename as argument\n");
				
				// turn on verbose
				std::vector<std::string> fcontents;
				for(unsigned int i=0; i<bkend.mem->size-4; i+=4)
				{	
					char hex [30];
					sprintf(hex, "0x%08x\t:\t0x%08x", i, bkend.mem->fetchWord(i));
					fcontents.push_back(hex);
				}
				fWrite(fcontents, token[1]);
			}
			#endif
			else if(token[0] == "for")
			{
				// run for specified cycles
				if(token.size()<2)
					throwError("CMD2", "\"for\" command expects one argument\n");
				else
					tick(std::stoi(token[1]), &bkend);
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
						bkend.tb->openTrace(tracefile.c_str());
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
					bkend.tb->closeTrace();
					std::cout << "Trace disabled\n";
					trace_enabled = false;
				}
				else
					std::cout << "Trace was not enabled \n";
			}
			else
			{
				throwError("CMD4", "Unknown command \"" + token[0] + "\"\n");
			}
			input.clear();
		}
	}
	else
	{
		tick(-1, &bkend, false);
	}

	if(trace_enabled) // if trace file is open, close it before exiting
		bkend.tb->closeTrace();
	
	if (verbose_flag)
		std::cout << "Simulation ended @ tick " << bkend.tb->m_tickcount_total << " due to : " << end_simulation_reason << std::endl;
	exit(EXIT_SUCCESS);    
}
