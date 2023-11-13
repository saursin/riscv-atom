#include <platform.h>
#include <stdint.h>
#include <stdbool.h>
#include <file.h>
#include <mmio.h>
#include <assert.h>
#include <serial.h>


#define UART_REG_THR 0x0

int __serial_read(char * bf, uint32_t sz){
    return 0;
}

int __serial_write(char * bf, uint32_t sz){
    assert(sz==1);
    serial_write(bf[0]);
    return 0;
}

void serial_init(uint32_t baud_rate)
{
    // Not required for BFM UART
    stddev[DEV_STDOUT].read=__serial_read;
    stddev[DEV_STDOUT].write=__serial_write;
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
        uint32_t v = REG32(UART_ADDR, UART_REG_THR);
        if(v != (uint32_t)-1)
            return (char) (v & 0xff);
    }
}
