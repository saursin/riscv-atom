#include "atomsim.hpp"
#include "simstate.hpp"
#include <iostream>
#include "util.hpp"

#include TARGET_HEADER
#define RV_INSTR_EBREAK 0x100073

Atomsim::Atomsim(Atomsim_config sim_config, Backend_config bk_config):
    sim_config_(sim_config),
    backend_(this, bk_config)  // create backend
{   
    // get input file disassembly   
    getDisassembly(&disassembly_, sim_config_.ifile);

    // clear breakpoints
    for(int i=0; i<NUM_MAX_BREAKPOINTS; i++) {
        breakpoints_[i].active = false;
    }
    
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
        // assume this; since simulation is just started
        bkend_running_ = true;

        // Should we be in debug mode at start?
        in_debug_mode_ = sim_config_.debug_flag;

        pending_steps = 0;
        
        // *** Simulation Loop ***
        Rcode rval = RC_NONE;
        while (bkend_running_)
        {
            // Refresh state
            backend_.refresh_state();
            
            int breakpoint_hit = -1;

            // Evaluate breakpoints (early)
            for(int i=0; i<NUM_MAX_BREAKPOINTS; i++) {
                if(breakpoints_[i].active && breakpoints_[i].addr == simstate_.state_.pc_e){
                    breakpoint_hit = i;
                    break;
                }
            }

            // Display debug screen
            // if we are in debug mode OR we'll be in debug mode due to CTRL_C OR breakpoint occured
            if(in_debug_mode_ || CTRL_C_PRESSED || breakpoint_hit != -1) {
                display_dbg_screen();
            }
            
            if(breakpoint_hit != -1) {
                // Make sure we enter debug mode after breakpoint hit
                printf("Breakpoint %d hit %s0x%08x%s\n", breakpoint_hit, ansicode(FG_BLUE), simstate_.state_.pc_e, ansicode(FG_RESET));
                in_debug_mode_ = true;
                pending_steps = 0;
            }

            // check ebreak
            if(simstate_.state_.ins_e == RV_INSTR_EBREAK) {
                printf("EBreak hit at %s0x%08x%s\n", ansicode(FG_BLUE), simstate_.state_.pc_e, ansicode(FG_RESET));

                if(sim_config_.dump_on_ebreak_flag)  // For SCAR
                    simstate_.dump_simstate(sim_config_.dump_file);

                // ebreak hit while debug mode was enabled through cli
                if(sim_config_.debug_flag) {
                    // back to interactive mode
                    in_debug_mode_ = true;
                    pending_steps = 0;
                } else {
                    printf("Exiting..\n");
                    return 0;
                }
            }

            // check sim iterations
            if(simstate_.state_.tickcount_total > sim_config_.maxitr) {
                throwError("SIM0", "Simulation iterations exceeded maxitr("+std::to_string(sim_config_.maxitr)+")\n");
                return 1;
            }
            
            // Enter interactive mode if we aren's stepping and we are already in debug mode or run mode
            // was interrupted by user (CTRL_C)
            if((pending_steps == 0) && (in_debug_mode_ || CTRL_C_PRESSED)) {
                // explictly set: since we can also enter if CTRL_C_PRESSED
                in_debug_mode_ = true;
                pending_steps = 0;

                // run interactive mode
                rval = run_interactive_mode();

                // if we return, that's either for exiting simulation, single/multi steping, or 
                // switching to run mode
                if(rval == RC_STEP){
                    // Number of steps to be taken is set in pending_steps variable
                    // We just pass here
                } else if (rval == RC_RUN) {
                    /* code */
                    in_debug_mode_ = false;
                    CTRL_C_PRESSED = false;
                } else if (rval == RC_EXIT) {
                    // Exit sim
                    break;
                }
            }

            // Step
            this->step();
            
            // Decrement pending_steps if we are in Step mode
            if(rval == RC_STEP && pending_steps > 0) {
                pending_steps--;
            }
        }
    }
    catch(std::exception &e)
    {
        std::cerr << "Runtime exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
