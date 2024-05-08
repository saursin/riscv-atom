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
#define ATOMSIM_HISTORY_FILE ".atomsim_history"
#define ATOMSIM_HISTORY_LENGTH 1000

#define DEBUG_SCREEN_RF_COLS 2

void Atomsim::display_dbg_screen()
{   
    uint64_t tickcount = backend_.get_total_tick_count();
    uint64_t pc = backend_.read_reg("pc");
    uint64_t ir = backend_.read_reg("ir");

    std::string disasm = trimstr((disassembly_[pc].instr == ir) ? disassembly_[pc].disassembly : "_", 40);

    ////////////////////////////////////////////////////////////////////////////
    // Non verbose debug screen
    if(!sim_config_.verbose_flag){
        printf("[%10ld] PC: 0x%08lx, IR: 0x%08lx, %s%s%s\n", tickcount, pc, ir, ansicode(FG_BLUE), disasm.c_str(), ansicode(FG_RESET));
        return;
    }
    
    ////////////////////////////////////////////////////////////////////////////
    // Verbose debug screen
    static uint64_t last_pc = 0;
    int64_t pc_change = pc - last_pc;
    last_pc = pc;
    
    printf("┌─[%10ld]─────────────────────────────────────────────┐\n", tickcount);
    printf("│ %sPC: 0x%08lx%s  %s%-+15ld%s                          │\n", ansicode(S_BOLD), pc, ansicode(SN_BOLD), 
                                                                        ansicode(S_DIM), pc_change, ansicode(SN_DIM));
    printf("│ IR: 0x%08lx  %s%s%s │\n", ir, ansicode(FG_CYAN), disasm.c_str(), ansicode(FG_RESET));
    printf("└──────────────────────────────────────────────────────────┘\n");

    // Print architecture registers in DEBUG_SCREEN_RF_COLS columns
    cmd_info({"reg", "-a", "-c", std::to_string(DEBUG_SCREEN_RF_COLS)});
}

void print_reg(ArchReg_t reg, bool append_alt_name){
    printf("%-10s : ", (reg.name + ((append_alt_name && reg.alt_name != "") ? (" ("+reg.alt_name+")") : "")).c_str());
    switch(reg.width){
        case R8:    printf("%02x",   (uint8_t)  (*((uint64_t*)reg.ptr) & 0xffULL)); break;
        case R16:   printf("%04x",   (uint16_t) (*((uint64_t*)reg.ptr) & 0xffffULL)); break;
        case R32:   printf("%08x",   (uint32_t) (*((uint64_t*)reg.ptr) & 0xffffffffULL)); break;
        case R64:   printf("%016lx", (uint64_t) (*((uint64_t*)reg.ptr))); break;
        default:
            printf("%16s", " - ");
        break;
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

void Atomsim::init_interactive_mode(){
    // init
    using_history();

    // Set history list length
    stifle_history(ATOMSIM_HISTORY_LENGTH);

    // read history file
    read_history(ATOMSIM_HISTORY_FILE);
}


void Atomsim::deinit_interactive_mode(){
    // write history file
    write_history(ATOMSIM_HISTORY_FILE);
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

        // Add input to history if it's != previous input
        if(cmd != prev_cmd){
            add_history(input.c_str());
        }

        // prev <= current
        prev_cmd = cmd;
        prev_args = args;

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


Rcode Atomsim::cmd_while(const std::vector<std::string> &/*args*/)
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
    if(args.size() == 0)
    {
        display_dbg_screen();
    }
    else if(args.size() >= 1)
    {
        if(args[0] == "b" || args[0] == "break") {
            // show breakpoints
            printf("Num  Address\n");
            for(int i=0; i<NUM_MAX_BREAKPOINTS; i++){
                if (breakpoints_[i].active) {
                    printf("%3d  %s0x%08x%s\n", i, ansicode(FG_BLUE), breakpoints_[i].addr, ansicode(FG_RESET));
                }
            }
        }
        else if(args[0] == "r" || args[0] == "reg") {
            // Print registers
            bool arch_regs_only = false;
            unsigned cols=1;
            bool no_alt_names = false;
            std::string regname;
            for(unsigned i=1; i<args.size(); i++){
                if (args[i] == "-a") {
                    arch_regs_only = true;
                }
                else if (args[i] == "-c") {
                    cols = std::atoi(args[i+1].c_str());   // FIXME: unprotected
                    i++;
                }
                else if (args[i] == "--no-alt-names") {
                    no_alt_names=true;
                }
                else {
                    regname = args[i];
                }
            }

            std::vector<ArchReg_t> regs;
            if(regname.length()) {
                // If regname provided, search for it
                auto it = backend_.regs_.begin();
                for(; it != backend_.regs_.end(); it++) {
                    if(it->name == regname || it->alt_name == regname) {
                        regs.push_back(*it); break;
                    }
                }

                if(it == backend_.regs_.end()) {
                    throw Atomsim_exception("Invalid register: "+ regname);
                }
            }
            else {
                // Show all regs
                regs = backend_.regs_;
            }

            // Filter out non arch registers
            if(arch_regs_only) {
                regs.erase(std::remove_if(regs.begin(), regs.end(), [](const ArchReg_t& r) {
                        return !r.is_arch_reg;
                    }), regs.end());
            }
            
            unsigned nregs = regs.size();
            unsigned nregs_per_col = ceil((float)nregs/(float)cols);

            for(unsigned i=0; i<nregs_per_col; i++){
                for(unsigned j=0; j<cols; j++) {
                    if(i+nregs_per_col*j >= nregs) break;

                    auto reg = regs[i+nregs_per_col*j];
                    // regname
                    print_reg(reg, !no_alt_names);

                    printf("     ");
                }
                printf("\n");
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
