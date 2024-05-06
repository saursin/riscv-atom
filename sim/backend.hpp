#pragma once

#include "testbench.hpp"
#include "util.hpp"
#include "except.hpp"

#include <string>

enum Regwidth_t {
    R8=8, 
    R16=16, 
    R32=32, 
    R64=64
};

struct ArchReg_t {
    std::string name;
    std::string alt_name;
    Regwidth_t width;
    void * ptr;
    bool is_arch_reg;
};

// Forward declaration
class Atomsim;

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

    Backend(Atomsim *sim_ptr);

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

    /**
     * @brief read register value                       [** MAY OVERRIDE **]
     * 
     * @param reg_id register ID
    */
    virtual uint64_t read_reg(const std::string name);

    /**
     * @brief write register value                      [** MAY OVERRIDE **]
     * 
     * @param reg_id register ID
     * @param buf value
    */
    virtual void write_reg(const std::string name, uint64_t value);

protected:
	/**
     * @brief Pointer to Atomsim object
     */
    Atomsim *sim_;

    /**
	 * @brief Pointer to testbench object
     * NOTE: To be initialized and deleted by child class
	 */
	Testbench<VTarget> *tb;

    /**
     * @brief Map or architectural registers
    */
    std::vector<ArchReg_t> regs_;

    friend class Atomsim;
};


template <class VTarget>
Backend<VTarget>::Backend(Atomsim *sim_ptr):
    sim_(sim_ptr)
{}

template <class VTarget>
void Backend<VTarget>::reset()
{
    tb->reset();
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

template <class VTarget>
uint64_t Backend<VTarget>::read_reg(const std::string name)
{
    if (regs_.size() == 0) 
        throw Atomsim_exception("Reading register value in current target is not supported");

    uint64_t val;
    for(auto it = regs_.begin(); it != regs_.end(); it++){
        if(it->name == name || it->alt_name == name) {
            switch(it->width){
                case R8:    val = *((uint64_t*)it->ptr) & 0xffULL; break;
                case R16:   val = *((uint64_t*)it->ptr) & 0xffffULL; break;
                case R32:   val = *((uint64_t*)it->ptr) & 0xffffffffULL; break;
                case R64:
                default:
                    val = *((uint64_t*)it->ptr); break;
            }
            return val;
        }
    }
    
    throw Atomsim_exception("Invalid register: " + name);
}

template <class VTarget>
void Backend<VTarget>::write_reg(const std::string name, uint64_t value)
{
    if (regs_.size() == 0) 
        throw Atomsim_exception("W`riting register value in current target is not supported");

    for(auto it = regs_.begin(); it != regs_.end(); it++){
        if(it->name == name || it->alt_name == name) {
            switch(it->width){
                case R8:    *((uint8_t*)it->ptr) = (uint8_t) (value & 0xffULL); break;
                case R16:   *((uint16_t*)it->ptr) = (uint16_t) (value & 0xffffULL); break;
                case R32:   *((uint32_t*)it->ptr) = (uint32_t) (value & 0xffffffffULL); break;
                case R64:
                default:
                    *((uint64_t*)it->ptr) = value; break;
            }
        }
    }

    throw Atomsim_exception("Invalid register: " + name);
}