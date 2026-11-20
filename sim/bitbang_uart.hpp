#pragma once
#include "device.hpp"
#include <cstdint>
#include <memory>

class Vuart;

/**
 * @brief A serial terminal on the SoC's UART pins
 * @details Decodes what the SoC sends (8N1, LSB first) and passes each byte to the virtual UART
 * and/or stdout; bytes typed on the virtual UART are sent to the SoC. The bit time follows the
 * SoC UART's programmed divisor (divisor + 2 cycles per bit, see UART_core), so programs may
 * change the baud rate.
 */
class BitbangUART: public Device
{
public:
    /**
     * @brief Construct, bound to the SoC's UART pins
     * @param rx_pin the SoC's TX (SoC output)
     * @param tx_pin the SoC's RX (SoC input)
     * @param div the SoC UART's divisor register
     * @param vuart virtual UART to relay to (nullptr: none)
     * @param dump also print received bytes on stdout
     */
    BitbangUART(const uint8_t *rx_pin, uint8_t *tx_pin, const uint32_t *div, std::unique_ptr<Vuart> vuart, bool dump);
    ~BitbangUART();

    void eval() override;

private:
    enum UART_State {
        IDLE,
        START_BIT,
        DATA_BITS,
        STOP_BIT
    };

    void rx_eval(unsigned fr);
    void tx_eval(unsigned fr);
    void received(uint8_t byte);

    const uint8_t *rx_pin_;
    uint8_t *tx_pin_;
    const uint32_t *div_;
    std::unique_ptr<Vuart> vuart_;
    bool dump_;

    // receiver
    bool rx_prev_ = false;          // rx pin in the previous sampled cycle
    UART_State rx_state_ = IDLE;
    int rx_wait_ = 0;               // cycles to wait
    int rx_bits_ = 0;               // bits received
    uint8_t rx_byte_ = 0;

    // transmitter
    bool tx_val_ = true;            // value driven on the tx pin
    UART_State tx_state_ = IDLE;
    int tx_wait_ = 0;               // cycles to wait
    int tx_bits_ = 0;               // bits sent
    uint8_t tx_byte_ = 0;
};
