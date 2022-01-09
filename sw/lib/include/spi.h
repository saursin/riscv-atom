#ifndef __SPI_H__
#define __SPI_H__

// Timing macros (to be used in implementing CS)
#define T_WAIT_AFTER_CS_LOW 10
#define T_WAIT_BEFORE_CS_HIGH 10


/**
 * @brief Initialize SPI
 */
void spi_init();


/**
 * @brief Transfer a byte of data (Send as well as recieve)
 * 
 * @param b 
 * @return char 
 */
char spi_transfer(char b);


/**
 * @brief Transfer multiple bytes of data (Send as well as recieve)
 * 
 * @param send_buf 
 * @param recv_buf 
 * @param len 
 * @return char 
 */
char *spi_transfer_buf(char *send_buf, char *recv_buf, unsigned int len);


#endif //__SPI_H__