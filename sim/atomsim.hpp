#pragma once

#include <string>
#include <vector>
// #include <memory>

#include TARGET_HEADER

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

    std::string ifile       = "";       // input file
    
    unsigned long int maxitr    = 10000000;     // max iterations
    std::string trace_file      =       // trace file, default location: current dir
#ifdef TRACE_FST
        "trace.fst";  
#else
        "trace.vcd";
#endif
    std::string dump_file       = "dump.txt";   // dump file, default location: current dir
    std::string signature_file  = "";           // signature file, default location: current dir

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
     * @brief run simulation until finished
     * @return int return code (non 0 if finished)
     */
    int run();

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


    friend class Backend_atomsim;
    
    Breakpoint_t breakpoints_[NUM_MAX_BREAKPOINTS];

    // used to provide cycles for step command
    long long pending_steps = 0;

    /**
     * @brief step simulation by a cycle
     */
    void step();

    /**
     * @brief initialize interactive mode
    */
    void init_interactive_mode();

    /**
     * @brief Deinitialize interactive mode
    */
    void deinit_interactive_mode();

    /**
     * @brief enter interactive mode
     */
    Rcode run_interactive_mode();


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
