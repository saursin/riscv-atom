#pragma once

#include "testbench.hpp"
#include "util.hpp"
#include "except.hpp"

#include <string>


// Forward declaration
class Atomsim;
class Simstate;

/**
 * @brief Backend class
 * Backend class encapsulates low level querying & control of the RTL
 * 
 * - Methods marked with [** OVERRIDE **] need to be overridden by 
 *   child class, these are responsible for critical functions to work 
 *   correctly.
 * 
 * - Methods marked with [** MAY OVERRIDE **] may/may not be overriden, 
 *   these are only used for additional functionality.
 */
template <class VTarget>
class Backend
{
public:

    Backend(Atomsim *sim_ptr, Simstate *simstate_ptr);

    /**
     * @brief Get the Target Name                       [** OVERRIDE **]
     * @return std::string 
     */
    virtual std::string get_target_name() = 0;

	/**
	 * @brief reset the backend
	 */
	void reset();

	/**
	 * @brief probe all internal signals and registers and 
	 * update state of middle-end                       [** OVERRIDE **]
	 */
	virtual void refresh_state() = 0;

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
    void open_trace(std::string file);

    /**
     * @brief Open trace file 
     * @param file 
     */
    void close_trace();

    /**
     * @brief Get the total tick count from tb
     * @return uint64_t 
     */
    uint64_t get_total_tick_count();

    /**
     * @brief Get the tick count from tb
     * @return uint64_t 
     */
    uint64_t get_tick_count();

    /**
     * @brief fetch bytes from target memory            [** MAY OVERRIDE **]
     * 
     * @param start_addr starting address
     * @param buf byte buffer 
     * @param buf_sz buffer size
     */
    virtual void fetch(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz);

    /**
     * @brief store bytes to target memory              [** MAY OVERRIDE **]
     * 
     * @param start_addr starting address
     * @param buf byte buffer 
     * @param buf_sz buffer size
     */
    virtual void store(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz);

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
     * NOTE: To be initialized and deleted by child class
	 */
	Testbench<VTarget> *tb;
};


template <class VTarget>
Backend<VTarget>::Backend(Atomsim *sim_ptr, Simstate *simstate_ptr):
    sim_(sim_ptr),
    simstate_(simstate_ptr)
{}

template <class VTarget>
void Backend<VTarget>::reset()
{
    tb->reset();
    // tb->m_core->eval();

    // Update simstate
    refresh_state();
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
void Backend<VTarget>::open_trace(std::string file)
{
    tb->openTrace(file.c_str());
}

template <class VTarget>
void Backend<VTarget>::close_trace()
{
    tb->closeTrace();
}

template <class VTarget>
uint64_t Backend<VTarget>::get_total_tick_count()
{
    return tb->get_total_tickcount();
}

template <class VTarget>
uint64_t Backend<VTarget>::get_tick_count()
{
    return tb->get_tickcount();
}

template <class VTarget>
void Backend<VTarget>::fetch(const uint32_t /*start_addr*/, uint8_t */*buf*/, const uint32_t /*buf_sz*/)
{
    throw Atomsim_exception("fetching from current target's memory is not supported");
}

template <class VTarget>
void Backend<VTarget>::store(const uint32_t /*start_addr*/, uint8_t */*buf*/, const uint32_t /*buf_sz*/)
{
    throw Atomsim_exception("storing to current target's memory is not supported");
}