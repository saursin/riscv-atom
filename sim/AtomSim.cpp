#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <string>
#include <vector>


// Definitions

const unsigned int Sec = 1000000;
const unsigned int mSec = 1000;

// Simulation Parameters
unsigned int delay_amt = 100 * mSec; //default 

#include "defs.hpp"
#include "backend.hpp"


bool verbose_flag = false;
bool debug_mode = false;
std::string ifile = "";

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
        std::cerr << "!Error: too few arguments\n For help, try : atomsim --help\n";
        return true;
    }
    int i = 1;
    while(i < argc)
    {
        std::string argument = argv[i];

        // check if it is a flag
        if(argument[0] == '-')
        {
            if(argument == "-v")                                    // turn on verbose
            {
                verbose_flag = true;
                i++;
            }
			else if(argument == "-d")                                    // turn on verbose
            {
                debug_mode = true;
                i++;
            }
            else if(argument == "-h")                                    // show short help message
            {
                std::cout << Info_short_help_msg;
                return true;
            }
            else if(argument == "--help")                           // print long help message
            {
                std::cout << Info_long_help_msg;
                return true;
            }
            else if(argument == "--version")                        // print charon version info
            {
                std::cout << Info_version << std::endl << Info_copyright;
                return true;
            }
            else
            {
                std::cerr << "!Error: Unknown argument: " << argument << "\n";
                return true;
            }
        }
        else
        {
            // specify input files
            if(ifile != "")
            {
                std::cerr << "!ERROR: Multiple Input files povided\n";
                return true;
            }
            else
                ifile = argument;
            i++;
        }
    }

	if (ifile == "")
	{
		std::cerr << "!ERROR: No input file povided\n";
		return true;
	}
    return false;
}

void tick(long unsigned int cycles, Backend * b)
{
	for(long unsigned int i=0; i<cycles && !b->done(); i++)
	{
	 	b->refreshData();
	 	b->displayData();
	 	b->tick();
	}
}




int main(int argc, char **argv)
{
	if(parse_commandline_args(argc, argv))
		return 0;
	
	std::cout << "|=================================================== \n";
	std::cout << "|                   AtomSim v1.0\n";
	std::cout << "|=================================================== \n\n";
	std::cout << "  Author : Saurabh Singh (saurabh.s99100@gmail.com)\n\n";
	std::cout << "  File : "<< ifile <<"      Ready...\n\n";

	// Initialize verilator
	Verilated::commandArgs(argc, argv);	

	Backend bkend(ifile);

	std::string input;
	if(debug_mode)
	{
		while(true)
		{
			// Parse Input
			std::cout << ": ";
			getline(std::cin, input);
			
			std::vector<std::string> token;
			tokenize(input, token, ' ');

			//std::cout <<"[";
			//for(int i=0; i<token.size(); i++)
			//	std::cout <<" \""<< token[i] <<"\"";
			//std::cout <<" ]\n";

			if(token[0] == "q")
			{
				break;
			}
			else if(token[0] == "r")
			{
				bkend.reset();
			}
			else if(token[0] == "")
			{
				tick(2, &bkend);
			}
			else if(token[0] == "run")
			{
				if(token.size()<2)
					std::cout << "!ERROR: run expects one argument" <<"\n";
				else
					tick(std::stoi(token[1]), &bkend);
			}
			else
			{
				std::cout << "!ERROR: Unknown command" <<"\n";
			}

			input.clear();
		}
	}
	else
	{
		tick(-1, &bkend);
	}
	exit(EXIT_SUCCESS);    
}


