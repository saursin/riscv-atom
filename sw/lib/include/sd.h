#pragma once
#include <stdint.h>
#include <spi.h>

// Error codes
#define SD_SUCCESS              0
#define SD_ERR_UNKNOWN_CARD     1
#define SD_ERR_MAYBE_VER1CARD   2

#define SD_DATA_ACCEPTED        0x05
#define SD_DATA_REJECTED_CRC    0x0B
#define SD_DATA_REJECTED_WRITE  0x0D

#define SD_RESP1_NO_ERROR(x)    ((x) == 0)

#define SD_START_TOKEN          0xFE
#define SD_ERROR_TOKEN          0x00
#define SD_BLOCK_LEN            512

/**
 * @brief Initialize SDCARD
 * 
 * @param cfg SPI config
 * @return uint8_t resp1
 */
uint8_t sd_init(struct SPI_Config *cfg);

/**
 * @brief Read a block from SDCARD
 * 
 * @param cfg SPI config
 * @param addr block address
 * @param buf buffer
 * @param token response token
 * @return uint8_t resp1
 */
uint8_t sdc_read_block(struct SPI_Config *cfg, uint32_t addr, uint8_t *buf, uint8_t *token);

/**
 * @brief Write a block to SDCard
 * 
 * @param cfg SPI Config
 * @param addr block address
 * @param buf buffer
 * @param token response token
 * @return uint8_t resp1
 */
uint8_t sdc_write_block(struct SPI_Config *cfg, uint32_t addr, uint8_t *buf, uint8_t *token);
