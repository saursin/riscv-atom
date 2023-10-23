#pragma once

#include <string>
#include <vector>
// #include <memory>

#include TARGET_HEADER
#include "simstate.hpp"

enum Rcode{
    RC_NONE, RC_OK, RC_STEP, RC_RUN, RC_EXIT
};

#define NUM_MAX_BREAKPOINTS 16

// forward declarations
// class Backend_atomsim;
// class Simstate;
struct Backend_config;
struct DisassembledLine;

/**
 * @brief Configuration struct for Atomsim class
 */
struct Atomsim_config
{
    // CLI flags
    bool verbose_flag       = false;    // set verbosity
    bool debug_flag         = false;    // start in debug mode
    bool trace_flag         = false;    // enable tracing
    bool dump_on_ebreak_flag= false;    // Dump registers at ebreak (Used by SCAR)
    bool no_color_flag      = false;    // Disable colored output
    bool no_banner_flag     = false;    // Disable banner flag


    // input file
    std::string ifile       = "";
    
    unsigned long int maxitr    = 1000000;
    std::string trace_file      = "trace.vcd";  //  default loc: curr directory
    std::string dump_file       = "dump.txt";
    std::string signature_file  = "";

    bool print_info_topdown = true;
};

struct Breakpoint_t {
    bool active = false;
    uint32_t addr = 0;
};

class Atomsim
{
public:
    /**
     * @brief Construct a new Atomsim object
     * 
     * @param sim_config config struct for sim
     * @param bk_config config struct for backend
     */
    Atomsim(Atomsim_config sim_config, Backend_config bk_config);

    /**
     * @brief step simulation by a cycle
     */
    void step();

    /**
     * @brief run simulation until finished
     * @return int return code (non 0 if finished)
     */
    int run();

    /**
     * @brief enter interactive mode
     * 
     * @returns int return code [if 2: exit sim]
     */
    Rcode run_interactive_mode();
        

private:
    /**
     * @brief config struct object for sim
     */
    Atomsim_config sim_config_;

    /**
     * @brief Backend object
     */
    Backend_atomsim backend_;

    /**
     * @brief Middle-end object (cached state of backend)
     */
    Simstate simstate_;
    
    /**
     * @brief tracks if the VCD trace is opened
     */
    bool trace_opened_ = false;

    /**
     * @brief Keeps track of backend status
     */
    bool bkend_running_ = false;

    bool in_debug_mode_ = false;

    /**
     * @brief Disassembly of input file
     */
	std::map<uint32_t, DisassembledLine> disassembly_;

    /**
     * @brief Register ABI names used in debug display
     */
    const std::vector<std::string> reg_names_ = 
    {
        "x0  (zero) ",	"x1  (ra)   ",	"x2  (sp)   ",	"x3  (gp)   ",	"x4  (tp)   ",	"x5  (t0)   ",	"x6  (t1)   ",	"x7  (t2)   ",
        "x8  (s0/fp)",	"x9  (s1)   ",	"x10 (a0)   ",	"x11 (a1)   ",	"x12 (a2)   ",	"x13 (a3)   ",	"x14 (a4)   ",	"x15 (a5)   ",
        "x16 (a6)   ",	"x17 (a7)   ",	"x18 (s2)   ",	"x19 (s3)   ",	"x20 (s4)   ",	"x21 (s5)   ",	"x22 (s6)   ",	"x23 (s7)   ",
        "x24 (s8)   ",	"x25 (s9)   ",	"x26 (s10)  ",	"x27 (s11)  ",	"x28 (t3)   ",	"x29 (t4)   ",	"x30 (t5)   ",	"x31 (t6)   "
    };

    friend class Backend_atomsim;
    
    Breakpoint_t breakpoints_[NUM_MAX_BREAKPOINTS];

    // used to provide cycles for step command
    long long pending_steps = 0;

    // display debug screen
    void display_dbg_screen();

    // interactive Commands

    // General Commands
    Rcode cmd_help(const std::vector<std::string>&);
    Rcode cmd_quit(const std::vector<std::string>&);
    Rcode cmd_verbose(const std::vector<std::string>&);
    Rcode cmd_trace(const std::vector<std::string>&);
    
    // Control Commands
    Rcode cmd_reset(const std::vector<std::string>&);
    Rcode cmd_step(const std::vector<std::string>&);
    Rcode cmd_run(const std::vector<std::string>&);
    Rcode cmd_rst(const std::vector<std::string>&);
    Rcode cmd_while(const std::vector<std::string>&);
    Rcode cmd_break(const std::vector<std::string>&);
    Rcode cmd_info(const std::vector<std::string>&);

    // Query Commands
    Rcode cmd_reg(const std::vector<std::string>&);
    Rcode cmd_dereference_reg(const std::vector<std::string>&);
    Rcode cmd_pc(const std::vector<std::string>&);
    Rcode cmd_str(const std::vector<std::string>&);
    Rcode cmd_mem(const std::vector<std::string>&);
    Rcode cmd_dumpmem(const std::vector<std::string>&);
    Rcode cmd_load(const std::vector<std::string>&);
    
};

extern volatile bool CTRL_C_PRESSED;
