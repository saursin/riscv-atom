#pragma once
#include "spi_device.hpp"
#include <deque>
#include <string>
#include <vector>

/**
 * @brief SD card in SPI mode: the commands the libcatom SD driver uses
 * @details A high-capacity card (block addressing, 512-byte blocks): CMD0 (go idle), CMD8 (interface
 * condition, echoes the check pattern), CMD55 + ACMD41 (initialize), CMD58 (OCR), CMD17 (read a
 * block) and CMD24 (write a block); others get "illegal command". CRCs are not checked, as in SPI
 * mode by default. The capacity is the image size rounded up to a whole block. Writes stay in
 * memory; the image file is only read.
 */
class SDCard: public SPIDevice
{
public:
    static const uint32_t BLOCK = 512;

    /**
     * @brief Construct, bound to the SoC's SPI pins (see SPIDevice)
     * @param image the card's contents
     */
    SDCard(const uint8_t *sck, const uint8_t *mosi, const uint8_t *cs, unsigned cs_bit, uint8_t *miso,
           const std::string &image);

protected:
    void select() override;
    uint8_t transfer(uint8_t in) override;

private:
    enum State {
        COMMAND,        // receiving a command frame
        WRITE_TOKEN,    // CMD24: waiting for the start token
        WRITE_DATA      // CMD24: receiving the block and its CRC
    };

    std::vector<uint8_t> mem_;
    std::deque<uint8_t> out_;       // bytes to send, in order (0xFF when empty)
    State state_ = COMMAND;
    uint8_t frame_[6];              // command frame: 0x40|cmd, 4-byte argument, CRC
    unsigned n_ = 0;                // bytes of the frame received
    bool idle_ = true;              // in the idle state (not yet initialized by ACMD41)
    bool app_ = false;              // the previous command was CMD55: this one is an application command
    uint32_t waddr_ = 0;            // CMD24: byte address of the block
    unsigned wn_ = 0;               // CMD24: bytes received (block + 2 CRC bytes)

    void command(uint8_t cmd, uint32_t arg);
    uint8_t r1(uint8_t err = 0) { return (idle_ ? 0x01 : 0x00) | err; }
};
