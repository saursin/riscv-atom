#include "atomsim.hpp"
#include "backend.hpp"
#include "except.hpp"
#include "util.hpp"
#include "memory.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <math.h>
#include <map>

#include <readline/readline.h>
#include <readline/history.h>

#define DEBUG_PRINT_T2B
#define DEFAULT_DUMPMEM_PATH "memdump.txt"
#define DEFAULT_TRACEFILE_PATH "trace.vcd"
#define ATOMSIM_PROMPT "atomsim> "


void Atomsim::display_dbg_screen()
{
    // calculate change in PC.    
    unsigned int pc_change = simstate_.state_.pc_f - simstate_.state_.pc_e;

    bool isJump = simstate_.signals_.jump_decision;
    static bool wasJump = false;
    
    // Print debug screen
    std::cout << "-< " << backend_.get_total_tick_count() <<" >------------------------------------------------\n";
    printf("F  pc : 0x%08x  (%+d) <%s> \n", simstate_.state_.pc_f , pc_change, (isJump ? "jump": " ")); 
    
    #define STYLE_BOLD         "\033[1m"
    #define STYLE_NO_BOLD      "\033[22m"

    #define STYLE_UNDERLINE    "\033[4m"
    #define STYLE_NO_UNDERLINE "\033[24m"

    printf("E  ");

    printf(STYLE_BOLD);
    printf("pc : 0x%08x   ir : 0x%08x\n", simstate_.state_.pc_e, simstate_.state_.ins_e);
    printf(STYLE_NO_BOLD);
    
    std::cout << "[ " <<  ((this->disassembly_[ simstate_.state_.pc_e].instr==simstate_.state_.ins_e) ? this->disassembly_[simstate_.state_.pc_e].disassembly : "-" )<< " ]";

    if(wasJump)
        std::cout << " => nop (pipeline flush)";
    
    std::cout << "\n\n";
    wasJump = isJump;

    // Print Register File
    if(sim_config_.verbose_flag)
    {
        int cols = 2; // no of columns per rows
        #ifndef DEBUG_PRINT_T2B
        for(int i=0; i<32; i++)	// print in left-right fashion
        {
            printf("r%-2d: 0x%08x   ", i, simstate_.state_.rf[i]);
            if(i%cols == cols-1)
                printf("\n");
        }
        #else
        for(int i=0; i<32/cols; i++)	// print in topdown fashion
        {
            for(int j=0; j<cols; j++)
            {
                printf(" %s: 0x%08x  ", this->reg_names_[i+(32/cols)*j].c_str(), simstate_.state_.rf[i+(32/cols)*j]);
            }
            printf("\n");
        }
        #endif
    }
}


void _parse_line(const std::string s, std::string &cmd, std::vector<std::string> &args)
{
    std::stringstream ss(s);
    std::string tmp;

    // get command
    if (!(ss >> cmd))
    {
        return; // no command
    }
    
    // get args
    while (ss >> tmp)
        args.push_back(tmp);
}


/**
 * @brief Parse integer, hexadecimal or binary number from string
 * @param s string
 * @return int 
 */
long long _parse_num(std::string s)
{
    long long r;
    if(s.substr(0, 2) == "0x")			// Hex Number
        r = std::stoll(s, nullptr, 16);
    else if(s.substr(0, 2) == "0b")		// Binary Number
        r = std::stoll(s, nullptr , 2);
    else								// Decimal Number
        r = std::stoll(s, nullptr, 10);
    return r;
}

/**
 * @brief print hex dump of a memory buffer
 * 
 * @param buf mem buffer
 * @param bufsz buffer size
 * @param base_addr base address of buffer
 * @param enable_ascii enable ascii view 
 */
void _hexdump(const unsigned char *buf, size_t bufsz, uint32_t base_addr, bool enable_ascii=true) {
    const unsigned bytes_per_word = 4;
    const unsigned words_per_line = 4;

    for (int i = 0; i < bufsz; i += bytes_per_word * words_per_line) {
        // Print address
        printf("0x%08X: ", base_addr+i);

        // Print words in a line
        for (int j = 0; j < words_per_line; j++) {
            
            // print bytes in a word
            for (int k = 0; k < bytes_per_word; k++) {
                if (i + j * bytes_per_word + k < bufsz) {
                    printf("%02X", buf[i + j * bytes_per_word + k] & 0xFF);
                } else {
                    printf("  ");
                }
                if (k < bytes_per_word - 1) {
                    printf(" ");
                }
            }
            printf("  ");
        }

        // print ascii view
        if (enable_ascii) {
            printf(" | ");
            for (int j = 0; j < words_per_line; j++) {
                for (int k = 0; k < bytes_per_word; k++) {
                    char c = (i + j * bytes_per_word + k < bufsz) ? buf[i + j * bytes_per_word + k] : ' ';
                    printf("%c", (c >= 32 && c <= 126) ? c : '.');
                }
                printf(" ");
            }
        }
        printf("\n");
    }
}

int Atomsim::run_interactive_mode()
{
    typedef int (Atomsim::*interactive_func)(const std::vector<std::string>&);
    
    // construct look up table of functions
    std::map<std::string,interactive_func> funcs;

    funcs["h"] =    funcs["help"]       = &Atomsim::cmd_help;
    funcs["q"] =    funcs["quit"]       = &Atomsim::cmd_quit;
    funcs["v"] =    funcs["verbose"]    = &Atomsim::cmd_verbose;
                    funcs["trace"]      = &Atomsim::cmd_trace;

    funcs["s"] =    funcs["step"]       = &Atomsim::cmd_step;
    funcs["r"] =    funcs["run"]        = &Atomsim::cmd_run;
    funcs["w"] =    funcs["while"]      = &Atomsim::cmd_while;

    funcs["x"] =    funcs["reg"]        = &Atomsim::cmd_reg;
                    funcs["*reg"]       = &Atomsim::cmd_dereference_reg;
                    funcs["pc"]         = &Atomsim::cmd_pc;
                    funcs["str"]        = &Atomsim::cmd_str;
    funcs["m"] =    funcs["mem"]        = &Atomsim::cmd_mem;
                    funcs["dumpmem"]    = &Atomsim::cmd_dumpmem;
                    funcs["load"]       = &Atomsim::cmd_load;
  
    static std::string prev_cmd;
    static std::vector<std::string> prev_args;

    while(!backend_.done())
    {
        // get input
        char* raw_input = readline(ATOMSIM_PROMPT);

        // Handle Ctrl+D (EOF)
        if (!raw_input) {
            std::cout << std::endl;
            return ATOMSIM_RCODE_EXIT_SIM;
        }

        // convert to std::string
        std::string input(raw_input);
        free(raw_input);
        
        // parse input
        std::string cmd;
        std::vector<std::string> args;
        _parse_line(input, cmd, args);

        // preprocess: if input blank, use last command instead
        if(input=="")
        {
            // curr cmd <= last cmd
            cmd = prev_cmd;
            args = prev_args;
        }
        
        // prev <= current
        prev_cmd = cmd;
        prev_args = args;

        // Add to history
        if(input!=""){
            add_history(input.c_str());
        }

        // execute
        if (funcs.count(cmd)) // check if command exists
        {
            try
            {
                // Execute command
                int rval = (this->*funcs[cmd])(args);

                // analyze rval
                if(rval != ATOMSIM_RCODE_OK)
                    return rval;

            } catch(std::exception &e)
            {
                throwError("CMDERR", e.what(), false);
            }
        }
        else
        {
            std::cout << "Unknown command \"" << cmd << "\"" << std::endl;
        }
    }
    
    // control reaches here only if backend is done
    CTRL_C_PRESSED = false;
    return ATOMSIM_RCODE_EXIT_SIM;
}


int Atomsim::cmd_help(const std::vector<std::string>&)
{
    std::cout << 
    // -------------------------------- 80 coloumns ---------------------------------|
    "AtomSim Command Help  \n"
    "====================  \n"
    "*** General Commands ***\n"
    "  h, help                       : Show command help (this)\n"
    "  q, quit                       : Quit atomsim\n"
    "  v, verbose [\"on\"/\"off\"]       : set verbosity (toggle if ommitted)\n"
    "      trace <on> [filepath]   : Enable VCD tracing. (default: " DEFAULT_TRACEFILE_PATH ")\n"
    "            <off>               Disable VCD tracing\n"
    "\n"
    "*** Control commands ***\n"
    "  s, step [cycles]              : Step for specified cycles (default: 1)\n"
    "  r, run                        : Run until finished (press ctrl+c to return\n" 
    "                                  to console)\n"
    // "  w,  while reg [reg] [cond] [val]     : Run while value of [reg] [cond] [val] is true\n"
    // "            pc [cond] [val]               Run while value of PC [cond] [val] is true\n"
    // "            mem [cond] [hex addr] [val]   Run while value at address [hex addr] [cond] [val] is true\n"
    // "                                            [cond]: can be any one of {==, !=, <, >, <=, >=}\n"
    "\n"
    "*** Query Commands ***\n"
    // "  x,  reg [reg]                        : Display contents of [reg] (all if omitted)\n"
    // "      *reg [reg] (bytes)               : Display contents of memory at address stored in [reg]\n"
    // "                                           (bytes): number of bytes to display\n"
    // "      pc                               : Display current program counter value\n"
    // "      str [addr]                   : Show NUL-terminated C string at [hex addr]\n"
    "  m, mem <addr> [bytes] [flags]     : Show contents of memory at <addr>\n"
    "                                        addr: start address\n"
    "                                        bytes: number of bytes (default: 4)\n"
    "                                        flags: \n"
    "                                          -w : display word view\n"
    "                                          -a : display ascii view\n"
    "     dumpmem <addr> <bytes> <file>  : Dump contents of memory to a file\n"
    "                                        addr: start address\n"
    "                                        bytes: number of bytes\n"
    "                                        file: file path (default: " DEFAULT_DUMPMEM_PATH ")\n"
    "     load <file> <addr>            : Load contents of binary file into memory\n"
    "                                        fike: file path\n"
    "                                        addr: load address\n"
    "\n" 
    "Note:\n"
    "  - <> are used for compulsory and [] for optional arguments.\n"
    "  - while running, press ctrl+c to stop & return to console immediately.\n"
    "  - pressing enter repeats last used command.\n"
    "  - numeric arguments can be given in decimal, hexadecimal (prefix with 0x) \n"
    "    or binary (prefix with 0b)\n"
    << std::flush;
    return ATOMSIM_RCODE_OK;
}


int Atomsim::cmd_quit(const std::vector<std::string> &/*args*/)
{
    return ATOMSIM_RCODE_EXIT_SIM;
}


int Atomsim::cmd_verbose(const std::vector<std::string> &args)
{
    if(args.size() == 0)   // argument omitted: toggle verbosity
    {
        sim_config_.verbose_flag = !sim_config_.verbose_flag;
        std::cout << "verbose: " << sim_config_.verbose_flag << std::endl;
    }
    else if(args.size() == 1)
    {
        if(args[0] == "on")
            sim_config_.verbose_flag = true;
        else if(args[0] == "off")
            sim_config_.verbose_flag = false;
        else 
            throw Atomsim_exception("arg can only be on/off");
    }
    else
        throw Atomsim_exception("too few/many args\n verbose commands expect one argument (on/off)");
    
    return ATOMSIM_RCODE_OK;
}


int Atomsim::cmd_trace(const std::vector<std::string> &args)
{
    if (args.size() < 1)
        throw Atomsim_exception("trace command expects \"on\"/\"off\" as 1st argument");
    else
    {
        if(args[0] == "on")
        {
            if(sim_config_.trace_flag)
            {
                std::cout << "Trace already enabled" << std::endl;
                return ATOMSIM_RCODE_OK;
            }
            
            // enable trace
            std::string tracefile = DEFAULT_TRACEFILE_PATH;
            if(args.size() >= 2)
                tracefile = args[1];

            backend_.open_trace(tracefile);
            std::cout << "Trace enabled: \"" << tracefile << "\" opened for VCD output.\n";
            sim_config_.trace_flag = true;
        }
        else if(args[0] == "off")
        {
            if(!sim_config_.trace_flag)
            {
                std::cout << "Trace was not enabled" << std::endl;
                return ATOMSIM_RCODE_OK;
            }

            // disable trace
            backend_.close_trace();
            std::cout << "Trace disabled\n";
            sim_config_.trace_flag = false;

        }
        else
            throw Atomsim_exception("1st arg can be only be \"on\"/\"off\"");
    }
    return ATOMSIM_RCODE_OK;
}


int Atomsim::cmd_step(const std::vector<std::string> &args)
{
    if(args.size() == 0)   // step 1
    {
        this->step();
    }
    else if(args.size() == 1)  // step n
    {
        long long cycles = _parse_num(args[0]);
        if (cycles < 0)
            throw Atomsim_exception("cycles out of bounds\n");
        for(long long c = 0; c < cycles; c++)
            this->step();
    }
    else
        throw Atomsim_exception("too few/many args\n");
    
    return ATOMSIM_RCODE_OK;
}


int Atomsim::cmd_run(const std::vector<std::string> &/*args*/)
{
    // exit interactive mode & enter normal mode
    return ATOMSIM_RCODE_EXIT;

    // CTRL_C_PRESSED = false;
    // while(!CTRL_C_PRESSED)
    //     this->step();

    // return ATOMSIM_RCODE_OK;
}


int Atomsim::cmd_while(const std::vector<std::string> &/*args*/)
{
    std::cout << "command not implemented" << std::endl;
    return ATOMSIM_RCODE_OK;
}


int Atomsim::cmd_reg(const std::vector<std::string> &/*args*/)
{
    std::cout << "command not implemented" << std::endl;
    return ATOMSIM_RCODE_OK;
}


int Atomsim::cmd_dereference_reg(const std::vector<std::string> &/*args*/)
{
    std::cout << "command not implemented" << std::endl;
    return ATOMSIM_RCODE_OK;
}


int Atomsim::cmd_pc(const std::vector<std::string> &/*args*/)
{
    std::cout << "command not implemented" << std::endl;
    return ATOMSIM_RCODE_OK;
}


int Atomsim::cmd_str(const std::vector<std::string> &/*args*/)
{
    std::cout << "command not implemented" << std::endl;
    return ATOMSIM_RCODE_OK;
}


int Atomsim::cmd_mem(const std::vector<std::string> &args)
{
    if(args.size() == 0)
        throw Atomsim_exception("\"mem\" command expects address as 1st argument\n");
    else if(args.size() == 1 || args.size() == 2 || args.size() == 3 || args.size() == 4)     //  base address, num bytes, coloumns
    {
        long long addr;
        long long size = 4;
        bool asciiview = false;

        // parse base address
        addr = _parse_num(args[0]);
        if (addr < 0 || addr > UINT32_MAX)
            throw Atomsim_exception("addr out of bounds\n");

        // parse fetch size
        if(args.size() >= 2)
        {
            size = _parse_num(args[1]);          
            if (size < 0 || size > UINT32_MAX)
                throw Atomsim_exception("size out of bounds\n");
        }


        for(unsigned i=2; i<args.size(); i++)
        {
            if (args[i] == "-a")
                asciiview = true;
            else
                throw Atomsim_exception("invlid arg \""+args[i]+"\"\n");
        }
        
        uint8_t buf [size];
        backend_.fetch(addr, buf, size);

        _hexdump(buf, size, addr, asciiview);
    }
    else
        throw Atomsim_exception("too many arguments for \"mem\" command\n");
    return ATOMSIM_RCODE_OK;
}


int Atomsim::cmd_dumpmem(const std::vector<std::string> &args)
{
    if(args.size() < 1)
        throw Atomsim_exception("expected address as 1st argument\n");
    if(args.size() < 2)
        throw Atomsim_exception("expected size as 2nd argument\n");
    
    long long addr = _parse_num(args[0]);
    if (addr < 0 || addr > UINT32_MAX)
        throw Atomsim_exception("addr out of bounds\n");

    long long size = _parse_num(args[1]);
    if (size < 0 || size > UINT32_MAX)
        throw Atomsim_exception("size out of bounds\n");
    
    std::string fpath = DEFAULT_DUMPMEM_PATH;
    if(args.size() > 2)
        fpath = std::string(args[2]);
       
    uint8_t buf [size];
    backend_.fetch(addr, buf, size);

        
    FILE * fptr = fopen(fpath.c_str(), "w");
    
    if(!fptr)
        throw Atomsim_exception("cant open/create filen\n");
    
    FILE * tmp = stdout;    // preserve the original stdout
    stdout = fptr; // redirect stdout file
    _hexdump(buf, size, addr, true);
    stdout = tmp; // restore stdout
    fclose(fptr);
    
    return ATOMSIM_RCODE_OK;
}


int Atomsim::cmd_load(const std::vector<std::string> &args)
{
    if(args.size() < 1)
        throw Atomsim_exception("expected filename as 1st argument\n");
    if(args.size() < 2)
        throw Atomsim_exception("expected address as 2nd argument\n");
    
    std::string file_path = args[0];
    uint32_t addr = _parse_num(args[1]);

    // read file
    std::ifstream file(file_path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        throw Atomsim_exception("Unable to open file " + file_path);
    }

    // Get the file size
    file.seekg(0, std::ios_base::end);
    uint32_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read the file into the buffer & close
    uint8_t buffer [file_size];
    file.read(reinterpret_cast<char*>(buffer), file_size);
    file.close();

    printf("Loading %d bytes from \"%s\" @ 0x%08x\n", file_size, file_path.c_str(), addr);

    // write to memory
    backend_.store(addr, buffer, file_size);
    return ATOMSIM_RCODE_OK;
}
