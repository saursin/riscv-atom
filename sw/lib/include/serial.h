#pragma once 
#include <stdbool.h>
#include <stdint.h>

// Baud Rates
#ifdef SIM
/* CLKfreq/3 Gives highest possible baud rate, for sim (div=1) 
 */
#define UART_BAUD_4800      CLK_FREQ/3
#define UART_BAUD_9600      CLK_FREQ/3
#define UART_BAUD_14400     CLK_FREQ/3
#define UART_BAUD_19200     CLK_FREQ/3
#define UART_BAUD_28800     CLK_FREQ/3
#define UART_BAUD_38400     CLK_FREQ/3
#define UART_BAUD_57600     CLK_FREQ/3
#define UART_BAUD_115200    CLK_FREQ/3
#define UART_BAUD_DEFAULT   CLK_FREQ/3
#else
#define UART_BAUD_4800      4800
#define UART_BAUD_9600      9600
#define UART_BAUD_14400     14400
#define UART_BAUD_19200     19200
#define UART_BAUD_28800     28800
#define UART_BAUD_38400     38400
#define UART_BAUD_57600     57600
#define UART_BAUD_115200    115200
#define UART_BAUD_DEFAULT   115200
#endif

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
