#ifndef __SERIAL_H__
#define __SERIAL_H__

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

/**
 * @brief Initialize SERIAL port
 * 
 * @param baud baud rate
 */
void serial_init(serial_baudrate baud);


/**
 * @brief Get current baud rate
 * 
 * @return unsigned int baud rate
 */
serial_baudrate serial_getBaud();


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