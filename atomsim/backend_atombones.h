#pragma once

#include "backend.h"
#include "build/verilated/VAtomBones.h"

#include <string>
#include <map>
#include <memory>

#define ATOMSIM_TARGETNAME  "atombones"
#define TARGET_ATOMBONES

struct Backend_config
{
    uint32_t imem_offset    = 0x00000000;
    uint32_t imem_size_kb   = (64*1024);    // default: 64 MB
    
    uint32_t dmem_offset    = 0x04000000;
    uint32_t dmem_size_kb   = (64*1024);    // default: 64 MB

    bool using_vuart            = false;
    std::string vuart_portname  = "";
    uint32_t vuart_baudrate     = 9600;
};


class Atomsim;
class Simstate;
class Memory;
class Vuart;

class Backend_atomsim: public Backend<VAtomBones>
{
public:
    /**
     * @brief Construct a new Backend object
     */
    Backend_atomsim(Atomsim * sim, Backend_config config);

    /**
	 * @brief Destroy the Backend object
	 */
	~Backend_atomsim();

    /**
     * @brief Get the Target Name
     * @return std::string 
     */
    std::string getTargetName() { return ATOMSIM_TARGETNAME; }

	void serviceMemoryRequest();

	void refresh_state();

    void UART();

    int tick();

    void dumpmem(std::string file, uint32_t strt_addr, uint32_t end_addr);

private:
    /**
     * @brief Backend configuration parameters
     */
    Backend_config config_;

    /**
     * @brief Map of pointers to memory devices
     */
    std::map<std::string, std::shared_ptr<Memory>> mem_;

    /**
	 * @brief Pointer to Vuart object
	 */
	Vuart *vuart_ = nullptr;

    /**
     * @brief Are we using vuart?
     */
    bool using_vuart_ = false;
};