#pragma once

#include "backend.hpp"
#include "build/verilated/VHydrogenSoC.h"

#include <memory>

#define ATOMSIM_TARGETNAME  "hydrogensoc"
#define TARGET_HYDROGENSOC

class Vuart;
class BitbangUART;

struct Backend_config
{
    std::string vuart_portname  = "";
    uint32_t vuart_baudrate     = 115200;
    bool enable_uart_dump       = false;
    int bootmode                = 1;    // Jump to RAM
};


class Backend_atomsim: public Backend<VHydrogenSoC>
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
    std::string get_target_name() { return ATOMSIM_TARGETNAME; }


	void refresh_state();

    void UART();

    int tick();
   
    void fetch(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz);

    void store(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz);

private:
    /**
     * @brief Backend configuration parameters
     */
    Backend_config config_;

    /**
	 * @brief Pointer to Vuart object
	 */
	Vuart *vuart_ = nullptr;

    /**
     * @brief Are we using vuart?
     */
    bool using_vuart_ = false;

    /**
     * @brief UART btbang drver
     */
    BitbangUART *bb_uart_;
};