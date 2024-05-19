#include "atomsim.hpp"
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
    int exitcode=EXIT_SUCCESS;

    // tick backend and update backend status, until ctrl+c is.
    try
    {
        // assume this; since simulation is just started
        bkend_running_ = true;

        // Should we be in debug mode at start?
        in_debug_mode_ = sim_config_.debug_flag;

        pending_steps = 0;

        init_interactive_mode();
        
        // *** Simulation Loop ***
        while (bkend_running_)
        {
            int breakpoint_hit = -1;

            // Evaluate breakpoints (early)
            for(int i=0; i<NUM_MAX_BREAKPOINTS; i++) {
                if(breakpoints_[i].active && breakpoints_[i].addr == backend_.read_reg("pc")){
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
                printf("Breakpoint %d hit %s0x%08lx%s\n", breakpoint_hit, ansicode(FG_BLUE), backend_.read_reg("pc"), ansicode(FG_RESET));
                in_debug_mode_ = true;
                pending_steps = 0;
            }

            // check ebreak
            if(backend_.read_reg("ir") == RV_INSTR_EBREAK) {
                printf("EBreak hit at %ld ticks, PC=%s0x%08lx%s\n", backend_.get_total_tick_count() , ansicode(FG_BLUE), backend_.read_reg("pc"), ansicode(FG_RESET));

                if(sim_config_.dump_on_ebreak_flag){  // For SCAR
                    // Temporarily redirect stdout to file
                    FILE *stream;
                    if((stream = freopen(sim_config_.dump_file.c_str(), "w", stdout)) == NULL)
                        Atomsim_exception("Cannot open dump file");

                    // call info command
                    cmd_info({"reg", "--no-alt-names"});
                    
                    // restore stdout
                    stream = freopen("/dev/tty", "w", stdout);
                    if(sim_config_.verbose_flag)
                        printf("State dumped to: %s\n", sim_config_.dump_file.c_str());
                }

                // ebreak hit while debug mode was enabled through cli
                if(sim_config_.debug_flag) {
                    // back to interactive mode
                    in_debug_mode_ = true;
                    pending_steps = 0;
                } else {
                    printf("Exiting..\n");
                    exitcode = EXIT_SUCCESS;
                    break;
                }
            }

            // check sim iterations
            if(backend_.get_total_tick_count() > sim_config_.maxitr) {
                throwError("SIM0", "Simulation iterations exceeded maxitr("+std::to_string(sim_config_.maxitr)+")\n");
                exitcode = EXIT_FAILURE;
                break;
            }
            
            // Enter interactive mode if we aren's stepping and we are already in debug mode or run mode
            // was interrupted by user (CTRL_C)
            Rcode rval = RC_NONE;
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
                    exitcode = EXIT_SUCCESS;
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

    // Exiting sim
    deinit_interactive_mode();

    return exitcode;
}
