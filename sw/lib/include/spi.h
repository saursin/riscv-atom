#ifndef __SPI_H__
#define __SPI_H__
#include <stdint.h>
#include <stdbool.h>

/**
 * Enable: enable/disable SPI
 * pol: Polarity
 *  - true:     inactive state of SCK is high
 *  - false:    inactive state of SCK is low
 * pha: Phase
 *  - true:     data shifted on leading edge, sampled on trailing edge
 *  - false:    data sampled on leading edge, shifted on trailing edge
 * end: Endianness
 *  - true:     LSB first
 *  - false:    MSB first
 * Baudrate:    number of bits transferred per second
 * Post_cs_low_delay: number of cycles to wait after de-asserting CS
 * Pre_cs_high_delay: number of cycles to wait before asserting CS
*/

typedef enum {CSMODE_AUTO=0b00, CSMODE_DISABLE=0b11} csmode_t;

struct SPI_Config {
    uint32_t base_addr;
    bool enable;
    bool pol;
    bool pha;
    bool lsb_first;
    uint32_t baudrate;
    uint8_t device_num;
    csmode_t cs_mode;
    uint8_t post_cs_low_delay;
    uint8_t pre_cs_high_delay;
    bool loopback_enable;
};


#define SPI_Config_defaults  { \
    .base_addr = SPI_ADDR,  \
    .enable = true,         \
    .pha = false,           \
    .pol = false,           \
    .lsb_first = false,     \
    .baudrate = 1000000,    \
    .device_num = 0,        \
    .cs_mode = CSMODE_AUTO, \
    .post_cs_low_delay = 1, \
    .pre_cs_high_delay = 1, \
    .loopback_enable=false  \
};  \

/**
 * @brief Initialize SPI
 */
int spi_init(struct SPI_Config * cfg);

/**
 * @brief Starts SPI transaction by deasserting CS pin
 * @param cfg config struct
 */
void spi_select(struct SPI_Config * cfg);

/**
 * @brief Finishes SPI transaction by re-asserting CS pin
 * @param cfg config struct
 */
void spi_deselect(struct SPI_Config * cfg);

/**
 * @brief Transfer a byte of data (Send as well as recieve)
 * @param cfg config struct
 * @param b byte to be transferred
 */
char spi_transfer(struct SPI_Config * cfg, char b);

/**
 * @brief Transfer multiple bytes of data (Send as well as recieve)
 * @param cfg config struct
 * @param send_buf char buffer to send
 * @param recv_buf char buffer to recieve data in
 * @param len lenth of send buffer
 * @return char 
 */
char *spi_transfer_buf(struct SPI_Config * cfg, char *send_buf, char *recv_buf, unsigned int len);

#endif // __SPI_H__