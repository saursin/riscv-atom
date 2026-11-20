#pragma once

#include "backend.hpp"
#include "device.hpp"
#include "VHydrogenSoC_sim.h"

#include <memory>
#include <vector>

#define ATOMSIM_TARGETNAME  "hydrogensoc"

struct Backend_config
{
    std::string vuart_portname  = "";
    uint32_t vuart_baudrate     = 115200;
    bool vuart_no_wait          = false;   // with a pseudo-terminal: start without waiting for a terminal
    bool enable_uart_dump       = false;
    int bootmode                = 1;    // Jump to RAM
    std::string spi_flash_image = "";   // attach an SPI flash with this image (SPI chip select 0)
    std::string sdcard_image    = "";   // attach an SD card with this image (SPI chip select 0)
};


class Backend_atomsim: public Backend<VHydrogenSoC_sim>
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

    int tick();
   
    void fetch(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz);

    void store(const uint32_t start_addr, uint8_t *buf, const uint32_t buf_sz);

private:
    /**
     * @brief Backend configuration parameters
     */
    Backend_config config_;

    /**
     * @brief Devices attached to the SoC's pins, evaluated every cycle (see device.hpp)
     */
    std::vector<std::unique_ptr<Device>> devices_;
};