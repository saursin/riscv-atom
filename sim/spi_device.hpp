#pragma once
#include "device.hpp"
#include <cstdint>

/**
 * @brief A device on the SPI bus: the bit level (chip select, SCK edges, MOSI and MISO)
 * @details SPI mode 0 (SCK idles low; data is sampled on the rising edge and changes on the
 * falling edge), MSB first, chip select active low. MISO changes only on falling edges, as the
 * SoC samples it at the end of each bit. A device drives MISO only while selected, so several
 * can share the bus; the backend pulls MISO high before evaluating them. Subclasses see only
 * bytes, through select(), transfer() and deselect().
 */
class SPIDevice: public Device
{
public:
    /**
     * @brief Construct, bound to the SoC's SPI pins
     * @param sck SCK (SoC output)
     * @param mosi MOSI (SoC output)
     * @param cs chip select lines (SoC output), active low
     * @param cs_bit this device's line in cs
     * @param miso MISO (SoC input)
     */
    SPIDevice(const uint8_t *sck, const uint8_t *mosi, const uint8_t *cs, unsigned cs_bit, uint8_t *miso);

    void eval() override;

protected:
    /**
     * @brief Chip select asserted: a new transaction starts
     */
    virtual void select() {}

    /**
     * @brief A byte was received
     * @param in byte from the SoC
     * @return the byte to send during the next byte
     */
    virtual uint8_t transfer(uint8_t in) = 0;

    /**
     * @brief Chip select released: the transaction ends
     */
    virtual void deselect() {}

private:
    const uint8_t *sck_, *mosi_, *cs_;
    unsigned cs_bit_;
    uint8_t *miso_;

    bool selected_ = false;
    bool last_sck_ = false;
    uint8_t in_ = 0;        // bits received so far, MSB first
    uint8_t out_ = 0xff;    // byte being sent
    unsigned bit_ = 0;      // bits of the current byte done
    uint8_t miso_bit_ = 1;  // bit driven on MISO while selected
};
