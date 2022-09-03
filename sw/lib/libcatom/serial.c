#include "platform.h"
#include "serial.h"
#include "time.h"


/**
 * @brief Initialize SERIAL port
 * 
 * @param baud baud rate
 */
void serial_init(serial_baudrate baud)
{
    // set baud
    serial_setBaud(B_DEFAULT);

    // clear any garbage from data register by reading it once
    *((volatile char*) UART_D_REG_ADDR);
}


/**
 * @brief Get current baud rate
 * 
 * @return unsigned int baud rate
 */
serial_baudrate serial_getBaud()
{
    int val = *((volatile int*) UART_CD_REG_ADDR);
    return CLK_FREQ/(val+2);
}


/**
 * @brief Set baudrate
 * 
 * @param baud baudrate
 */
void serial_setBaud(serial_baudrate baud)
{
    int cd_reg_val = (CLK_FREQ/baud)-2;
    *((volatile int*) UART_CD_REG_ADDR) = cd_reg_val;
}

/**
 * @brief Get serial status
 * 
 * @return char 
 */
char serial_getStatus()
{
    return *((volatile char*) UART_S_REG_ADDR);
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
        if((*((volatile char*) UART_S_REG_ADDR) & 0x02) >> 1)
        {
            sleep(1);
            continue;
        }

        *((volatile char*) UART_D_REG_ADDR) = c;    // send character
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
    char c = (char)-1;
    while(c == (char)-1)
    {
        sleep(1);
        c = *((volatile char*) UART_D_REG_ADDR);
    }
    return c;
}
