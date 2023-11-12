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
{   // calculate change in PC.    
    unsigned int pc_change = simstate_.state_.pc_f - simstate_.state_.pc_e;

    // Check if it's a jump
    bool isJump = simstate_.signals_.jump_decision;

    // Fetch Values   
    uint64_t tickcount = backend_.get_total_tick_count();

    const int disam_width = 39;
    std::string disam = (disassembly_[simstate_.state_.pc_e].instr == simstate_.state_.ins_e) ? disassembly_[simstate_.state_.pc_e].disassembly : "_";
    if(disam.length() > disam_width) {
        disam.resize(disam_width-3);
        disam += " ..";
    }
    else if (disam.length()< disam_width)
        disam.append(disam_width-disam.length(), ' ');
    
    //////////////////////////////////////////////////////////////////////////////////////////
    // Print debug screen
    if(!sim_config_.verbose_flag){
        printf("[%10ld] PC: 0x%08x, IR: 0x%08x, %s%s%s\n", tickcount, simstate_.state_.pc_e, simstate_.state_.ins_e, ansicode(FG_BLUE), disam.c_str(), ansicode(FG_RESET));
        return;
    }

    printf("┌─[%10ld]─────────────────────────────────────────────┐\n", tickcount);
    printf("│ %sPC: 0x%08x%s%s    PC_f: 0x%08x     (%+10d%s)%s   │\n", ansicode(S_BOLD), simstate_.state_.pc_e, ansicode(SN_BOLD), 
                                                    ansicode(S_DIM), simstate_.state_.pc_f, pc_change, (isJump ? ", J": "   "), ansicode(SN_DIM));
    printf("│ IR: 0x%08x    %s%s%s│\n", simstate_.state_.ins_e, ansicode(FG_CYAN), disam.c_str(), ansicode(FG_RESET));
    printf("└──────────────────────────────────────────────────────────┘\n");

    // Print Register File
    int cols = 2; // no of columns per rows
    #ifndef DEBUG_PRINT_T2B
    for(int i=0; i<32; i++)	// print in left-right fashion
    {
        printf("r%-2d: 0x%08x    ", i, simstate_.state_.rf[i]);
        if(i%cols == cols-1)
            printf("\n");
    }
    #else
    for(int i=0; i<32/cols; i++)	// print in topdown fashion
    {
        for(int j=0; j<cols; j++)
        {
            printf("  %s: 0x%08x  ", this->reg_names_[i+(32/cols)*j].c_str(), simstate_.state_.rf[i+(32/cols)*j]);
        }
        printf("\n");
    }
    #endif

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

    for (size_t i = 0; i < bufsz; i += bytes_per_word * words_per_line) {
        // Print address
        printf("0x%08lx: ", base_addr+i);

        // Print words in a line
        for (unsigned j = 0; j < words_per_line; j++) {
            
            // print bytes in a word
            for (unsigned k = 0; k < bytes_per_word; k++) {
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
            for (unsigned j = 0; j < words_per_line; j++) {
                for (unsigned k = 0; k < bytes_per_word; k++) {
                    char c = (i + j * bytes_per_word + k < bufsz) ? buf[i + j * bytes_per_word + k] : ' ';
                    printf("%c", (c >= 32 && c <= 126) ? c : '.');
                }
                printf(" ");
            }
        }
        printf("\n");
    }
}

Rcode Atomsim::run_interactive_mode()
{
    typedef Rcode (Atomsim::*interactive_func)(const std::vector<std::string>&);
    
    // construct look up table of functions
    std::map<std::string,interactive_func> funcs;

    funcs["h"] =    funcs["help"]       = &Atomsim::cmd_help;
    funcs["q"] =    funcs["quit"]       = &Atomsim::cmd_quit;
    funcs["v"] =    funcs["verbose"]    = &Atomsim::cmd_verbose;
                    funcs["trace"]      = &Atomsim::cmd_trace;

                    funcs["reset"]      = &Atomsim::cmd_reset; 
    funcs["s"] =    funcs["step"]       = &Atomsim::cmd_step;
    funcs["r"] =    funcs["run"]        = &Atomsim::cmd_run;
    funcs["w"] =    funcs["while"]      = &Atomsim::cmd_while;
    funcs["b"] =    funcs["break"]      = &Atomsim::cmd_break;
    funcs["i"] =    funcs["info"]       = &Atomsim::cmd_info;


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
            return RC_EXIT;
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
                Rcode rval = (this->*funcs[cmd])(args);

                // If not RC_OK, caller needs to handle it and get back
                if(rval != RC_OK)
                    return rval;

            } catch(std::exception &e)
            {
                throwError("CMDERR", e.what(), false);
            }
        }
        else
        {
            if(cmd != "")
                std::cout << "Unknown command \"" << cmd << "\"" << std::endl;
        }
    }
    
    // control reaches here only if backend is done
    CTRL_C_PRESSED = false;
    return RC_EXIT;
}


Rcode Atomsim::cmd_help(const std::vector<std::string>&)
{
    std::cout << 
    // -------------------------------- 80 coloumns ---------------------------------|
    "AtomSim Command Help  \n"
    "====================  \n"
    "*** General Commands ***\n"
    "  h, help                      : Show command help (this)\n"
    "  q, quit                      : Quit atomsim\n"
    "  v, verbose [\"on\"/\"off\"]  : set verbosity (toggle if ommitted)\n"
    "     trace <on> [filepath]     : Enable VCD tracing.\n"
    "                                 (default: " DEFAULT_TRACEFILE_PATH ")\n"
    "           <off>               : Disable VCD tracing\n"
    "\n"
    "*** Control commands ***\n"
    "     reset                      : Reset\n"
    "  s, step [cycles]              : Step for specified cycles (default: 1)\n"
    "  r, run                        : Run until finished (press ctrl+c to return\n" 
    "                                  to console)\n"
    "  b, break [addr]               : Set breakpoint at given address\n"
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
    "  i, info [something]              : Show information about something\n"
    "                                     Display dbg screen if no arg provided \n"
    "                                       b|break:    show all breakpoints\n"
    "                                       r|reg:      show all registers\n"
    // "                                       s|symbols:  show all symbols\n"
    "  m, mem <addr> [bytes] [flags]    : Show contents of memory at <addr>\n"
    "                                       addr: start address\n"
    "                                       bytes: number of bytes (default: 4)\n"
    "                                       flags: \n"
    "                                           -a : display ascii view\n"
    "     dumpmem <addr> <bytes> <file> : Dump contents of memory to a file\n"
    "                                        addr:  start address\n"
    "                                        bytes: number of bytes\n"
    "                                        file:  file path (default: " DEFAULT_DUMPMEM_PATH ")\n"
    "     load <file> <addr>            : Load contents of binary file into memory\n"
    "                                        file: file path\n"
    "                                        addr: load address\n"
    "\n" 
    "Note:\n"
    "  - <> are used for compulsory and [] for optional arguments.\n"
    "  - while running, press ctrl+c to stop & return to console immediately.\n"
    "  - pressing enter repeats last used command.\n"
    "  - numeric arguments can be given in decimal, hexadecimal (prefix with 0x) \n"
    "    or binary (prefix with 0b)\n"
    << std::flush;
    return RC_OK;
}


Rcode Atomsim::cmd_quit(const std::vector<std::string> &/*args*/)
{
    return RC_EXIT;
}


Rcode Atomsim::cmd_verbose(const std::vector<std::string> &args)
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
    
    return RC_OK;
}


Rcode Atomsim::cmd_trace(const std::vector<std::string> &args)
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
                return RC_OK;
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
                return RC_OK;
            }

            // disable trace
            backend_.close_trace();
            std::cout << "Trace disabled\n";
            sim_config_.trace_flag = false;

        }
        else
            throw Atomsim_exception("1st arg can be only be \"on\"/\"off\"");
    }
    return RC_OK;
}

Rcode Atomsim::cmd_reset(const std::vector<std::string> &/*args*/)
{
    std::cout << "Resetting..." << std::endl;
    backend_.reset();
    return RC_OK;
}

Rcode Atomsim::cmd_step(const std::vector<std::string> &args)
{
    if(args.size() == 0)   // step 1
    {
        pending_steps = 1;
        return RC_STEP;
    }
    else if(args.size() == 1)  // step n
    {
        long long cycles = _parse_num(args[0]);
        if (cycles < 0)
            throw Atomsim_exception("cycles out of bounds\n");
        pending_steps = cycles;
        return RC_STEP;
    }
    else
        throw Atomsim_exception("too few/many args\n");
    
    return RC_OK;
}


Rcode Atomsim::cmd_run(const std::vector<std::string> &/*args*/)
{
    // exit interactive mode & enter normal mode
    return RC_RUN;
}


Rcode Atomsim::cmd_while(const std::vector<std::string> &args)
{
    std::cout << "command not implemented" << std::endl;
    return RC_OK;
}

Rcode Atomsim::cmd_break(const std::vector<std::string> &args)
{
    if(args.size() == 0)   // step 1
    {
        throw Atomsim_exception("too few/many args\n");
    }
    else if(args.size() == 1)  // step n
    {
        // find empty slot and set breakpoint
        int i=0;
        for(i=0; i<NUM_MAX_BREAKPOINTS; i++){
            if(breakpoints_[i].active == false) {
                breakpoints_[i].addr = _parse_num(args[0]);
                breakpoints_[i].active = true;
                break;
            }
        }
        if(i == NUM_MAX_BREAKPOINTS)
            Atomsim_exception("Cannot set breakpoint, max used\n");
     
        printf("Breakpoint %d at %s0x%08x%s\n", i, ansicode(FG_BLUE), breakpoints_[i].addr, ansicode(FG_RESET));
    }
    else
        throw Atomsim_exception("too few/many args\n");
    
    return RC_OK;
}

Rcode Atomsim::cmd_info(const std::vector<std::string> &args)
{
    if(args.size() == 0)   // step 1
    {
        display_dbg_screen();
    }
    else if(args.size() == 1)  // step n
    {
        if(args[0] == "b" || args[0] == "break") {
            // show breakpoints
            printf("Num  Address\n");
            for(int i=0; i<NUM_MAX_BREAKPOINTS; i++){
                if (breakpoints_[i].active) {
                    printf("%3d  %s0x%08x%s\n", i, ansicode(FG_BLUE), breakpoints_[i].addr, ansicode(FG_RESET));
                }
            }
        } else if (args[0] == "r" || args[0] == "reg") {
            printf(" %s:    0x%08x\n", "pc         ", simstate_.state_.pc_e);
            for(unsigned i=0; i<32; i++) {
                printf(" %s:    0x%08x\n", reg_names_[i].c_str(), simstate_.state_.rf[i]);
            }
        }
    }
    else
        throw Atomsim_exception("too few/many args\n");
    return RC_OK;
}


Rcode Atomsim::cmd_reg(const std::vector<std::string> &/*args*/)
{
    std::cout << "command not implemented" << std::endl;
    return RC_OK;
}


Rcode Atomsim::cmd_dereference_reg(const std::vector<std::string> &/*args*/)
{
    std::cout << "command not implemented" << std::endl;
    return RC_OK;
}


Rcode Atomsim::cmd_pc(const std::vector<std::string> &/*args*/)
{
    std::cout << "command not implemented" << std::endl;
    return RC_OK;
}


Rcode Atomsim::cmd_str(const std::vector<std::string> &/*args*/)
{
    std::cout << "command not implemented" << std::endl;
    return RC_OK;
}


Rcode Atomsim::cmd_mem(const std::vector<std::string> &args)
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
    return RC_OK;
}


Rcode Atomsim::cmd_dumpmem(const std::vector<std::string> &args)
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
        throw Atomsim_exception("can't open/create file\n");
    
    FILE * tmp = stdout;    // preserve the original stdout
    stdout = fptr; // redirect stdout file
    _hexdump(buf, size, addr, true);
    stdout = tmp; // restore stdout
    fclose(fptr);
    
    return RC_OK;
}


Rcode Atomsim::cmd_load(const std::vector<std::string> &args)
{
    if(args.size() < 1)
        throw Atomsim_exception("expected filename as 1st argument\n");
    if(args.size() < 2)
        throw Atomsim_exception("expected address as 2nd argument\n");
    
    std::string file_path = args[0];
    uint32_t addr = _parse_num(args[1]);

    std::vector<char> fcontents = fReadBin(file_path);
    
    printf("Loading %ld bytes from \"%s\" @ 0x%08x\n", fcontents.size(), file_path.c_str(), addr);

    // write to memory
    backend_.store(addr, (uint8_t*)fcontents.data(), fcontents.size());
    return RC_OK;
}
