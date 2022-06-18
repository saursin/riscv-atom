#include "atomsim.hpp"
#include "backend.hpp"
#include "except.hpp"

#include <iostream>
#include <sstream>
#include <map>

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
    
    // std::cout << "[ " <<  ((disassembly[state.pc_e].instr==state.ins_e) ? disassembly[state.pc_e].disassembly : "-" )<< " ]";

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
                printf(" %s: 0x%08x  ", reg_names[i+(32/cols)*j].c_str(), simstate_.state_.rf[i+(32/cols)*j]);
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

void Atomsim::run_interactive_mode()
{
    typedef void (Atomsim::*interactive_func)(const std::string &cmd, const std::vector<std::string>&);
    
    // construct look up table of functions
    std::map<std::string,interactive_func> funcs;

    funcs["h"] =    funcs["help"]       = &Atomsim::cmd_help;
    funcs["q"] =    funcs["quit"]       = &Atomsim::cmd_quit;
    funcs["v"] =    funcs["verbose"]    = &Atomsim::cmd_verbose;
                    funcs["trace"]      = &Atomsim::cmd_trace;

    funcs["s"] =    funcs["step"]       = &Atomsim::cmd_step;
    funcs["r"] =    funcs["run"]        = &Atomsim::cmd_run;
                    funcs["rst"]        = &Atomsim::cmd_rst;
    funcs["w"] =    funcs["while"]      = &Atomsim::cmd_while;

    funcs["x"] =    funcs["reg"]        = &Atomsim::cmd_reg;
                    funcs["*reg"]       = &Atomsim::cmd_dereference_reg;
                    funcs["pc"]         = &Atomsim::cmd_pc;
                    funcs["str"]        = &Atomsim::cmd_str;
    funcs["m"] =    funcs["mem"]        = &Atomsim::cmd_mem;
                    funcs["dumpmem"]    = &Atomsim::cmd_dumpmem;

    static std::string last_input;

    
    while(!backend_.done())
    {
        std::string input;

        // get input
        std::cout << ": ";
        getline(std::cin, input);
        
        // parse input
        std::string cmd;
        std::vector<std::string> args;
        _parse_line(input, cmd, args);

        // execute
        try
        {
            if (funcs.count(cmd))
                (this->*funcs[cmd])(cmd, args);
            else
                std::cout << "Unknown command " << cmd << std::endl;
        } catch(std::exception& e) 
        {
            std::cout << "Bad or missing arguments for command: " << e.what() << cmd << std::endl;
        }
    }
    
    CTRL_C_PRESSED = false;
}


void Atomsim::cmd_help(const std::string&, const std::vector<std::string>&)
{
    std::cout << 
    "AtomSim Command Help  \n"
    "====================\n"
    "\n"
    "*** General Commands ***\n"
    "  h,  help                    : Show command help\n"
    "  q,  quit                    : Exit atomsim\n"
    "  v,  verbose [on]            : set verbosity on\n"
    "              [off]             set verbosity off\n"
    "      trace [on] (file)       : Enable VCD tracing; filename: (file)\n"
    "            [off]               Disable VCD tracing\n"
    "\n"
    "*** Control commands ***\n"
    "  s,  step [cycles]                     : Step for specified [cycles] (1 if omitted)\n"
    "  r,  run                               : Run until finished\n"
    "      rst                               : Reset simulation\n"
    "  w,  while reg [reg] [cond] [val]      : Run while value of [reg] [cond] [val] is true\n"
    "            pc [cond] [val]               Run while value of PC [cond] [val] is true\n"
    "            mem [cond] [hex addr] [val]   Run while value at address [hex addr] [cond] [val] is true\n"
    "                                            [cond]: can be any one of {==, !=, <, >, <=, >=}\n"
    "\n"
    "*** Query Commands ***\n"
    "  x,  reg [reg]                   : Display contents of [reg] (all if omitted)\n"
    "      *reg [reg] (bytes)          : Display contents of memory at address stored in [reg]\n"
    "                                      (bytes): number of bytes to display\n"
    "      pc                          : Display current program counter value\n"
    "      str [hex addr]              : Show NUL-terminated C string at [hex addr]\n"
    "  m,  mem [hex addr] (bytes)      : Show contents of memory at [hex addr]\n"
    "                                      (bytes): number of bytes to display\n"
    "      dumpmem [hex addr] (bytes)  : Dump contents of memory to a file\n"
    "                                      (bytes): number of bytes to dump   \n"
    "\n" 
    "Note:\n"
    "    - [] are used for compulsory and () for optional argument.\n"
    "    - while running, press ctrl+c to stop & return to console immediately\n"
    "    - pressing enter repeats last used command\n"
    << std::flush;
}


void Atomsim::cmd_quit(const std::string &cmd, const std::vector<std::string> &args)
{
    exit(EXIT_SUCCESS);         // FIXME: replace with a proper way of exiting
}


void Atomsim::cmd_verbose(const std::string &cmd, const std::vector<std::string> &args)
{
    if (args.size() != 1)
        throw Atomsim_exception("too few/many args");
    
    if(args[0] == "on")
        sim_config_.verbose_flag = true;
    else if(args[0] == "off")
        sim_config_.verbose_flag = false;
    else 
        throw Atomsim_exception("arg can be on/off");
}


void Atomsim::cmd_trace(const std::string &cmd, const std::vector<std::string> &args)
{
    std::cout << "command not implemented" << std::endl;
}


void Atomsim::cmd_step(const std::string &cmd, const std::vector<std::string> &args)
{
    step();
}


void Atomsim::cmd_run(const std::string &cmd, const std::vector<std::string> &args)
{
    std::cout << "command not implemented" << std::endl;
}


void Atomsim::cmd_rst(const std::string &cmd, const std::vector<std::string> &args)
{
    std::cout << "command not implemented" << std::endl;
}


void Atomsim::cmd_while(const std::string &cmd, const std::vector<std::string> &args)
{
    std::cout << "command not implemented" << std::endl;
}


void Atomsim::cmd_reg(const std::string &cmd, const std::vector<std::string> &args)
{
    std::cout << "command not implemented" << std::endl;
}


void Atomsim::cmd_dereference_reg(const std::string &cmd, const std::vector<std::string> &args)
{
    std::cout << "command not implemented" << std::endl;
}


void Atomsim::cmd_pc(const std::string &cmd, const std::vector<std::string> &args)
{
    std::cout << "command not implemented" << std::endl;
}


void Atomsim::cmd_str(const std::string &cmd, const std::vector<std::string> &args)
{
    std::cout << "command not implemented" << std::endl;
}


void Atomsim::cmd_mem(const std::string &cmd, const std::vector<std::string> &args)
{
    std::cout << "command not implemented" << std::endl;
}


void Atomsim::cmd_dumpmem(const std::string &cmd, const std::vector<std::string> &args)
{
    std::cout << "command not implemented" << std::endl;
}

