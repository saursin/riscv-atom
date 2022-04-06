#pragma once

#include "util.h"
#include "testbench.h"

// Forward declaration
template <class VTop> class Testbench;

/**
 * @brief Backend class
 * Backend class encapsulates low level querying & control of the RTL
 */
template <class VTarget>
class Backend
{
public:
    /**
     * @brief Get the Target Name
     * @return std::string 
     */
    virtual std::string getTargetName() = 0;


	/**
	 * @brief reset the backend
	 */
	void reset()
    {
        tb->reset();
    }


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
    void dumpmem(std::string file, uint32_t strt_addr, uint32_t end_addr)
    {
        throwError("", "Memory dumps not supported in current target");
    }


    /**
     * @brief Get contents of a memory location
     */
    uint32_t getMemContents(uint32_t addr)
    {
        throwError("", "Viewing memory content not supported in current target", true);
        return 0;
    }


	/**
	 * @brief Tick for one cycle
	 */
	void tick()
    {
        tb->tick();
    }


	/**
	 * @brief check if simulation is done
	 * @return true 
	 * @return false 
	 */
	bool done()
    {
        return tb->done();
    }


    /**
     * @brief Open trace file 
     * @param file 
     */
    void openTrace(std::string file)
    {
        tb->openTrace(file.c_str());
    }


    /**
     * @brief Open trace file 
     * @param file 
     */
    void closeTrace()
    {
        tb->closeTrace();
    }

protected:
	/**
	 * @brief Pointer to testbench object
	 */
	Testbench<VTarget> *tb;
};