#include <platform.h>
#include <mmio.h>
#include <time.h>
#include <file.h>
#include <utils.h>
#include <assert.h>
#include <serial.h>

// Register Offsets
#define UART_REG_RBR 0x00
#define UART_REG_THR 0x00
#define UART_REG_LCR 0x04
#define UART_REG_LSR 0x08
#define UART_REG_DIV 0x0c

// THR
#define UART_REG_THR_TXDA 0xff
#define UART_REG_RBR_RXDA 0xff

// LCR
#define UART_REG_LCR_RXEN 0b00000001
#define UART_REG_LCR_TXEN 0b00000010
#define UART_REG_LCR_STPB 0b00000100
#define UART_REG_LCR_PARB 0b00001000
#define UART_REG_LCR_EPAR 0b00010000
#define UART_REG_LCR_LPBK 0b10000000

// LSR
#define UART_REG_LCR_RVAL 0b00000001
#define UART_REG_LCR_TEMT 0b00000010
#define UART_REG_LCR_FERR 0b00000100
#define UART_REG_LCR_PERR 0b00001000

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
    Serial_Config uart_config = Serial_Config_default;
    uart_config.baud = baud_rate;
    serial_set_config(&uart_config);

    stddev[DEV_STDOUT].read=__serial_read;
    stddev[DEV_STDOUT].write=__serial_write;
}

void serial_set_config(Serial_Config *cfg)
{
    // Set Baud 
    uint32_t fratio = (CLK_FREQ/cfg->baud);
    REG32(UART_ADDR, UART_REG_DIV) = fratio - 2;
    
    // Initialize UART
    uint32_t lcr = 0;
    lcr |= cfg->rx_enable ? UART_REG_LCR_RXEN: 0;
    lcr |= cfg->tx_enable ? UART_REG_LCR_TXEN: 0;
    lcr |= cfg->dual_stop_bits ? UART_REG_LCR_STPB: 0;
    lcr |= cfg->enable_parity_bit ? UART_REG_LCR_PARB: 0;
    lcr |= cfg->even_parity ? UART_REG_LCR_EPAR: 0;
    lcr |= cfg->loopback_enable ? UART_REG_LCR_LPBK: 0;
    REG32(UART_ADDR, UART_REG_LCR) = lcr;

    // clear any garbage from data register by reading it once
    REG32(UART_ADDR, UART_REG_RBR);
}

Serial_Config serial_get_config()
{
    Serial_Config cfg;

    // Read LCR
    uint32_t lcr = REG32(UART_ADDR, UART_REG_LCR);
    cfg.baud = REG32(UART_ADDR, UART_REG_DIV);
    cfg.rx_enable = lcr & UART_REG_LCR_RXEN;
    cfg.tx_enable = lcr & UART_REG_LCR_TXEN;
    cfg.dual_stop_bits = lcr & UART_REG_LCR_STPB;
    cfg.enable_parity_bit = lcr & UART_REG_LCR_EPAR;
    cfg.loopback_enable = lcr & UART_REG_LCR_LPBK;

    return cfg;
}


void serial_write(char c)
{
    // Wait till THR is clear
    while(!bitcheck(REG32(UART_ADDR, UART_REG_LSR), 1))
        asm volatile("");
    REG8(UART_ADDR, UART_REG_THR) = c;
}


char serial_read()
{
    // Wait till RBR has anything
    while(!bitcheck(REG32(UART_ADDR, UART_REG_LSR), 0))
        asm volatile("");
    return REG8(UART_ADDR, UART_REG_RBR);
}
