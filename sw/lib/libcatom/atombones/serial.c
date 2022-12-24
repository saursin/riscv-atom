#include "platform.h"
#include "mmio.h"
#include "time.h"
#include "serial.h"

void serial_init(uint32_t baud_rate)
{
    // Not required for BFM UART
}

void serial_set_config(Serial_Config *cfg)
{
    // Not Required for BFM URT
}

Serial_Config serial_get_config()
{
    Serial_Config cfg;

    // Read LCR
    cfg.baud = 112500;
    cfg.rx_enable = true;
    cfg.tx_enable = true;
    cfg.dual_stop_bits = false;
    cfg.enable_parity_bit = false;
    cfg.loopback_enable = false;
    return cfg;
}


void serial_write(char c)
{
    REG8(UART_ADDR, 0x0) = c;
}


char serial_read()
{
    while(1)
    {
        uint32_t v = REG32(UART_ADDR, 0x0);
        if(v != (uint32_t)-1)
            return (char) (v & 0xff);
    }
}
