#include "atomsim.hpp"
#include "simstate.hpp"
#include <iostream>

#include TARGET_HEADER

Atomsim::Atomsim(Atomsim_config sim_config, Backend_config bk_config):
    sim_config_(sim_config),
    backend_(this, bk_config)  // create backend
{   
    if (sim_config_.verbose_flag)
		std::cout << "AtomSim [" << backend_.get_target_name() << "]" << std::endl;

    // get input file disassembly   
    disassembly_ = getDisassembly(sim_config_.ifile);
    
    // Open trace if specified at CLI
    if (sim_config_.trace_flag)
    {
        backend_.open_trace(sim_config.trace_file.c_str());
        trace_opened_ = true;
        if (sim_config_.verbose_flag)
            std::cout << "Trace enabled : \"" << sim_config.trace_file << "\" opened for output.\n";
    }
}


void Atomsim::step()
{
    // tick backend and update backend status
    int rcode = backend_.tick();
    bkend_running_ = (rcode == 0) ? true : false;
}


int Atomsim::run()
{
    // tick backend and update backend status, until ctrl+c is.
    try
    {
        do
        {
            if(CTRL_C_PRESSED || sim_config_.debug_flag)
            {
                run_interactive_mode();
            }
            else
            {
                step();
            }
        }
        while (bkend_running_);
    }
    catch(std::exception &e)
    {
        std::cerr << "Runtime exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
