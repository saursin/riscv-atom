#pragma once

#include <string>
#include <vector>
// #include <memory>

#include TARGET_HEADER
#include "simstate.hpp"

// forward declarations
// class Backend_atomsim;
// class Simstate;
struct Backend_config;

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

    // input file
    std::string ifile       = "";
    
    unsigned long int maxitr    = 1000000;
    std::string trace_file      = "";
    std::string dump_file       = "";
    std::string signature_file  = "";

    bool print_info_topdown = true;
};


class Atomsim
{
public:
    Atomsim(Atomsim_config sim_config, Backend_config bk_config);

    void step();

    int run();

    void run_interactive_mode();
        
    // void displayData();


private:
    Atomsim_config sim_config_;

    // Back-end
    Backend_atomsim backend_;

    // Middle-end
    Simstate simstate_;
    


    
    bool trace_opened = false;
    bool is_running = false;
    
    /**
     * @brief Register ABI names used in debug display
     */
    const std::vector<std::string> reg_names = 
    {
        "x0  (zero) ",	"x1  (ra)   ",	"x2  (sp)   ",	"x3  (gp)   ",	"x4  (tp)   ",	"x5  (t0)   ",	"x6  (t1)   ",	"x7  (t2)   ",
        "x8  (s0/fp)",	"x9  (s1)   ",	"x10 (a0)   ",	"x11 (a1)   ",	"x12 (a2)   ",	"x13 (a3)   ",	"x14 (a4)   ",	"x15 (a5)   ",
        "x16 (a6)   ",	"x17 (a7)   ",	"x18 (s2)   ",	"x19 (s3)   ",	"x20 (s4)   ",	"x21 (s5)   ",	"x22 (s6)   ",	"x23 (s7)   ",
        "x24 (s8)   ",	"x25 (s9)   ",	"x26 (s10)  ",	"x27 (s11)  ",	"x28 (t3)   ",	"x29 (t4)   ",	"x30 (t5)   ",	"x31 (t6)   "
    };

    friend class Backend_atomsim;
    
    // display debug screen
    void display_dbg_screen();

    // interactive Commands

    // General Commands
    void cmd_help(const std::string&, const std::vector<std::string>&);
    void cmd_quit(const std::string&, const std::vector<std::string>&);
    void cmd_verbose(const std::string&, const std::vector<std::string>&);
    void cmd_trace(const std::string&, const std::vector<std::string>&);
    
    // Control Commands
    void cmd_step(const std::string&, const std::vector<std::string>&);
    void cmd_run(const std::string&, const std::vector<std::string>&);
    void cmd_rst(const std::string&, const std::vector<std::string>&);
    void cmd_while(const std::string&, const std::vector<std::string>&);

    // Query Commands
    void cmd_reg(const std::string&, const std::vector<std::string>&);
    void cmd_dereference_reg(const std::string&, const std::vector<std::string>&);
    void cmd_pc(const std::string&, const std::vector<std::string>&);
    void cmd_str(const std::string&, const std::vector<std::string>&);
    void cmd_mem(const std::string&, const std::vector<std::string>&);
    void cmd_dumpmem(const std::string&, const std::vector<std::string>&);

    
};

extern volatile bool CTRL_C_PRESSED;
