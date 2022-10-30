#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdint.h>
#include <stdbool.h>

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


// Baud Rates
typedef enum
{
    B_4800   = 4800,
    B_9600   = 9600,
    B_14400  = 14400,
    B_19200  = 19200,
    B_28800  = 28800,
    B_38400  = 38400,
    B_57600  = 57600,
    B_115200 = 115200,

    B_DEFAULT = 9600
} serial_baudrate;

typedef struct {
    unsigned baud;
    bool rx_enable;
    bool tx_enable;
    bool dual_stop_bits;
    bool enable_parity_bit;
    bool even_parity;
    bool loopback_enable;
}  UART_Config;

/**
 * @brief Initialize Serial port
 * 
 * @param baud baud rate
 */
void serial_init(UART_Config * cfg);


/**
 * @brief Get current config
 * 
 * @return UART_Config cfg
 */
UART_Config serial_get_config();


/**
 * @brief Set baudrate
 * 
 * @param baud baudrate
 */
void serial_setBaud(serial_baudrate baud);


/**
 * @brief Get serial status
 * 
 * @return char 
 */
char serial_getStatus();


/**
 * @brief Write a chracter to serial port
 * 
 * @param c chaaracter
 * @return int 
 */
void serial_write(char c);


/**
 * @brief Read character from serial port
 * 
 * @return char 
 */
char serial_read();


#endif //__SERIAL_H__