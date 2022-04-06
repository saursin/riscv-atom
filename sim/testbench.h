#pragma once

#include <verilated_vcd_c.h>
#include <stdint.h>

/**
 * @brief TESTBENCH Class
 * 
 * @tparam VTop module to be instantiated
 */
template <class VTop>
class Testbench
{
public:

	/**
	 * @brief Construct a new TESTBENCH object
	 * 
	 */
	Testbench(void);


	/**
	 * @brief Destroy the TESTBENCH object
	 * 
	 */
	virtual ~Testbench(void);


	/**
	 * @brief Open/create a trace file
	 * 
	 * @param vcdname name of vcd file
	 */
	virtual	void openTrace(const char *vcdname);


	/**
	 * @brief Close a trace file
	 * 
	 */
	virtual void closeTrace(void);


	/**
	 * @brief Check if trace is open or not
	 */
	virtual bool isTraceOpen();


	/**
	 * @brief Reset topmodule
	 * 
	 */
	virtual void reset(void);


	/**
	 * @brief Run for one cycle
	 * 
	 */
	virtual void tick(void);


	/**
	 * @brief Check if simulation ended
	 * 
	 * @return true if verilator has encountered $finish
	 * @return false if verilator hasn't encountered $finish yet
	 */
	virtual bool  done(void);


    /**
     * @brief Get the tickcount (since last reset)
     * @return uint64_t ticks
     */
    uint64_t get_tickcount()        { return m_tickcount; }


    /**
     * @brief Get the total tickcounts
     * @return uint64_t ticks
     */
    uint64_t get_total_tickcount()  { return m_tickcount_total; }

private:
	VTop 			* m_core = NULL;
	VerilatedVcdC	* m_trace = NULL;
	uint64_t        m_tickcount = 0;     			// TickCounter to count clock cycles fom last reset
	uint64_t        m_tickcount_total = 0;   		// TickCounter to count clock cycles

	friend class Backend_atomsim;
};
