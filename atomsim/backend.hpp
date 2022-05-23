#pragma once

#include <string>

#include "util.hpp"
#include "testbench.hpp"

// Forward declaration
template <class VTop> class Testbench;
class Atomsim;
class Simstate;

/**
 * @brief Backend class
 * Backend class encapsulates low level querying & control of the RTL
 */
template <class VTarget>
class Backend
{
public:

    Backend(Atomsim *sim_ptr, Simstate *simstate_ptr);
    ~Backend();

    /**
     * @brief Get the Target Name
     * @return std::string 
     */
    virtual std::string getTargetName() = 0;


	/**
	 * @brief reset the backend
	 */
	void reset();


	/**
	 * @brief probe all internal signals and registers and 
	 * update state of middle-end
	 */
	virtual void refresh_state() = 0;


    /**
     * @brief Dump contents of memory into a file
     * 
     * @param file dumpfile
     * @param strt_addr starting address
     * @param end_addr end address
     */
    void dumpmem(std::string file, uint32_t strt_addr, uint32_t end_addr);


    /**
     * @brief Get contents of a memory location
     */
    uint32_t getMemContents(uint32_t addr);


	/**
	 * @brief Tick for one cycle
	 */
	int tick();


	/**
	 * @brief check if simulation is done
	 * @return true 
	 * @return false 
	 */
	bool done();


    /**
     * @brief Open trace file 
     * @param file 
     */
    void openTrace(std::string file);


    /**
     * @brief Open trace file 
     * @param file 
     */
    void closeTrace();

protected:
	/**
     * @brief Pointer to Atomsim object
     */
    Atomsim *sim_;

    /**
     * @brief Pointer to middle-end
     */
    Simstate *simstate_;

    /**
	 * @brief Pointer to testbench object
	 */
	Testbench<VTarget> *tb;  
};


template <class VTarget>
Backend<VTarget>::Backend(Atomsim *sim_ptr, Simstate *simstate_ptr):
    sim_(sim_ptr),
    simstate_(simstate_ptr)
{
    tb = new Testbench<VTarget>();
}

template <class VTarget>
Backend<VTarget>::~Backend()
{
    delete tb;
}

template <class VTarget>
void Backend<VTarget>::reset()
{
    tb->reset();
}

template <class VTarget>
void Backend<VTarget>::dumpmem(std::string file, uint32_t strt_addr, uint32_t end_addr)
{
    throwError("", "Memory dumps not supported in current target");
}

template <class VTarget>
uint32_t Backend<VTarget>::getMemContents(uint32_t addr)
{
    throwError("", "Viewing memory content not supported in current target", true);
    return 0;
}


template <class VTarget>
int Backend<VTarget>::tick()
{
    if(done())
        return 1;

    tb->tick();
    return 0;
}


template <class VTarget>
bool Backend<VTarget>::done()
{
    return tb->done();
}


template <class VTarget>
void Backend<VTarget>::openTrace(std::string file)
{
    tb->openTrace(file.c_str());
}

template <class VTarget>
void Backend<VTarget>::closeTrace()
{
    tb->closeTrace();
}