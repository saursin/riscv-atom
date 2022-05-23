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
    virtual uint64_t get_tickcount()        { return m_tickcount; }


    /**
     * @brief Get the total tickcounts
     * @return uint64_t ticks
     */
    virtual uint64_t get_total_tickcount()  { return m_tickcount_total; }

private:
	VTop 			* m_core = NULL;
	VerilatedVcdC	* m_trace = NULL;
	uint64_t        m_tickcount = 0;     			// TickCounter to count clock cycles fom last reset
	uint64_t        m_tickcount_total = 0;   		// TickCounter to count clock cycles

	friend class Backend_atomsim;
};



// Constructor: Instantiates a new VTop
template <class VTop>
Testbench<VTop>::Testbench(void)
{
    m_core = new VTop;
    Verilated::traceEverOn(true);
    m_tickcount = 0l;
}


// Destructor 
template <class VTop>
Testbench<VTop>::~Testbench(void)
{
    closeTrace();

    delete m_core;
    m_core = NULL;
}


template <class VTop>
void Testbench<VTop>::openTrace(const char *vcdname) 
{
    if (m_trace==NULL)
    {
        m_trace = new VerilatedVcdC;
        m_core->trace(m_trace, 99);
        m_trace->open(vcdname);
    }
}


template <class VTop>
void Testbench<VTop>::closeTrace(void) 
{
    if (m_trace!=NULL)
    {
        m_trace->close();
        delete m_trace;
    }
}


template <class VTop>
bool Testbench<VTop>::isTraceOpen()
{
    return (m_trace!=NULL);
}


template <class VTop>
void Testbench<VTop>::reset(void) 
{
    m_core-> rst_i = 1;
    this -> tick();	// Make sure any inheritance gets applied
    m_tickcount = 0;
    m_core-> rst_i = 0;
}


template <class VTop>
void Testbench<VTop>::tick(void) 
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


template <class VTop>
bool Testbench<VTop>::done(void)
{
    return (Verilated::gotFinish()); 
}

