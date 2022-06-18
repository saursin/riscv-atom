#include "atomsim.hpp"
#include "simstate.hpp"
#include <iostream>

#include TARGET_HEADER

Atomsim::Atomsim(Atomsim_config sim_config, Backend_config bk_config):
    sim_config_(sim_config),
    backend_(this, bk_config)  // create backend
{   
    if (sim_config_.verbose_flag)
		std::cout << "AtomSim [" << backend_.getTargetName() << "]" << std::endl;

    // get input file disassembly   
    disassembly = getDisassembly(sim_config_.ifile);
    
    // Open trace if specified at CLI
    if (sim_config_.trace_flag)
    {
        backend_.openTrace(sim_config.trace_file.c_str());
        trace_opened = true;
        if (sim_config_.verbose_flag)
            std::cout << "Trace enabled : \"" << sim_config.trace_file << "\" opened for output.\n";
    }
}


void Atomsim::step()
{
    backend_.tick();
}


int Atomsim::run()
{
    try
    {
        while (!backend_.done())
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
    }
    catch(std::exception &e)
    {
        std::cerr << "Runtime exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}




















// void Atomsim::displayData()
// {
//     // calculate change in PC.
//     unsigned int pc_change = simstate_->state_.pc_f - simstate_->state_.pc_e;

//     // check if jumped
//     bool isJump = simstate_->signals_.jump_decision;
//     static bool wasJump = false;
    
//     // Print debug screen
//     std::cout << "-< " << simstate_->state_.tickcount_total <<" >------------------------------------------------\n";
//     printf("F  pc : 0x%08x  (%+d) <%s> \n", simstate_->state_.pc_f , pc_change, (isJump ? "jump": " ")); 
    
//     #define STYLE_BOLD         "\033[1m"
//     #define STYLE_NO_BOLD      "\033[22m"

//     #define STYLE_UNDERLINE    "\033[4m"
//     #define STYLE_NO_UNDERLINE "\033[24m"

//     printf("E  ");

//     printf(STYLE_BOLD);
//     printf("pc : 0x%08x   ir : 0x%08x\n", simstate_->state_.pc_e , simstate_->state_.ins_e);
//     printf(STYLE_NO_BOLD);
    
//     //std::cout << "[ " <<  ((disassembly[state.pc_e].instr==state.ins_e) ? disassembly[state.pc_e].disassembly : "-" )<< " ]";

//     if(wasJump)
//         std::cout << " => nop (pipeline flush)";
    
//     std::cout << "\n\n";
//     wasJump = isJump;

//     // Print Register File
//     if(sim_config_.verbose_flag)
//     {
//         int cols = 2; // no of columns per rows
//         if(sim_config_.print_info_topdown)
//         {
//             for(int i=0; i<32; i++)	// print in left-right fashion
//             {
//                 printf("r%-2d: 0x%08x   ", i, simstate_->state_.rf[i]);
//                 if(i%cols == cols-1)
//                     printf("\n");
//             }
//         }
//         else
//         {
//             for(int i=0; i<32/cols; i++)	// print in topdown fashion
//             {
//                 for(int j=0; j<cols; j++)
//                 {
//                     printf(" %s: 0x%08x  ", reg_names[i+(32/cols)*j].c_str(), simstate_->state_.rf[i+(32/cols)*j]);
//                 }
//                 printf("\n");
//             }
//         }
//     }
// }