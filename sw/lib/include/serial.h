#pragma once 
#include <stdbool.h>
#include <stdint.h>

// Baud Rates
#define UART_BAUD_4800      4800
#define UART_BAUD_9600      9600
#define UART_BAUD_14400     14400
#define UART_BAUD_19200     19200
#define UART_BAUD_28800     28800
#define UART_BAUD_38400     38400
#define UART_BAUD_57600     57600
#define UART_BAUD_115200    115200
#define UART_BAUD_DEFAULT   115200

/**
 * @brief Serial configuration struct
 */
typedef struct {
    unsigned baud;
    bool rx_enable;
    bool tx_enable;
    bool dual_stop_bits;
    bool enable_parity_bit;
    bool even_parity;
    bool loopback_enable;
}  Serial_Config;


// Default settings
#define Serial_Config_default  {  \
    .baud = UART_BAUD_DEFAULT,  \
    .rx_enable = true,          \
    .tx_enable = true,          \
    .dual_stop_bits = false,    \
    .enable_parity_bit = false, \
    .even_parity = false,       \
    .loopback_enable = false    \
}                               \


/**
 * @brief Initializes uart with default settings 
 * and provided baud rate
 * 
 * @param baud_rate baud rate
 */
void serial_init(uint32_t baud_rate);


/**
 * @brief Initializes serial port with given configuration
 * @param cfg pointer to config struct
 */
void serial_set_config(Serial_Config * cfg);


/**
 * @brief Get current serial configuration
 * 
 * @return UART_Config cfg
 */
Serial_Config serial_get_config();


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

/**
 * @brief Read a character, waiting at most timeout_ms (0: only if one is waiting).
 * Uses the platform clock (platform_clk()).
 *
 * @return the character, or -1 on timeout
 */
int serial_read_timeout(uint32_t timeout_ms);

/**
 * @brief Wait until every character written so far has been sent
 * (TX FIFO empty and transmitter idle)
 */
void serial_flush();
