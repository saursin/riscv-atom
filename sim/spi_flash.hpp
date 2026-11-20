#pragma once
#include "spi_device.hpp"
#include <string>
#include <vector>

/**
 * @brief SPI NOR flash: the commands of a Winbond W25Q64 (8 MiB) that software here uses
 * @details Read (0x03), fast read (0x0B), JEDEC ID (0x9F), read status (0x05), write enable and
 * disable (0x06, 0x04), page program (0x02) and erase: 4 KiB sector (0x20), 64 KiB block (0xD8)
 * and chip (0xC7, 0x60). As on a real chip, programming only clears bits, a page program wraps
 * within its 256-byte page, and program and erase need a write enable first. Writes stay in
 * memory; the image file is only read.
 */
class SPIFlash: public SPIDevice
{
public:
    static const uint32_t SIZE = 8 << 20;

    /**
     * @brief Construct, bound to the SoC's SPI pins (see SPIDevice)
     * @param image file loaded at address 0 ("" for an erased chip)
     */
    SPIFlash(const uint8_t *sck, const uint8_t *mosi, const uint8_t *cs, unsigned cs_bit, uint8_t *miso,
             const std::string &image);

protected:
    void select() override;
    uint8_t transfer(uint8_t in) override;
    void deselect() override;

private:
    std::vector<uint8_t> mem_;
    bool wel_ = false;              // write enable latch
    uint8_t cmd_ = 0;               // command of this transaction
    unsigned n_ = 0;                // bytes received in this transaction
    uint32_t addr_ = 0;             // address being built, then the current address

    uint8_t data_byte(uint8_t in);  // a byte after the command and its address
};
