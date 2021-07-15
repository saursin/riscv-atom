#ifndef __TESTBENCH_H__
#define __TESTBENCH_H__
#include <verilated_vcd_c.h>

/**
 * @brief TESTBENCH Class
 * 
 * @tparam VTop module to be instantiated
 */
template <class VTop>
class Testbench
{
	public:

	VTop 			* m_core = NULL;
	VerilatedVcdC	* m_trace = NULL;
	unsigned long 	m_tickcount = 0;     			// TickCounter to count clock cycles fom last reset
	unsigned long 	m_tickcount_total = 0;   		// TickCounter to count clock cycles

	/**
	 * @brief Construct a new TESTBENCH object
	 * 
	 */
	Testbench(void)                // Constructor: Instantiates a new VTop
    {
		m_core = new VTop;
		Verilated::traceEverOn(true);
		m_tickcount = 0l;
	}


	/**
	 * @brief Destroy the TESTBENCH object
	 * 
	 */
	virtual ~Testbench(void)       // Destructor 
    {
		delete m_core;
		m_core = NULL;
	}


	/**
	 * @brief Open/create a trace file
	 * 
	 * @param vcdname name of vcd file
	 */
	virtual	void openTrace(const char *vcdname) 
	{
		if (!m_trace) 
		{
			m_trace = new VerilatedVcdC;
			m_core->trace(m_trace, 99);
			m_trace->open(vcdname);
		}
	}


	/**
	 * @brief Close a trace file
	 * 
	 */
	virtual void closeTrace(void) 
	{
		if (m_trace) 
		{
			m_trace->close();
			m_trace = NULL;
		}
	}


	/**
	 * @brief Reset topmodule
	 * 
	 */
	virtual void reset(void) 
    {
		m_core-> rst_i = 1;
		this -> tick();	// Make sure any inheritance gets applied
		m_tickcount = 0;
		m_core-> rst_i = 0;
	}


	/**
	 * @brief Run for one cycle
	 * 
	 */
	virtual void tick(void) 
    {
		// Increment our own internal time reference
		m_tickcount++;
		m_tickcount_total++;

        // Make sure any combinatorial logic depending upon
		// inputs that may have changed before we called tick()
		// has settled before the rising edge of the clock.
		m_core -> clk_i = 0;
		m_core -> eval();

		//	Dump values to our trace file before clock edge
		if(m_trace) 
		{
			m_trace->dump(10*m_tickcount-2);
		}

		// ---------- Toggle the clock ------------

		// Rising edge
		m_core -> clk_i = 1;
		m_core -> eval();

		//	Dump values to our trace file after clock edge
		if(m_trace) m_trace->dump(10*m_tickcount);


		// Falling edge
		m_core -> clk_i = 0;
		m_core -> eval();


		if (m_trace) {
			// This portion, though, is a touch different.
			// After dumping our values as they exist on the
			// negative clock edge ...
			m_trace->dump(10*m_tickcount+5);
			//
			// We'll also need to make sure we flush any I/O to
			// the trace file, so that we can use the assert()
			// function between now and the next tick if we want to.
			m_trace->flush();
		}
	}


	/**
	 * @brief Check if simulation ended
	 * 
	 * @return true if verilator has encountered $finish
	 * @return false if verilator hasn't encountered $finish yet
	 */
	virtual bool  done(void)
	{
		return (Verilated::gotFinish()); 
	}
};
#endif // __TESTBENCH_H__