#include "platform.h"
#include "serial.h"
#include "time.h"

void serial_init(UART_Config * cfg)
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

UART_Config serial_get_config()
{
    UART_Config cfg;

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


/**
 * @brief Write a chracter to serial port
 * 
 * @param c chaaracter
 * @return int 
 */
void serial_write(char c)
{
    while(1)    // wait loop
    {
        // check if Tx is busy    
        if(! bitcheck(REG32(UART_ADDR, UART_REG_LSR), 1))
        {
            sleep(1);
            continue;
        }

        REG8(UART_ADDR, UART_REG_THR) = c;
        break;
    }
}


/**
 * @brief Read character from serial port
 * 
 * @return char 
 */
char serial_read()
{
    while(1)
    {
        if(! bitcheck(REG32(UART_ADDR, UART_REG_LSR), 0))
        {
            sleep(1);
            continue;
        }

        return REG8(UART_ADDR, UART_REG_RBR);
    }
}
