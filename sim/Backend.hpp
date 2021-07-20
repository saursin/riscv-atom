#ifndef __BACKEND_HPP__
#define __BACKEND_HPP__

#include <stdint.h>
#include <map>
#include <string>
#include "Testbench.hpp"

/**
 * @brief Backend class
 * Backend class encapsulates the data 
 * probing and printing operations
 */
template <class VTarget>
class Backend
{
	public:
	/**
	 * @brief Pointer to testbench object
	 */
	Testbench<VTarget> *tb;

	/**
	 * @brief Struct to hold CPU state
	 */
    struct CPUState
    {
        // Fetch-Stage
        unsigned int pc_f;
        unsigned int ins_f;

        // Execute Stage
        unsigned int pc_e;
        unsigned int ins_e;
        
        // Register File
        unsigned int rf[32];
    } state;

    /**
     * @brief Struct to hold CPU signal values
     */
    struct CPUSignals
    {
        bool jump_decision;
    } signals;

    /**
     * @brief Disassembly of input file
     */
	std::map<uint32_t, std::string> disassembly;

    private:
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

    public:
	/**
	 * @brief reset the backend
	 */
	void reset()
    {
        if(verbose_flag)
            std::cout << "Resetting..\n";
        tb->reset();
    }

	/**
	 * @brief probe all internal signals and registers and 
	 * update backend state
	 */
	virtual void refreshData() = 0;

	/**
	 * @brief Display state data on console
	 */
	void displayData()
    {
        // calculate change in PC.
        unsigned int pc_change = state.pc_f - state.pc_e;

        bool isJump = signals.jump_decision;
        static bool wasJump = false;
        
        // Print debug screen
        std::cout << "-< " << tb->m_tickcount_total <<" >------------------------------------------------\n";
        printf("F  pc : 0x%08x  (%+d) <%s> \n", state.pc_f , pc_change, (isJump ? "jump": " ")); 
        
        #define STYLE_BOLD         "\033[1m"
        #define STYLE_NO_BOLD      "\033[22m"

        #define STYLE_UNDERLINE    "\033[4m"
        #define STYLE_NO_UNDERLINE "\033[24m"

        printf("E  ");

        printf(STYLE_BOLD);
        printf("pc : 0x%08x   ir : 0x%08x\n", state.pc_e , state.ins_e); 
        printf(STYLE_NO_BOLD);
        
        std::cout << "[ " <<  disassembly[state.pc_e] << " ]";

        if(wasJump)
            std::cout << " => nop (pipeline flush)";
        
        std::cout << "\n\n";
        wasJump = isJump;

        // Print Register File
        if(verbose_flag)
        {
            int cols = 2; // no of columns per rows
            #ifndef DEBUG_PRINT_T2B
            for(int i=0; i<32; i++)	// print in left-right fashion
            {
                printf("r%-2d: 0x%08x   ", i, state.rf[i]);
                if(i%cols == cols-1)
                    printf("\n");
            }
            #else
            for(int i=0; i<32/cols; i++)	// print in topdown fashion
            {
                for(int j=0; j<cols; j++)
                {
                    printf(" %s: 0x%08x  ", reg_names[i+(32/cols)*j].c_str(), state.rf[i+(32/cols)*j]);
                }
                printf("\n");
            }
            #endif
        }
    }

	/**
	 * @brief Tick for one cycle
	 * 
	 */
	void tick()
    {
        tb->tick();
    }

	/**
	 * @brief check if simulation is done
	 * 
	 * @return true 
	 * @return false 
	 */
	bool done()
    {
        return tb->done();
    }
};

#endif //__BACKEND_HPP__