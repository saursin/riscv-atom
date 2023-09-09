#ifndef __SPI_BITBANG_H__
#define __SPI_BITBANG_H__
#include <stdint.h>

struct SPIB_Config {
    uint8_t cs_pin;
    uint8_t sck_pin;
    uint8_t mosi_pin;
    uint8_t miso_pin;
    bool auto_cs;       // enabling this wil deassert/reassert CS autometically 
};


#define SPIB_Config_default { \
    .cs_pin = 12,    \
    .sck_pin = 15,   \
    .mosi_pin = 13,  \
    .miso_pin = 14,  \
    .auto_cs = true, \
}   \

/**
 * @brief Initialize SPI
 */
void spib_init(struct SPIB_Config * cfg);


/**
 * @brief Transfer a byte of data (Send as well as recieve)
 * @param cfg config struct
 * @param b byte to be transferred
 */
char spib_transfer(struct SPIB_Config * cfg, char b);


/**
 * @brief Transfer multiple bytes of data (Send as well as recieve)
 * @param cfg config struct
 * @param send_buf char buffer to send
 * @param recv_buf char buffer to recieve data in
 * @param len lenth of send buffer
 * @return char 
 */
char *spib_transfer_buf(struct SPIB_Config * cfg, char *send_buf, char *recv_buf, unsigned int len);


/**
 * @brief Starts SPI transaction by deasserting CS pin
 * @param cfg config struct
 */
void spib_start(struct SPIB_Config * cfg);


/**
 * @brief Finishes SPI transaction by re-asserting CS pin
 * @param cfg config struct
 */
void spib_end(struct SPIB_Config * cfg);

#endif //__SPI_BITBANG_H__
